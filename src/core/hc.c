#include "../internal.h"

bool tendril_pci_hc_type(uint32_t class_code, enum tendril_hc_type *type) {
    if ((class_code >> 8) != 0x0c03) {
        return false;
    }
    switch (class_code & 0xff) {
        case 0x00: {
            *type = TENDRIL_HC_UHCI;
            return true;
        }
        case 0x10: {
            *type = TENDRIL_HC_OHCI;
            return true;
        }
        case 0x20: {
            *type = TENDRIL_HC_EHCI;
            return true;
        }
        case 0x30: {
            *type = TENDRIL_HC_XHCI;
            return true;
        }
        default: {
            return false;
        }
    }
}

void tendril_delay_us(uint64_t us) {
    uint64_t end = tendril_now() + us * NS_PER_US;
    while (tendril_now() < end) {
    }
}

uint32_t tendril_frame_update(struct tendril_hc *hc, uint32_t raw, uint32_t mask) {
    hc->frame += (raw - hc->frame) & mask;
    return hc->frame;
}

bool tendril_hc_wait32(struct tendril_hc *hc, uint32_t off, uint32_t mask, uint32_t value, uint64_t timeout_us) {
    uint64_t end = tendril_now() + timeout_us * NS_PER_US;
    for (;;) {
        uint32_t v = hc_read32(hc, off);
        if ((v & mask) == value) {
            return true;
        }
        if (v == 0xffffffff && mask != 0xffffffff) {
            return false;
        }
        if (tendril_now() >= end) {
            return (hc_read32(hc, off) & mask) == value;
        }
    }
}

bool tendril_hc_wait16(struct tendril_hc *hc, uint32_t off, uint16_t mask, uint16_t value, uint64_t timeout_us) {
    uint64_t end = tendril_now() + timeout_us * NS_PER_US;
    for (;;) {
        if ((hc_read16(hc, off) & mask) == value) {
            return true;
        }
        if (tendril_now() >= end) {
            return (hc_read16(hc, off) & mask) == value;
        }
    }
}

/* Work and timers */

void tendril_work_init(struct tendril_work *work, void (*fn)(struct tendril_work *work)) {
    list_init(&work->node);
    work->fn = fn;
}

void tendril_queue_work(struct tendril_hc *hc, struct tendril_work *work) {
    if (!list_linked(&work->node)) {
        list_add_tail(&hc->work, &work->node);
    }
}

void tendril_async_complete(struct tendril_hc *hc, struct tendril_async *async, enum tendril_status status) {
    async->status = status;
    tendril_queue_work(hc, &async->work);
}

void tendril_timer_init(struct tendril_timer *timer, void (*fn)(struct tendril_work *work)) {
    list_init(&timer->node);
    timer->deadline = 0;
    tendril_work_init(&timer->work, fn);
}

bool tendril_timer_armed(struct tendril_timer *timer) {
    return list_linked(&timer->node) || list_linked(&timer->work.node);
}

void tendril_timer_cancel(struct tendril_hc *hc, struct tendril_timer *timer) {
    (void)hc;
    if (list_linked(&timer->node)) {
        list_del(&timer->node);
    }
    if (list_linked(&timer->work.node)) {
        list_del(&timer->work.node);
    }
}

void tendril_timer_arm(struct tendril_hc *hc, struct tendril_timer *timer, uint64_t delay_ns) {
    struct tendril_list *it;

    tendril_timer_cancel(hc, timer);
    timer->deadline = tendril_now() + delay_ns;
    LIST_FOR_EACH(it, &hc->timers) {
        struct tendril_timer *t = CONTAINER_OF(it, struct tendril_timer, node);
        if (t->deadline > timer->deadline) {
            break;
        }
    }
    list_insert(it->prev, &timer->node);
}

static uint64_t next_deadline(struct tendril_hc *hc) {
    if (list_empty(&hc->timers)) {
        return UINT64_MAX;
    }
    return CONTAINER_OF(hc->timers.next, struct tendril_timer, node)->deadline;
}

/* Addresses */

uint8_t tendril_address_alloc(struct tendril_hc *hc) {
    for (unsigned a = 1; a < 128; a++) {
        if (!(hc->addresses[a / 32] & (1u << (a % 32)))) {
            hc->addresses[a / 32] |= 1u << (a % 32);
            return (uint8_t)a;
        }
    }
    return 0;
}

void tendril_address_free(struct tendril_hc *hc, uint8_t address) {
    if (address != 0) {
        hc->addresses[address / 32] &= ~(1u << (address % 32));
    }
}

/* Root hub */

static struct tendril_hc *port_hc(struct tendril_port *port) {
    return port->hub->hc;
}

static void root_get_status(struct tendril_port *port) {
    struct tendril_hc *hc = port_hc(port);
    hc_lock(hc);
    if (hc->dead) {
        tendril_async_complete(hc, &port->async, TENDRIL_ERR_NO_DEVICE);
    } else {
        hc->ops->port_status(hc, port->number, &port->st);
        tendril_async_complete(hc, &port->async, TENDRIL_OK);
    }
    hc_unlock(hc);
}

static void root_set_feature(struct tendril_port *port, unsigned feature) {
    struct tendril_hc *hc = port_hc(port);
    hc_lock(hc);
    enum tendril_status st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->port_set_feature(hc, port->number, feature);
    tendril_async_complete(hc, &port->async, st);
    hc_unlock(hc);
}

static void root_clear_feature(struct tendril_port *port, unsigned feature) {
    struct tendril_hc *hc = port_hc(port);
    hc_lock(hc);
    enum tendril_status st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->port_clear_feature(hc, port->number, feature);
    tendril_async_complete(hc, &port->async, st);
    hc_unlock(hc);
}

static const struct tendril_hub_ops root_hub_ops = {
    .get_status = root_get_status,
    .set_feature = root_set_feature,
    .clear_feature = root_clear_feature
};

/* Called by controller drivers from start() once the port count is known. */
enum tendril_status tendril_roothub_init(struct tendril_hc *hc, unsigned num_ports) {
    hc->root.hc = hc;
    hc->root.dev = NULL;
    hc->root.ops = &root_hub_ops;
    hc->root.num_ports = num_ports;
    hc->root.ports = tendril_zalloc(num_ports * sizeof(struct tendril_port));
    if (hc->root.ports == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    for (unsigned i = 0; i < num_ports; i++) {
        tendril_port_init(&hc->root.ports[i], &hc->root, i + 1);
    }
    return TENDRIL_OK;
}

void tendril_roothub_changed(struct tendril_hc *hc, unsigned port) {
    if (hc->root.ports == NULL || hc->removing) {
        return;
    }
    if (port == 0) {
        for (unsigned i = 0; i < hc->root.num_ports; i++) {
            tendril_port_kick(&hc->root.ports[i]);
        }
    } else if (port <= hc->root.num_ports) {
        tendril_port_kick(&hc->root.ports[port - 1]);
    }
}

/* Controllers */

static const struct tendril_hcd_ops *hcd_ops(enum tendril_hc_type type) {
    switch (type) {
#ifndef TENDRIL_NO_UHCI
        case TENDRIL_HC_UHCI: {
            return &tendril_uhci_ops;
        }
#endif
#ifndef TENDRIL_NO_OHCI
        case TENDRIL_HC_OHCI: {
            return &tendril_ohci_ops;
        }
#endif
#ifndef TENDRIL_NO_EHCI
        case TENDRIL_HC_EHCI: {
            return &tendril_ehci_ops;
        }
#endif
#ifndef TENDRIL_NO_XHCI
        case TENDRIL_HC_XHCI: {
            return &tendril_xhci_ops;
        }
#endif
        default: {
            return NULL;
        }
    }
}

static void pci_enable(struct tendril_hc *hc) {
    /* I/O or memory decoding, and bus mastering. */
    uint16_t cmd = tendril_glue_pci_read16(hc->ctx, 0x04);
    cmd |= hc->config.type == TENDRIL_HC_UHCI ? 0x0001 : 0x0002;
    cmd |= 0x0004;
    tendril_glue_pci_write16(hc->ctx, 0x04, cmd);
}

enum tendril_status tendril_hc_add(const struct tendril_hc_config *config, struct tendril_hc **out) {
    const struct tendril_hcd_ops *ops = hcd_ops(config->type);
    if (ops == NULL) {
        return TENDRIL_ERR_UNSUPPORTED;
    }

    struct tendril_hc *hc = tendril_zalloc(sizeof(*hc));
    if (hc == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    hc->config = *config;
    hc->ctx = config->ctx;
    hc->ops = ops;
    list_init(&hc->devices);
    list_init(&hc->work);
    list_init(&hc->timers);
    list_init(&hc->enum_queue);

    if (config->pci) {
        hc->pci_vendor = tendril_glue_pci_read16(hc->ctx, 0x00);
        hc->pci_device = tendril_glue_pci_read16(hc->ctx, 0x02);
        hc->pci_revision = tendril_glue_pci_read8(hc->ctx, 0x08);
        pci_enable(hc);
    }
    hc->quirks = config->quirks | tendril_hc_quirk_lookup(hc);

    LOG_INFO("tendril: %s controller%s%04x:%04x, quirks 0x%x", ops->name,
             config->pci ? " " : "", config->pci ? hc->pci_vendor : 0,
             config->pci ? hc->pci_device : 0, hc->quirks);

    enum tendril_status st = ops->start(hc);
    if (st != TENDRIL_OK) {
        LOG_ERROR("tendril: %s controller failed to start: %s", ops->name, tendril_status_str(st));
        tendril_free(hc->root.ports, hc->root.num_ports * sizeof(struct tendril_port));
        tendril_free(hc, sizeof(*hc));
        return st;
    }

    hc_lock(hc);
    tendril_roothub_changed(hc, 0);
    hc_unlock(hc);
    *out = hc;
    return TENDRIL_OK;
}

static void drain(struct tendril_hc *hc) {
    for (;;) {
        hc_lock(hc);
        if (list_empty(&hc->work)) {
            hc_unlock(hc);
            return;
        }
        struct tendril_work *w = CONTAINER_OF(hc->work.next, struct tendril_work, node);
        list_del(&w->node);
        hc_unlock(hc);
        w->fn(w);
    }
}

/* Halts the controller and tears every device down, leaving the controller state to destroy(). */
static void teardown(struct tendril_hc *hc) {
    hc_lock(hc);
    hc->removing = true;
    hc_unlock(hc);

    /* Stop the hardware first, so teardown never waits on it. */
    hc_lock(hc);
    hc->ops->stop(hc);
    hc->dead = true;
    hc_unlock(hc);

    for (unsigned i = 0; i < hc->root.num_ports; i++) {
        struct tendril_port *port = &hc->root.ports[i];
        hc_lock(hc);
        tendril_timer_cancel(hc, &port->timer);
        hc_unlock(hc);
        if (port->device != NULL) {
            tendril_device_disconnect(port->device);
        }
    }

    /* Teardown is asynchronous; with the controller dead, it needs no timers. */
    for (;;) {
        drain(hc);
        hc_lock(hc);
        bool idle = list_empty(&hc->work) && list_empty(&hc->devices);
        hc_unlock(hc);
        if (idle) {
            break;
        }
        tendril_hc_poll(hc);
    }
    hc->ops->destroy(hc);
    tendril_free(hc->root.ports, hc->root.num_ports * sizeof(struct tendril_port));
    hc->root.ports = NULL;
    hc->root.num_ports = 0;
}

void tendril_hc_remove(struct tendril_hc *hc) {
    teardown(hc);
    tendril_free(hc, sizeof(*hc));
}

/* System sleep */

struct pm_wait {
    volatile bool done;
    enum tendril_status status;
};

static void pm_wait_done(void *ctx, enum tendril_status status) {
    struct pm_wait *w = ctx;
    w->status = status;
    w->done = true;
}

static enum tendril_status run_device_pm(struct tendril_hc *hc, struct tendril_device *dev, bool suspend, bool wake) {
    struct pm_wait w = { false, TENDRIL_OK };
    tendril_device_ref(dev);
    enum tendril_status st = suspend ? tendril_device_suspend(dev, wake, pm_wait_done, &w)
                                    : tendril_device_resume(dev, pm_wait_done, &w);
    if (st == TENDRIL_OK) {
        tendril_hc_wait(hc, &w.done);
        st = w.status;
    }
    tendril_device_unref(dev);
    return st;
}

/* The deepest awake device to suspend next, or the shallowest system-suspended one to resume. */
static struct tendril_device *next_pm_device(struct tendril_hc *hc, bool suspend) {
    struct tendril_device *best = NULL;
    struct tendril_list *it;
    hc_lock(hc);
    LIST_FOR_EACH(it, &hc->devices) {
        struct tendril_device *dev = CONTAINER_OF(it, struct tendril_device, node);
        if (dev->gone || dev->pm_busy) {
            continue;
        }
        if (suspend ? dev->suspended : !(dev->suspended && dev->pm_system)) {
            continue;
        }
        if (best == NULL || (suspend ? dev->depth > best->depth : dev->depth < best->depth)) {
            best = dev;
        }
    }
    hc_unlock(hc);
    return best;
}

/* No port is mid-way through something, and no device is enumerating. */
static bool settled(struct tendril_hc *hc) {
    hc_lock(hc);
    bool idle = hc->enum_port == NULL && list_empty(&hc->enum_queue);
    for (unsigned i = 0; i < hc->root.num_ports && idle; i++) {
        idle = tendril_port_idle(&hc->root.ports[i]);
    }
    struct tendril_list *it;
    LIST_FOR_EACH(it, &hc->devices) {
        struct tendril_device *dev = CONTAINER_OF(it, struct tendril_device, node);
        if (dev->gone) {
            continue;
        }
        if (dev->pm_busy) {
            idle = false;
        }
        for (unsigned i = 0; dev->hub != NULL && dev->hub->ports != NULL && i < dev->hub->num_ports; i++) {
            if (!tendril_port_idle(&dev->hub->ports[i])) {
                idle = false;
            }
        }
    }
    hc_unlock(hc);
    return idle;
}

static void resume_devices(struct tendril_hc *hc) {
    struct tendril_device *dev;
    while ((dev = next_pm_device(hc, false)) != NULL) {
        enum tendril_status st = run_device_pm(hc, dev, false, false);
        if (st != TENDRIL_OK && !dev->gone) {
            /* Left suspended; its port gives up on it if it is gone. */
            LOG_WARN("tendril: device %u did not resume: %s", dev->address, tendril_status_str(st));
            dev->pm_system = false;
        }
    }
}

#define SETTLE_TIMEOUT_MS 3000

enum tendril_status tendril_hc_suspend(struct tendril_hc *hc, bool wake) {
    if (hc->dead) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (hc->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    uint64_t deadline = tendril_now() + SETTLE_TIMEOUT_MS * NS_PER_MS;
    while (!settled(hc)) {
        if (tendril_now() >= deadline) {
            return TENDRIL_ERR_BUSY;
        }
        tendril_hc_poll(hc);
    }

    struct tendril_device *dev;
    unsigned rounds = 0;
    while ((dev = next_pm_device(hc, true)) != NULL) {
        /* A device that keeps signalling wakeup would otherwise hold this up forever. */
        if (++rounds > 256) {
            resume_devices(hc);
            return TENDRIL_ERR_BUSY;
        }
        enum tendril_status st = run_device_pm(hc, dev, true, wake);
        if (st == TENDRIL_OK) {
            dev->pm_system = true;
        } else if (!dev->gone) {
            LOG_WARN("tendril: device %u cannot be suspended: %s", dev->address, tendril_status_str(st));
            resume_devices(hc);
            return st;
        }
    }

    hc_lock(hc);
    enum tendril_status st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->suspend != NULL ? hc->ops->suspend(hc) : TENDRIL_OK;
    if (st == TENDRIL_OK) {
        hc->suspended = true;
    }
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        resume_devices(hc);
    }
    return st;
}

/* After a loss of power: everything is gone, so it all starts over. */
static enum tendril_status restart(struct tendril_hc *hc) {
    LOG_WARN("tendril: %s controller lost its state during sleep, restarting it", hc->ops->name);
    teardown(hc);
    /* Whatever is still queued belonged to the controller state just freed. */
    list_init(&hc->work);
    list_init(&hc->timers);
    list_init(&hc->enum_queue);
    hc->enum_port = NULL;
    hc->enum_dev = NULL;
    hc->removing = false;
    hc->dead = false;
    memset(hc->addresses, 0, sizeof(hc->addresses));
    if (hc->config.pci) {
        pci_enable(hc);
    }
    enum tendril_status st = hc->ops->start(hc);
    if (st != TENDRIL_OK) {
        LOG_ERROR("tendril: %s controller failed to restart: %s", hc->ops->name, tendril_status_str(st));
        hc->dead = true;
        return st;
    }
    hc_lock(hc);
    tendril_roothub_changed(hc, 0);
    hc_unlock(hc);
    return TENDRIL_OK;
}

enum tendril_status tendril_hc_resume(struct tendril_hc *hc) {
    if (!hc->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    if (hc->config.pci && !hc->dead) {
        /* Firmware may have left the device's PCI state as reset left it. */
        pci_enable(hc);
    }
    hc_lock(hc);
    hc->suspended = false;
    enum tendril_status st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->resume != NULL ? hc->ops->resume(hc) : TENDRIL_OK;
    if (st == TENDRIL_OK) {
        /* Ports may have seen wakeups, connects, or disconnects while asleep. */
        tendril_roothub_changed(hc, 0);
    }
    hc_unlock(hc);
    if (st == TENDRIL_ERR_IO) {
        return restart(hc);
    }
    if (st != TENDRIL_OK) {
        return st;
    }
    resume_devices(hc);
    return TENDRIL_OK;
}

bool tendril_hc_irq(struct tendril_hc *hc) {
    if (hc->dead) {
        return false;
    }
    return hc->ops->irq(hc);
}

uint64_t tendril_hc_poll(struct tendril_hc *hc) {
    hc_lock(hc);
    if (!hc->dead) {
        hc->ops->poll(hc);
    }
    uint64_t now = tendril_now();
    while (!list_empty(&hc->timers)) {
        struct tendril_timer *t = CONTAINER_OF(hc->timers.next, struct tendril_timer, node);
        if (t->deadline > now) {
            break;
        }
        list_del(&t->node);
        tendril_queue_work(hc, &t->work);
    }
    hc_unlock(hc);

    for (;;) {
        hc_lock(hc);
        if (list_empty(&hc->work)) {
            uint64_t deadline = next_deadline(hc);
            hc_unlock(hc);
            return deadline;
        }
        struct tendril_work *w = CONTAINER_OF(hc->work.next, struct tendril_work, node);
        list_del(&w->node);
        hc_unlock(hc);
        w->fn(w);
    }
}

void tendril_hc_wait(struct tendril_hc *hc, volatile bool *flag) {
    while (!*flag) {
        tendril_hc_poll(hc);
    }
}

enum tendril_hc_type tendril_hc_type(struct tendril_hc *hc) {
    return hc->config.type;
}

uint32_t tendril_hc_quirks(struct tendril_hc *hc) {
    return hc->quirks;
}

unsigned tendril_hc_num_ports(struct tendril_hc *hc) {
    return hc->root.num_ports;
}

struct tendril_device *tendril_hc_port_device(struct tendril_hc *hc, unsigned port) {
    if (port == 0 || port > hc->root.num_ports) {
        return NULL;
    }
    return hc->root.ports[port - 1].device;
}

uint32_t tendril_hc_frame_number(struct tendril_hc *hc) {
    hc_lock(hc);
    uint32_t frame = hc->dead ? 0 : hc->ops->frame_number(hc);
    hc_unlock(hc);
    return frame;
}
