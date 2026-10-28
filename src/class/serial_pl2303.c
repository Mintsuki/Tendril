/*
 * Prolific PL2303 serial adapters. Prolific publishes no protocol
 * documentation: the chip types, the vendor requests, the rate divisor, and
 * the status byte come from the clean-room specification. Line coding, the
 * control lines, and break follow CDC PSTN 1.2.
 */

#include "serial_internal.h"

enum pl2303_chip {
    PL2303_ORIGINAL,
    PL2303_HX,
    PL2303_HXD,
    PL2303_HXN
};

#define PL2303_VENDOR 0x01
#define PL2303_HXN_WRITE 0x80

/* CDC PSTN 1.2, 6.3 */
#define PSTN_SET_LINE_CODING 0x20
#define PSTN_SET_CONTROL_LINE_STATE 0x22
#define PSTN_SEND_BREAK 0x23

#define PSTN_LINE_DTR (1u << 0)
#define PSTN_LINE_RTS (1u << 1)

#define DIVISOR_BASE 383385600u
#define DIVISOR_FORM 0x80000000u

/* Bring-up of the original PL2303, HX, and HXD; reads' answers are ignored. */
struct init_step {
    bool read;
    uint16_t value;
    uint16_t index;
};

static const struct init_step init_steps[] = {
    { true, 0x8484, 0 },
    { false, 0x0404, 0 },
    { true, 0x8484, 0 },
    { true, 0x8383, 0 },
    { true, 0x8484, 0 },
    { false, 0x0404, 1 },
    { true, 0x8484, 0 },
    { true, 0x8383, 0 },
    { false, 0, 1 },
    { false, 1, 0 }
};

/*
 * Rates written as they are, and which rates each chip takes, follow
 * FreeBSD's sys/dev/usb/serial/uplcom.c (BSD-2-Clause, uplcom_rates and
 * uplcom_pre_param).
 */
static const uint32_t listed_rates[] = {
    75, 150, 300, 600, 900, 1200, 1800, 2400, 3600, 4800, 7200, 9600, 14400, 19200, 28800, 38400,
    56000, 57600, 115200, 128000, 134400, 161280, 201600, 230400, 268800, 403200, 460800, 614400,
    806400, 921600, 1228800, 2457600, 3000000, 6000000, 12000000
};

static void put_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static void vendor_read(struct serial_op *op, uint16_t value, uint16_t index) {
    serial_op_add(op, REQ_VENDOR_IN, PL2303_VENDOR, value, index, NULL, 1);
}

static void vendor_write(struct serial_op *op, uint16_t value, uint16_t index) {
    serial_op_add(op, REQ_VENDOR_OUT, PL2303_VENDOR, value, index, NULL, 0);
}

static void hxn_write(struct serial_op *op, uint16_t value, uint16_t index) {
    serial_op_add(op, REQ_VENDOR_OUT, PL2303_HXN_WRITE, value, index, NULL, 0);
}

void serial_pl2303_init(struct tendril_serial *port) {
    const struct tendril_device_descriptor *desc = &port->dev->desc;
    uint16_t bcd = tendril_le16(desc->bcd_device);
    enum pl2303_chip chip;

    if (bcd == 0x0300) {
        chip = PL2303_HX;
    } else if (bcd == 0x0400 || bcd == 0x0500) {
        chip = PL2303_HXD;
    } else if (desc->device_class == TENDRIL_CLASS_CDC) {
        chip = PL2303_ORIGINAL;
    } else if (desc->max_packet_size0 == 64) {
        chip = PL2303_HX;
    } else {
        chip = PL2303_ORIGINAL;
    }
    port->chip = (unsigned)chip;
}

static void add_setup(struct tendril_serial *port, struct serial_op *op) {
    if (port->chip == PL2303_HXN) {
        /* Resets the data pipes. */
        hxn_write(op, 0x07, 0x03);
    } else {
        for (size_t i = 0; i < ARRAY_SIZE(init_steps); i++) {
            if (init_steps[i].read) {
                vendor_read(op, init_steps[i].value, init_steps[i].index);
            } else {
                vendor_write(op, init_steps[i].value, init_steps[i].index);
            }
        }
        if (port->chip == PL2303_ORIGINAL) {
            vendor_write(op, 2, 0x24);
        } else {
            vendor_write(op, 2, 0x44);
            vendor_write(op, 8, 0);
            vendor_write(op, 9, 0);
        }
    }
    serial_pl2303_line(port, op);
    serial_pl2303_lines(port, op);
}

static void probe_done(struct tendril_serial *port, struct serial_op *op, unsigned index, enum tendril_status status) {
    if (index != 0 || status == TENDRIL_ERR_NO_DEVICE) {
        return;
    }
    if (status != TENDRIL_OK) {
        port->chip = PL2303_HXN;
    }
    add_setup(port, op);
}

void serial_pl2303_setup(struct tendril_serial *port, struct serial_op *op) {
    if (port->chip == PL2303_HX) {
        /* An HXN, guessed as an HX, rejects the older request code (clean-room specification). */
        vendor_read(op, 0x8080, port->intf->number);
        op->after = probe_done;
        return;
    }
    add_setup(port, op);
}

static bool rate_listed(uint32_t rate) {
    for (size_t i = 0; i < ARRAY_SIZE(listed_rates); i++) {
        if (listed_rates[i] == rate) {
            return true;
        }
    }
    return false;
}

static bool encode_rate(unsigned chip, uint32_t rate, uint32_t *out) {
    uint32_t max;

    switch (chip) {
        case PL2303_ORIGINAL: {
            max = 1228800;
            break;
        }
        case PL2303_HX: {
            max = 6000000;
            break;
        }
        default: {
            max = 12000000;
            break;
        }
    }
    if (rate == 0 || rate > max) {
        return false;
    }
    if (rate_listed(rate)) {
        *out = rate;
        return true;
    }
    if (chip == PL2303_ORIGINAL) {
        return false;
    }

    /* rate = 383385600 / (m * 4^e), 9-bit m and 3-bit e (clean-room specification). */
    uint32_t m = DIVISOR_BASE / rate;
    uint32_t e = 0;
    if (m == 0) {
        m = 1;
    }
    while (m >= 512) {
        if (e < 7) {
            m /= 4;
            e++;
        } else {
            m = 511;
            break;
        }
    }
    *out = DIVISOR_FORM | (e << 1 | m >> 8) << 8 | (m & 0xffu);
    return true;
}

void serial_pl2303_line(struct tendril_serial *port, struct serial_op *op) {
    const struct tendril_serial_config *config = &port->config;
    uint32_t rate;
    uint8_t coding[7];

    if (config->data_bits < 5 || config->data_bits > 8 || config->parity > TENDRIL_SERIAL_PARITY_SPACE ||
        config->stop_bits > TENDRIL_SERIAL_STOP_2 || !encode_rate(port->chip, config->baud, &rate)) {
        op->status = TENDRIL_ERR_INVALID;
        return;
    }

    /* CDC PSTN 1.2, 6.3.10, Table 17 */
    put_le32(coding, rate);
    coding[4] = config->stop_bits;
    coding[5] = config->parity;
    coding[6] = config->data_bits;
    serial_op_add(op, REQ_CLASS_OUT, PSTN_SET_LINE_CODING, 0, port->intf->number, coding, (uint16_t)sizeof(coding));

    /* RTS/CTS flow control (clean-room specification). */
    if (port->chip == PL2303_HXN) {
        uint16_t flow = 0xff;
        if (config->rtscts) {
            flow = 0xfa;
        }
        hxn_write(op, 0x0a, flow);
    } else {
        uint16_t flow = 0;
        if (config->rtscts && port->chip == PL2303_ORIGINAL) {
            flow = 0x41;
        } else if (config->rtscts) {
            flow = 0x61;
        }
        vendor_write(op, 0, flow);
    }
}

/* SET_CONTROL_LINE_STATE (CDC PSTN 1.2, 6.3.12). */
void serial_pl2303_lines(struct tendril_serial *port, struct serial_op *op) {
    uint32_t value = 0;

    if (port->dtr) {
        value |= PSTN_LINE_DTR;
    }
    if (port->rts) {
        value |= PSTN_LINE_RTS;
    }
    serial_op_add(op, REQ_CLASS_OUT, PSTN_SET_CONTROL_LINE_STATE, (uint16_t)value, port->intf->number, NULL, 0);
}

/* SEND_BREAK, where 0xffff holds the break until one with 0 (CDC PSTN 1.2, 6.3.13). */
void serial_pl2303_break(struct tendril_serial *port, struct serial_op *op) {
    uint16_t duration = 0;

    if (port->in_break) {
        duration = 0xffff;
    }
    serial_op_add(op, REQ_CLASS_OUT, PSTN_SEND_BREAK, duration, port->intf->number, NULL, 0);
}

/* Byte 8 of an interrupt packet holds TENDRIL_SERIAL_* bits (clean-room specification). */
void serial_pl2303_status(struct tendril_serial *port, const uint8_t *buf, size_t len) {
    if (len <= 8) {
        return;
    }
    serial_status_update(port, (uint32_t)buf[8]);
}
