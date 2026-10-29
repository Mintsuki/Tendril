/* SPDX-License-Identifier: BSD-2-Clause */

/* Copyright (C) 2026 Mintsuki and contributors.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../internal.h"
#include <tendril/glue.h>
#include <tendril/tendril.h>

bool tendril_pci_hc_type(uint32_t class_code, int *type) {
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

uint32_t tendril_frame_update(
    struct tendril_hc *hc, uint32_t raw, uint32_t mask
) {
    hc->frame += (raw - hc->frame) & mask;
    return hc->frame;
}

bool tendril_hc_wait32(
    struct tendril_hc *hc, uint32_t off, uint32_t mask, uint32_t value,
    uint64_t timeout_us
) {
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

bool tendril_hc_wait16(
    struct tendril_hc *hc, uint32_t off, uint16_t mask, uint16_t value,
    uint64_t timeout_us
) {
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

void tendril_work_init(
    struct tendril_work *work, void (*fn)(struct tendril_work *work)
) {
    list_init(&work->node);
    work->fn = fn;
}

void tendril_queue_work(struct tendril_hc *hc, struct tendril_work *work) {
    if (!list_linked(&work->node)) {
        list_add_tail(&hc->work, &work->node);
    }
}

void tendril_async_complete(
    struct tendril_hc *hc, struct tendril_async *async, int status
) {
    async->status = status;
    tendril_queue_work(hc, &async->work);
}

void tendril_timer_init(
    struct tendril_timer *timer, void (*fn)(struct tendril_work *work)
) {
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

void tendril_timer_arm(
    struct tendril_hc *hc, struct tendril_timer *timer, uint64_t delay_ns
) {
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
    for (size_t a = 1; a < 128; a++) {
        if ((hc->addresses[a / 32] & ((uint32_t)1 << (a % 32))) == 0) {
            hc->addresses[a / 32] |= (uint32_t)1 << (a % 32);
            return (uint8_t)a;
        }
    }
    return 0;
}

void tendril_address_free(struct tendril_hc *hc, uint8_t address) {
    if (address != 0) {
        hc->addresses[address / 32] &= ~((uint32_t)1 << (address % 32));
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

static void root_set_feature(struct tendril_port *port, int feature) {
    struct tendril_hc *hc = port_hc(port);
    hc_lock(hc);
    int st = hc->dead ? TENDRIL_ERR_NO_DEVICE
        : hc->ops->port_set_feature(hc, port->number, feature);
    tendril_async_complete(hc, &port->async, st);
    hc_unlock(hc);
}

static void root_clear_feature(struct tendril_port *port, int feature) {
    struct tendril_hc *hc = port_hc(port);
    hc_lock(hc);
    int st = hc->dead ? TENDRIL_ERR_NO_DEVICE
        : hc->ops->port_clear_feature(hc, port->number, feature);
    tendril_async_complete(hc, &port->async, st);
    hc_unlock(hc);
}

static const struct tendril_hub_ops root_hub_ops = {
    .get_status = root_get_status,
    .set_feature = root_set_feature,
    .clear_feature = root_clear_feature
};

/* Called by controller drivers from start() once the port count is known. */
int tendril_roothub_init(struct tendril_hc *hc, size_t num_ports) {
    hc->root.hc = hc;
    hc->root.dev = NULL;
    hc->root.ops = &root_hub_ops;
    hc->root.num_ports = num_ports;
    hc->root.ports = tendril_zalloc(num_ports * sizeof(struct tendril_port));
    if (hc->root.ports == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    for (size_t i = 0; i < num_ports; i++) {
        tendril_port_init(&hc->root.ports[i], &hc->root, i + 1);
    }
    return TENDRIL_OK;
}

void tendril_roothub_changed(struct tendril_hc *hc, uint8_t port) {
    if (hc->root.ports == NULL || hc->removing) {
        return;
    }
    if (port == 0) {
        for (size_t i = 0; i < hc->root.num_ports; i++) {
            tendril_port_kick(&hc->root.ports[i]);
        }
    } else if (port <= hc->root.num_ports) {
        tendril_port_kick(&hc->root.ports[port - 1]);
    }
}

void tendril_roothub_hold(struct tendril_hc *hc, uint8_t port, uint64_t ms) {
    if (hc->root.ports != NULL && port >= 1 && port <= hc->root.num_ports) {
        tendril_timer_arm(hc, &hc->root.ports[port - 1].hold, ms * NS_PER_MS);
    }
}

void tendril_roothub_powered(
    struct tendril_hc *hc, uint8_t port, uint64_t settle_ms
) {
    if (hc->root.ports != NULL && port >= 1 && port <= hc->root.num_ports) {
        tendril_port_powered(&hc->root.ports[port - 1], settle_ms);
    }
}

/* Controllers */

static const struct tendril_hcd_ops *hcd_ops(int type) {
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

int tendril_hc_add(
    const struct tendril_hc_config *config, struct tendril_hc **out
) {
    const struct tendril_hcd_ops *ops = hcd_ops(config->type);
    if (ops == NULL) {
        return TENDRIL_ERR_UNSUPPORTED;
    }

    struct tendril_hc *hc = tendril_zalloc(sizeof(struct tendril_hc));
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

    LOG_INFO(
        "%s controller%s%04x:%04x, quirks 0x%x", ops->name,
        config->pci ? " " : "", config->pci ? hc->pci_vendor : 0,
        config->pci ? hc->pci_device : 0, hc->quirks
    );

    int st = ops->start(hc);
    if (st != TENDRIL_OK) {
        LOG_ERROR(
            "%s controller failed to start: %s", ops->name,
            tendril_status_str(st)
        );
        tendril_free(
            hc->root.ports, hc->root.num_ports * sizeof(struct tendril_port)
        );
        tendril_free(hc, sizeof(struct tendril_hc));
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
        struct tendril_work *w = CONTAINER_OF(
            hc->work.next, struct tendril_work, node
        );
        list_del(&w->node);
        hc_unlock(hc);
        w->fn(w);
    }
}

/*
 * Halts the controller and tears every device down, leaving the controller
 * state to destroy().
 */
static void teardown(struct tendril_hc *hc) {
    hc_lock(hc);
    hc->removing = true;
    hc_unlock(hc);

    /* Stop the hardware first, so teardown never waits on it. */
    hc_lock(hc);
    hc->ops->stop(hc);
    hc->dead = true;
    hc_unlock(hc);

    for (size_t i = 0; i < hc->root.num_ports; i++) {
        struct tendril_port *port = &hc->root.ports[i];
        hc_lock(hc);
        tendril_timer_cancel(hc, &port->timer);
        tendril_timer_cancel(hc, &port->hold);
        tendril_timer_cancel(hc, &port->connect_wait);
        hc_unlock(hc);
        if (port->device != NULL) {
            tendril_device_disconnect(port->device);
        }
    }

    /*
     * Teardown is asynchronous; with the controller dead, it needs no timers.
     */
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
    tendril_free(
        hc->root.ports, hc->root.num_ports * sizeof(struct tendril_port)
    );
    hc->root.ports = NULL;
    hc->root.num_ports = 0;
}

void tendril_hc_remove(struct tendril_hc *hc) {
    teardown(hc);
    tendril_free(hc, sizeof(struct tendril_hc));
}

/* System sleep */

struct pm_wait {
    volatile bool done;
    int status;
};

static void pm_wait_done(void *ctx, int status) {
    struct pm_wait *w = ctx;
    w->status = status;
    w->done = true;
}

static int run_device_pm(
    struct tendril_hc *hc, struct tendril_device *dev, bool suspend, bool wake
) {
    struct pm_wait w = { false, TENDRIL_OK };
    tendril_device_ref(dev);
    int st = suspend ? tendril_device_suspend(dev, wake, pm_wait_done, &w)
        : tendril_device_resume(dev, pm_wait_done, &w);
    if (st == TENDRIL_OK) {
        tendril_hc_wait(hc, &w.done);
        st = w.status;
    }
    tendril_device_unref(dev);
    return st;
}

/*
 * The deepest awake device to suspend next, or the shallowest system-suspended
 * one to resume.
 */
static struct tendril_device *next_pm_device(
    struct tendril_hc *hc, bool suspend
) {
    struct tendril_device *best = NULL;
    struct tendril_list *it;
    hc_lock(hc);
    LIST_FOR_EACH(it, &hc->devices) {
        struct tendril_device *dev = CONTAINER_OF(
            it, struct tendril_device, node
        );
        if (dev->gone || dev->pm_busy) {
            continue;
        }
        if (suspend ? dev->suspended : !(dev->suspended && dev->pm_system)) {
            continue;
        }
        if (best == NULL
         || (suspend ? dev->depth > best->depth : dev->depth < best->depth)) {
            best = dev;
        }
    }
    hc_unlock(hc);
    return best;
}

/*
 * At rest, and not enumerating a device, which holds no enumeration slot past
 * getting its address; for `ready`, with a device or past the time one may
 * take to connect after the port was switched on.
 */
static bool port_settled(struct tendril_port *port, bool ready) {
    if (!tendril_port_idle(port) || port->state == PORT_WAIT_ENUM
     || port->state == PORT_ENUM || tendril_timer_armed(&port->hold)) {
        return false;
    }
    return !ready || port->device != NULL
        || !tendril_timer_armed(&port->connect_wait);
}

/*
 * No port is busy, and for `ready`, no device is changing configuration and no
 * driver is still setting an interface up. Lock held.
 */
static bool at_rest(struct tendril_hc *hc, bool ready) {
    if (hc->dead) {
        return true;
    }
    if (hc->enum_port != NULL || !list_empty(&hc->enum_queue)) {
        return false;
    }
    for (size_t i = 0; i < hc->root.num_ports; i++) {
        if (!port_settled(&hc->root.ports[i], ready)) {
            return false;
        }
    }
    struct tendril_list *it;
    LIST_FOR_EACH(it, &hc->devices) {
        struct tendril_device *dev = CONTAINER_OF(
            it, struct tendril_device, node
        );
        if (dev->gone) {
            continue;
        }
        if (dev->pm_busy
         || (ready && (dev->config_busy || tendril_interfaces_busy(dev)))) {
            return false;
        }
        struct tendril_hub *hub = dev->hub;
        for (size_t i = 0;
             hub != NULL && hub->ports != NULL && i < hub->num_ports; i++) {
            if (!port_settled(&hub->ports[i], ready)) {
                return false;
            }
        }
    }
    return true;
}

bool tendril_hc_settled(struct tendril_hc *hc) {
    hc_lock(hc);
    bool settled = at_rest(hc, false);
    hc_unlock(hc);
    return settled;
}

bool tendril_hc_ready(struct tendril_hc *hc) {
    hc_lock(hc);
    bool ready = at_rest(hc, true);
    hc_unlock(hc);
    return ready;
}

static void resume_devices(struct tendril_hc *hc) {
    struct tendril_device *dev;
    while ((dev = next_pm_device(hc, false)) != NULL) {
        int st = run_device_pm(hc, dev, false, false);
        if (st != TENDRIL_OK && !dev->gone) {
            /* Left suspended; its port gives up on it if it is gone. */
            LOG_WARN(
                "device %u did not resume: %s", dev->address,
                tendril_status_str(st)
            );
            dev->pm_system = false;
        }
    }
}

#define SETTLE_TIMEOUT_MS 3000

int tendril_hc_suspend(struct tendril_hc *hc, bool wake) {
    if (hc->dead) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (hc->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    uint64_t deadline = tendril_now() + SETTLE_TIMEOUT_MS * NS_PER_MS;
    while (!tendril_hc_settled(hc)) {
        if (tendril_now() >= deadline) {
            return TENDRIL_ERR_BUSY;
        }
        tendril_hc_poll(hc);
    }

    struct tendril_device *dev;
    size_t rounds = 0;
    while ((dev = next_pm_device(hc, true)) != NULL) {
        /*
         * A device that keeps signalling wakeup would otherwise hold this up
         * forever.
         */
        if (++rounds > 256) {
            resume_devices(hc);
            return TENDRIL_ERR_BUSY;
        }
        int st = run_device_pm(hc, dev, true, wake);
        if (st == TENDRIL_OK) {
            dev->pm_system = true;
        } else if (!dev->gone) {
            LOG_WARN(
                "device %u cannot be suspended: %s", dev->address,
                tendril_status_str(st)
            );
            resume_devices(hc);
            return st;
        }
    }

    hc_lock(hc);
    int st = hc->dead ? TENDRIL_ERR_NO_DEVICE
        : hc->ops->suspend != NULL ? hc->ops->suspend(hc)
        : TENDRIL_OK;
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
static int restart(struct tendril_hc *hc) {
    LOG_WARN(
        "%s controller lost its state during sleep, restarting it",
        hc->ops->name
    );
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
    int st = hc->ops->start(hc);
    if (st != TENDRIL_OK) {
        LOG_ERROR(
            "%s controller failed to restart: %s", hc->ops->name,
            tendril_status_str(st)
        );
        hc->dead = true;
        return st;
    }
    hc_lock(hc);
    tendril_roothub_changed(hc, 0);
    hc_unlock(hc);
    return TENDRIL_OK;
}

int tendril_hc_resume(struct tendril_hc *hc) {
    if (!hc->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    if (hc->config.pci && !hc->dead) {
        /* Firmware may have left the device's PCI state as reset left it. */
        pci_enable(hc);
    }
    hc_lock(hc);
    hc->suspended = false;
    int st = hc->dead ? TENDRIL_ERR_NO_DEVICE
        : hc->ops->resume != NULL ? hc->ops->resume(hc)
        : TENDRIL_OK;
    if (st == TENDRIL_OK) {
        /*
         * Ports may have seen wakeups, connects, or disconnects while asleep.
         */
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
        struct tendril_timer *t = CONTAINER_OF(
            hc->timers.next, struct tendril_timer, node
        );
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
        struct tendril_work *w = CONTAINER_OF(
            hc->work.next, struct tendril_work, node
        );
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

int tendril_hc_type(struct tendril_hc *hc) {
    return hc->config.type;
}

uint32_t tendril_hc_quirks(struct tendril_hc *hc) {
    return hc->quirks;
}

size_t tendril_hc_num_ports(struct tendril_hc *hc) {
    return hc->root.num_ports;
}

struct tendril_device *tendril_hc_port_device(
    struct tendril_hc *hc, uint8_t port
) {
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
