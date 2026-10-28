#ifndef TENDRIL_TENDRIL_H
#define TENDRIL_TENDRIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tendril/glue.h>
#include <tendril/spec.h>

#ifdef __cplusplus
extern "C" {
#endif

enum tendril_status {
    TENDRIL_OK,
    TENDRIL_ERR_NO_MEMORY,
    TENDRIL_ERR_INVALID,
    TENDRIL_ERR_UNSUPPORTED,
    TENDRIL_ERR_BUSY,
    TENDRIL_ERR_TIMEOUT,
    TENDRIL_ERR_CANCELLED,
    TENDRIL_ERR_NO_DEVICE,
    TENDRIL_ERR_STALL,
    TENDRIL_ERR_IO,
    TENDRIL_ERR_BABBLE,
    TENDRIL_ERR_OVERRUN,
    TENDRIL_ERR_SHORT,
    TENDRIL_ERR_BANDWIDTH,
    TENDRIL_ERR_MISSED,
    TENDRIL_ERR_CONTROLLER,
    TENDRIL_ERR_PROTOCOL,
    TENDRIL_ERR_NO_MEDIUM,
    TENDRIL_ERR_MEDIUM_CHANGED,
    TENDRIL_ERR_WRITE_PROTECTED,
    TENDRIL_ERR_SUSPENDED
};

enum tendril_speed {
    TENDRIL_SPEED_LOW,
    TENDRIL_SPEED_FULL,
    TENDRIL_SPEED_HIGH,
    TENDRIL_SPEED_SUPER,
    TENDRIL_SPEED_SUPER_PLUS,
    TENDRIL_SPEED_SUPER_PLUS_X2
};

enum tendril_hc_type {
    TENDRIL_HC_UHCI,
    TENDRIL_HC_OHCI,
    TENDRIL_HC_EHCI,
    TENDRIL_HC_XHCI
};

const char *tendril_status_str(enum tendril_status status);
const char *tendril_speed_str(enum tendril_speed speed);
const char *tendril_hc_type_str(enum tendril_hc_type type);

/* Messages above `level` are dropped before formatting. */
void tendril_set_log_level(enum tendril_log_level level);

struct tendril_hc;
struct tendril_device;
struct tendril_interface;
struct tendril_endpoint;

/* Private bookkeeping embedded in public structures. */
struct tendril_list {
    struct tendril_list *prev;
    struct tendril_list *next;
};

struct tendril_work {
    struct tendril_list node;
    void (*fn)(struct tendril_work *work);
};

struct tendril_timer {
    struct tendril_list node;
    uint64_t deadline;
    struct tendril_work work;
};

struct tendril_dma_segment {
    uint64_t addr;
    size_t len;
};

/* Host controllers */

/*
 * Controller workarounds, normally applied from the built-in tables. Those
 * with a counterpart in FreeBSD's sys/dev/usb/controller/xhci.c and
 * xhci_pci.c (BSD-2-Clause) mean the same as there.
 */
#define TENDRIL_HC_QUIRK_NO_64BIT (1u << 0)
#define TENDRIL_HC_QUIRK_NO_HANDOFF (1u << 1)
/* Advisory for the embedder: MSI is broken on this controller. */
#define TENDRIL_HC_QUIRK_NO_MSI (1u << 2)
#define TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING (1u << 3)
#define TENDRIL_HC_QUIRK_NO_PORT_DISABLE (1u << 4)
#define TENDRIL_HC_QUIRK_NO_STREAMS (1u << 5)
#define TENDRIL_HC_QUIRK_XHCI_CTRL_STEP (1u << 6)

struct tendril_hc_config {
    enum tendril_hc_type type;
    /* Handed back to every glue function called for this controller. */
    void *ctx;
    /* The PCI configuration space glue functions work for this controller. */
    bool pci;
    /* The embedder routes the controller's interrupt to tendril_hc_irq(). */
    bool irq;
    /* TENDRIL_HC_QUIRK_* to apply on top of the built-in tables. */
    uint32_t quirks;
};

/* Maps a PCI class code (class << 16 | subclass << 8 | prog-if) to a type. */
bool tendril_pci_hc_type(uint32_t class_code, enum tendril_hc_type *type);

/* The built-in quirks of a PCI controller, for checks before tendril_hc_add(). */
uint32_t tendril_pci_hc_quirks(enum tendril_hc_type type, uint16_t vendor, uint16_t device);

/*
 * Takes the controller over from firmware, resets and starts it. Enumeration
 * then proceeds as tendril_hc_poll() is called.
 */
enum tendril_status tendril_hc_add(const struct tendril_hc_config *config, struct tendril_hc **hc);

/*
 * Halts and resets the controller, then detaches every device, whose
 * transfers fail with TENDRIL_ERR_NO_DEVICE, and frees the controller.
 */
void tendril_hc_remove(struct tendril_hc *hc);

/*
 * Interrupt handler entry point. Safe in any context, as it only acknowledges
 * and masks the interrupt; tendril_hc_poll() must be called afterwards.
 * Returns whether the controller was interrupting.
 */
bool tendril_hc_irq(struct tendril_hc *hc);

/*
 * Processes completions, port changes and timers, and invokes callbacks.
 * Returns the time, per tendril_glue_time_ns(), by which it must be called
 * again, or UINT64_MAX. Calls for one controller must not overlap, except
 * when nested from within callbacks.
 */
uint64_t tendril_hc_poll(struct tendril_hc *hc);

/* Calls tendril_hc_poll() until *flag becomes true. */
void tendril_hc_wait(struct tendril_hc *hc, volatile bool *flag);

/*
 * Prepares the controller for system sleep: suspends every device, arming
 * remote wakeup on those that support it if `wake`, then stops the
 * controller. On failure, the devices suspended so far are resumed. Calls
 * tendril_hc_poll() until done, so it must not be called from a callback.
 */
enum tendril_status tendril_hc_suspend(struct tendril_hc *hc, bool wake);

/*
 * Restarts the controller after system sleep and resumes its devices. A
 * controller that lost its state is reset instead, and its devices enumerated
 * again. Calls tendril_hc_poll() until done.
 */
enum tendril_status tendril_hc_resume(struct tendril_hc *hc);

enum tendril_hc_type tendril_hc_type(struct tendril_hc *hc);
uint32_t tendril_hc_quirks(struct tendril_hc *hc);
unsigned tendril_hc_num_ports(struct tendril_hc *hc);
struct tendril_device *tendril_hc_port_device(struct tendril_hc *hc, unsigned port);

/*
 * The current frame, counting 1 ms frames in 32 bits; isochronous start
 * frames count the same way. The count follows the controller's own counter,
 * which wraps every 2048 frames, so it falls behind by whole wraps if
 * tendril_hc_poll() goes uncalled for that long.
 */
uint32_t tendril_hc_frame_number(struct tendril_hc *hc);

/* Devices */

/*
 * Device workarounds, normally applied from the built-in table: FreeBSD's
 * UQ_NO_STRINGS, UQ_SWAP_UNICODE, and UQ_CFG_INDEX_* from its
 * sys/dev/usb/quirk/usb_quirk.h (BSD-2-Clause).
 */
#define TENDRIL_DEV_QUIRK_NO_STRINGS (1u << 0)
#define TENDRIL_DEV_QUIRK_SWAP_UNICODE (1u << 1)
#define TENDRIL_DEV_QUIRK_CONFIG_INDEX_0 (1u << 2)
#define TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 (1u << 3)
#define TENDRIL_DEV_QUIRK_CONFIG_INDEX_2 (1u << 4)
#define TENDRIL_DEV_QUIRK_CONFIG_INDEX_3 (1u << 5)
#define TENDRIL_DEV_QUIRK_CONFIG_INDEX_4 (1u << 6)

struct tendril_hc *tendril_device_hc(struct tendril_device *dev);
struct tendril_device *tendril_device_parent(struct tendril_device *dev);
/* Port on the parent hub, or on the root hub, counting from 1. */
unsigned tendril_device_port(struct tendril_device *dev);
/* Number of hubs between the device and the root hub. */
unsigned tendril_device_depth(struct tendril_device *dev);
enum tendril_speed tendril_device_speed(struct tendril_device *dev);
uint8_t tendril_device_address(struct tendril_device *dev);
uint32_t tendril_device_quirks(struct tendril_device *dev);
bool tendril_device_gone(struct tendril_device *dev);
const struct tendril_device_descriptor *tendril_device_descriptor(struct tendril_device *dev);
/* Configuration `index`, with all of its subordinate descriptors. */
const struct tendril_config_descriptor *tendril_device_config_descriptor(struct tendril_device *dev, unsigned index);
/* The active configuration, or NULL if unconfigured. */
const struct tendril_config_descriptor *tendril_device_active_config(struct tendril_device *dev);
const struct tendril_bos_descriptor *tendril_device_bos(struct tendril_device *dev);
/* UTF-8 strings read at enumeration, or NULL. */
const char *tendril_device_manufacturer(struct tendril_device *dev);
const char *tendril_device_product(struct tendril_device *dev);
const char *tendril_device_serial(struct tendril_device *dev);
struct tendril_device *tendril_device_child(struct tendril_device *hub, unsigned port);
unsigned tendril_device_num_ports(struct tendril_device *hub);
struct tendril_endpoint *tendril_device_ep0(struct tendril_device *dev);
unsigned tendril_device_num_interfaces(struct tendril_device *dev);
struct tendril_interface *tendril_device_interface(struct tendril_device *dev, unsigned index);
struct tendril_interface *tendril_device_find_interface(struct tendril_device *dev, uint8_t number);

/* Keeps the device structure valid past its removal. */
void tendril_device_ref(struct tendril_device *dev);
void tendril_device_unref(struct tendril_device *dev);

/*
 * Selects configuration `value`, or unconfigures the device for 0, rebinding
 * drivers. `done` is called once finished. A value no configuration has fails
 * with TENDRIL_ERR_INVALID, leaving the device as it was.
 */
enum tendril_status tendril_device_set_configuration(
    struct tendril_device *dev, uint8_t value,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Reads string descriptor `index` as UTF-8, in the device's first language. */
enum tendril_status tendril_device_read_string(
    struct tendril_device *dev, uint8_t index, char *buf, size_t size,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

/*
 * Selectively suspends a device, arming its remote wakeup if `remote_wake`
 * and the device supports it. Drivers stop their transfers first, through
 * their suspend() hooks, and a hub can only be suspended once all its
 * children are. `done` is called once finished. Transfers to a suspended
 * device fail with TENDRIL_ERR_SUSPENDED.
 */
enum tendril_status tendril_device_suspend(
    struct tendril_device *dev, bool remote_wake,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Resumes a suspended device, resuming its suspended hubs first. */
enum tendril_status tendril_device_resume(
    struct tendril_device *dev,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

bool tendril_device_suspended(struct tendril_device *dev);
/* Whether the active configuration supports remote wakeup. */
bool tendril_device_can_wake(struct tendril_device *dev);

/* Interfaces */

struct tendril_device *tendril_interface_device(struct tendril_interface *intf);
/* The descriptor of the current alternate setting. */
const struct tendril_interface_descriptor *tendril_interface_descriptor(struct tendril_interface *intf);
/*
 * The descriptors of the current alternate setting: its interface descriptor
 * and everything following up to the next interface descriptor.
 */
const uint8_t *tendril_interface_descriptors(struct tendril_interface *intf, size_t *size);
/* The interface association covering this interface, or NULL. */
const struct tendril_interface_assoc_descriptor *tendril_interface_association(struct tendril_interface *intf);
unsigned tendril_interface_num_alts(struct tendril_interface *intf);
const struct tendril_interface_descriptor *tendril_interface_alt_descriptor(struct tendril_interface *intf, unsigned index);
unsigned tendril_interface_num_endpoints(struct tendril_interface *intf);
struct tendril_endpoint *tendril_interface_endpoint(struct tendril_interface *intf, unsigned index);
/* The first endpoint of the current alternate setting of a type and direction. */
struct tendril_endpoint *tendril_interface_find_endpoint(struct tendril_interface *intf, uint8_t type, uint8_t dir);
void tendril_interface_set_data(struct tendril_interface *intf, void *data);
void *tendril_interface_data(struct tendril_interface *intf);

/*
 * Selects an alternate setting. Transfers on the interface's endpoints must
 * have completed first.
 */
enum tendril_status tendril_interface_set_alt(
    struct tendril_interface *intf, uint8_t alt,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Endpoints */

struct tendril_device *tendril_endpoint_device(struct tendril_endpoint *ep);
const struct tendril_endpoint_descriptor *tendril_endpoint_descriptor(struct tendril_endpoint *ep);
const struct tendril_ss_ep_companion_descriptor *tendril_endpoint_companion(struct tendril_endpoint *ep);
uint8_t tendril_endpoint_address(struct tendril_endpoint *ep);
uint8_t tendril_endpoint_type(struct tendril_endpoint *ep);
uint16_t tendril_endpoint_max_packet(struct tendril_endpoint *ep);
/* Streams the endpoint supports, 0 if none. */
unsigned tendril_endpoint_max_streams(struct tendril_endpoint *ep);

/* Clears a halt on the device and resets the endpoint's data toggle. */
enum tendril_status tendril_endpoint_clear_halt(
    struct tendril_endpoint *ep,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

/*
 * Gives bulk endpoints `streams` streams (IDs 1 to `streams`), shrunk to what
 * the endpoints and controller support, in which case `*streams` is updated.
 */
enum tendril_status tendril_endpoint_alloc_streams(
    struct tendril_endpoint **eps, unsigned count, unsigned *streams,
    void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Transfers */

/* A short packet ends the transfer with TENDRIL_ERR_SHORT. */
#define TENDRIL_XFER_SHORT_NOT_OK (1u << 0)
/* End an OUT transfer that is a multiple of the packet size with a 0-byte packet. */
#define TENDRIL_XFER_ZERO_PACKET (1u << 1)
/* `dma` is the bus address of `buffer`, contiguous and coherent. */
#define TENDRIL_XFER_DMA (1u << 2)
/* Start isochronous packets as soon as possible, after those queued on the endpoint, instead of at `start_frame`. */
#define TENDRIL_XFER_ISO_ASAP (1u << 3)

struct tendril_iso_packet {
    uint32_t length;
    uint32_t actual;
    enum tendril_status status;
};

struct tendril_transfer_internal {
    struct tendril_list node;
    struct tendril_work done;
    struct tendril_timer timeout;
    struct tendril_hc *hc;
    void *hcd;
    uintptr_t hcd_data[6];
    struct tendril_dma_segment *segments;
    size_t num_segments;
    struct tendril_dma_segment segment;
    void *bounce;
    uint64_t bounce_dma;
    size_t bounce_size;
    size_t mapped;
    enum tendril_status cancel_status;
    uint8_t state;
};

struct tendril_transfer {
    struct tendril_endpoint *endpoint;
    /* Control transfers only. */
    struct tendril_setup setup;
    void *buffer;
    size_t length;
    uint64_t dma;
    uint32_t flags;
    uint32_t timeout_ms;
    uint16_t stream;
    /* Isochronous packets, laid out back to back in `buffer`. */
    uint32_t num_iso_packets;
    struct tendril_iso_packet *iso_packets;
    /*
     * As tendril_hc_frame_number() counts. Submission sets it to the frame the
     * transfer starts in, which xHCI can only estimate with TENDRIL_XFER_ISO_ASAP.
     */
    uint32_t start_frame;
    void (*callback)(struct tendril_transfer *xfer);
    void *context;
    /* Results */
    enum tendril_status status;
    size_t actual;
    struct tendril_transfer_internal internal;
};

void tendril_transfer_init(struct tendril_transfer *xfer, struct tendril_endpoint *ep);
enum tendril_status tendril_transfer_submit(struct tendril_transfer *xfer);
/* Completes the transfer with TENDRIL_ERR_CANCELLED, asynchronously. */
void tendril_transfer_cancel(struct tendril_transfer *xfer);
/* Submits the transfer and calls tendril_hc_poll() until it completes. */
enum tendril_status tendril_transfer_run(struct tendril_transfer *xfer);
bool tendril_transfer_done(struct tendril_transfer *xfer);

/* Synchronous control transfer on endpoint 0, by tendril_transfer_run(). */
enum tendril_status tendril_control(
    struct tendril_device *dev, uint8_t request_type, uint8_t request,
    uint16_t value, uint16_t index, void *data, uint16_t length,
    size_t *actual, uint32_t timeout_ms);

/* Drivers */

struct tendril_driver {
    const char *name;
    /* Offered each unclaimed interface of a configured device; true claims it. */
    bool (*probe)(struct tendril_interface *intf);
    /* The claimed interface is going away; its transfers have all completed. */
    void (*disconnect)(struct tendril_interface *intf);
    /* A device finished configuration, after its interfaces were offered. */
    void (*device_added)(struct tendril_device *dev);
    /* A device is going away, after its interfaces were disconnected. */
    void (*device_removed)(struct tendril_device *dev);
    /*
     * The device is being suspended: the driver stops submitting on the
     * interface, after which the core cancels whatever is still outstanding
     * on its endpoints. False refuses, failing the suspend with
     * TENDRIL_ERR_BUSY. A device bound to a driver without this hook cannot be
     * suspended.
     */
    bool (*suspend)(struct tendril_interface *intf);
    /* The device resumed, or its suspend failed after suspend() was called. */
    void (*resume)(struct tendril_interface *intf);
    /* A device resumed because it signalled remote wakeup, after the resume() hooks ran. */
    void (*device_woke)(struct tendril_device *dev);
    struct tendril_driver *next;
};

/* Drivers must be registered before the first controller is added. */
void tendril_driver_register(struct tendril_driver *driver);
/* Claims an interface besides the one probed, such as a CDC data interface. */
bool tendril_interface_claim(struct tendril_interface *intf, struct tendril_driver *driver);

#ifdef __cplusplus
}
#endif

#endif
