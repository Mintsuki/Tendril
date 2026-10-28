#ifndef TENDRIL_SERIAL_INTERNAL_H
#define TENDRIL_SERIAL_INTERNAL_H

#include "../internal.h"
#include <tendril/serial.h>

/* Shared by serial.c and the vendor protocol modules. */

#define RX_BUFS 2
#define MAX_REQS 24

#define REQ_CLASS_OUT (TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE)
#define REQ_VENDOR_OUT (TENDRIL_DIR_OUT | TENDRIL_TYPE_VENDOR | TENDRIL_RECIP_DEVICE)
#define REQ_VENDOR_IN (TENDRIL_DIR_IN | TENDRIL_TYPE_VENDOR | TENDRIL_RECIP_DEVICE)
#define REQ_VENDOR_IFACE_OUT (TENDRIL_DIR_OUT | TENDRIL_TYPE_VENDOR | TENDRIL_RECIP_INTERFACE)
#define REQ_VENDOR_IFACE_IN (TENDRIL_DIR_IN | TENDRIL_TYPE_VENDOR | TENDRIL_RECIP_INTERFACE)

struct serial_req {
    uint8_t type;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    uint16_t length;
    uint8_t data[16];
};

struct tendril_serial;

/* Control requests, run one at a time in the order added. */
struct serial_op {
    struct tendril_list node;
    struct serial_req reqs[MAX_REQS];
    unsigned count;
    unsigned next;
    /* Setting up a port carries on past failed requests; later operations stop at the first. */
    bool tolerant;
    enum tendril_status status;
    /* Called after each request, to read its answer or to add requests. */
    void (*after)(struct tendril_serial *port, struct serial_op *op, unsigned index, enum tendril_status status);
    void (*done)(void *ctx, enum tendril_status status);
    void *ctx;
};

struct serial_write {
    struct tendril_list node;
    const void *data;
    size_t len;
    void (*done)(void *ctx, enum tendril_status status);
    void *ctx;
};

struct tendril_serial {
    /* The interface with the data endpoints, and the one taking requests and reporting status. */
    struct tendril_interface *intf;
    struct tendril_interface *ctrl_intf;
    struct tendril_device *dev;
    struct tendril_hc *hc;
    enum tendril_serial_kind kind;
    /* Owned by the vendor module: the chip model, and the wIndex value naming the port. */
    unsigned chip;
    uint16_t port_index;
    uint8_t ch_version;
    struct tendril_endpoint *in_ep;
    struct tendril_endpoint *out_ep;
    struct tendril_endpoint *intr_ep;
    bool stopped;
    bool attached;
    /* Requests and writes wait while suspended; nothing is received. */
    bool suspended;
    uint32_t status;
    struct tendril_serial_config config;
    bool dtr;
    bool rts;
    bool in_break;
    void *user;
    struct tendril_transfer rx[RX_BUFS];
    uint8_t *rx_buf[RX_BUFS];
    size_t rx_size;
    unsigned rx_idle;
    unsigned rx_errors;
    struct tendril_transfer intr;
    uint8_t intr_buf[64];
    bool intr_idle;
    unsigned intr_errors;
    struct tendril_timer timer;
    bool poll_queued;
    struct tendril_list ops;
    struct serial_op *op;
    bool in_kick;
    struct tendril_transfer ctrl;
    struct tendril_list writes;
    struct serial_write *writing;
    struct tendril_transfer tx;
};

/*
 * Appends a request; IN requests read up to 16 bytes into its data. Setting
 * op->status instead fails the operation before anything is sent.
 */
void serial_op_add(struct serial_op *op, uint8_t type, uint8_t request, uint16_t value, uint16_t index,
                   const void *data, uint16_t length);
/* Reports line states and events (TENDRIL_SERIAL_*). */
void serial_status_update(struct tendril_serial *port, uint32_t status);
/* Hands received data to the embedder. */
void serial_deliver(struct tendril_serial *port, const uint8_t *data, size_t len);

/* FTDI (serial_ftdi.c) */

/* Works out the chip and port->port_index; false leaves the port unsupported. */
bool serial_ftdi_init(struct tendril_serial *port);
/* Brings the port up with port->config, port->dtr, and port->rts. */
void serial_ftdi_setup(struct tendril_serial *port, struct serial_op *op);
/* Applies port->config. */
void serial_ftdi_line(struct tendril_serial *port, struct serial_op *op);
/* Applies port->dtr and port->rts. */
void serial_ftdi_lines(struct tendril_serial *port, struct serial_op *op);
/* Applies port->in_break. */
void serial_ftdi_break(struct tendril_serial *port, struct serial_op *op);
/* Data from the bulk IN endpoint. */
void serial_ftdi_receive(struct tendril_serial *port, const uint8_t *buf, size_t len);

/* Prolific PL2303 (serial_pl2303.c) */

/* Makes a first guess at the chip. */
void serial_pl2303_init(struct tendril_serial *port);
void serial_pl2303_setup(struct tendril_serial *port, struct serial_op *op);
void serial_pl2303_line(struct tendril_serial *port, struct serial_op *op);
void serial_pl2303_lines(struct tendril_serial *port, struct serial_op *op);
void serial_pl2303_break(struct tendril_serial *port, struct serial_op *op);
/* Data from the interrupt IN endpoint. */
void serial_pl2303_status(struct tendril_serial *port, const uint8_t *buf, size_t len);

#endif
