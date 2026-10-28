#ifndef TENDRIL_SERIAL_H
#define TENDRIL_SERIAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tendril/tendril.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Serial ports: CDC ACM, and FTDI, Prolific PL2303, WCH CH34x, and Silicon Labs CP210x adapters. */

struct tendril_serial;

enum tendril_serial_kind {
    TENDRIL_SERIAL_ACM,
    TENDRIL_SERIAL_FTDI,
    TENDRIL_SERIAL_PL2303,
    TENDRIL_SERIAL_CH34X,
    TENDRIL_SERIAL_CP210X
};

/*
 * Status bits. DCD, DSR, RI, and CTS are line states; the rest are events,
 * reported once each. Not every adapter reports every bit.
 */
#define TENDRIL_SERIAL_DCD (1u << 0)
#define TENDRIL_SERIAL_DSR (1u << 1)
#define TENDRIL_SERIAL_BREAK (1u << 2)
#define TENDRIL_SERIAL_RI (1u << 3)
#define TENDRIL_SERIAL_FRAMING_ERROR (1u << 4)
#define TENDRIL_SERIAL_PARITY_ERROR (1u << 5)
#define TENDRIL_SERIAL_OVERRUN (1u << 6)
#define TENDRIL_SERIAL_CTS (1u << 7)

struct tendril_serial_callbacks {
    /* A port is ready: 115200 baud, 8 data bits, no parity, 1 stop bit, DTR and RTS on. */
    void (*attach)(struct tendril_serial *port);
    /* The port is gone; its writes and operations have all completed. */
    void (*detach)(struct tendril_serial *port);
    /* Received data, valid only during the call. */
    void (*receive)(struct tendril_serial *port, const uint8_t *data, size_t len);
    void (*status)(struct tendril_serial *port, uint32_t status);
};

/* Registers the serial class driver; call before the first controller is added. */
void tendril_serial_register(const struct tendril_serial_callbacks *callbacks);

#define TENDRIL_SERIAL_PARITY_NONE 0
#define TENDRIL_SERIAL_PARITY_ODD 1
#define TENDRIL_SERIAL_PARITY_EVEN 2
#define TENDRIL_SERIAL_PARITY_MARK 3
#define TENDRIL_SERIAL_PARITY_SPACE 4

#define TENDRIL_SERIAL_STOP_1 0
#define TENDRIL_SERIAL_STOP_1_5 1
#define TENDRIL_SERIAL_STOP_2 2

struct tendril_serial_config {
    uint32_t baud;
    /* 5 to 8 */
    uint8_t data_bits;
    uint8_t parity;
    uint8_t stop_bits;
    /* RTS/CTS flow control, where the adapter can do it. */
    bool rtscts;
};

/*
 * Line settings, modem control lines, and break. Each completes through `done`;
 * those still queued fail with TENDRIL_ERR_NO_DEVICE if the port goes away.
 */
enum tendril_status tendril_serial_configure(struct tendril_serial *port, const struct tendril_serial_config *config,
                                           void (*done)(void *ctx, enum tendril_status status), void *ctx);
enum tendril_status tendril_serial_set_lines(struct tendril_serial *port, bool dtr, bool rts,
                                           void (*done)(void *ctx, enum tendril_status status), void *ctx);
enum tendril_status tendril_serial_set_break(struct tendril_serial *port, bool on,
                                           void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Queues `len` bytes, which must stay valid until `done`; writes go out in order. */
enum tendril_status tendril_serial_write(struct tendril_serial *port, const void *data, size_t len,
                                       void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Synchronous helpers, running tendril_hc_poll() until done. */
enum tendril_status tendril_serial_configure_sync(struct tendril_serial *port, const struct tendril_serial_config *config);
enum tendril_status tendril_serial_write_sync(struct tendril_serial *port, const void *data, size_t len);

struct tendril_device *tendril_serial_device(struct tendril_serial *port);
enum tendril_serial_kind tendril_serial_kind(struct tendril_serial *port);
/* The number of the interface carrying the port's data. */
uint8_t tendril_serial_interface(struct tendril_serial *port);
uint32_t tendril_serial_status(struct tendril_serial *port);
void tendril_serial_set_user(struct tendril_serial *port, void *user);
void *tendril_serial_user(struct tendril_serial *port);

#ifdef __cplusplus
}
#endif

#endif
