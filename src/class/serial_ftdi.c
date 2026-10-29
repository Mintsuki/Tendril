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

/*
 * FTDI serial adapters: chip identification, the vendor control requests, and
 * the status bytes heading received data. Follows FTDI AN232B-04 Version 1.1
 * and FTDI AN_120 Version 1.2.
 */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/serial/uftdi.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (lennart@augustsson.net).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
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
#include "../../include/tendril/serial.h"
#include "../../include/tendril/spec.h"
#include "../../include/tendril/tendril.h"
#include "../internal.h"
#include "serial_internal.h"

/* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c */
#define FTDI_FT232A 0
#define FTDI_FT232B 1
#define FTDI_FT2232D 2
#define FTDI_FT232R 3
#define FTDI_FT2232H 4
#define FTDI_FT4232H 5
#define FTDI_FT232H 6
#define FTDI_FT230X 7
/* END FreeBSD sys/dev/usb/serial/uftdi.c */

/* Vendor requests and their fields (clean-room specification). */
#define FTDI_RESET 0x00
#define FTDI_MODEM_CTRL 0x01
#define FTDI_SET_FLOW_CTRL 0x02
#define FTDI_SET_BAUD_RATE 0x03
#define FTDI_SET_DATA 0x04
#define FTDI_SET_LATENCY_TIMER 0x09

#define FTDI_MODEM_DTR ((uint32_t)1 << 0)
#define FTDI_MODEM_RTS ((uint32_t)1 << 1)
#define FTDI_MODEM_SET_DTR ((uint32_t)1 << 8)
#define FTDI_MODEM_SET_RTS ((uint32_t)1 << 9)

#define FTDI_FLOW_RTS_CTS 0x01

#define FTDI_DATA_BREAK ((uint32_t)1 << 14)

#define FTDI_LATENCY_MS 2

/* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c */
/* Chips with several ports, which take the port index in wIndex. */
static bool multi_port(int chip) {
    switch (chip) {
        case FTDI_FT2232D:
        case FTDI_FT2232H:
        case FTDI_FT4232H:
        case FTDI_FT232H:
        case FTDI_FT230X: {
            return true;
        }
        default: {
            return false;
        }
    }
}

/* Chips with the 12 MHz baud rate reference (FTDI AN_120 Version 1.2, 4.4). */
static bool fast_clock(int chip) {
    switch (chip) {
        case FTDI_FT2232H:
        case FTDI_FT4232H:
        case FTDI_FT232H: {
            return true;
        }
        default: {
            return false;
        }
    }
}
/* END FreeBSD sys/dev/usb/serial/uftdi.c */

static uint16_t req_index(const struct tendril_serial *port, uint8_t high) {
    return (uint16_t)((uint32_t)high << 8 | (port->port_index & (uint32_t)0xff));
}

static void ftdi_req(struct serial_op *op, uint8_t request, uint16_t value, uint16_t index) {
    serial_op_add(op, REQ_VENDOR_OUT, request, value, index, NULL, 0);
}

bool serial_ftdi_init(struct tendril_serial *port) {
    const struct tendril_device_descriptor *desc = &port->dev->desc;
    uint16_t bcd = tendril_le16(desc->bcd_device);
    int chip;

    /* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c */
    /*
     * As FreeBSD's sys/dev/usb/serial/uftdi.c (BSD-2-Clause,
     * uftdi_devtype_setup) does.
     */
    switch (bcd) {
        case 0x0200: {
            if (desc->serial_string == 0) {
                chip = FTDI_FT232B;
            } else {
                chip = FTDI_FT232A;
            }
            break;
        }
        case 0x0400: {
            chip = FTDI_FT232B;
            break;
        }
        case 0x0500: {
            chip = FTDI_FT2232D;
            break;
        }
        case 0x0600: {
            chip = FTDI_FT232R;
            break;
        }
        case 0x0700: {
            chip = FTDI_FT2232H;
            break;
        }
        case 0x0800: {
            chip = FTDI_FT4232H;
            break;
        }
        case 0x0900: {
            chip = FTDI_FT232H;
            break;
        }
        case 0x1000: {
            chip = FTDI_FT230X;
            break;
        }
        default: {
            if (bcd < 0x0200) {
                LOG_WARN("serial: FTDI SIO parts are not supported");
                return false;
            }
            chip = FTDI_FT232R;
            break;
        }
    }
    /* END FreeBSD sys/dev/usb/serial/uftdi.c */

    port->chip = chip;
    if (multi_port(port->chip)) {
        /*
         * 1, 2, 3, ... select ports A, B, C, ... (clean-room specification).
         */
        port->port_index = (uint16_t)((uint32_t)1 + port->intf->number);
    } else {
        port->port_index = 0;
    }
    return true;
}

/* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c */
/*
 * The SET_BAUD_RATE divisor of a 3 MHz or 12 MHz reference (FTDI AN_120
 * Version 1.2, 3, 4.3, and 4.4), rounded and with the 12 MHz reference picked
 * as FreeBSD's sys/dev/usb/serial/uftdi.c (BSD-2-Clause,
 * uftdi_encode_baudrate) does.
 */
static bool encode_baud(int chip, uint32_t rate, uint32_t *encoded) {
    /* Fraction codes by eighths (FTDI AN_120 Version 1.2, 4.3). */
    static const uint8_t frac_code[8] = { 0, 3, 2, 4, 1, 5, 6, 7 };
    /*
     * Moves sixteenths to the nearest fraction the FT232A has: 0, 1/8, 1/4, or
     * 1/2.
     */
    static const int8_t roundoff[16] = { 0, 1, 0, 1, 0, -1, 2, 1, 0, -1, -2, -3, 4, 3, 2, 1 };
    bool fast = fast_clock(chip) && rate >= 1200;
    uint32_t clock = fast ? (uint32_t)12000000 : (uint32_t)3000000;

    if (rate < clock / 16384 || rate > clock) {
        return false;
    }

    uint32_t d16 = clock * 16 / rate;
    if ((d16 & 0xf) == 1) {
        d16 &= ~(uint32_t)7;
    } else if (chip == FTDI_FT232A) {
        d16 = (uint32_t)((int32_t)d16 + roundoff[d16 & 0xf]);
    } else {
        d16++;
    }

    uint32_t d8 = d16 / 2;
    uint32_t actual = clock * 8 / d8;
    if (actual < rate * 100 / 103 || actual > rate * 100 / 97) {
        return false;
    }

    uint32_t n = d8 / 8;
    uint32_t value;
    if (n == 1) {
        /*
         * Divisors 0 and 1 mean 3 and 2 MBaud, or 12 and 8 on 12 MHz (FTDI
         * AN_120 Version 1.2, 3 and 4.4).
         */
        value = (d8 % 8) == 0 ? (uint32_t)0 : (uint32_t)1;
    } else if (n > 16383) {
        return false;
    } else {
        value = n | (uint32_t)frac_code[d8 % 8] << 14;
    }
    if (fast) {
        value |= (uint32_t)1 << 17;
    }
    *encoded = value;
    return true;
}
/* END FreeBSD sys/dev/usb/serial/uftdi.c */

static bool data_value(const struct tendril_serial *port, uint16_t *value) {
    const struct tendril_serial_config *config = &port->config;

    if (config->data_bits < 5 || config->data_bits > 8
     || config->parity > TENDRIL_SERIAL_PARITY_SPACE || config->stop_bits > TENDRIL_SERIAL_STOP_2) {
        return false;
    }

    uint32_t v = (uint32_t)config->data_bits | (uint32_t)config->parity << 8 | (uint32_t)config->stop_bits << 11;
    if (port->in_break) {
        v |= FTDI_DATA_BREAK;
    }
    *value = (uint16_t)v;
    return true;
}

void serial_ftdi_setup(struct tendril_serial *port, struct serial_op *op) {
    ftdi_req(op, FTDI_RESET, 0, req_index(port, 0));
    /*
     * The chip holds partial packets back until its latency timer, 16 ms by
     * default, runs out (FTDI AN232B-04 Version 1.1, 3.1).
     */
    ftdi_req(op, FTDI_SET_LATENCY_TIMER, FTDI_LATENCY_MS, req_index(port, 0));
    serial_ftdi_line(port, op);
    serial_ftdi_lines(port, op);
}

void serial_ftdi_line(struct tendril_serial *port, struct serial_op *op) {
    uint32_t divisor;
    uint16_t data;

    if (!encode_baud(port->chip, port->config.baud, &divisor) || !data_value(port, &data)) {
        op->status = TENDRIL_ERR_INVALID;
        return;
    }

    /* Where the divisor's upper bits go (clean-room specification). */
    uint16_t baud_index;
    if (multi_port(port->chip)) {
        baud_index = req_index(port, (uint8_t)(divisor >> 16));
    } else {
        baud_index = (uint16_t)(divisor >> 16);
    }

    uint8_t flow = 0;
    if (port->config.rtscts) {
        flow = FTDI_FLOW_RTS_CTS;
    }

    ftdi_req(op, FTDI_SET_BAUD_RATE, (uint16_t)(divisor & (uint32_t)0xffff), baud_index);
    ftdi_req(op, FTDI_SET_DATA, data, req_index(port, 0));
    ftdi_req(op, FTDI_SET_FLOW_CTRL, 0, req_index(port, flow));
}

void serial_ftdi_lines(struct tendril_serial *port, struct serial_op *op) {
    uint32_t value = FTDI_MODEM_SET_DTR | FTDI_MODEM_SET_RTS;

    if (port->dtr) {
        value |= FTDI_MODEM_DTR;
    }
    if (port->rts) {
        value |= FTDI_MODEM_RTS;
    }
    ftdi_req(op, FTDI_MODEM_CTRL, (uint16_t)value, req_index(port, 0));
}

void serial_ftdi_break(struct tendril_serial *port, struct serial_op *op) {
    uint16_t data;

    if (!data_value(port, &data)) {
        op->status = TENDRIL_ERR_INVALID;
        return;
    }
    ftdi_req(op, FTDI_SET_DATA, data, req_index(port, 0));
}

/* The modem and line status bytes (clean-room specification). */
static uint32_t status_bits(uint8_t modem, uint8_t line) {
    uint32_t status = 0;

    if ((modem & 0x10) != 0) {
        status |= TENDRIL_SERIAL_CTS;
    }
    if ((modem & 0x20) != 0) {
        status |= TENDRIL_SERIAL_DSR;
    }
    if ((modem & 0x40) != 0) {
        status |= TENDRIL_SERIAL_RI;
    }
    if ((modem & 0x80) != 0) {
        status |= TENDRIL_SERIAL_DCD;
    }
    if ((line & 0x02) != 0) {
        status |= TENDRIL_SERIAL_OVERRUN;
    }
    if ((line & 0x04) != 0) {
        status |= TENDRIL_SERIAL_PARITY_ERROR;
    }
    if ((line & 0x08) != 0) {
        status |= TENDRIL_SERIAL_FRAMING_ERROR;
    }
    if ((line & 0x10) != 0) {
        status |= TENDRIL_SERIAL_BREAK;
    }
    return status;
}

/*
 * Every packet starts with two status bytes (FTDI AN232B-04 Version 1.1, 3.1).
 */
void serial_ftdi_receive(struct tendril_serial *port, const uint8_t *buf, size_t len) {
    size_t chunk = port->in_ep->max_packet;
    size_t off = 0;

    if (chunk == 0) {
        chunk = len;
    }

    while (off < len && !port->stopped) {
        size_t n = MIN(chunk, len - off);
        if (n < 2) {
            break;
        }
        serial_status_update(port, status_bits(buf[off], buf[off + 1]));
        if (n > 2 && !port->stopped) {
            serial_deliver(port, buf + off + 2, n - 2);
        }
        off += n;
    }
}
