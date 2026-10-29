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

#define TENDRIL_SERIAL_ACM 0
#define TENDRIL_SERIAL_FTDI 1
#define TENDRIL_SERIAL_PL2303 2
#define TENDRIL_SERIAL_CH34X 3
#define TENDRIL_SERIAL_CP210X 4

/*
 * Status bits. DCD, DSR, RI, and CTS are line states; the rest are events,
 * reported once each. Not every adapter reports every bit.
 */
#define TENDRIL_SERIAL_DCD ((uint32_t)1 << 0)
#define TENDRIL_SERIAL_DSR ((uint32_t)1 << 1)
#define TENDRIL_SERIAL_BREAK ((uint32_t)1 << 2)
#define TENDRIL_SERIAL_RI ((uint32_t)1 << 3)
#define TENDRIL_SERIAL_FRAMING_ERROR ((uint32_t)1 << 4)
#define TENDRIL_SERIAL_PARITY_ERROR ((uint32_t)1 << 5)
#define TENDRIL_SERIAL_OVERRUN ((uint32_t)1 << 6)
#define TENDRIL_SERIAL_CTS ((uint32_t)1 << 7)

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
int tendril_serial_configure(
    struct tendril_serial *port, const struct tendril_serial_config *config, void (*done)(void *ctx, int status),
    void *ctx
);
int tendril_serial_set_lines(
    struct tendril_serial *port, bool dtr, bool rts, void (*done)(void *ctx, int status), void *ctx
);
int tendril_serial_set_break(struct tendril_serial *port, bool on, void (*done)(void *ctx, int status), void *ctx);

/* Queues `len` bytes, which must stay valid until `done`; writes go out in order. */
int tendril_serial_write(
    struct tendril_serial *port, const void *data, size_t len, void (*done)(void *ctx, int status), void *ctx
);

/* Synchronous helpers, running tendril_hc_poll() until done. */
int tendril_serial_configure_sync(struct tendril_serial *port, const struct tendril_serial_config *config);
int tendril_serial_write_sync(struct tendril_serial *port, const void *data, size_t len);

struct tendril_device *tendril_serial_device(struct tendril_serial *port);
int tendril_serial_kind(struct tendril_serial *port);
/* The number of the interface carrying the port's data. */
uint8_t tendril_serial_interface(struct tendril_serial *port);
uint32_t tendril_serial_status(struct tendril_serial *port);
void tendril_serial_set_user(struct tendril_serial *port, void *user);
void *tendril_serial_user(struct tendril_serial *port);

#ifdef __cplusplus
}
#endif

#endif
