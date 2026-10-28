/*
 * FTDI serial adapters: chip identification, the vendor control requests,
 * and the status bytes heading received data. Follows FTDI AN232B-04 Version
 * 1.1 and FTDI AN_120 Version 1.2.
 */

#include "serial_internal.h"

enum ftdi_chip {
    FTDI_FT232A,
    FTDI_FT232B,
    FTDI_FT2232D,
    FTDI_FT232R,
    FTDI_FT2232H,
    FTDI_FT4232H,
    FTDI_FT232H,
    FTDI_FT230X
};

/* Vendor requests and their fields (clean-room specification). */
#define FTDI_RESET 0x00
#define FTDI_MODEM_CTRL 0x01
#define FTDI_SET_FLOW_CTRL 0x02
#define FTDI_SET_BAUD_RATE 0x03
#define FTDI_SET_DATA 0x04
#define FTDI_SET_LATENCY_TIMER 0x09

#define FTDI_MODEM_DTR (1u << 0)
#define FTDI_MODEM_RTS (1u << 1)
#define FTDI_MODEM_SET_DTR (1u << 8)
#define FTDI_MODEM_SET_RTS (1u << 9)

#define FTDI_FLOW_RTS_CTS 0x01

#define FTDI_DATA_BREAK (1u << 14)

#define FTDI_LATENCY_MS 2

/* Chips with several ports, which take the port index in wIndex. */
static bool multi_port(unsigned chip) {
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
static bool fast_clock(unsigned chip) {
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

static uint16_t req_index(const struct tendril_serial *port, uint8_t high) {
    return (uint16_t)((unsigned)high << 8 | (port->port_index & 0xffu));
}

static void ftdi_req(struct serial_op *op, uint8_t request, uint16_t value, uint16_t index) {
    serial_op_add(op, REQ_VENDOR_OUT, request, value, index, NULL, 0);
}

bool serial_ftdi_init(struct tendril_serial *port) {
    const struct tendril_device_descriptor *desc = &port->dev->desc;
    uint16_t bcd = tendril_le16(desc->bcd_device);
    enum ftdi_chip chip;

    /* As FreeBSD's sys/dev/usb/serial/uftdi.c (BSD-2-Clause, uftdi_devtype_setup) does. */
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
                LOG_WARN("tendril: serial: FTDI SIO parts are not supported");
                return false;
            }
            chip = FTDI_FT232R;
            break;
        }
    }

    port->chip = (unsigned)chip;
    if (multi_port(port->chip)) {
        /* 1, 2, 3, ... select ports A, B, C, ... (clean-room specification). */
        port->port_index = (uint16_t)(1u + port->intf->number);
    } else {
        port->port_index = 0;
    }
    return true;
}

/*
 * The SET_BAUD_RATE divisor of a 3 MHz or 12 MHz reference (FTDI AN_120
 * Version 1.2, 3, 4.3, and 4.4), rounded and with the 12 MHz reference
 * picked as FreeBSD's sys/dev/usb/serial/uftdi.c (BSD-2-Clause,
 * uftdi_encode_baudrate) does.
 */
static bool encode_baud(unsigned chip, uint32_t rate, uint32_t *encoded) {
    /* Fraction codes by eighths (FTDI AN_120 Version 1.2, 4.3). */
    static const uint8_t frac_code[8] = { 0, 3, 2, 4, 1, 5, 6, 7 };
    /* Moves sixteenths to the nearest fraction the FT232A has: 0, 1/8, 1/4, or 1/2. */
    static const int8_t roundoff[16] = { 0, 1, 0, 1, 0, -1, 2, 1, 0, -1, -2, -3, 4, 3, 2, 1 };
    bool fast = fast_clock(chip) && rate >= 1200;
    uint32_t clock = fast ? 12000000u : 3000000u;

    if (rate < clock / 16384 || rate > clock) {
        return false;
    }

    uint32_t d16 = clock * 16 / rate;
    if ((d16 & 0xf) == 1) {
        d16 &= ~7u;
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
        /* Divisors 0 and 1 mean 3 and 2 MBaud, or 12 and 8 on 12 MHz (FTDI AN_120 Version 1.2, 3 and 4.4). */
        value = (d8 % 8) == 0 ? 0u : 1u;
    } else if (n > 16383) {
        return false;
    } else {
        value = n | (uint32_t)frac_code[d8 % 8] << 14;
    }
    if (fast) {
        value |= 1u << 17;
    }
    *encoded = value;
    return true;
}

static bool data_value(const struct tendril_serial *port, uint16_t *value) {
    const struct tendril_serial_config *config = &port->config;

    if (config->data_bits < 5 || config->data_bits > 8 ||
        config->parity > TENDRIL_SERIAL_PARITY_SPACE || config->stop_bits > TENDRIL_SERIAL_STOP_2) {
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

    ftdi_req(op, FTDI_SET_BAUD_RATE, (uint16_t)(divisor & 0xffffu), baud_index);
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

    if (modem & 0x10) {
        status |= TENDRIL_SERIAL_CTS;
    }
    if (modem & 0x20) {
        status |= TENDRIL_SERIAL_DSR;
    }
    if (modem & 0x40) {
        status |= TENDRIL_SERIAL_RI;
    }
    if (modem & 0x80) {
        status |= TENDRIL_SERIAL_DCD;
    }
    if (line & 0x02) {
        status |= TENDRIL_SERIAL_OVERRUN;
    }
    if (line & 0x04) {
        status |= TENDRIL_SERIAL_PARITY_ERROR;
    }
    if (line & 0x08) {
        status |= TENDRIL_SERIAL_FRAMING_ERROR;
    }
    if (line & 0x10) {
        status |= TENDRIL_SERIAL_BREAK;
    }
    return status;
}

/* Every packet starts with two status bytes (FTDI AN232B-04 Version 1.1, 3.1). */
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
