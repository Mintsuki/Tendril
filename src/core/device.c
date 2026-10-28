#include "../internal.h"

/*
 * Device enumeration and teardown. Like the port state machine, these steps
 * run from work items or transfer callbacks, one at a time per controller.
 */

#define CTRL_TIMEOUT_MS 5000
#define SET_ADDRESS_RECOVERY_MS 2
#define MAX_CONFIGS 8
#define MAX_CONFIG_SIZE 65535

enum dev_step {
    STEP_NONE,
    STEP_INIT,
    STEP_HC_ADDRESS,
    STEP_DESC8,
    STEP_EP0_MPS,
    STEP_SET_ADDRESS,
    STEP_ADDRESS_WAIT,
    STEP_EP0_ADDRESS,
    STEP_DESC18,
    STEP_CONFIG_HEAD,
    STEP_CONFIG_FULL,
    STEP_BOS_HEAD,
    STEP_BOS_FULL,
    STEP_LANGID,
    STEP_STRING,
    STEP_CONFIGURE,
    STEP_SET_CONFIG,
    STEP_HUB,
    STEP_RUNNING,
    STEP_TEARDOWN_WAIT,
    STEP_TEARDOWN_FREE,
    STEP_FREED
};

static void dev_async_work(struct tendril_work *work);
static void dev_timer_work(struct tendril_work *work);
static void start_teardown(struct tendril_device *dev);

bool tendril_device_is_ss(struct tendril_device *dev) {
    return dev->speed >= TENDRIL_SPEED_SUPER;
}

static uint16_t default_ep0_mps(enum tendril_speed speed) {
    switch (speed) {
        case TENDRIL_SPEED_LOW:
        case TENDRIL_SPEED_FULL: {
            return 8;
        }
        case TENDRIL_SPEED_HIGH: {
            return 64;
        }
        default: {
            return 512;
        }
    }
}

static void set_ep0_mps(struct tendril_device *dev, uint16_t mps) {
    dev->ep0_desc.max_packet_size = tendril_le16(mps);
    dev->ep0.max_packet = mps;
}

struct tendril_device *tendril_device_alloc(struct tendril_hc *hc, struct tendril_port *port, enum tendril_speed speed) {
    struct tendril_device *dev = tendril_zalloc(sizeof(*dev));
    if (dev == NULL) {
        return NULL;
    }
    struct tendril_device *parent = port->hub->dev;

    dev->hc = hc;
    dev->parent = parent;
    dev->port = port;
    dev->refs = 1;
    dev->state = DEV_ENUMERATING;
    dev->speed = speed;
    dev->active_config = -1;
    list_init(&dev->node);
    tendril_async_init(&dev->async, dev_async_work);
    tendril_timer_init(&dev->timer, dev_timer_work);
    if (parent != NULL) {
        tendril_device_ref(parent);
    }

    if (parent == NULL) {
        dev->depth = 0;
        dev->root_port = port->number;
        dev->route = 0;
    } else {
        dev->depth = (uint8_t)(parent->depth + 1);
        dev->root_port = parent->root_port;
        dev->route = parent->route | ((uint32_t)MIN(port->number, 15) << (4 * parent->depth));
        if (speed <= TENDRIL_SPEED_FULL) {
            if (parent->speed == TENDRIL_SPEED_HIGH) {
                dev->tt_hub = parent;
                dev->tt_port = port->number;
            } else {
                dev->tt_hub = parent->tt_hub;
                dev->tt_port = parent->tt_port;
            }
        }
    }

    dev->ep0_desc.length = sizeof(dev->ep0_desc);
    dev->ep0_desc.type = TENDRIL_DT_ENDPOINT;
    dev->ep0_desc.endpoint_address = 0;
    dev->ep0_desc.attributes = TENDRIL_ENDPOINT_XFER_CONTROL;
    tendril_endpoint_setup(&dev->ep0, dev, NULL, &dev->ep0_desc, NULL);
    set_ep0_mps(dev, default_ep0_mps(speed));

    hc_lock(hc);
    list_add_tail(&hc->devices, &dev->node);
    hc_unlock(hc);
    return dev;
}

static void free_interfaces(struct tendril_device *dev) {
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        tendril_free(intf->alts, intf->num_alts * sizeof(*intf->alts));
        tendril_free(intf->alt_sizes, intf->num_alts * sizeof(*intf->alt_sizes));
        tendril_free(intf->endpoints, intf->num_endpoints * sizeof(*intf->endpoints));
    }
    tendril_free(dev->interfaces, dev->num_interfaces * sizeof(*dev->interfaces));
    dev->interfaces = NULL;
    dev->num_interfaces = 0;
}

static void free_device(struct tendril_device *dev) {
    /* Children keep their ports, inside the hub, until they are freed too. */
    if (dev->hub != NULL) {
        tendril_hub_free(dev->hub);
    }
    free_interfaces(dev);
    for (unsigned i = 0; i < dev->num_configs; i++) {
        tendril_free(dev->configs[i], dev->config_sizes[i]);
    }
    tendril_free(dev->configs, dev->num_configs * sizeof(*dev->configs));
    tendril_free(dev->config_sizes, dev->num_configs * sizeof(*dev->config_sizes));
    tendril_free(dev->bos, dev->bos_size);
    tendril_strfree(dev->manufacturer);
    tendril_strfree(dev->product);
    tendril_strfree(dev->serial);
    tendril_free(dev->buf, dev->buf_size);
    struct tendril_device *parent = dev->parent;
    tendril_free(dev, sizeof(*dev));
    if (parent != NULL) {
        tendril_device_unref(parent);
    }
}

void tendril_device_ref(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    hc_lock(hc);
    dev->refs++;
    hc_unlock(hc);
}

void tendril_device_unref(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    hc_lock(hc);
    bool last = --dev->refs == 0;
    hc_unlock(hc);
    if (last) {
        free_device(dev);
    }
}

void tendril_device_put(struct tendril_device *dev) {
    tendril_device_unref(dev);
}

/* Endpoints */

void tendril_endpoint_setup(struct tendril_endpoint *ep, struct tendril_device *dev, struct tendril_interface *intf,
                           const struct tendril_endpoint_descriptor *desc, const uint8_t *end) {
    memset(ep, 0, sizeof(*ep));
    ep->dev = dev;
    ep->intf = intf;
    ep->desc = desc;
    ep->address = desc->endpoint_address;
    ep->type = desc->attributes & TENDRIL_ENDPOINT_XFER_MASK;
    list_init(&ep->transfers);

    uint16_t wmps = tendril_le16(desc->max_packet_size);
    ep->max_packet = wmps & TENDRIL_ENDPOINT_MAXP_MASK;
    ep->mult = 1;
    ep->max_burst = 0;

    if (end != NULL) {
        const uint8_t *p = tendril_desc_next((const uint8_t *)desc, end);
        if (p != NULL && p[1] == TENDRIL_DT_SS_ENDPOINT_COMPANION && p[0] >= sizeof(struct tendril_ss_ep_companion_descriptor)) {
            ep->companion = (const void *)p;
            p = tendril_desc_next(p, end);
            if (p != NULL && p[1] == TENDRIL_DT_SSP_ISO_ENDPOINT_COMPANION &&
                p[0] >= sizeof(struct tendril_ssp_iso_ep_companion_descriptor)) {
                ep->ssp_companion = (const void *)p;
            }
        }
    }

    bool periodic = ep->type == TENDRIL_ENDPOINT_XFER_INT || ep->type == TENDRIL_ENDPOINT_XFER_ISOC;
    uint8_t binterval = desc->interval;

    if (tendril_device_is_ss(dev)) {
        if (ep->companion != NULL) {
            ep->max_burst = ep->companion->max_burst > 15 ? 15 : ep->companion->max_burst;
            if (ep->type == TENDRIL_ENDPOINT_XFER_BULK) {
                unsigned n = ep->companion->attributes & 0x1f;
                ep->max_streams = n == 0 ? 0 : 1u << MIN(n, 16);
            } else if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC && !(ep->companion->attributes & 0x80)) {
                ep->mult = (uint8_t)((ep->companion->attributes & 0x3) + 1);
            }
        }
        if (periodic) {
            if (ep->ssp_companion != NULL) {
                ep->max_esit_payload = tendril_le32(ep->ssp_companion->bytes_per_interval);
            } else if (ep->companion != NULL) {
                ep->max_esit_payload = tendril_le16(ep->companion->bytes_per_interval);
            } else {
                ep->max_esit_payload = (uint32_t)ep->max_packet * (ep->max_burst + 1u) * ep->mult;
            }
        }
    } else if (dev->speed == TENDRIL_SPEED_HIGH && periodic) {
        ep->mult = (uint8_t)(((wmps & TENDRIL_ENDPOINT_MAXP_MULT_MASK) >> TENDRIL_ENDPOINT_MAXP_MULT_SHIFT) + 1);
        if (ep->mult > 3) {
            ep->mult = 3;
        }
        ep->max_esit_payload = (uint32_t)ep->max_packet * ep->mult;
    } else if (periodic) {
        ep->max_esit_payload = ep->max_packet;
    }

    if (!periodic) {
        /* HS bulk/control OUT NAK rate, used by xHCI only. */
        ep->interval_log2 = 0;
        ep->interval = 0;
        if (dev->speed == TENDRIL_SPEED_HIGH && binterval != 0 && ep->type != TENDRIL_ENDPOINT_XFER_ISOC) {
            ep->interval_log2 = (uint8_t)MIN(ilog2(binterval), 15);
        }
        return;
    }

    if (binterval == 0) {
        binterval = 1;
    }
    if (dev->speed <= TENDRIL_SPEED_FULL) {
        if (ep->type == TENDRIL_ENDPOINT_XFER_INT) {
            /* xHCI 1.2, 6.2.3.6: round bInterval * 8 down to a power of two. */
            ep->interval_log2 = (uint8_t)ilog2((uint32_t)binterval * 8);
        } else {
            ep->interval_log2 = (uint8_t)(MIN(binterval, 16) - 1 + 3);
        }
    } else {
        ep->interval_log2 = (uint8_t)(MIN(binterval, 16) - 1);
    }
    if (ep->interval_log2 > 15) {
        ep->interval_log2 = 15;
    }
    ep->interval = 1u << ep->interval_log2;
}

/* Control transfers for internal state machines */

static void control_async_done(struct tendril_transfer *xfer) {
    struct tendril_async *async = xfer->context;
    async->status = xfer->status;
    async->work.fn(&async->work);
}

enum tendril_status tendril_control_async(struct tendril_device *dev, struct tendril_transfer *xfer,
                                        uint8_t request_type, uint8_t request, uint16_t value,
                                        uint16_t index, void *data, uint16_t length,
                                        struct tendril_async *async) {
    tendril_transfer_init(xfer, &dev->ep0);
    xfer->setup.request_type = request_type;
    xfer->setup.request = request;
    xfer->setup.value = tendril_le16(value);
    xfer->setup.index = tendril_le16(index);
    xfer->setup.length = tendril_le16(length);
    xfer->buffer = data;
    xfer->length = length;
    xfer->timeout_ms = CTRL_TIMEOUT_MS;
    xfer->callback = control_async_done;
    xfer->context = async;
    return tendril_transfer_submit(xfer);
}

/* Enumeration */

static bool ensure_buf(struct tendril_device *dev, size_t size) {
    if (dev->buf_size >= size) {
        return true;
    }
    uint8_t *buf = tendril_zalloc(size);
    if (buf == NULL) {
        return false;
    }
    tendril_free(dev->buf, dev->buf_size);
    dev->buf = buf;
    dev->buf_size = size;
    return true;
}

static void dev_name(struct tendril_device *dev, char *buf, size_t size) {
    tendril_snprintf(buf, size, "device %u (%04x:%04x)", dev->address,
                    tendril_le16(dev->desc.vendor_id), tendril_le16(dev->desc.product_id));
}

static void enum_fail(struct tendril_device *dev, enum tendril_status status, const char *what) {
    char name[48];
    dev_name(dev, name, sizeof(name));
    if (status != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN("tendril: %s: %s failed: %s", name, what, tendril_status_str(status));
    }
    dev->step = STEP_NONE;
    tendril_enum_done(dev->port, dev, status);
    if (!dev->gone) {
        tendril_device_disconnect(dev);
    }
}

static void get_descriptor(struct tendril_device *dev, enum dev_step step, uint8_t type, uint8_t index,
                           uint16_t langid, uint16_t length) {
    dev->step = step;
    dev->op_busy = true;
    if (!ensure_buf(dev, length)) {
        enum_fail(dev, TENDRIL_ERR_NO_MEMORY, "buffer allocation");
        return;
    }
    enum tendril_status st = tendril_control_async(dev, &dev->xfer, TENDRIL_DIR_IN | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                                                 TENDRIL_REQ_GET_DESCRIPTOR, (uint16_t)(type << 8 | index), langid,
                                                 dev->buf, length, &dev->async);
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, "GET_DESCRIPTOR");
    }
}

static void hc_op(struct tendril_device *dev, enum dev_step step,
                  enum tendril_status (*op)(struct tendril_hc *, struct tendril_device *, struct tendril_async *),
                  const char *what) {
    struct tendril_hc *hc = dev->hc;
    dev->step = step;
    dev->op_busy = true;
    hc_lock(hc);
    enum tendril_status st = hc->dead ? TENDRIL_ERR_NO_DEVICE : op(hc, dev, &dev->async);
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, what);
    }
}

static void dev_wait(struct tendril_device *dev, enum dev_step step, uint64_t ms) {
    struct tendril_hc *hc = dev->hc;
    dev->step = step;
    dev->op_busy = true;
    dev->async.status = TENDRIL_OK;
    hc_lock(hc);
    tendril_timer_arm(hc, &dev->timer, ms * NS_PER_MS);
    hc_unlock(hc);
}

static void read_descriptor_8(struct tendril_device *dev) {
    get_descriptor(dev, STEP_DESC8, TENDRIL_DT_DEVICE, 0, 0, 8);
}

static void read_device_descriptor(struct tendril_device *dev) {
    get_descriptor(dev, STEP_DESC18, TENDRIL_DT_DEVICE, 0, 0, sizeof(struct tendril_device_descriptor));
}

static void next_config(struct tendril_device *dev);
static void after_configs(struct tendril_device *dev);
static void next_string(struct tendril_device *dev);
static void choose_and_configure(struct tendril_device *dev);

void tendril_device_enumerate(struct tendril_device *dev) {
    char name[48];
    dev_name(dev, name, sizeof(name));
    LOG_DEBUG("tendril: new %s device on root port %u, depth %u", tendril_speed_str(dev->speed),
              dev->root_port, dev->depth);
    hc_op(dev, STEP_INIT, dev->hc->ops->device_init, "device setup");
}

static void addressed(struct tendril_device *dev) {
    dev->state = DEV_ADDRESSED;
    tendril_enum_release_dev(dev);
    read_device_descriptor(dev);
}

static bool parse_device_descriptor(struct tendril_device *dev, size_t actual) {
    const struct tendril_device_descriptor *d = (const void *)dev->buf;
    if (actual < sizeof(*d) || d->type != TENDRIL_DT_DEVICE) {
        return false;
    }
    memcpy(&dev->desc, d, sizeof(*d));
    dev->quirks |= tendril_device_quirk_lookup(&dev->desc);
    unsigned n = dev->desc.num_configurations;
    if (n == 0) {
        return false;
    }
    if (n > MAX_CONFIGS) {
        n = MAX_CONFIGS;
    }
    dev->configs = tendril_zalloc(n * sizeof(*dev->configs));
    dev->config_sizes = tendril_zalloc(n * sizeof(*dev->config_sizes));
    if (dev->configs == NULL || dev->config_sizes == NULL) {
        tendril_free(dev->configs, n * sizeof(*dev->configs));
        tendril_free(dev->config_sizes, n * sizeof(*dev->config_sizes));
        dev->configs = NULL;
        dev->config_sizes = NULL;
        return false;
    }
    dev->num_configs = n;
    return true;
}

static void next_config(struct tendril_device *dev) {
    if (dev->index >= dev->num_configs) {
        after_configs(dev);
        return;
    }
    get_descriptor(dev, STEP_CONFIG_HEAD, TENDRIL_DT_CONFIGURATION, (uint8_t)dev->index, 0,
                   sizeof(struct tendril_config_descriptor));
}

static void after_configs(struct tendril_device *dev) {
    uint16_t bcd = tendril_le16(dev->desc.bcd_usb);
    if (bcd >= 0x0201) {
        get_descriptor(dev, STEP_BOS_HEAD, TENDRIL_DT_BOS, 0, 0, sizeof(struct tendril_bos_descriptor));
        return;
    }
    if (dev->quirks & TENDRIL_DEV_QUIRK_NO_STRINGS) {
        choose_and_configure(dev);
        return;
    }
    get_descriptor(dev, STEP_LANGID, TENDRIL_DT_STRING, 0, 0, 255);
}

static void after_bos(struct tendril_device *dev) {
    if (dev->quirks & TENDRIL_DEV_QUIRK_NO_STRINGS) {
        choose_and_configure(dev);
        return;
    }
    get_descriptor(dev, STEP_LANGID, TENDRIL_DT_STRING, 0, 0, 255);
}

static uint8_t string_index(struct tendril_device *dev, unsigned which) {
    switch (which) {
        case 0: {
            return dev->desc.manufacturer_string;
        }
        case 1: {
            return dev->desc.product_string;
        }
        case 2: {
            return dev->desc.serial_string;
        }
        default: {
            return 0;
        }
    }
}

static void next_string(struct tendril_device *dev) {
    while (dev->index < 3 && string_index(dev, dev->index) == 0) {
        dev->index++;
    }
    if (dev->index >= 3) {
        choose_and_configure(dev);
        return;
    }
    get_descriptor(dev, STEP_STRING, TENDRIL_DT_STRING, string_index(dev, dev->index), dev->langid, 255);
}

/* Decodes a UTF-16LE string descriptor into newly allocated UTF-8. */
static char *decode_string(const uint8_t *buf, size_t actual, bool swap) {
    if (actual < 2 || buf[1] != TENDRIL_DT_STRING) {
        return NULL;
    }
    size_t len = MIN((size_t)buf[0], actual);
    size_t units = (len - 2) / 2;
    char tmp[4 * 127 + 1];
    size_t o = 0;

    for (size_t i = 0; i < units; i++) {
        uint32_t c = (uint32_t)buf[2 + i * 2] | (uint32_t)buf[3 + i * 2] << 8;
        if (swap) {
            c = ((c & 0xff) << 8) | (c >> 8);
        }
        if (c >= 0xd800 && c < 0xdc00 && i + 1 < units) {
            uint32_t lo = (uint32_t)buf[4 + i * 2] | (uint32_t)buf[5 + i * 2] << 8;
            if (lo >= 0xdc00 && lo < 0xe000) {
                c = 0x10000 + ((c - 0xd800) << 10) + (lo - 0xdc00);
                i++;
            }
        }
        if (c >= 0xd800 && c < 0xe000) {
            c = 0xfffd;
        }
        if (c == 0) {
            break;
        }
        if (c < 0x80) {
            tmp[o++] = (char)c;
        } else if (c < 0x800) {
            tmp[o++] = (char)(0xc0 | c >> 6);
            tmp[o++] = (char)(0x80 | (c & 0x3f));
        } else if (c < 0x10000) {
            tmp[o++] = (char)(0xe0 | c >> 12);
            tmp[o++] = (char)(0x80 | ((c >> 6) & 0x3f));
            tmp[o++] = (char)(0x80 | (c & 0x3f));
        } else {
            tmp[o++] = (char)(0xf0 | c >> 18);
            tmp[o++] = (char)(0x80 | ((c >> 12) & 0x3f));
            tmp[o++] = (char)(0x80 | ((c >> 6) & 0x3f));
            tmp[o++] = (char)(0x80 | (c & 0x3f));
        }
    }
    /* Trailing spaces are common padding. */
    while (o > 0 && tmp[o - 1] == ' ') {
        o--;
    }
    tmp[o] = '\0';
    return tendril_strdup(tmp);
}

/* Configuration selection and setup */

/*
 * The first configuration, unless a quirk names another, as FreeBSD's
 * sys/dev/usb/usb_device.c (BSD-2-Clause, usb_alloc_device) chooses.
 */
static int choose_config(struct tendril_device *dev) {
    static const uint32_t index_quirks[] = {
        TENDRIL_DEV_QUIRK_CONFIG_INDEX_0, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1, TENDRIL_DEV_QUIRK_CONFIG_INDEX_2,
        TENDRIL_DEV_QUIRK_CONFIG_INDEX_3, TENDRIL_DEV_QUIRK_CONFIG_INDEX_4
    };
    for (unsigned i = 0; i < ARRAY_SIZE(index_quirks); i++) {
        if ((dev->quirks & index_quirks[i]) && i < dev->num_configs && dev->configs[i] != NULL) {
            return (int)i;
        }
    }
    for (unsigned i = 0; i < dev->num_configs; i++) {
        if (dev->configs[i] != NULL) {
            return (int)i;
        }
    }
    return -1;
}

static struct tendril_interface *find_interface(struct tendril_device *dev, uint8_t number) {
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        if (dev->interfaces[i].number == number) {
            return &dev->interfaces[i];
        }
    }
    return NULL;
}

/* Builds interface objects for configuration `index`, with no endpoints yet. */
static enum tendril_status build_interfaces(struct tendril_device *dev, unsigned index) {
    const uint8_t *config = dev->configs[index];
    const uint8_t *end = config + dev->config_sizes[index];
    const struct tendril_config_descriptor *cd = (const void *)config;
    unsigned max = cd->num_interfaces;

    /* Count distinct interface numbers rather than trusting bNumInterfaces. */
    unsigned count = 0;
    uint8_t seen[32] = { 0 };
    for (const uint8_t *p = tendril_desc_find(config, end, TENDRIL_DT_INTERFACE); p != NULL;
         p = tendril_desc_find(tendril_desc_next(p, end), end, TENDRIL_DT_INTERFACE)) {
        uint8_t n = ((const struct tendril_interface_descriptor *)(const void *)p)->interface_number;
        if (!(seen[n / 8] & (1u << (n % 8)))) {
            seen[n / 8] |= (uint8_t)(1u << (n % 8));
            count++;
        }
    }
    (void)max;
    if (count == 0) {
        return TENDRIL_OK;
    }

    dev->interfaces = tendril_zalloc(count * sizeof(*dev->interfaces));
    if (dev->interfaces == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }

    const struct tendril_interface_assoc_descriptor *iad = NULL;
    for (const uint8_t *p = tendril_desc_next(config, end); p != NULL; p = tendril_desc_next(p, end)) {
        if (p[1] == TENDRIL_DT_INTERFACE_ASSOCIATION && p[0] >= sizeof(*iad)) {
            iad = (const void *)p;
            continue;
        }
        if (p[1] != TENDRIL_DT_INTERFACE || p[0] < sizeof(struct tendril_interface_descriptor)) {
            continue;
        }
        const struct tendril_interface_descriptor *id = (const void *)p;
        struct tendril_interface *intf = find_interface(dev, id->interface_number);
        if (intf == NULL) {
            intf = &dev->interfaces[dev->num_interfaces++];
            intf->dev = dev;
            intf->number = id->interface_number;
            if (iad != NULL && id->interface_number >= iad->first_interface &&
                id->interface_number < iad->first_interface + iad->interface_count) {
                intf->iad = iad;
            }
        }

        /* The alternate setting runs up to the next interface descriptor. */
        const uint8_t *q = tendril_desc_next(p, end);
        while (q != NULL && q[1] != TENDRIL_DT_INTERFACE && q[1] != TENDRIL_DT_INTERFACE_ASSOCIATION) {
            q = tendril_desc_next(q, end);
        }
        size_t size = (size_t)((q != NULL ? q : end) - p);

        const struct tendril_interface_descriptor **alts = tendril_zalloc((intf->num_alts + 1) * sizeof(*alts));
        size_t *sizes = tendril_zalloc((intf->num_alts + 1) * sizeof(*sizes));
        if (alts == NULL || sizes == NULL) {
            tendril_free(alts, (intf->num_alts + 1) * sizeof(*alts));
            tendril_free(sizes, (intf->num_alts + 1) * sizeof(*sizes));
            return TENDRIL_ERR_NO_MEMORY;
        }
        if (intf->num_alts != 0) {
            memcpy(alts, intf->alts, intf->num_alts * sizeof(*alts));
            memcpy(sizes, intf->alt_sizes, intf->num_alts * sizeof(*sizes));
            tendril_free(intf->alts, intf->num_alts * sizeof(*alts));
            tendril_free(intf->alt_sizes, intf->num_alts * sizeof(*sizes));
        }
        alts[intf->num_alts] = id;
        sizes[intf->num_alts] = size;
        intf->alts = alts;
        intf->alt_sizes = sizes;
        intf->num_alts++;
    }

    /* Start in the alternate setting numbered 0, or the first one listed. */
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        intf->cur_alt = 0;
        for (unsigned a = 0; a < intf->num_alts; a++) {
            if (intf->alts[a]->alternate_setting == 0) {
                intf->cur_alt = a;
                break;
            }
        }
    }
    return TENDRIL_OK;
}

/* Creates endpoint objects for alternate setting `alt` of an interface. */
enum tendril_status tendril_interface_build_endpoints(struct tendril_interface *intf, unsigned alt,
                                                    struct tendril_endpoint **eps, unsigned *count) {
    const uint8_t *p = (const uint8_t *)intf->alts[alt];
    const uint8_t *end = p + intf->alt_sizes[alt];
    unsigned n = 0;

    for (const uint8_t *q = tendril_desc_find(tendril_desc_next(p, end), end, TENDRIL_DT_ENDPOINT); q != NULL;
         q = tendril_desc_find(tendril_desc_next(q, end), end, TENDRIL_DT_ENDPOINT)) {
        if (q[0] >= sizeof(struct tendril_endpoint_descriptor) && (q[2] & TENDRIL_ENDPOINT_NUMBER_MASK) != 0) {
            n++;
        }
    }
    *count = n;
    *eps = NULL;
    if (n == 0) {
        return TENDRIL_OK;
    }
    struct tendril_endpoint *arr = tendril_zalloc(n * sizeof(*arr));
    if (arr == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    unsigned i = 0;
    for (const uint8_t *q = tendril_desc_find(tendril_desc_next(p, end), end, TENDRIL_DT_ENDPOINT); q != NULL;
         q = tendril_desc_find(tendril_desc_next(q, end), end, TENDRIL_DT_ENDPOINT)) {
        if (q[0] >= sizeof(struct tendril_endpoint_descriptor) && (q[2] & TENDRIL_ENDPOINT_NUMBER_MASK) != 0) {
            tendril_endpoint_setup(&arr[i++], intf->dev, intf, (const void *)q, end);
        }
    }
    *eps = arr;
    return TENDRIL_OK;
}

/* Collects the endpoints of the current alternate settings into `list`. */
static unsigned collect_endpoints(struct tendril_device *dev, struct tendril_endpoint **list, unsigned max) {
    unsigned n = 0;
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (unsigned e = 0; e < intf->num_endpoints && n < max; e++) {
            list[n++] = &intf->endpoints[e];
        }
    }
    return n;
}

#define MAX_DEVICE_ENDPOINTS 31

static void choose_and_configure(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    char name[48];

    dev_name(dev, name, sizeof(name));
    LOG_INFO("tendril: %s: %s %s%s%s%s", name, tendril_speed_str(dev->speed),
             dev->manufacturer != NULL ? dev->manufacturer : "",
             dev->manufacturer != NULL ? " " : "",
             dev->product != NULL ? dev->product : "",
             dev->product == NULL && dev->manufacturer == NULL ? "(no strings)" : "");

    int index = choose_config(dev);
    if (index < 0) {
        enum_fail(dev, TENDRIL_ERR_PROTOCOL, "configuration selection");
        return;
    }
    enum tendril_status st = build_interfaces(dev, (unsigned)index);
    if (st == TENDRIL_OK) {
        for (unsigned i = 0; i < dev->num_interfaces && st == TENDRIL_OK; i++) {
            struct tendril_interface *intf = &dev->interfaces[i];
            st = tendril_interface_build_endpoints(intf, intf->cur_alt, &intf->endpoints, &intf->num_endpoints);
        }
    }
    if (st != TENDRIL_OK) {
        free_interfaces(dev);
        enum_fail(dev, st, "configuration parsing");
        return;
    }
    dev->active_config = index;

    struct tendril_endpoint *add[MAX_DEVICE_ENDPOINTS];
    unsigned num_add = collect_endpoints(dev, add, MAX_DEVICE_ENDPOINTS);
    dev->step = STEP_CONFIGURE;
    dev->op_busy = true;
    hc_lock(hc);
    st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->configure(hc, dev, NULL, 0, add, num_add, &dev->async);
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        free_interfaces(dev);
        dev->active_config = -1;
        enum_fail(dev, st, "endpoint setup");
    }
}

static void set_configuration(struct tendril_device *dev) {
    const struct tendril_config_descriptor *cd = (const void *)dev->configs[dev->active_config];
    dev->step = STEP_SET_CONFIG;
    dev->op_busy = true;
    enum tendril_status st = tendril_control_async(dev, &dev->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                                                 TENDRIL_REQ_SET_CONFIGURATION, cd->configuration_value, 0, NULL, 0,
                                                 &dev->async);
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, "SET_CONFIGURATION");
    }
}

static bool is_hub(struct tendril_device *dev) {
    if (dev->desc.device_class == TENDRIL_CLASS_HUB) {
        return true;
    }
    return dev->num_interfaces > 0 && dev->interfaces[0].alts[dev->interfaces[0].cur_alt]->interface_class == TENDRIL_CLASS_HUB;
}

static void configured(struct tendril_device *dev) {
    dev->state = DEV_CONFIGURED;
    if (is_hub(dev) && dev->num_interfaces > 0) {
        dev->step = STEP_HUB;
        dev->op_busy = true;
        tendril_hub_start(dev);
        return;
    }
    dev->step = STEP_RUNNING;
    tendril_enum_done(dev->port, dev, TENDRIL_OK);
    tendril_bind_interfaces(dev);
    dev->announced = true;
    tendril_notify_added(dev);
}

/* Called by the hub driver when the hub finished initializing. */
void tendril_device_hub_ready(struct tendril_device *dev, enum tendril_status status) {
    dev->op_busy = false;
    if (dev->gone) {
        start_teardown(dev);
        return;
    }
    if (status != TENDRIL_OK) {
        enum_fail(dev, status, "hub setup");
        return;
    }
    dev->step = STEP_RUNNING;
    tendril_enum_done(dev->port, dev, TENDRIL_OK);
    tendril_bind_interfaces(dev);
    dev->announced = true;
    tendril_notify_added(dev);
}


static void control_step(struct tendril_device *dev, enum dev_step step, uint8_t request_type, uint8_t request,
                         uint16_t value, uint16_t index, const char *what) {
    dev->step = step;
    dev->op_busy = true;
    enum tendril_status st = tendril_control_async(dev, &dev->xfer, request_type, request, value, index, NULL, 0,
                                                 &dev->async);
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, what);
    }
}

static bool descriptor_ok(enum tendril_status st) {
    return st == TENDRIL_OK || st == TENDRIL_ERR_SHORT;
}

static void dev_step(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    enum tendril_status st = dev->async.status;
    size_t actual = dev->xfer.actual;

    switch (dev->step) {
        case STEP_INIT: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "device setup");
                break;
            }
            if (hc->ops->device_address != NULL) {
                hc_op(dev, STEP_HC_ADDRESS, hc->ops->device_address, "address assignment");
                break;
            }
            read_descriptor_8(dev);
            break;
        }
        case STEP_HC_ADDRESS: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "address assignment");
                break;
            }
            /* xHCI 1.2, 4.3, step 7: only full-speed EP0 packet sizes vary. */
            if (dev->speed == TENDRIL_SPEED_FULL) {
                dev->state = DEV_ADDRESSED;
                tendril_enum_release_dev(dev);
                read_descriptor_8(dev);
                break;
            }
            addressed(dev);
            break;
        }
        case STEP_DESC8: {
            if (!descriptor_ok(st) || actual < 8 || dev->buf[1] != TENDRIL_DT_DEVICE) {
                enum_fail(dev, descriptor_ok(st) ? TENDRIL_ERR_PROTOCOL : st, "reading the device descriptor");
                break;
            }
            unsigned mps = dev->buf[7];
            if (tendril_device_is_ss(dev)) {
                mps = 512;
            } else if (mps != 8 && mps != 16 && mps != 32 && mps != 64) {
                mps = dev->speed == TENDRIL_SPEED_HIGH ? 64 : 8;
            }
            set_ep0_mps(dev, (uint16_t)mps);
            if (hc->ops->device_address == NULL) {
                hc_lock(hc);
                dev->new_address = tendril_address_alloc(hc);
                hc_unlock(hc);
                if (dev->new_address == 0) {
                    enum_fail(dev, TENDRIL_ERR_NO_MEMORY, "address allocation");
                    break;
                }
            }
            hc_op(dev, STEP_EP0_MPS, hc->ops->ep0_update, "endpoint 0 update");
            break;
        }
        case STEP_EP0_MPS: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "endpoint 0 update");
                break;
            }
            if (hc->ops->device_address != NULL) {
                read_device_descriptor(dev);
                break;
            }
            /* EP0 keeps targeting address 0 until STEP_EP0_ADDRESS. */
            control_step(dev, STEP_SET_ADDRESS, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                         TENDRIL_REQ_SET_ADDRESS, dev->new_address, 0, "SET_ADDRESS");
            break;
        }
        case STEP_SET_ADDRESS: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "SET_ADDRESS");
                break;
            }
            dev_wait(dev, STEP_ADDRESS_WAIT, SET_ADDRESS_RECOVERY_MS);
            break;
        }
        case STEP_ADDRESS_WAIT: {
            dev->address = dev->new_address;
            hc_op(dev, STEP_EP0_ADDRESS, hc->ops->ep0_update, "endpoint 0 update");
            break;
        }
        case STEP_EP0_ADDRESS: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "endpoint 0 update");
                break;
            }
            addressed(dev);
            break;
        }
        case STEP_DESC18: {
            if (!descriptor_ok(st)) {
                enum_fail(dev, st, "reading the device descriptor");
                break;
            }
            if (!parse_device_descriptor(dev, actual)) {
                enum_fail(dev, TENDRIL_ERR_PROTOCOL, "parsing the device descriptor");
                break;
            }
            dev->index = 0;
            next_config(dev);
            break;
        }
        case STEP_CONFIG_HEAD: {
            const struct tendril_config_descriptor *cd = (const void *)dev->buf;
            if (!descriptor_ok(st) || actual < sizeof(*cd) || cd->type != TENDRIL_DT_CONFIGURATION) {
                enum_fail(dev, descriptor_ok(st) ? TENDRIL_ERR_PROTOCOL : st, "reading a configuration descriptor");
                break;
            }
            uint16_t total = tendril_le16(cd->total_length);
            if (total < sizeof(*cd)) {
                total = sizeof(*cd);
            }
            get_descriptor(dev, STEP_CONFIG_FULL, TENDRIL_DT_CONFIGURATION, (uint8_t)dev->index, 0, total);
            break;
        }
        case STEP_CONFIG_FULL: {
            if (!descriptor_ok(st)) {
                enum_fail(dev, st, "reading a configuration descriptor");
                break;
            }
            if (!tendril_config_valid(dev->buf, actual)) {
                char name[48];
                dev_name(dev, name, sizeof(name));
                LOG_WARN("tendril: %s: configuration %u is malformed", name, dev->index);
            } else {
                uint8_t *copy = tendril_glue_alloc(actual);
                if (copy == NULL) {
                    enum_fail(dev, TENDRIL_ERR_NO_MEMORY, "configuration copy");
                    break;
                }
                memcpy(copy, dev->buf, actual);
                /* Keep wTotalLength consistent with what was read. */
                ((struct tendril_config_descriptor *)(void *)copy)->total_length = tendril_le16((uint16_t)actual);
                dev->configs[dev->index] = copy;
                dev->config_sizes[dev->index] = actual;
            }
            dev->index++;
            next_config(dev);
            break;
        }
        case STEP_BOS_HEAD: {
            const struct tendril_bos_descriptor *bd = (const void *)dev->buf;
            if (!descriptor_ok(st) || actual < sizeof(*bd) || bd->type != TENDRIL_DT_BOS ||
                tendril_le16(bd->total_length) < sizeof(*bd)) {
                after_bos(dev);
                break;
            }
            get_descriptor(dev, STEP_BOS_FULL, TENDRIL_DT_BOS, 0, 0, tendril_le16(bd->total_length));
            break;
        }
        case STEP_BOS_FULL: {
            if (descriptor_ok(st) && actual >= sizeof(struct tendril_bos_descriptor) && dev->buf[1] == TENDRIL_DT_BOS) {
                dev->bos = tendril_glue_alloc(actual);
                if (dev->bos != NULL) {
                    memcpy(dev->bos, dev->buf, actual);
                    dev->bos_size = actual;
                }
            }
            after_bos(dev);
            break;
        }
        case STEP_LANGID: {
            if (!descriptor_ok(st) || actual < 4 || dev->buf[1] != TENDRIL_DT_STRING) {
                choose_and_configure(dev);
                break;
            }
            dev->langid = (uint16_t)(dev->buf[2] | dev->buf[3] << 8);
            dev->index = 0;
            next_string(dev);
            break;
        }
        case STEP_STRING: {
            if (descriptor_ok(st)) {
                char *s = decode_string(dev->buf, actual, (dev->quirks & TENDRIL_DEV_QUIRK_SWAP_UNICODE) != 0);
                if (s != NULL) {
                    char **slot = dev->index == 0 ? &dev->manufacturer : dev->index == 1 ? &dev->product : &dev->serial;
                    tendril_strfree(*slot);
                    *slot = s;
                }
            }
            dev->index++;
            next_string(dev);
            break;
        }
        case STEP_CONFIGURE: {
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                dev->active_config = -1;
                enum_fail(dev, st, "endpoint setup");
                break;
            }
            set_configuration(dev);
            break;
        }
        case STEP_SET_CONFIG: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "SET_CONFIGURATION");
                break;
            }
            configured(dev);
            break;
        }
        case STEP_TEARDOWN_WAIT: {
            hc_lock(hc);
            bool idle = dev->pending == 0;
            hc_unlock(hc);
            if (idle) {
                start_teardown(dev);
            }
            break;
        }
        case STEP_TEARDOWN_FREE: {
            hc_lock(hc);
            if (hc->ops->device_address == NULL) {
                tendril_address_free(hc, dev->new_address);
            }
            list_del(&dev->node);
            hc_unlock(hc);
            dev->step = STEP_FREED;
            dev->state = DEV_GONE;
            tendril_device_unref(dev);
            break;
        }
        default: {
            break;
        }
    }
}

static void dev_continue(struct tendril_device *dev) {
    dev->op_busy = false;
    if (dev->gone && dev->step < STEP_TEARDOWN_WAIT) {
        start_teardown(dev);
        return;
    }
    dev_step(dev);
}

static void dev_async_work(struct tendril_work *work) {
    dev_continue(CONTAINER_OF(work, struct tendril_device, async.work));
}

static void dev_timer_work(struct tendril_work *work) {
    dev_continue(CONTAINER_OF(work, struct tendril_device, timer.work));
}

/* Teardown */

static void cancel_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep, enum tendril_status status) {
    struct tendril_list *it;
    LIST_FOR_EACH(it, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        tendril_transfer_cancel_locked(hc, xfer, status);
    }
}

/* With the lock held. */
static void cancel_all(struct tendril_device *dev, enum tendril_status status, bool include_ep0) {
    struct tendril_hc *hc = dev->hc;
    if (include_ep0) {
        cancel_endpoint(hc, &dev->ep0, status);
    }
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (unsigned e = 0; e < intf->num_endpoints; e++) {
            cancel_endpoint(hc, &intf->endpoints[e], status);
        }
    }
}

void tendril_device_disconnect(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;

    if (dev->gone) {
        return;
    }
    dev->gone = true;
    if (dev->hub != NULL) {
        tendril_hub_stop(dev->hub);
    }

    hc_lock(hc);
    cancel_all(dev, TENDRIL_ERR_NO_DEVICE, true);
    bool waiting_timer = list_linked(&dev->timer.node) || list_linked(&dev->timer.work.node);
    if (waiting_timer) {
        tendril_timer_cancel(hc, &dev->timer);
        dev->op_busy = false;
    }
    hc_unlock(hc);

    if (!dev->op_busy) {
        start_teardown(dev);
    }
}

static void start_teardown(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;

    if (dev->step >= STEP_TEARDOWN_FREE) {
        return;
    }
    if (dev->step != STEP_NONE && dev->step < STEP_RUNNING) {
        /* Enumeration was cut short; report it so the slot is released. */
        dev->step = STEP_NONE;
        tendril_enum_done(dev->port, dev, TENDRIL_ERR_NO_DEVICE);
    }
    dev->step = STEP_TEARDOWN_WAIT;

    hc_lock(hc);
    bool idle = dev->pending == 0;
    dev->teardown_wait = !idle;
    hc_unlock(hc);
    if (!idle) {
        return;
    }

    tendril_unbind_interfaces(dev);
    if (dev->announced) {
        dev->announced = false;
        tendril_notify_removed(dev);
    }

    dev->step = STEP_TEARDOWN_FREE;
    dev->op_busy = true;
    hc_lock(hc);
    hc->ops->device_free(hc, dev, &dev->async);
    hc_unlock(hc);
}

/* Called by transfer completion when the last transfer of a gone device finished. */
void tendril_device_transfers_idle(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    hc_lock(hc);
    if (dev->teardown_wait && dev->step == STEP_TEARDOWN_WAIT) {
        dev->teardown_wait = false;
        dev->async.status = TENDRIL_OK;
        tendril_queue_work(hc, &dev->async.work);
    }
    hc_unlock(hc);
}

/* Accessors */

struct tendril_hc *tendril_device_hc(struct tendril_device *dev) {
    return dev->hc;
}

struct tendril_device *tendril_device_parent(struct tendril_device *dev) {
    return dev->parent;
}

unsigned tendril_device_port(struct tendril_device *dev) {
    return dev->port->number;
}

unsigned tendril_device_depth(struct tendril_device *dev) {
    return dev->depth;
}

enum tendril_speed tendril_device_speed(struct tendril_device *dev) {
    return dev->speed;
}

uint8_t tendril_device_address(struct tendril_device *dev) {
    return dev->address;
}

uint32_t tendril_device_quirks(struct tendril_device *dev) {
    return dev->quirks;
}

bool tendril_device_gone(struct tendril_device *dev) {
    return dev->gone;
}

const struct tendril_device_descriptor *tendril_device_descriptor(struct tendril_device *dev) {
    return &dev->desc;
}

const struct tendril_config_descriptor *tendril_device_config_descriptor(struct tendril_device *dev, unsigned index) {
    if (index >= dev->num_configs) {
        return NULL;
    }
    return (const void *)dev->configs[index];
}

const struct tendril_config_descriptor *tendril_device_active_config(struct tendril_device *dev) {
    if (dev->active_config < 0) {
        return NULL;
    }
    return (const void *)dev->configs[dev->active_config];
}

const struct tendril_bos_descriptor *tendril_device_bos(struct tendril_device *dev) {
    return (const void *)dev->bos;
}

const char *tendril_device_manufacturer(struct tendril_device *dev) {
    return dev->manufacturer;
}

const char *tendril_device_product(struct tendril_device *dev) {
    return dev->product;
}

const char *tendril_device_serial(struct tendril_device *dev) {
    return dev->serial;
}

struct tendril_device *tendril_device_child(struct tendril_device *hub, unsigned port) {
    if (hub->hub == NULL || port == 0 || port > hub->hub->num_ports) {
        return NULL;
    }
    return hub->hub->ports[port - 1].device;
}

unsigned tendril_device_num_ports(struct tendril_device *hub) {
    return hub->hub != NULL ? hub->hub->num_ports : 0;
}

struct tendril_endpoint *tendril_device_ep0(struct tendril_device *dev) {
    return &dev->ep0;
}

unsigned tendril_device_num_interfaces(struct tendril_device *dev) {
    return dev->num_interfaces;
}

struct tendril_interface *tendril_device_interface(struct tendril_device *dev, unsigned index) {
    if (index >= dev->num_interfaces) {
        return NULL;
    }
    return &dev->interfaces[index];
}

struct tendril_interface *tendril_device_find_interface(struct tendril_device *dev, uint8_t number) {
    return find_interface(dev, number);
}

struct tendril_device *tendril_interface_device(struct tendril_interface *intf) {
    return intf->dev;
}

const struct tendril_interface_descriptor *tendril_interface_descriptor(struct tendril_interface *intf) {
    return intf->alts[intf->cur_alt];
}

const uint8_t *tendril_interface_descriptors(struct tendril_interface *intf, size_t *size) {
    *size = intf->alt_sizes[intf->cur_alt];
    return (const uint8_t *)intf->alts[intf->cur_alt];
}

const struct tendril_interface_assoc_descriptor *tendril_interface_association(struct tendril_interface *intf) {
    return intf->iad;
}

unsigned tendril_interface_num_alts(struct tendril_interface *intf) {
    return intf->num_alts;
}

const struct tendril_interface_descriptor *tendril_interface_alt_descriptor(struct tendril_interface *intf, unsigned index) {
    if (index >= intf->num_alts) {
        return NULL;
    }
    return intf->alts[index];
}

unsigned tendril_interface_num_endpoints(struct tendril_interface *intf) {
    return intf->num_endpoints;
}

struct tendril_endpoint *tendril_interface_endpoint(struct tendril_interface *intf, unsigned index) {
    if (index >= intf->num_endpoints) {
        return NULL;
    }
    return &intf->endpoints[index];
}

struct tendril_endpoint *tendril_interface_find_endpoint(struct tendril_interface *intf, uint8_t type, uint8_t dir) {
    for (unsigned i = 0; i < intf->num_endpoints; i++) {
        struct tendril_endpoint *ep = &intf->endpoints[i];
        if (ep->type == type && (ep->address & TENDRIL_ENDPOINT_DIR_MASK) == dir) {
            return ep;
        }
    }
    return NULL;
}

void tendril_interface_set_data(struct tendril_interface *intf, void *data) {
    intf->driver_data = data;
}

void *tendril_interface_data(struct tendril_interface *intf) {
    return intf->driver_data;
}

struct tendril_device *tendril_endpoint_device(struct tendril_endpoint *ep) {
    return ep->dev;
}

const struct tendril_endpoint_descriptor *tendril_endpoint_descriptor(struct tendril_endpoint *ep) {
    return ep->desc;
}

const struct tendril_ss_ep_companion_descriptor *tendril_endpoint_companion(struct tendril_endpoint *ep) {
    return ep->companion;
}

uint8_t tendril_endpoint_address(struct tendril_endpoint *ep) {
    return ep->address;
}

uint8_t tendril_endpoint_type(struct tendril_endpoint *ep) {
    return ep->type;
}

uint16_t tendril_endpoint_max_packet(struct tendril_endpoint *ep) {
    return ep->max_packet;
}

unsigned tendril_endpoint_max_streams(struct tendril_endpoint *ep) {
    return ep->max_streams;
}

/* Embedder-requested operations */

struct dev_op {
    struct tendril_device *dev;
    struct tendril_async async;
    struct tendril_timer timer;
    struct tendril_transfer xfer;
    unsigned step;
    void (*done)(void *ctx, enum tendril_status status);
    void *ctx;
    /* Endpoint and interface state being replaced. */
    struct tendril_interface *intf;
    unsigned alt;
    struct tendril_endpoint *new_eps;
    unsigned num_new_eps;
    struct tendril_endpoint *ep;
    uint8_t value;
    char *out;
    size_t out_size;
    /* Power management: a request is out, the give-up time, and the result once undone. */
    bool requested;
    uint64_t deadline;
    enum tendril_status result;
    uint8_t buf[256];
};

static void config_work(struct tendril_work *work);

static struct dev_op *op_alloc(struct tendril_device *dev, void (*fn)(struct tendril_work *),
                               void (*done)(void *, enum tendril_status), void *ctx) {
    struct dev_op *op = tendril_zalloc(sizeof(*op));
    if (op == NULL) {
        return NULL;
    }
    op->dev = dev;
    op->done = done;
    op->ctx = ctx;
    tendril_async_init(&op->async, fn);
    tendril_device_ref(dev);
    /* Counted like a transfer, so drivers are not disconnected before `done` runs. */
    hc_lock(dev->hc);
    dev->pending++;
    hc_unlock(dev->hc);
    return op;
}

static void op_finish(struct dev_op *op, enum tendril_status status) {
    if (op->async.work.fn == config_work) {
        op->dev->config_busy = false;
        op->dev->config_xfer = NULL;
    }
    void (*done)(void *, enum tendril_status) = op->done;
    void *ctx = op->ctx;
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    tendril_free(op, sizeof(*op));
    if (done != NULL) {
        done(ctx, status);
    }
    hc_lock(hc);
    dev->pending--;
    bool idle = dev->gone && dev->pending == 0;
    hc_unlock(hc);
    if (idle) {
        tendril_device_transfers_idle(dev);
    }
    tendril_device_unref(dev);
}

static unsigned pending_on_interface(struct tendril_interface *intf) {
    unsigned n = 0;
    for (unsigned e = 0; e < intf->num_endpoints; e++) {
        struct tendril_list *it;
        LIST_FOR_EACH(it, &intf->endpoints[e].transfers) {
            n++;
        }
    }
    return n;
}

/* Alternate settings */

enum {
    ALT_CONFIGURE,
    ALT_SET_INTERFACE
};

static void alt_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    struct tendril_interface *intf = op->intf;
    struct tendril_device *dev = op->dev;
    enum tendril_status st = op->async.status;

    if (dev->gone) {
        tendril_free(op->new_eps, op->num_new_eps * sizeof(*op->new_eps));
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    switch (op->step) {
        case ALT_CONFIGURE: {
            if (st != TENDRIL_OK) {
                tendril_free(op->new_eps, op->num_new_eps * sizeof(*op->new_eps));
                op_finish(op, st);
                return;
            }
            /* The old endpoints are closed; switch to the new ones before the request. */
            tendril_free(intf->endpoints, intf->num_endpoints * sizeof(*intf->endpoints));
            intf->endpoints = op->new_eps;
            intf->num_endpoints = op->num_new_eps;
            intf->cur_alt = op->alt;
            for (unsigned e = 0; e < intf->num_endpoints; e++) {
                intf->endpoints[e].intf = intf;
            }
            op->new_eps = NULL;
            op->num_new_eps = 0;
            op->step = ALT_SET_INTERFACE;
            st = tendril_control_async(dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_INTERFACE,
                                      TENDRIL_REQ_SET_INTERFACE, intf->alts[op->alt]->alternate_setting, intf->number,
                                      NULL, 0, &op->async);
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case ALT_SET_INTERFACE: {
            /* USB 2.0 9.4.10: a device with one setting may stall the request. */
            if (st == TENDRIL_ERR_STALL && intf->num_alts == 1) {
                st = TENDRIL_OK;
            }
            op_finish(op, st);
            return;
        }
        default: {
            op_finish(op, TENDRIL_ERR_INVALID);
            return;
        }
    }
}

enum tendril_status tendril_interface_set_alt(
    struct tendril_interface *intf, uint8_t alt,
    void (*done)(void *ctx, enum tendril_status status), void *ctx) {
    struct tendril_device *dev = intf->dev;
    struct tendril_hc *hc = dev->hc;
    unsigned index = intf->num_alts;

    for (unsigned a = 0; a < intf->num_alts; a++) {
        if (intf->alts[a]->alternate_setting == alt) {
            index = a;
            break;
        }
    }
    if (index == intf->num_alts) {
        return TENDRIL_ERR_INVALID;
    }
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }

    struct dev_op *op = op_alloc(dev, alt_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->intf = intf;
    op->alt = index;
    enum tendril_status st = tendril_interface_build_endpoints(intf, index, &op->new_eps, &op->num_new_eps);
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
        return st;
    }

    struct tendril_endpoint *drop[MAX_DEVICE_ENDPOINTS];
    struct tendril_endpoint *add[MAX_DEVICE_ENDPOINTS];
    unsigned num_drop = MIN(intf->num_endpoints, MAX_DEVICE_ENDPOINTS);
    unsigned num_add = MIN(op->num_new_eps, MAX_DEVICE_ENDPOINTS);
    for (unsigned e = 0; e < num_drop; e++) {
        drop[e] = &intf->endpoints[e];
    }
    for (unsigned e = 0; e < num_add; e++) {
        add[e] = &op->new_eps[e];
    }

    op->step = ALT_CONFIGURE;
    hc_lock(hc);
    if (pending_on_interface(intf) != 0) {
        st = TENDRIL_ERR_BUSY;
    } else if (hc->dead) {
        st = TENDRIL_ERR_NO_DEVICE;
    } else {
        st = hc->ops->configure(hc, dev, drop, num_drop, add, num_add, &op->async);
    }
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        tendril_free(op->new_eps, op->num_new_eps * sizeof(*op->new_eps));
        op->done = NULL;
        op_finish(op, st);
    }
    return st;
}

/* Halts */

enum {
    HALT_CLEAR_FEATURE,
    HALT_RESET
};

static void halt_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    enum tendril_status st = op->async.status;

    if (dev->gone) {
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    if (op->step == HALT_CLEAR_FEATURE) {
        if (st != TENDRIL_OK) {
            op_finish(op, st);
            return;
        }
        op->step = HALT_RESET;
        hc_lock(hc);
        st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->endpoint_reset(hc, op->ep, &op->async);
        hc_unlock(hc);
        if (st != TENDRIL_OK) {
            op_finish(op, st);
        }
        return;
    }
    op_finish(op, st);
}

enum tendril_status tendril_endpoint_clear_halt(
    struct tendril_endpoint *ep,
    void (*done)(void *ctx, enum tendril_status status), void *ctx) {
    struct tendril_device *dev = ep->dev;

    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    struct dev_op *op = op_alloc(dev, halt_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->ep = ep;
    op->step = HALT_CLEAR_FEATURE;
    enum tendril_status st = tendril_control_async(dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_ENDPOINT,
                                                 TENDRIL_REQ_CLEAR_FEATURE, TENDRIL_FEATURE_ENDPOINT_HALT, ep->address,
                                                 NULL, 0, &op->async);
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
    }
    return st;
}

/* Streams */

static void streams_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    op_finish(op, op->dev->gone ? TENDRIL_ERR_NO_DEVICE : op->async.status);
}

enum tendril_status tendril_endpoint_alloc_streams(
    struct tendril_endpoint **eps, unsigned count, unsigned *streams,
    void (*done)(void *ctx, enum tendril_status status), void *ctx) {
    if (count == 0 || *streams == 0) {
        return TENDRIL_ERR_INVALID;
    }
    struct tendril_device *dev = eps[0]->dev;
    struct tendril_hc *hc = dev->hc;

    if (hc->ops->alloc_streams == NULL || (hc->quirks & TENDRIL_HC_QUIRK_NO_STREAMS)) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    unsigned n = *streams;
    for (unsigned i = 0; i < count; i++) {
        if (eps[i]->dev != dev || eps[i]->type != TENDRIL_ENDPOINT_XFER_BULK || eps[i]->max_streams == 0) {
            return TENDRIL_ERR_UNSUPPORTED;
        }
        if (eps[i]->max_streams - 1 < n) {
            n = eps[i]->max_streams - 1;
        }
    }
    if (n == 0) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    struct dev_op *op = op_alloc(dev, streams_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    hc_lock(hc);
    enum tendril_status st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->alloc_streams(hc, eps, count, n, &op->async);
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
        return st;
    }
    /* The controller may have granted fewer. */
    *streams = eps[0]->streams;
    return TENDRIL_OK;
}

/* Strings */

static void string_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    struct tendril_device *dev = op->dev;
    enum tendril_status st = op->async.status;

    if (dev->gone) {
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    if (st != TENDRIL_OK && st != TENDRIL_ERR_SHORT) {
        op_finish(op, st);
        return;
    }
    char *s = decode_string(op->buf, op->xfer.actual, (dev->quirks & TENDRIL_DEV_QUIRK_SWAP_UNICODE) != 0);
    if (s == NULL) {
        op_finish(op, TENDRIL_ERR_PROTOCOL);
        return;
    }
    size_t n = tendril_strlen(s);
    if (op->out_size != 0) {
        size_t copy = MIN(n, op->out_size - 1);
        memcpy(op->out, s, copy);
        op->out[copy] = '\0';
    }
    tendril_strfree(s);
    op_finish(op, TENDRIL_OK);
}

enum tendril_status tendril_device_read_string(
    struct tendril_device *dev, uint8_t index, char *buf, size_t size,
    void (*done)(void *ctx, enum tendril_status status), void *ctx) {
    if (index == 0) {
        return TENDRIL_ERR_INVALID;
    }
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    struct dev_op *op = op_alloc(dev, string_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->out = buf;
    op->out_size = size;
    uint16_t langid = dev->langid != 0 ? dev->langid : 0x0409;
    enum tendril_status st = tendril_control_async(dev, &op->xfer, TENDRIL_DIR_IN | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                                                 TENDRIL_REQ_GET_DESCRIPTOR, (uint16_t)(TENDRIL_DT_STRING << 8 | index),
                                                 langid, op->buf, 255, &op->async);
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
    }
    return st;
}

/* Configuration changes */

enum {
    CFG_WAIT,
    CFG_DECONFIGURE_REQUEST,
    CFG_DROP,
    CFG_ADD,
    CFG_SET_CONFIGURATION
};

static void config_op_step(struct dev_op *op);

/* The index of the configuration with bConfigurationValue `value`, or -1. */
static int find_config(struct tendril_device *dev, uint8_t value) {
    for (unsigned i = 0; i < dev->num_configs; i++) {
        const struct tendril_config_descriptor *cd = (const void *)dev->configs[i];
        if (cd != NULL && cd->configuration_value == value) {
            return (int)i;
        }
    }
    return -1;
}

static void config_work(struct tendril_work *work) {
    config_op_step(CONTAINER_OF(work, struct dev_op, async.work));
}

static void config_timer_work(struct tendril_work *work) {
    config_op_step(CONTAINER_OF(work, struct dev_op, timer.work));
}

static void config_op_step(struct dev_op *op) {
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    enum tendril_status st = op->async.status;

    if (dev->gone) {
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    switch (op->step) {
        case CFG_WAIT: {
            /* Everything the drivers started must finish before they are unbound; this operation counts too. */
            hc_lock(hc);
            bool busy = dev->pending > 1;
            if (busy) {
                tendril_timer_arm(hc, &op->timer, NS_PER_MS);
            }
            hc_unlock(hc);
            if (busy) {
                return;
            }
            tendril_unbind_interfaces(dev);
            op->step = CFG_DECONFIGURE_REQUEST;
            st = tendril_control_async(dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                                      TENDRIL_REQ_SET_CONFIGURATION, 0, 0, NULL, 0, &op->async);
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case CFG_DECONFIGURE_REQUEST: {
            struct tendril_endpoint *drop[MAX_DEVICE_ENDPOINTS];
            unsigned num_drop = collect_endpoints(dev, drop, MAX_DEVICE_ENDPOINTS);
            op->step = CFG_DROP;
            hc_lock(hc);
            st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->configure(hc, dev, drop, num_drop, NULL, 0, &op->async);
            hc_unlock(hc);
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case CFG_DROP: {
            free_interfaces(dev);
            dev->active_config = -1;
            dev->state = DEV_ADDRESSED;
            if (st != TENDRIL_OK || op->value == 0) {
                op_finish(op, st);
                return;
            }
            int index = find_config(dev, op->value);
            if (index < 0) {
                op_finish(op, TENDRIL_ERR_INVALID);
                return;
            }
            st = build_interfaces(dev, (unsigned)index);
            for (unsigned i = 0; i < dev->num_interfaces && st == TENDRIL_OK; i++) {
                struct tendril_interface *intf = &dev->interfaces[i];
                st = tendril_interface_build_endpoints(intf, intf->cur_alt, &intf->endpoints, &intf->num_endpoints);
            }
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                op_finish(op, st);
                return;
            }
            dev->active_config = index;
            struct tendril_endpoint *add[MAX_DEVICE_ENDPOINTS];
            unsigned num_add = collect_endpoints(dev, add, MAX_DEVICE_ENDPOINTS);
            op->step = CFG_ADD;
            hc_lock(hc);
            st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->configure(hc, dev, NULL, 0, add, num_add, &op->async);
            hc_unlock(hc);
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                dev->active_config = -1;
                op_finish(op, st);
            }
            return;
        }
        case CFG_ADD: {
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                dev->active_config = -1;
                op_finish(op, st);
                return;
            }
            op->step = CFG_SET_CONFIGURATION;
            st = tendril_control_async(dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                                      TENDRIL_REQ_SET_CONFIGURATION, op->value, 0, NULL, 0, &op->async);
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case CFG_SET_CONFIGURATION: {
            /* The new drivers' requests go through from their probe on. */
            dev->config_busy = false;
            dev->config_xfer = NULL;
            if (st == TENDRIL_OK) {
                dev->state = DEV_CONFIGURED;
                tendril_bind_interfaces(dev);
            }
            op_finish(op, st);
            return;
        }
        default: {
            op_finish(op, TENDRIL_ERR_INVALID);
            return;
        }
    }
}

enum tendril_status tendril_device_set_configuration(
    struct tendril_device *dev, uint8_t value,
    void (*done)(void *ctx, enum tendril_status status), void *ctx) {
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->suspended) {
        return TENDRIL_ERR_SUSPENDED;
    }
    if (dev->step != STEP_RUNNING || dev->hub != NULL || dev->config_busy || dev->pm_busy) {
        return TENDRIL_ERR_BUSY;
    }
    /* Checked now, as the change starts by tearing the current configuration down. */
    if (value != 0 && find_config(dev, value) < 0) {
        return TENDRIL_ERR_INVALID;
    }
    struct dev_op *op = op_alloc(dev, config_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    tendril_timer_init(&op->timer, config_timer_work);
    dev->config_busy = true;
    dev->config_xfer = &op->xfer;
    op->value = value;
    op->step = CFG_WAIT;
    /*
     * Drivers must stop their transfers before they are unbound: everything
     * outstanding is cancelled, control requests too, and new ones are refused.
     */
    hc_lock(dev->hc);
    cancel_all(dev, TENDRIL_ERR_CANCELLED, true);
    tendril_queue_work(dev->hc, &op->async.work);
    hc_unlock(dev->hc);
    return TENDRIL_OK;
}

/* Power management */

/* How long drivers get to finish their transfers before a suspend gives up. */
#define PM_WAIT_TIMEOUT_MS 5000

enum {
    PM_SUSPEND_WAIT,
    PM_SUSPEND_WAKE,
    PM_SUSPEND_HCD,
    PM_SUSPEND_PORT,
    PM_SUSPEND_DONE,
    PM_RESUME_PARENT,
    PM_RESUME_PORT,
    PM_RESUME_CLEAR,
    PM_RESUME_DONE
};

static void pm_step(struct dev_op *op);

static void pm_work(struct tendril_work *work) {
    pm_step(CONTAINER_OF(work, struct dev_op, async.work));
}

static void pm_timer_work(struct tendril_work *work) {
    pm_step(CONTAINER_OF(work, struct dev_op, timer.work));
}

bool tendril_device_suspended(struct tendril_device *dev) {
    return dev->suspended;
}

bool tendril_device_can_wake(struct tendril_device *dev) {
    const struct tendril_config_descriptor *cd = tendril_device_active_config(dev);
    return cd != NULL && (cd->attributes & TENDRIL_CONFIG_ATTR_REMOTE_WAKEUP) != 0;
}

/* USB 3.2 revision 1.1 9.4.9: FUNCTION_SUSPEND goes to the first interface of each function. */
static bool function_head(struct tendril_interface *intf) {
    return intf->iad == NULL || intf->iad->first_interface == intf->number;
}

/* A suspended child counts on its hub to pass its wakeup on. */
static bool child_wake_armed(struct tendril_device *dev) {
    struct tendril_hub *hub = dev->hub;
    for (unsigned i = 0; hub != NULL && i < hub->num_ports; i++) {
        struct tendril_device *child = hub->ports[i].device;
        if (child != NULL && (child->pm_wake || child->pm_functions)) {
            return true;
        }
    }
    return false;
}

static void pm_finish(struct dev_op *op, enum tendril_status status) {
    struct tendril_device *dev = op->dev;
    bool woke = dev->pm_woke && dev->suspended && status == TENDRIL_OK;
    dev->pm_busy = false;
    if (woke) {
        tendril_device_ref(dev);
    }
    op_finish(op, status);
    if (woke) {
        /* The device signalled a wakeup while its suspend was finishing. */
        tendril_device_woke(dev);
        tendril_device_unref(dev);
    }
}

static void pm_request(struct dev_op *op, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index) {
    op->requested = true;
    enum tendril_status st = tendril_control_async(op->dev, &op->xfer, request_type, request, value, index, NULL, 0,
                                                 &op->async);
    if (st != TENDRIL_OK) {
        hc_lock(op->dev->hc);
        tendril_async_complete(op->dev->hc, &op->async, st);
        hc_unlock(op->dev->hc);
    }
}

/*
 * Sends the next request arming (or, on resume, disarming) remote wakeup:
 * DEVICE_REMOTE_WAKEUP for USB 2.0 devices, FUNCTION_SUSPEND to each function
 * of a USB 3 device. False once none are left.
 */
static bool pm_next_wake_request(struct dev_op *op, bool arm) {
    struct tendril_device *dev = op->dev;

    if (!tendril_device_is_ss(dev)) {
        if (op->alt++ != 0 || !(arm ? op->value != 0 : dev->pm_wake)) {
            return false;
        }
        pm_request(op, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                   arm ? TENDRIL_REQ_SET_FEATURE : TENDRIL_REQ_CLEAR_FEATURE, TENDRIL_FEATURE_DEVICE_REMOTE_WAKEUP, 0);
        return true;
    }
    if (!(arm ? op->value != 0 : dev->pm_functions)) {
        return false;
    }
    while (op->alt < dev->num_interfaces) {
        struct tendril_interface *intf = &dev->interfaces[op->alt++];
        if (!function_head(intf)) {
            continue;
        }
        unsigned options = arm ? TENDRIL_FUNCTION_SUSPEND_LOW_POWER | TENDRIL_FUNCTION_SUSPEND_REMOTE_WAKE : 0;
        pm_request(op, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_INTERFACE, TENDRIL_REQ_SET_FEATURE,
                   TENDRIL_FEATURE_FUNCTION_SUSPEND, (uint16_t)(intf->number | options << 8));
        return true;
    }
    return false;
}

/* Undoes a suspend that got partway: disarms wakeup, then lets the drivers carry on. */
static void pm_abort(struct dev_op *op, enum tendril_status status) {
    op->result = status;
    op->step = PM_RESUME_CLEAR;
    op->alt = 0;
    op->requested = false;
    op->async.status = TENDRIL_OK;
    pm_step(op);
}

static void pm_step(struct dev_op *op) {
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    enum tendril_status st = op->async.status;
    char name[24];

    if (dev->gone) {
        pm_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    switch (op->step) {
        case PM_SUSPEND_WAIT: {
            /* The drivers' transfers drain; this operation counts too. */
            hc_lock(hc);
            bool busy = dev->pending > 1;
            bool waiting = busy && tendril_now() < op->deadline;
            if (waiting) {
                tendril_timer_arm(hc, &op->timer, NS_PER_MS);
            }
            bool hub_ready = true;
            for (unsigned i = 0; dev->hub != NULL && i < dev->hub->num_ports && !busy; i++) {
                struct tendril_port *port = &dev->hub->ports[i];
                if (!tendril_port_idle(port) || (port->device != NULL && !port->device->suspended)) {
                    hub_ready = false;
                }
            }
            hc_unlock(hc);
            if (waiting) {
                return;
            }
            if (busy || !hub_ready) {
                dev_name(dev, name, sizeof(name));
                LOG_WARN("tendril: %s: suspend abandoned: %s", name, busy ? "transfers did not finish" : "hub busy");
                pm_abort(op, TENDRIL_ERR_BUSY);
                return;
            }
            if (dev->hub != NULL && child_wake_armed(dev) && tendril_device_can_wake(dev)) {
                op->value = 1;
            }
            op->step = PM_SUSPEND_WAKE;
            op->alt = 0;
            op->requested = false;
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_SUSPEND_WAKE: {
            if (op->requested && st != TENDRIL_OK) {
                if (st == TENDRIL_ERR_NO_DEVICE) {
                    pm_finish(op, st);
                    return;
                }
                dev_name(dev, name, sizeof(name));
                LOG_DEBUG("tendril: %s: arming remote wakeup failed: %s", name, tendril_status_str(st));
            } else if (op->requested) {
                if (tendril_device_is_ss(dev)) {
                    dev->pm_functions = true;
                } else {
                    dev->pm_wake = true;
                }
            }
            op->requested = false;
            if (pm_next_wake_request(op, true)) {
                return;
            }
            op->step = PM_SUSPEND_HCD;
            if (hc->ops->device_suspend != NULL) {
                hc_lock(hc);
                st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->device_suspend(hc, dev, &op->async);
                hc_unlock(hc);
                if (st != TENDRIL_OK) {
                    pm_abort(op, st);
                }
                return;
            }
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_SUSPEND_HCD: {
            if (st != TENDRIL_OK) {
                pm_abort(op, st);
                return;
            }
            op->step = PM_SUSPEND_PORT;
            tendril_port_pm(dev->port, PORT_PM_SUSPEND, &op->async);
            return;
        }
        case PM_SUSPEND_PORT: {
            if (st != TENDRIL_OK) {
                pm_abort(op, st);
                return;
            }
            dev->suspended = true;
            dev_name(dev, name, sizeof(name));
            LOG_DEBUG("tendril: %s: suspended%s", name, dev->pm_wake || dev->pm_functions ? ", remote wakeup armed" : "");
            pm_finish(op, TENDRIL_OK);
            return;
        }
        case PM_RESUME_PARENT: {
            /* The hub above comes back first, which may take its own resume. */
            struct tendril_device *parent = dev->parent;
            if (parent != NULL && (parent->suspended || parent->pm_busy)) {
                if (tendril_now() >= op->deadline) {
                    pm_finish(op, TENDRIL_ERR_TIMEOUT);
                    return;
                }
                if (!parent->pm_busy) {
                    if (op->requested) {
                        pm_finish(op, TENDRIL_ERR_IO);
                        return;
                    }
                    st = tendril_device_resume(parent, NULL, NULL);
                    if (st != TENDRIL_OK) {
                        pm_finish(op, st);
                        return;
                    }
                    op->requested = true;
                }
                hc_lock(hc);
                tendril_timer_arm(hc, &op->timer, NS_PER_MS);
                hc_unlock(hc);
                return;
            }
            op->step = PM_RESUME_PORT;
            tendril_port_pm(dev->port, PORT_PM_RESUME, &op->async);
            return;
        }
        case PM_RESUME_PORT: {
            if (st != TENDRIL_OK) {
                pm_finish(op, st);
                return;
            }
            dev->suspended = false;
            op->step = PM_RESUME_CLEAR;
            op->alt = 0;
            op->requested = false;
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_RESUME_CLEAR: {
            if (op->requested && st != TENDRIL_OK) {
                dev_name(dev, name, sizeof(name));
                LOG_DEBUG("tendril: %s: disarming remote wakeup failed: %s", name, tendril_status_str(st));
            }
            op->requested = false;
            if (pm_next_wake_request(op, false)) {
                return;
            }
            dev->pm_wake = false;
            dev->pm_functions = false;
            op->step = PM_RESUME_DONE;
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_RESUME_DONE: {
            bool woke = dev->pm_woke;
            dev->pm_woke = false;
            dev->pm_system = false;
            if (dev->hub != NULL) {
                tendril_hub_resume(dev->hub);
            }
            tendril_resume_interfaces(dev);
            if (op->result == TENDRIL_OK) {
                dev_name(dev, name, sizeof(name));
                LOG_DEBUG("tendril: %s: resumed%s", name, woke ? " by remote wakeup" : "");
            }
            if (woke && !dev->gone) {
                tendril_notify_woke(dev);
            }
            pm_finish(op, op->result);
            return;
        }
        default: {
            pm_finish(op, TENDRIL_ERR_INVALID);
            return;
        }
    }
}

enum tendril_status tendril_device_suspend(
    struct tendril_device *dev, bool remote_wake,
    void (*done)(void *ctx, enum tendril_status status), void *ctx) {
    struct tendril_hc *hc = dev->hc;

    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    if (dev->step != STEP_RUNNING || dev->pm_busy || dev->config_busy) {
        return TENDRIL_ERR_BUSY;
    }
    if (dev->hub != NULL && !tendril_hub_suspend(dev->hub)) {
        return TENDRIL_ERR_BUSY;
    }
    enum tendril_status st;
    if (!tendril_suspend_interfaces(dev, &st)) {
        if (dev->hub != NULL) {
            tendril_hub_resume(dev->hub);
        }
        return st;
    }
    struct dev_op *op = op_alloc(dev, pm_work, done, ctx);
    if (op == NULL) {
        if (dev->hub != NULL) {
            tendril_hub_resume(dev->hub);
        }
        tendril_resume_interfaces(dev);
        return TENDRIL_ERR_NO_MEMORY;
    }
    tendril_timer_init(&op->timer, pm_timer_work);
    dev->pm_busy = true;
    dev->pm_woke = false;
    op->value = remote_wake && tendril_device_can_wake(dev);
    op->result = TENDRIL_OK;
    op->step = PM_SUSPEND_WAIT;
    op->deadline = tendril_now() + PM_WAIT_TIMEOUT_MS * NS_PER_MS;
    hc_lock(hc);
    cancel_all(dev, TENDRIL_ERR_CANCELLED, false);
    tendril_queue_work(hc, &op->async.work);
    hc_unlock(hc);
    return TENDRIL_OK;
}

enum tendril_status tendril_device_resume(
    struct tendril_device *dev,
    void (*done)(void *ctx, enum tendril_status status), void *ctx) {
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->pm_busy) {
        return TENDRIL_ERR_BUSY;
    }
    if (!dev->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    struct dev_op *op = op_alloc(dev, pm_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    tendril_timer_init(&op->timer, pm_timer_work);
    dev->pm_busy = true;
    op->result = TENDRIL_OK;
    op->step = PM_RESUME_PARENT;
    op->deadline = tendril_now() + PM_WAIT_TIMEOUT_MS * NS_PER_MS;
    hc_lock(dev->hc);
    tendril_queue_work(dev->hc, &op->async.work);
    hc_unlock(dev->hc);
    return TENDRIL_OK;
}

void tendril_device_woke(struct tendril_device *dev) {
    if (dev->gone) {
        return;
    }
    dev->pm_woke = true;
    if (dev->pm_busy || !dev->suspended) {
        /* The operation under way reports it once finished. */
        if (!dev->pm_busy) {
            dev->pm_woke = false;
        }
        return;
    }
    struct dev_op *op = op_alloc(dev, pm_work, NULL, NULL);
    if (op == NULL) {
        return;
    }
    tendril_timer_init(&op->timer, pm_timer_work);
    dev->pm_busy = true;
    dev->suspended = false;
    op->result = TENDRIL_OK;
    op->step = PM_RESUME_CLEAR;
    hc_lock(dev->hc);
    tendril_queue_work(dev->hc, &op->async.work);
    hc_unlock(dev->hc);
}
