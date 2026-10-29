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

#ifndef TENDRIL_INTERNAL_H
#define TENDRIL_INTERNAL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tendril/tendril.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define CONTAINER_OF(ptr, type, member) ((type *)(void *)((char *)(ptr) - offsetof(type, member)))
#define ALIGN_UP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define NS_PER_US ((uint64_t)1000)
#define NS_PER_MS ((uint64_t)1000000)

#define memcpy __builtin_memcpy
#define memset __builtin_memset
#define memmove __builtin_memmove
#define memcmp __builtin_memcmp

/* Lists */

static inline void list_init(struct tendril_list *head) {
    head->prev = head;
    head->next = head;
}

static inline bool list_empty(const struct tendril_list *head) {
    return head->next == head;
}

static inline void list_insert(struct tendril_list *prev, struct tendril_list *node) {
    node->prev = prev;
    node->next = prev->next;
    prev->next->prev = node;
    prev->next = node;
}

static inline void list_add_tail(struct tendril_list *head, struct tendril_list *node) {
    list_insert(head->prev, node);
}

static inline void list_add_head(struct tendril_list *head, struct tendril_list *node) {
    list_insert(head, node);
}

static inline void list_del(struct tendril_list *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->prev = node;
    node->next = node;
}

static inline bool list_linked(const struct tendril_list *node) {
    return node->next != node && node->next != NULL;
}

#define LIST_FOR_EACH(it, head) \
    for ((it) = (head)->next; (it) != (head); (it) = (it)->next)

#define LIST_FOR_EACH_SAFE(it, tmp, head) \
    for ((it) = (head)->next, (tmp) = (it)->next; (it) != (head); (it) = (tmp), (tmp) = (it)->next)

/* Logging */

extern int tendril_log_threshold;

void tendril_log(int level, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void tendril_vlog(int level, const char *fmt, va_list args);
int tendril_vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int tendril_snprintf(char *buf, size_t size, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#define LOG_ERROR(...) tendril_log(TENDRIL_LOG_ERROR, __VA_ARGS__)
#define LOG_WARN(...) tendril_log(TENDRIL_LOG_WARN, __VA_ARGS__)
#define LOG_INFO(...) tendril_log(TENDRIL_LOG_INFO, __VA_ARGS__)
#define LOG_DEBUG(...) tendril_log(TENDRIL_LOG_DEBUG, __VA_ARGS__)
#define LOG_TRACE(...) tendril_log(TENDRIL_LOG_TRACE, __VA_ARGS__)

/* Memory */

void *tendril_zalloc(size_t size);
void tendril_free(void *ptr, size_t size);
char *tendril_strdup(const char *s);
void tendril_strfree(char *s);
size_t tendril_strlen(const char *s);

/* Time */

static inline uint64_t tendril_now(void) {
    return tendril_glue_time_ns();
}

void tendril_delay_us(uint64_t us);

/* DMA */

struct tendril_dma_pool {
    struct tendril_hc *hc;
    size_t size;
    size_t align;
    struct tendril_dma_chunk *chunks;
    struct tendril_dma_free *free;
};

void tendril_dma_pool_init(struct tendril_dma_pool *pool, struct tendril_hc *hc, size_t size, size_t align);
void *tendril_dma_pool_alloc(struct tendril_dma_pool *pool, uint64_t *dma);
void tendril_dma_pool_free(struct tendril_dma_pool *pool, void *ptr, uint64_t dma);
void tendril_dma_pool_destroy(struct tendril_dma_pool *pool);

void *tendril_dma_alloc(struct tendril_hc *hc, size_t size, size_t align, uint64_t *dma);
void *tendril_dma_alloc_limit(struct tendril_hc *hc, size_t size, size_t align, bool dma32, uint64_t *dma);
void tendril_dma_free(struct tendril_hc *hc, void *ptr, size_t size);

/* Asynchronous completion for internal state machines. */
struct tendril_async {
    struct tendril_work work;
    int status;
};

/* Ports and hubs */

/* Port status, as wPortStatus/wPortChange plus speed and protocol. */
struct tendril_port_status {
    uint16_t status;
    uint16_t change;
    int speed;
};

#define PORT_IDLE 0
#define PORT_DEBOUNCE 1
#define PORT_WAIT_ENUM 2
#define PORT_RESET 3
#define PORT_RECOVERY 4
#define PORT_ENUM 5
#define PORT_ATTACHED 6
#define PORT_DISABLED 7
#define PORT_SUSPENDING 8
#define PORT_SUSPENDED 9
#define PORT_RESUMING 10
#define PORT_RESUME_RECOVERY 11
#define PORT_RESUME_DELAY 12

#define PORT_PM_NONE 0
#define PORT_PM_SUSPEND 1
#define PORT_PM_RESUME 2

struct tendril_hub;

struct tendril_port {
    struct tendril_hub *hub;
    uint8_t number;
    bool superspeed;
    int state;
    int op;
    struct tendril_port_status st;
    uint16_t changes;
    struct tendril_device *device;
    struct tendril_async async;
    struct tendril_timer timer;
    struct tendril_list enum_node;
    uint64_t debounce_start;
    uint64_t debounce_first;
    uint64_t reset_start;
    unsigned int attempts;
    int enum_status;
    bool busy;
    bool kick;
    bool warm;
    bool reset_issued;
    bool enum_failed;
    /* Suspend or resume requested by the device layer, completing pm_async. */
    int pm_request;
    struct tendril_async *pm_async;
    uint64_t pm_start;
    /* The resume under way was signalled by the device. */
    bool woke;
    /* Reconnects after the hub disabled the port on its own, since the last connection. */
    unsigned int restarts;
    /* SET_CONFIGURATION failed during enumeration since the last connection. */
    bool config_failed;
    /* External hubs only. */
    struct tendril_transfer xfer;
    uint8_t buf[4];
    /* A TT request waiting for this port: CLEAR_TT_BUFFER with tt_value, RESET_TT, or 0. */
    uint8_t tt_request;
    uint16_t tt_value;
};

/* A hub port feature with the parameter USB 3 hubs take in wIndex bits 15:8. */
#define PORT_FEATURE(feature, param) ((unsigned int)(feature) | ((unsigned int)(param) << 8))
#define PORT_FEATURE_SELECTOR(f) ((f) & (uint32_t)0xff)
#define PORT_FEATURE_PARAM(f) (((f) >> 8) & (uint32_t)0xff)

struct tendril_hub_ops {
    /* Each completes by queueing port->async with the status in st. */
    void (*get_status)(struct tendril_port *port);
    void (*set_feature)(struct tendril_port *port, unsigned int feature);
    void (*clear_feature)(struct tendril_port *port, unsigned int feature);
};

struct tendril_hub {
    struct tendril_hc *hc;
    struct tendril_device *dev;
    const struct tendril_hub_ops *ops;
    size_t num_ports;
    struct tendril_port *ports;
    bool superspeed;
    bool multi_tt;
    /* Periodic split transaction bytes its TT carries in each microframe, for the controller's scheduling. */
    uint32_t uframe_usage[8];
    /* Power each port can give a device, in mA, at USB 2.0 and SuperSpeed. */
    uint16_t port_power;
    uint16_t port_power_ss;
    uint8_t think_time;
    uint16_t power_on_delay_ms;
    uint16_t characteristics;
    /* External hubs only. */
    struct tendril_transfer status_xfer;
    uint8_t status_buf[32];
    struct tendril_timer status_timer;
    unsigned int status_errors;
    struct tendril_transfer ctrl;
    uint8_t ctrl_buf[80];
    struct tendril_async init_async;
    int init_step;
    size_t init_port;
    struct tendril_timer init_timer;
    struct tendril_transfer tt_xfer;
    bool tt_busy;
    bool ctrl_busy;
    bool hub_change_pending;
    bool stopping;
    bool suspended;
};

void tendril_port_kick(struct tendril_port *port);
void tendril_port_init(struct tendril_port *port, struct tendril_hub *hub, size_t number);
void tendril_port_stop(struct tendril_port *port);
/* Suspends or resumes the port's device, completing `async` with the lock held. */
void tendril_port_pm(struct tendril_port *port, int request, struct tendril_async *async);
bool tendril_port_idle(struct tendril_port *port);
int tendril_roothub_init(struct tendril_hc *hc, size_t num_ports);
void tendril_hub_start(struct tendril_device *dev);
void tendril_hub_stop(struct tendril_hub *hub);
void tendril_hub_free(struct tendril_hub *hub);
void tendril_hub_check_status(struct tendril_hub *hub);
/* Stops a hub's own activity for a suspend; false if a port is busy or a child awake. */
bool tendril_hub_suspend(struct tendril_hub *hub);
void tendril_hub_resume(struct tendril_hub *hub);
/* After a failed transfer to a LS/FS device, frees the TT buffer it may have left busy. */
void tendril_hub_clear_tt(struct tendril_device *dev, struct tendril_endpoint *ep);
/* Before a LS/FS device behind a TT is enumerated again. */
void tendril_hub_reset_tt(struct tendril_device *dev);

/* Endpoints, interfaces, and devices */

struct tendril_endpoint {
    struct tendril_device *dev;
    struct tendril_interface *intf;
    const struct tendril_endpoint_descriptor *desc;
    const struct tendril_ss_ep_companion_descriptor *companion;
    const struct tendril_ssp_iso_ep_companion_descriptor *ssp_companion;
    uint8_t address;
    uint8_t type;
    uint16_t max_packet;
    /* Packets per service interval: HS high bandwidth or SS bursts times mult. */
    uint8_t mult;
    uint8_t max_burst;
    /* Service interval in microframes, and as a log2 of microframes. */
    uint32_t interval;
    uint8_t interval_log2;
    uint32_t max_esit_payload;
    unsigned int max_streams;
    unsigned int streams;
    struct tendril_list transfers;
    void *hcd;
    bool open;
};

struct tendril_interface {
    struct tendril_device *dev;
    uint8_t number;
    size_t num_alts;
    const struct tendril_interface_descriptor **alts;
    size_t *alt_sizes;
    size_t cur_alt;
    size_t num_endpoints;
    struct tendril_endpoint *endpoints;
    const struct tendril_interface_assoc_descriptor *iad;
    struct tendril_driver *driver;
    /* The interface whose probe() claimed this one. */
    struct tendril_interface *primary;
    void *driver_data;
};

#define DEV_ENUMERATING 0
#define DEV_ADDRESSED 1
#define DEV_CONFIGURED 2
#define DEV_GONE 3

struct tendril_device {
    struct tendril_hc *hc;
    struct tendril_device *parent;
    struct tendril_port *port;
    struct tendril_list node;
    unsigned int refs;
    int state;
    bool gone;
    int speed;
    uint8_t address;
    /* Allocated before SET_ADDRESS; endpoint 0 keeps address 0 until it succeeds. */
    uint8_t new_address;
    uint8_t depth;
    uint8_t root_port;
    /* xHCI route string. */
    uint32_t route;
    /* The high-speed hub whose transaction translator serves this LS/FS device. */
    struct tendril_device *tt_hub;
    uint8_t tt_port;
    uint32_t quirks;
    /* Per the active configuration and, for hubs, GET_STATUS. */
    bool self_powered;
    struct tendril_device_descriptor desc;
    size_t num_configs;
    uint8_t **configs;
    size_t *config_sizes;
    uint8_t *bos;
    size_t bos_size;
    uint16_t langid;
    char *manufacturer;
    char *product;
    char *serial;
    int active_config;
    /* Configuration index 0 was chosen with others left to try, which msc.c checks for an auto-install CD-ROM. */
    bool cdrom_check;
    size_t num_interfaces;
    struct tendril_interface *interfaces;
    struct tendril_endpoint ep0;
    struct tendril_endpoint_descriptor ep0_desc;
    struct tendril_hub *hub;
    struct tendril_interface *probing;
    void *hcd;
    /* Outstanding transfers, for teardown. */
    unsigned int pending;
    bool teardown_wait;
    bool op_busy;
    bool announced;
    bool config_busy;
    /* While a configuration change runs, the only transfer the device takes is its own. */
    struct tendril_transfer *config_xfer;
    /* Power management: the link is suspended, or an operation is changing that. */
    bool suspended;
    bool pm_busy;
    /* Remote wakeup was armed, or USB 3 functions were suspended, for the current suspend. */
    bool pm_wake;
    bool pm_functions;
    /* The resume under way was signalled by the device. */
    bool pm_woke;
    /* Suspended by tendril_hc_suspend(), for tendril_hc_resume() to undo. */
    bool pm_system;
    /* Enumeration and configuration state machine. */
    struct tendril_async async;
    int step;
    size_t index;
    struct tendril_transfer xfer;
    uint8_t *buf;
    size_t buf_size;
    struct tendril_timer timer;
};

/* Host controllers */

struct tendril_hcd_ops {
    const char *name;
    int (*start)(struct tendril_hc *hc);
    /* Halts the controller and completes everything outstanding with an error. */
    void (*stop)(struct tendril_hc *hc);
    /* Frees the controller state, once every device is gone. */
    void (*destroy)(struct tendril_hc *hc);
    bool (*irq)(struct tendril_hc *hc);
    /* Called with the lock held to reap completions and port changes. */
    void (*poll)(struct tendril_hc *hc);
    uint32_t (*frame_number)(struct tendril_hc *hc);

    /* Root hub, called with the lock held, completing immediately. */
    void (*port_status)(struct tendril_hc *hc, size_t port, struct tendril_port_status *st);
    int (*port_set_feature)(struct tendril_hc *hc, size_t port, unsigned int feature);
    int (*port_clear_feature)(struct tendril_hc *hc, size_t port, unsigned int feature);

    /*
     * Device and endpoint management. Asynchronous operations complete with
     * tendril_async_complete() on the given async, with the lock held.
     */
    int (*device_init)(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async);
    /* Controllers that assign addresses themselves; NULL means SET_ADDRESS. */
    int (*device_address)(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async);
    /* Endpoint 0 packet size or address changed. */
    int (*ep0_update)(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async);
    /* Hub fields changed, after the hub descriptor was read. */
    int (*hub_update)(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async);
    /* Drop then add endpoints, together, as for a configuration or alternate setting change. */
    int (*configure)(
        struct tendril_hc *hc, struct tendril_device *dev, struct tendril_endpoint **drop, size_t num_drop,
        struct tendril_endpoint **add, size_t num_add, struct tendril_async *async
    );
    int (*endpoint_reset)(struct tendril_hc *hc, struct tendril_endpoint *ep, struct tendril_async *async);
    int (*alloc_streams)(
        struct tendril_hc *hc, struct tendril_endpoint **eps, size_t count, unsigned int streams,
        struct tendril_async *async
    );
    void (*device_free)(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async);
    /* Prepares an idle device's endpoints for its link suspending; NULL if nothing is needed. */
    int (*device_suspend)(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async);

    /* System sleep, with every port suspended or idle; resume() answers TENDRIL_ERR_IO if state was lost. */
    int (*suspend)(struct tendril_hc *hc);
    int (*resume)(struct tendril_hc *hc);

    /* Transfers, with the lock held. */
    int (*submit)(struct tendril_hc *hc, struct tendril_transfer *xfer);
    void (*cancel)(struct tendril_hc *hc, struct tendril_transfer *xfer);
};

/* submit() asks for the buffer to be bounced into contiguous DMA memory. */
#define TENDRIL_ERR_BOUNCE 0x100

struct tendril_hc {
    struct tendril_hc_config config;
    void *ctx;
    const struct tendril_hcd_ops *ops;
    void *hcd;
    uint32_t quirks;
    uint16_t pci_vendor;
    uint16_t pci_device;
    uint8_t pci_revision;
    /* Controller structures and data buffers must sit below 4 GiB, respectively. */
    bool dma32;
    bool data32;
    bool dead;
    bool removing;
    /* Stopped for system sleep by tendril_hc_suspend(). */
    bool suspended;
    struct tendril_hub root;
    struct tendril_list devices;
    uint32_t addresses[4];
    struct tendril_list work;
    struct tendril_list timers;
    struct tendril_list enum_queue;
    /* Holder of the enumeration slot: a port resetting, then its new device. */
    struct tendril_port *enum_port;
    struct tendril_device *enum_dev;
    struct tendril_dma_pool bounce_pool;
    /*
     * The frame number, counted on in 32 bits from the controller's own
     * counter, which wraps every 2048 frames or more; it survives restarts.
     */
    uint32_t frame;
};

static inline void hc_lock(struct tendril_hc *hc) {
    tendril_glue_lock(hc->ctx);
}

static inline void hc_unlock(struct tendril_hc *hc) {
    tendril_glue_unlock(hc->ctx);
}

/* Work and timers, with the lock held. */
void tendril_queue_work(struct tendril_hc *hc, struct tendril_work *work);
void tendril_work_init(struct tendril_work *work, void (*fn)(struct tendril_work *work));
void tendril_timer_init(struct tendril_timer *timer, void (*fn)(struct tendril_work *work));
void tendril_timer_arm(struct tendril_hc *hc, struct tendril_timer *timer, uint64_t delay_ns);
void tendril_timer_cancel(struct tendril_hc *hc, struct tendril_timer *timer);
bool tendril_timer_armed(struct tendril_timer *timer);

static inline void tendril_async_init(struct tendril_async *async, void (*fn)(struct tendril_work *work)) {
    tendril_work_init(&async->work, fn);
    async->status = TENDRIL_OK;
}

void tendril_async_complete(struct tendril_hc *hc, struct tendril_async *async, int status);

/* Root hub port change notification from controller drivers, lock held. */
void tendril_roothub_changed(struct tendril_hc *hc, size_t port);

/* Transfer completion from controller drivers, lock held. */
void tendril_transfer_complete(struct tendril_hc *hc, struct tendril_transfer *xfer, int status, size_t actual);
void tendril_transfer_fail_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep, int status);
void tendril_transfer_cancel_locked(struct tendril_hc *hc, struct tendril_transfer *xfer, int status);

/* Address allocation for controllers without address assignment. */
uint8_t tendril_address_alloc(struct tendril_hc *hc);
void tendril_address_free(struct tendril_hc *hc, uint8_t address);

/* Devices */

struct tendril_device *tendril_device_alloc(struct tendril_hc *hc, struct tendril_port *port, int speed);
void tendril_device_enumerate(struct tendril_device *dev);
void tendril_device_disconnect(struct tendril_device *dev);
void tendril_device_put(struct tendril_device *dev);
void tendril_endpoint_setup(
    struct tendril_endpoint *ep, struct tendril_device *dev, struct tendril_interface *intf,
    const struct tendril_endpoint_descriptor *desc, const uint8_t *end
);
void tendril_enum_done(struct tendril_port *port, struct tendril_device *dev, int status);
void tendril_enum_release_dev(struct tendril_device *dev);
bool tendril_device_is_ss(struct tendril_device *dev);
int tendril_interface_build_endpoints(
    struct tendril_interface *intf, size_t alt, struct tendril_endpoint **eps, size_t *count
);
void tendril_device_hub_ready(struct tendril_device *dev, int status);
void tendril_device_transfers_idle(struct tendril_device *dev);
/* The port of a suspended device finished a resume the device signalled. */
void tendril_device_woke(struct tendril_device *dev);

/* Asynchronous control transfer on endpoint 0 completing into `async`. */
int tendril_control_async(
    struct tendril_device *dev, struct tendril_transfer *xfer, uint8_t request_type, uint8_t request, uint16_t value,
    uint16_t index, void *data, uint16_t length, struct tendril_async *async
);

/*
 * The initialisation request of mass storage devices with FreeBSD's
 * UQ_MSC_SHUTTLE_INIT quirk, written clean-room in src/class/msc_shuttle.c:
 * a control transfer through `xfer` completing into `async`, using `buf`, of
 * `size` bytes, for its data.
 */
int tendril_msc_shuttle_init(
    struct tendril_device *dev, uint8_t interface_number, struct tendril_transfer *xfer, uint8_t *buf, size_t size,
    struct tendril_async *async
);

/* Descriptors */

const uint8_t *tendril_desc_next(const uint8_t *p, const uint8_t *end);
const uint8_t *tendril_desc_find(const uint8_t *p, const uint8_t *end, uint8_t type);
bool tendril_config_valid(const uint8_t *config, size_t size);

/* Drivers and quirks */

extern struct tendril_driver *tendril_drivers;

void tendril_bind_interfaces(struct tendril_device *dev);
void tendril_unbind_interfaces(struct tendril_device *dev);
void tendril_notify_added(struct tendril_device *dev);
void tendril_notify_removed(struct tendril_device *dev);
void tendril_notify_woke(struct tendril_device *dev);
/* Calls the drivers' suspend() hooks; false if one refused or is missing, with those already called resumed. */
bool tendril_suspend_interfaces(struct tendril_device *dev, int *status);
void tendril_resume_interfaces(struct tendril_device *dev);
/* A quirk table row; product 0xffff matches every product of the vendor. */
struct tendril_id_quirk {
    uint16_t vendor;
    uint16_t product;
    uint16_t bcd_min;
    uint16_t bcd_max;
    uint32_t quirks;
};

uint32_t tendril_id_quirks(
    const struct tendril_id_quirk *table, size_t count, const struct tendril_device_descriptor *desc
);
uint32_t tendril_device_quirk_lookup(const struct tendril_device_descriptor *desc);
uint32_t tendril_hc_quirk_lookup(struct tendril_hc *hc);

/* Controller drivers */

extern const struct tendril_hcd_ops tendril_uhci_ops;
extern const struct tendril_hcd_ops tendril_ohci_ops;
extern const struct tendril_hcd_ops tendril_ehci_ops;
extern const struct tendril_hcd_ops tendril_xhci_ops;

/*
 * Chipset fixes for PCI EHCI controllers, made through the configuration
 * space of the controller whose glue context is `ctx` after the BIOS handoff,
 * before the reset; written clean-room in src/hcd/ehci_chipset.c.
 */
void tendril_ehci_chipset_fixes(void *ctx);

/* Register access helpers. */

static inline uint32_t hc_read32(struct tendril_hc *hc, uint32_t off) {
    return tendril_glue_read32(hc->ctx, off);
}

static inline void hc_write32(struct tendril_hc *hc, uint32_t off, uint32_t value) {
    tendril_glue_write32(hc->ctx, off, value);
}

static inline uint16_t hc_read16(struct tendril_hc *hc, uint32_t off) {
    return tendril_glue_read16(hc->ctx, off);
}

static inline void hc_write16(struct tendril_hc *hc, uint32_t off, uint16_t value) {
    tendril_glue_write16(hc->ctx, off, value);
}

static inline void dma_barrier(void) {
    tendril_glue_dma_barrier();
}

/*
 * Brings hc->frame up to date from a reading of the controller's frame
 * counter, `mask` being its range; with the lock held, at least once a wrap.
 */
uint32_t tendril_frame_update(struct tendril_hc *hc, uint32_t raw, uint32_t mask);

/* Waits up to `timeout_us` for (read32(off) & mask) == value. */
bool tendril_hc_wait32(struct tendril_hc *hc, uint32_t off, uint32_t mask, uint32_t value, uint64_t timeout_us);
bool tendril_hc_wait16(struct tendril_hc *hc, uint32_t off, uint16_t mask, uint16_t value, uint64_t timeout_us);

/* The index of the highest set bit of a non-zero value. */
static inline uint32_t ilog2(uint32_t v) {
    uint32_t n = 0;
    while (v >>= 1) {
        n++;
    }
    return n;
}

/* The index of the lowest set bit of a non-zero value. */
static inline size_t lowest_bit(size_t v) {
    size_t n = 0;
    while (!(v & 1)) {
        v >>= 1;
        n++;
    }
    return n;
}

#endif
