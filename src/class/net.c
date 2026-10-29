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
 * Ethernet adapters: the CDC Ethernet Control Model (CDC ECM 1.2) and Network
 * Control Model (CDC NCM 1.0 Errata 1), with the notifications of CDC 1.2, and
 * RNDIS ([MS-RNDIS] v20130118).
 */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/net/if_urndis.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * Copyright (c) 2010 Jonathan Armani <armani@openbsd.org>
 * Copyright (c) 2010 Fabien Romano <fabien@openbsd.org>
 * Copyright (c) 2010 Michael Knudsen <mk@openbsd.org>
 * Copyright (c) 2014 Hans Petter Selasky <hselasky@freebsd.org>
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Parts of this file are derived from OpenBSD's sys/dev/usb/if_urndis.c and
 * sys/dev/rndis.h (commit fba0a8da167d48e1d94a300389b91b0a48708155):
 *
 * Copyright (c) 2010 Jonathan Armani <armani@openbsd.org>
 * Copyright (c) 2010 Fabien Romano <fabien@openbsd.org>
 * Copyright (c) 2010 Michael Knudsen <mk@openbsd.org>
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usbdevs (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * Copyright (c) 1998-2004 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (lennart@augustsson.net) at
 * Carlstedt Research & Technology.
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

/*
 * Parts of this file are derived from OpenBSD's sys/dev/usb/if_uncm.c (commit
 * fba0a8da167d48e1d94a300389b91b0a48708155):
 *
 * Copyright (c) 2016 genua mbH
 * Copyright (c) 2026 Jan Schreiber
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tendril/net.h>
#include "../internal.h"
#include <tendril/spec.h>
#include <tendril/tendril.h>

#define CTRL_TIMEOUT_MS 5000
#define ECM_RX_XFERS 8
#define ECM_TX_XFERS 8
#define NCM_RX_XFERS 2
#define NCM_TX_XFERS 2
#define MAX_XFERS 8
#define RETRY_DELAY_MS 20
#define MAX_ERRORS 50

#define ETH_HEADER_LEN 14
#define ETH_CRC_LEN 4
#define DEFAULT_MAX_FRAME 1514
#define MAX_FRAME_LIMIT 16384

#define MAC_LEN 6
#define MAC_STRING_SIZE 32

#define INTR_MAX_PACKET 64
#define NOTIFY_HEADER_LEN 8
#define NOTIFY_BUF_SIZE 128

/* CDC 1.2, 4.3 and 5.2.3 */
#define CDC_SUBCLASS_MDLM 0x0a
#define CDC_SUBCLASS_ECM 0x06
#define CDC_SUBCLASS_NCM 0x0d
#define CDC_DESC_CALL_MANAGEMENT 0x01
#define CDC_DESC_UNION 0x06
#define CDC_DESC_ETHERNET 0x0f
#define CDC_DESC_NCM 0x1a

#define VENDOR_HUAWEI 0x12d1

#define REQ_OUT \
    (TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE)
#define REQ_IN (TENDRIL_DIR_IN | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE)

/* CDC ECM 1.2, 6.2, and CDC NCM 1.0 Errata 1, 6.2 */
#define SET_ETHERNET_MULTICAST_FILTERS 0x40
#define SET_ETHERNET_PACKET_FILTER 0x43
#define GET_NTB_PARAMETERS 0x80
#define SET_NTB_FORMAT 0x84
#define SET_NTB_INPUT_SIZE 0x86
#define GET_MAX_DATAGRAM_SIZE 0x87

/* CDC 1.2, 6.3 */
#define NOTIFY_NETWORK_CONNECTION 0x00
#define NOTIFY_SPEED_CHANGE 0x2a

/* bmNetworkCapabilities (CDC NCM 1.0 Errata 1, 5.2.1, Table 5-2) */
#define NCM_CAP_PACKET_FILTER ((uint32_t)1 << 0)
#define NCM_CAP_MAX_DATAGRAM ((uint32_t)1 << 3)

/* CDC NCM 1.0 Errata 1, 3 and 6.2.1 */
#define NTB_PARAMS_LEN 28
#define NTB_FORMAT_32 ((uint32_t)1 << 1)
#define NTB_SIZE_LIMIT 16384
#define NTB16_MAX_SIZE 65535
#define NTB_MIN_SIZE 64
#define NTB_MIN_FRAME_ROOM 60
#define NTB_MAX_DATAGRAMS 32
#define NDP_MIN_ALIGN 4
#define NDP_MIN_DIVISOR 4
#define NTH16_SIGNATURE ((uint32_t)0x484d434e)
#define NTH16_LEN 12
#define NDP16_SIGNATURE ((uint32_t)0x304d434e)
#define NDP16_SIGNATURE_CRC ((uint32_t)0x314d434e)
#define NDP16_HEADER_LEN 8
#define NDP16_MIN_LEN 16
#define NDP16_ENTRY_LEN 4

/* CDC 1.2, 6.2.1 and 6.2.2 */
#define SEND_ENCAPSULATED_COMMAND 0x00
#define GET_ENCAPSULATED_RESPONSE 0x01

/* [MS-RNDIS] v20130118, 2.2 */
#define RNDIS_PACKET_MSG ((uint32_t)0x00000001)
#define RNDIS_INITIALIZE_MSG ((uint32_t)0x00000002)
#define RNDIS_HALT_MSG ((uint32_t)0x00000003)
#define RNDIS_QUERY_MSG ((uint32_t)0x00000004)
#define RNDIS_SET_MSG ((uint32_t)0x00000005)
#define RNDIS_RESET_MSG ((uint32_t)0x00000006)
#define RNDIS_INDICATE_STATUS_MSG ((uint32_t)0x00000007)
#define RNDIS_KEEPALIVE_MSG ((uint32_t)0x00000008)
#define RNDIS_CMPLT ((uint32_t)0x80000000)
#define RNDIS_STATUS_SUCCESS ((uint32_t)0x00000000)
#define RNDIS_STATUS_NOT_SUPPORTED ((uint32_t)0xc00000bb)
#define RNDIS_STATUS_MEDIA_CONNECT ((uint32_t)0x4001000b)
#define RNDIS_STATUS_MEDIA_DISCONNECT ((uint32_t)0x4001000c)
#define RNDIS_INITIALIZE_LEN 24
#define RNDIS_QUERY_LEN 28
#define RNDIS_SET_LEN 28
#define RNDIS_KEEPALIVE_CMPLT_LEN 16
#define RNDIS_KEEPALIVE_LEN 12
#define RNDIS_RESET_LEN 12
/* Windows' KeepAliveTimer (3.1.2, footnote <16>). */
#define RNDIS_KEEPALIVE_MS 5000
#define RNDIS_PACKET_HEADER_LEN 44
/* What Windows asks for in REMOTE_NDIS_INITIALIZE_MSG (footnote <3>). */
#define RNDIS_MAX_TRANSFER 16384
/* BEGIN FreeBSD sys/dev/usb/net/if_urndis.c */
/*
 * The largest frame FreeBSD's sys/dev/usb/net/if_urndis.c (ISC license)
 * accepts: MCLBYTES - ETHER_ALIGN.
 */
#define RNDIS_MAX_FRAME 2046
/* END FreeBSD sys/dev/usb/net/if_urndis.c */
#define RNDIS_MIN_FRAME 64
#define RNDIS_MIN_TRANSFER (RNDIS_PACKET_HEADER_LEN + 60)
#define RNDIS_MAX_MC 64
#define RNDIS_RX_XFERS 8
#define RNDIS_TX_XFERS 8
/* Windows' ControlTimeoutTimer (3.1.2, footnote <17>). */
#define RNDIS_TIMEOUT_MS 8000
#define RNDIS_REREAD_MS 10
/* BEGIN OpenBSD sys/dev/usb/if_urndis.c, FreeBSD sys/dev/usb/net/if_urndis.c */
/* The response buffer of OpenBSD's and FreeBSD's if_urndis.c (ISC license). */
#define RNDIS_RESPONSE_SIZE 1024
/* END OpenBSD sys/dev/usb/if_urndis.c, FreeBSD sys/dev/usb/net/if_urndis.c */

/* BEGIN OpenBSD sys/dev/rndis.h */
/* NDIS values, from OpenBSD's sys/dev/rndis.h (ISC license) */
#define OID_GEN_MAXIMUM_FRAME_SIZE ((uint32_t)0x00010106)
#define OID_GEN_LINK_SPEED ((uint32_t)0x00010107)
#define OID_GEN_CURRENT_PACKET_FILTER ((uint32_t)0x0001010e)
#define OID_GEN_MEDIA_CONNECT_STATUS ((uint32_t)0x00010114)
#define OID_802_3_PERMANENT_ADDRESS ((uint32_t)0x01010101)
#define OID_802_3_CURRENT_ADDRESS ((uint32_t)0x01010102)
#define OID_802_3_MULTICAST_LIST ((uint32_t)0x01010103)
#define OID_802_3_MAXIMUM_LIST_SIZE ((uint32_t)0x01010104)
#define RNDIS_MEDIUM_802_3 ((uint32_t)0x00000000)
#define NDIS_PACKET_TYPE_DIRECTED ((uint32_t)0x01)
#define NDIS_PACKET_TYPE_MULTICAST ((uint32_t)0x02)
#define NDIS_PACKET_TYPE_ALL_MULTICAST ((uint32_t)0x04)
#define NDIS_PACKET_TYPE_BROADCAST ((uint32_t)0x08)
#define NDIS_PACKET_TYPE_PROMISCUOUS ((uint32_t)0x20)
/* END OpenBSD sys/dev/rndis.h */

/* Device quirks (clean-room specification). */
#define QUIRK_NO_UNION ((uint32_t)1 << 0)
#define QUIRK_CRC32 ((uint32_t)1 << 1)

/* BEGIN FreeBSD sys/dev/usb/usbdevs */
/* Names as FreeBSD's sys/dev/usb/usbdevs (BSD-2-Clause) gives the IDs. */
static const struct tendril_id_quirk net_quirks[] = {
    { 0x0402, 0x5632, 0x0000, 0xffff, QUIRK_NO_UNION }, /* ACERLABS M5632 */
    { 0x0bb2, 0x6098, 0x0000, 0xffff, QUIRK_NO_UNION }, /* AMBIT NTL_250 */
    { 0x049f, 0x505a, 0x0000, 0xffff, QUIRK_NO_UNION }, /* COMPAQ IPAQLINUX */
    { 0x0e7e, 0x1001, 0x0000, 0xffff, QUIRK_NO_UNION }, /* GMATE YP3X00 */
    /* MOTOROLA2 USBLAN */
    { 0x22b8, 0x600c, 0x0000, 0xffff, QUIRK_CRC32 | QUIRK_NO_UNION },
    /* MOTOROLA2 USBLAN2 */
    { 0x22b8, 0x6027, 0x0000, 0xffff, QUIRK_CRC32 | QUIRK_NO_UNION },
    { 0x067b, 0x2501, 0x0000, 0xffff, QUIRK_NO_UNION }, /* PROLIFIC PL2501 */
    { 0x04dd, 0x8004, 0x0000, 0xffff, QUIRK_CRC32 }, /* SHARP SL5500 */
    /* SHARP SLA300 */
    { 0x04dd, 0x8005, 0x0000, 0xffff, QUIRK_CRC32 | QUIRK_NO_UNION },
    /* SHARP SL5600 */
    { 0x04dd, 0x8006, 0x0000, 0xffff, QUIRK_CRC32 | QUIRK_NO_UNION },
    /* SHARP SLC700 */
    { 0x04dd, 0x8007, 0x0000, 0xffff, QUIRK_CRC32 | QUIRK_NO_UNION },
    /* SHARP SLC750 */
    { 0x04dd, 0x9031, 0x0000, 0xffff, QUIRK_CRC32 | QUIRK_NO_UNION }
};
/* END FreeBSD sys/dev/usb/usbdevs */

#define STEP_MAC 0
#define STEP_NTB_PARAMS 1
#define STEP_NTB_FORMAT 2
#define STEP_NTB_INPUT_SIZE 3
#define STEP_MAX_DATAGRAM 4
#define STEP_DATA_ALT 5
#define STEP_FILTER 6
#define STEP_DONE 7

/* A frame queued by tendril_net_send(). */
struct net_frame {
    struct tendril_list node;
    const uint8_t *data;
    size_t len;
    void (*done)(void *ctx, int status);
    void *ctx;
    /* What `done` reports, once the frame is finished. */
    int status;
};

/*
 * A queued control request: a class request of the API, or an RNDIS message.
 */
struct net_op {
    struct tendril_list node;
    uint8_t request;
    uint16_t value;
    uint8_t *data;
    size_t length;
    void (*done)(void *ctx, int status);
    void *ctx;
    /*
     * RNDIS: `data` holds the message, answered by its completion unless
     * no_reply.
     */
    uint32_t rndis_type;
    uint32_t rndis_id;
    bool no_reply;
    /* Reads the completion, valid only during the call, before `done` runs. */
    void (*answer)(
        struct tendril_net *net, int status, const uint8_t *msg, size_t len
    );
};

struct net_rx {
    struct tendril_net *net;
    struct tendril_transfer xfer;
    uint8_t *buf;
    bool busy;
    /* Failed, and resubmitted by the retry timer. */
    bool waiting;
};

struct net_tx {
    struct tendril_net *net;
    struct tendril_transfer xfer;
    /* The NTB (NCM) or the frame and its CRC (ECM with CRC32), else NULL. */
    uint8_t *buf;
    /* The frames the transfer carries, in queue order. */
    struct tendril_list frames;
    bool busy;
};

struct tendril_net {
    struct tendril_device *dev;
    struct tendril_hc *hc;
    struct tendril_interface *ctrl_intf;
    struct tendril_interface *data_intf;
    int kind;
    /* Matched by the Huawei rule: carries on as ECM without NTB parameters. */
    bool vendor_ncm;
    bool crc32;
    size_t data_alt_index;
    uint8_t data_alt;
    struct tendril_endpoint *in_ep;
    struct tendril_endpoint *out_ep;
    struct tendril_endpoint *intr_ep;
    void *user;

    /* From the functional descriptors. */
    uint8_t mac_string;
    size_t mc_filters;
    uint8_t ncm_caps;

    uint8_t mac[MAC_LEN];
    size_t max_frame;

    /* NCM parameters, from GET_NTB_PARAMETERS. */
    uint16_t ntb_formats;
    size_t ntb_in_max;
    size_t ntb_out_max;
    size_t ntb_in_size;
    size_t ntb_out_size;
    size_t ntb_frame_cap;
    size_t ndp_out_divisor;
    size_t ndp_out_remainder;
    size_t ndp_out_align;
    size_t ntb_out_datagrams;
    uint16_t ntb_sequence;

    /* Setup, and the control transfer it shares with the operations. */
    int step;
    bool stopped;
    bool ready;
    bool attached;
    /* Between the suspend() and resume() hooks: nothing is submitted. */
    bool suspended;
    struct tendril_transfer ctrl;
    uint8_t ctrl_buf[NTB_PARAMS_LEN];
    char mac_str[MAC_STRING_SIZE];
    struct tendril_list ops;
    struct net_op *op;

    size_t num_rx;
    struct net_rx rx[MAX_XFERS];
    size_t rx_buf_size;
    size_t rx_errors;
    bool rx_gave_up;
    bool rx_clearing;

    struct tendril_transfer intr;
    uint8_t intr_buf[INTR_MAX_PACKET];
    size_t intr_asked;
    uint8_t notify_buf[NOTIFY_BUF_SIZE];
    size_t notify_len;
    /* What is left of a notification too long for the buffer. */
    size_t notify_skip;
    bool intr_busy;
    bool intr_waiting;
    size_t intr_errors;
    bool intr_gave_up;
    bool intr_clearing;

    size_t num_tx;
    struct net_tx tx[MAX_XFERS];
    size_t tx_buf_size;
    /* Frames not yet handed to a transfer. */
    struct tendril_list frames;
    bool tx_clearing;

    /* Resubmits failed receive and interrupt transfers. */
    struct tendril_timer retry_timer;

    /* RNDIS, from REMOTE_NDIS_INITIALIZE_CMPLT and the filters last set. */
    uint32_t rndis_next_id;
    size_t rndis_max_packets;
    size_t rndis_max_transfer;
    uint32_t rndis_align;
    uint32_t rndis_filter;
    uint8_t rndis_mc[RNDIS_MAX_MC * MAC_LEN];
    size_t rndis_mc_count;
    bool rndis_mc_set;
    /* Malformed REMOTE_NDIS_PACKET_MSG messages received, for the log. */
    size_t rndis_rx_bad;
    /*
     * The control channel is in an exchange (the outstanding request's, or
     * unsolicited reads without one), and a transfer of it is in flight.
     */
    bool ctrl_busy;
    bool ctrl_pending;
    bool rndis_timed_out;
    struct tendril_timer rndis_read_timer;
    struct tendril_timer rndis_timeout_timer;
    struct tendril_timer rndis_keepalive_timer;
    /* When a receive transfer completed or a control message came last. */
    uint64_t rndis_last_rx;
    uint8_t rndis_buf[RNDIS_RESPONSE_SIZE];

    bool link_up;
    uint32_t rx_bps;
    uint32_t tx_bps;
};

static uint16_t get_le16(const uint8_t *p) {
    return (uint16_t)((uint32_t)p[0] | (uint32_t)p[1] << 8);
}

static uint32_t get_le32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16
        | (uint32_t)p[3] << 24;
}

static void put_le16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

static void put_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

/*
 * IEEE 802.3: reflected polynomial 0xedb88320, preset to all ones, result
 * complemented.
 */
static uint32_t ether_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = (uint32_t)0xffffffff;

    for (size_t i = 0; i < len; i++) {
        crc ^= (uint32_t)data[i];
        for (size_t bit = 0; bit < 8; bit++) {
            if ((crc & 1) != 0) {
                crc = crc >> 1 ^ (uint32_t)0xedb88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc;
}

static struct tendril_driver net_driver;
static const struct tendril_net_callbacks *net_callbacks;

static void retry_timer_fn(struct tendril_work *work);
static void rx_start(struct tendril_net *net);
static void intr_start(struct tendril_net *net);
static void tx_kick(struct tendril_net *net);
static void op_kick(struct tendril_net *net);
static void rndis_setup(struct tendril_net *net);
static int rndis_send(struct tendril_net *net, struct net_op *op);
static void rndis_poll(struct tendril_net *net);

/*
 * The first class-specific interface descriptor of `subtype` at least
 * `min_len` bytes long.
 */
static const uint8_t *find_cs_desc(
    const uint8_t *p, const uint8_t *end, int subtype, size_t min_len
) {
    for (; p != NULL && end - p >= 3; p = tendril_desc_next(p, end)) {
        if (p[1] == TENDRIL_DT_CS_INTERFACE && p[2] == subtype
         && p[0] >= min_len && p[0] <= end - p) {
            return p;
        }
    }
    return NULL;
}

static bool has_bulk_pair(const uint8_t *p, const uint8_t *end) {
    bool in = false;
    bool out = false;

    for (; p != NULL && end - p >= 2; p = tendril_desc_next(p, end)) {
        if (p[1] != TENDRIL_DT_ENDPOINT || p[0] < 7 || p[0] > end - p) {
            continue;
        }
        if ((p[3] & TENDRIL_ENDPOINT_XFER_MASK)
         != TENDRIL_ENDPOINT_XFER_BULK) {
            continue;
        }
        if ((p[2] & TENDRIL_ENDPOINT_DIR_MASK) != 0) {
            in = true;
        } else {
            out = true;
        }
    }
    return in && out;
}

/*
 * The first alternate setting with a bulk IN and a bulk OUT endpoint (CDC ECM
 * 1.2, 3.3; CDC NCM 1.0 Errata 1, 5.3).
 */
static bool find_data_alt(struct tendril_interface *intf, size_t *alt) {
    for (size_t a = 0; a < intf->num_alts; a++) {
        const uint8_t *p = (const uint8_t *)intf->alts[a];
        if (p != NULL && has_bulk_pair(p, p + intf->alt_sizes[a])) {
            *alt = a;
            return true;
        }
    }
    return false;
}

static bool match_class(
    struct tendril_device *dev, const struct tendril_interface_descriptor *id,
    int *kind, bool *vendor_ncm
) {
    if (id->interface_class == TENDRIL_CLASS_CDC) {
        switch (id->interface_subclass) {
            case CDC_SUBCLASS_ECM: {
                *kind = TENDRIL_NET_ECM;
                return true;
            }
            case CDC_SUBCLASS_NCM: {
                *kind = TENDRIL_NET_NCM;
                return true;
            }
            case CDC_SUBCLASS_MDLM: {
                /*
                 * Some handheld devices running Linux present their Ethernet
                 * function so, with ECM's descriptors and protocol (clean-room
                 * specification).
                 */
                *kind = TENDRIL_NET_ECM;
                return true;
            }
            default: {
                return false;
            }
        }
    }

    /* Huawei NCM functions under vendor codes (clean-room specification). */
    if (tendril_le16(dev->desc.vendor_id) == VENDOR_HUAWEI
     && id->interface_class == TENDRIL_CLASS_VENDOR) {
        uint8_t sub = id->interface_subclass;
        uint8_t proto = id->interface_protocol;
        if ((sub == 0x02 && (proto == 0x16 || proto == 0x46 || proto == 0x76))
         || (sub == 0x03 && proto == 0x16)) {
            *kind = TENDRIL_NET_NCM;
            *vendor_ncm = true;
            return true;
        }
    }
    return false;
}

static struct tendril_interface *find_data_interface(
    struct tendril_interface *intf
) {
    size_t size = 0;
    const uint8_t *desc = tendril_interface_descriptors(intf, &size);
    const uint8_t *un = NULL;
    struct tendril_interface *data = NULL;

    /* Union Functional Descriptor (CDC 1.2, 5.2.3.2, Table 16) */
    if (desc != NULL) {
        un = find_cs_desc(desc, desc + size, CDC_DESC_UNION, 5);
    }
    if (un != NULL) {
        data = tendril_device_find_interface(intf->dev, un[4]);
    }

    /* BEGIN OpenBSD sys/dev/usb/if_uncm.c */
    /*
     * An IAD may group the two interfaces instead (CDC NCM 1.0 Errata 1, 7.4);
     * OpenBSD's sys/dev/usb/if_uncm.c (ISC license) falls back the same way.
     */
    if (data == NULL && intf->iad != NULL
     && intf->iad->first_interface == intf->number
     && intf->iad->interface_count >= 2 && intf->number < 0xff) {
        data = tendril_device_find_interface(
            intf->dev, (uint8_t)(intf->number + 1)
        );
    }
    /* END OpenBSD sys/dev/usb/if_uncm.c */
    return data;
}

/* BEGIN OpenBSD sys/dev/usb/if_urndis.c, FreeBSD sys/dev/usb/net/if_urndis.c */
/* RNDIS control interfaces: class, subclass, and protocol. */
static const uint8_t rndis_classes[][3] = {
    /*
     * CDC ACM with a vendor protocol, as OpenBSD's and FreeBSD's if_urndis.c
     * match it
     */
    { 0x02, 0x02, 0xff },
    /* Wireless Controller, RF, RNDIS, as both match it */
    { 0xe0, 0x01, 0x03 },
    /* Miscellaneous, Sync, ActiveSync, as both match it */
    { 0xef, 0x01, 0x01 },
    /* Miscellaneous, RNDIS, Ethernet, as both match it */
    { 0xef, 0x04, 0x01 },
    /* Novatel Wireless 8800/8000/etc, as FreeBSD's if_urndis.c matches it */
    { 0xef, 0xef, 0x03 }
};

/*
 * The table follows OpenBSD's sys/dev/usb/if_urndis.c and FreeBSD's
 * sys/dev/usb/net/if_urndis.c (ISC license).
 */
static bool match_rndis(
    struct tendril_device *dev, const struct tendril_interface_descriptor *id
) {
    /*
     * FUJITSUCOMP VIRTETH advertises RNDIS and CDC Ethernet, but its RNDIS
     * does not work: OpenBSD's sys/dev/usb/if_urndis.c (ISC license) excludes
     * it, leaving its ECM function.
     */
    if (tendril_le16(dev->desc.vendor_id) == 0x0430
     && tendril_le16(dev->desc.product_id) == 0xa4a2) {
        return false;
    }
    for (size_t i = 0; i < ARRAY_SIZE(rndis_classes); i++) {
        if (id->interface_class == rndis_classes[i][0]
         && id->interface_subclass == rndis_classes[i][1]
         && id->interface_protocol == rndis_classes[i][2]) {
            return true;
        }
    }
    return false;
}
/* END OpenBSD sys/dev/usb/if_urndis.c, FreeBSD sys/dev/usb/net/if_urndis.c */

/* BEGIN FreeBSD sys/dev/usb/net/if_urndis.c */
/*
 * An RNDIS function's data interface: the one the Union Functional Descriptor
 * names (CDC 1.2, 5.2.3.2), else the Call Management Functional Descriptor's
 * bDataInterface, else the next interface, the last two as FreeBSD's
 * sys/dev/usb/net/if_urndis.c (ISC license) finds it.
 */
static struct tendril_interface *find_rndis_data_interface(
    struct tendril_interface *intf
) {
    size_t size = 0;
    const uint8_t *desc = tendril_interface_descriptors(intf, &size);
    const uint8_t *fd = NULL;
    uint32_t number = intf->number + (uint32_t)1;

    if (desc != NULL) {
        fd = find_cs_desc(desc, desc + size, CDC_DESC_UNION, 5);
        if (fd == NULL) {
            fd = find_cs_desc(desc, desc + size, CDC_DESC_CALL_MANAGEMENT, 5);
        }
    }
    if (fd != NULL) {
        number = fd[4];
    }
    if (number > 0xff) {
        return NULL;
    }
    return tendril_device_find_interface(intf->dev, (uint8_t)number);
}
/* END FreeBSD sys/dev/usb/net/if_urndis.c */

static void read_functional(struct tendril_net *net) {
    struct tendril_device *dev = net->dev;
    size_t size = 0;
    const uint8_t *desc = tendril_interface_descriptors(net->ctrl_intf, &size);
    const uint8_t *eth = NULL;
    const uint8_t *ncm = NULL;

    /* Ethernet Networking Functional Descriptor (CDC ECM 1.2, 5.4, Table 3) */
    if (desc != NULL) {
        eth = find_cs_desc(desc, desc + size, CDC_DESC_ETHERNET, 13);
        ncm = find_cs_desc(desc, desc + size, CDC_DESC_NCM, 6);
    }
    if (eth == NULL && dev->active_config >= 0
     && (size_t)dev->active_config < dev->num_configs) {
        /*
         * Some devices put it elsewhere in the configuration (clean-room
         * specification).
         */
        const uint8_t *config = dev->configs[dev->active_config];
        if (config != NULL) {
            eth = find_cs_desc(
                config, config + dev->config_sizes[dev->active_config],
                CDC_DESC_ETHERNET, 13
            );
        }
    }

    net->max_frame = DEFAULT_MAX_FRAME;
    if (eth != NULL) {
        uint16_t segment = get_le16(eth + 8);
        net->mac_string = eth[3];
        net->mc_filters = (uint16_t)(get_le16(eth + 10) & (uint32_t)0x7fff);
        if (segment >= ETH_HEADER_LEN) {
            net->max_frame = MIN((size_t)segment, (size_t)MAX_FRAME_LIMIT);
        }
    }

    /* NCM Functional Descriptor (CDC NCM 1.0 Errata 1, 5.2.1, Table 5-2) */
    if (ncm != NULL) {
        net->ncm_caps = ncm[5];
    }
}

static int ctrl_submit(
    struct tendril_net *net, uint8_t type, uint8_t request, uint16_t value,
    void *data, size_t length, void (*callback)(struct tendril_transfer *xfer)
) {
    struct tendril_transfer *xfer = &net->ctrl;

    tendril_transfer_init(xfer, tendril_device_ep0(net->dev));
    xfer->setup.request_type = type;
    xfer->setup.request = request;
    xfer->setup.value = tendril_le16(value);
    xfer->setup.index = tendril_le16(net->ctrl_intf->number);
    xfer->setup.length = tendril_le16((uint16_t)length);
    xfer->buffer = data;
    xfer->length = length;
    xfer->timeout_ms = CTRL_TIMEOUT_MS;
    xfer->callback = callback;
    xfer->context = net;
    return tendril_transfer_submit(xfer);
}

static int hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

/*
 * iMACAddress: 12 hexadecimal digits, most significant nibble first (CDC ECM
 * 1.2, 5.4).
 */
static bool parse_mac(struct tendril_net *net) {
    uint8_t mac[MAC_LEN];
    uint8_t any = 0;

    net->mac_str[MAC_STRING_SIZE - 1] = '\0';
    if (tendril_strlen(net->mac_str) != 2 * MAC_LEN) {
        return false;
    }
    for (size_t i = 0; i < MAC_LEN; i++) {
        int hi = hex_value(net->mac_str[2 * i]);
        int lo = hex_value(net->mac_str[2 * i + 1]);
        if (hi < 0 || lo < 0) {
            return false;
        }
        mac[i] = (uint8_t)(hi << 4 | lo);
        any = (uint8_t)(any | mac[i]);
    }
    if (any == 0 || (mac[0] & 1) != 0) {
        return false;
    }
    memcpy(net->mac, mac, MAC_LEN);
    return true;
}

/* BEGIN OpenBSD sys/dev/usb/if_uncm.c */
/*
 * A locally administered unicast address. OpenBSD's sys/dev/usb/if_uncm.c (ISC
 * license) also replaces missing, multicast, and all-zero addresses.
 */
static void make_up_mac(struct tendril_net *net) {
    uint64_t now = tendril_now();
    uint32_t mix = ((uint32_t)now ^ (uint32_t)(now >> 32))
        * (uint32_t)0x9e3779b1;
    const uint8_t *mac = net->mac;

    net->mac[0] = 0x02;
    net->mac[1] = net->dev->address;
    put_le32(net->mac + 2, mix);
    LOG_INFO(
        "net: interface %u has no usable MAC address, using "
        "%02x:%02x:%02x:%02x:%02x:%02x",
        (unsigned int)net->ctrl_intf->number, mac[0], mac[1], mac[2], mac[3],
        mac[4], mac[5]
    );
}
/* END OpenBSD sys/dev/usb/if_uncm.c */

static bool power_of_two(uint32_t v) {
    return v != 0 && (v & (v - 1)) == 0;
}

/* BEGIN OpenBSD sys/dev/usb/if_uncm.c */
/*
 * GET_NTB_PARAMETERS (CDC NCM 1.0 Errata 1, 6.2.1, Table 6-3), used as
 * OpenBSD's sys/dev/usb/if_uncm.c (ISC license, uncm_ntb_setup) does unless
 * noted.
 */
static bool ntb_params(struct tendril_net *net) {
    const uint8_t *p = net->ctrl_buf;
    uint32_t in_max = get_le32(p + 4);
    uint32_t out_max = get_le32(p + 16);
    uint32_t divisor = get_le16(p + 20);
    uint32_t remainder = get_le16(p + 22);
    uint32_t align = get_le16(p + 24);
    uint32_t datagrams = get_le16(p + 26);

    /* This threshold is this implementation's own. */
    if (in_max < NTB_MIN_SIZE || out_max < NTB_MIN_SIZE) {
        LOG_WARN(
            "net: interface %u: NTB sizes %u and %u are too small",
            (unsigned int)net->ctrl_intf->number, (unsigned int)in_max,
            (unsigned int)out_max
        );
        return false;
    }

    size_t out_size = MIN(out_max, (uint32_t)NTB_SIZE_LIMIT);
    if (!power_of_two(align) || align < NDP_MIN_ALIGN || align >= out_size) {
        align = NDP_MIN_ALIGN;
    }
    if (!power_of_two(divisor) || divisor < NDP_MIN_DIVISOR
     || divisor >= out_size) {
        divisor = NDP_MIN_DIVISOR;
    }
    if (remainder >= divisor) {
        remainder = 0;
    }
    /* The cap of 32 is this implementation's own. */
    if (datagrams == 0 || datagrams > NTB_MAX_DATAGRAMS) {
        datagrams = NTB_MAX_DATAGRAMS;
    }

    /* Room for the NTH16, both paddings, and an NDP16 with one datagram. */
    size_t overhead = NTH16_LEN + (divisor - 1) + (align - 1) + NDP16_MIN_LEN;
    if (out_size < overhead + NTB_MIN_FRAME_ROOM) {
        LOG_WARN(
            "net: interface %u: OUT NTB size %u leaves no room for frames",
            (unsigned int)net->ctrl_intf->number, (unsigned int)out_size
        );
        return false;
    }

    net->ntb_formats = get_le16(p + 2);
    net->ntb_in_max = in_max;
    net->ntb_out_max = out_max;
    net->ntb_in_size = MIN(in_max, (uint32_t)NTB_SIZE_LIMIT);
    net->ntb_out_size = out_size;
    net->ndp_out_divisor = divisor;
    net->ndp_out_remainder = remainder;
    net->ndp_out_align = align;
    net->ntb_out_datagrams = datagrams;
    net->ntb_frame_cap = out_size - overhead;
    net->max_frame = MIN(net->max_frame, net->ntb_frame_cap);
    return true;
}
/* END OpenBSD sys/dev/usb/if_uncm.c */

static bool alloc_buffers(struct tendril_net *net) {
    if (net->kind == TENDRIL_NET_NCM) {
        net->num_rx = NCM_RX_XFERS;
        net->num_tx = NCM_TX_XFERS;
        net->rx_buf_size = net->ntb_in_size;
        net->tx_buf_size = net->ntb_out_size;
    } else if (net->kind == TENDRIL_NET_RNDIS) {
        /*
         * No transfer from the device exceeds what REMOTE_NDIS_INITIALIZE_MSG
         * asked for.
         */
        net->num_rx = RNDIS_RX_XFERS;
        net->num_tx = RNDIS_TX_XFERS;
        net->rx_buf_size = RNDIS_MAX_TRANSFER;
        net->tx_buf_size = MIN(
            (size_t)net->rndis_max_transfer, (size_t)RNDIS_MAX_TRANSFER
        );
    } else {
        size_t len = net->max_frame;
        size_t packet = net->in_ep->max_packet;
        if (net->crc32) {
            len += ETH_CRC_LEN;
        }
        if (packet == 0) {
            packet = 1;
        }
        net->num_rx = ECM_RX_XFERS;
        net->num_tx = ECM_TX_XFERS;
        net->rx_buf_size = (len + packet - 1) / packet * packet;
        net->tx_buf_size = 0;
        if (net->crc32) {
            net->tx_buf_size = net->max_frame + ETH_CRC_LEN;
        }
    }

    for (size_t i = 0; i < net->num_rx; i++) {
        net->rx[i].buf = tendril_zalloc(net->rx_buf_size);
        if (net->rx[i].buf == NULL) {
            return false;
        }
    }
    if (net->tx_buf_size != 0) {
        for (size_t i = 0; i < net->num_tx; i++) {
            net->tx[i].buf = tendril_zalloc(net->tx_buf_size);
            if (net->tx[i].buf == NULL) {
                return false;
            }
        }
    }
    return true;
}

/*
 * The data interface's bulk endpoints, and the control interface's optional
 * notification endpoint.
 */
static bool find_endpoints(struct tendril_net *net) {
    net->in_ep = tendril_interface_find_endpoint(
        net->data_intf, TENDRIL_ENDPOINT_XFER_BULK, TENDRIL_DIR_IN
    );
    net->out_ep = tendril_interface_find_endpoint(
        net->data_intf, TENDRIL_ENDPOINT_XFER_BULK, TENDRIL_DIR_OUT
    );
    net->intr_ep = tendril_interface_find_endpoint(
        net->ctrl_intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_IN
    );
    return net->in_ep != NULL && net->out_ep != NULL;
}

static void setup_run(struct tendril_net *net);

/* Setup stops; the adapter stays claimed, inert, until disconnect. */
static void setup_fail(struct tendril_net *net, const char *what, int status) {
    net->stopped = true;
    if (status != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN(
            "net: interface %u: %s failed (%s)",
            (unsigned int)net->ctrl_intf->number, what,
            tendril_status_str(status)
        );
    }
}

static void setup_goto(struct tendril_net *net, int step) {
    net->step = step;
    setup_run(net);
}

static void setup_result(struct tendril_net *net, int status, size_t actual) {
    if (status == TENDRIL_ERR_NO_DEVICE) {
        net->stopped = true;
        return;
    }

    switch (net->step) {
        case STEP_MAC: {
            if (status != TENDRIL_OK || !parse_mac(net)) {
                make_up_mac(net);
            }
            setup_goto(net, STEP_NTB_PARAMS);
            break;
        }
        case STEP_NTB_PARAMS: {
            if (status == TENDRIL_OK && actual < NTB_PARAMS_LEN) {
                status = TENDRIL_ERR_SHORT;
            }
            if (status != TENDRIL_OK && net->vendor_ncm) {
                LOG_DEBUG(
                    "net: interface %u: no NTB parameters, using ECM",
                    (unsigned int)net->ctrl_intf->number
                );
                net->kind = TENDRIL_NET_ECM;
                setup_goto(net, STEP_DATA_ALT);
            } else if (status != TENDRIL_OK) {
                setup_fail(net, "GET_NTB_PARAMETERS", status);
            } else if (!ntb_params(net)) {
                net->stopped = true;
            } else {
                setup_goto(net, STEP_NTB_FORMAT);
            }
            break;
        }
        case STEP_NTB_FORMAT: {
            /*
             * A failure is harmless: alternate setting 0 selected NTB-16
             * already (CDC NCM 1.0 Errata 1, 7.2).
             */
            setup_goto(net, STEP_NTB_INPUT_SIZE);
            break;
        }
        case STEP_NTB_INPUT_SIZE: {
            if (status != TENDRIL_OK) {
                /* BEGIN OpenBSD sys/dev/usb/if_uncm.c */
                /*
                 * Refused: keep the function's maximum, as OpenBSD's if_uncm.c
                 * does, within NTB-16's limit.
                 */
                net->ntb_in_size = MIN(
                    net->ntb_in_max, (uint32_t)NTB16_MAX_SIZE
                );
                /* END OpenBSD sys/dev/usb/if_uncm.c */
            }
            setup_goto(net, STEP_MAX_DATAGRAM);
            break;
        }
        case STEP_MAX_DATAGRAM: {
            if (status == TENDRIL_OK && actual >= 2) {
                uint16_t size = get_le16(net->ctrl_buf);
                if (size >= ETH_HEADER_LEN && size <= MAX_FRAME_LIMIT) {
                    net->max_frame = MIN((size_t)size, net->ntb_frame_cap);
                }
            }
            setup_goto(net, STEP_DATA_ALT);
            break;
        }
        case STEP_DATA_ALT: {
            if (status != TENDRIL_OK) {
                setup_fail(
                    net, "selecting the data alternate setting", status
                );
                break;
            }
            if (!find_endpoints(net)) {
                setup_fail(
                    net, "finding the bulk endpoints", TENDRIL_ERR_UNSUPPORTED
                );
                break;
            }
            setup_goto(net, STEP_FILTER);
            break;
        }
        case STEP_FILTER: {
            /* A failure is ignored. */
            setup_goto(net, STEP_DONE);
            break;
        }
        case STEP_DONE: {
            break;
        }
        default: {
            break;
        }
    }
}

static void setup_ctrl_done(struct tendril_transfer *xfer) {
    setup_result(xfer->context, xfer->status, xfer->actual);
}

static void setup_async_done(void *ctx, int status) {
    setup_result(ctx, status, 0);
}

static void setup_request(
    struct tendril_net *net, uint8_t type, uint8_t request, uint16_t value,
    size_t length
) {
    void *buf = NULL;
    int status;

    if (length != 0) {
        buf = net->ctrl_buf;
    }
    status = ctrl_submit(
        net, type, request, value, buf, length, setup_ctrl_done
    );
    if (status != TENDRIL_OK) {
        setup_result(net, status, 0);
    }
}

static const char *kind_name(int kind) {
    switch (kind) {
        case TENDRIL_NET_NCM: {
            return "CDC NCM";
        }
        case TENDRIL_NET_RNDIS: {
            return "RNDIS";
        }
        default: {
            return "CDC ECM";
        }
    }
}

static void setup_finish(struct tendril_net *net) {
    const uint8_t *mac = net->mac;

    if (!alloc_buffers(net)) {
        LOG_WARN(
            "net: interface %u: out of memory for buffers",
            (unsigned int)net->ctrl_intf->number
        );
        net->stopped = true;
        return;
    }

    net->ready = true;
    net->attached = true;
    if (net->kind == TENDRIL_NET_RNDIS) {
        net->rndis_last_rx = tendril_now();
        hc_lock(net->hc);
        tendril_timer_arm(
            net->hc, &net->rndis_keepalive_timer,
            RNDIS_KEEPALIVE_MS * NS_PER_MS
        );
        hc_unlock(net->hc);
    }
    LOG_INFO(
        "net: interface %u ready (%s, %02x:%02x:%02x:%02x:%02x:%02x)",
        (unsigned int)net->ctrl_intf->number, kind_name(net->kind), mac[0],
        mac[1], mac[2], mac[3], mac[4], mac[5]
    );
    if (net_callbacks->attach != NULL) {
        net_callbacks->attach(net);
    }
    if (net->kind == TENDRIL_NET_RNDIS && net_callbacks->link != NULL) {
        /* The state queried during setup. */
        net_callbacks->link(net, net->link_up, net->rx_bps, net->tx_bps);
    }

    rx_start(net);
    intr_start(net);
    tx_kick(net);
    op_kick(net);
}

/*
 * Each step starts the next from its completion. The data interface is in
 * alternate setting 0, which reset the function's network state (CDC ECM 1.2,
 * 3.3; CDC NCM 1.0 Errata 1, 7.2).
 */
static void setup_run(struct tendril_net *net) {
    int status;

    switch (net->step) {
        case STEP_MAC: {
            if (net->mac_string == 0) {
                make_up_mac(net);
                setup_goto(net, STEP_NTB_PARAMS);
                break;
            }
            status = tendril_device_read_string(
                net->dev, net->mac_string, net->mac_str, sizeof(net->mac_str),
                setup_async_done, net
            );
            if (status != TENDRIL_OK) {
                setup_result(net, status, 0);
            }
            break;
        }
        case STEP_NTB_PARAMS: {
            if (net->kind == TENDRIL_NET_NCM) {
                setup_request(
                    net, REQ_IN, GET_NTB_PARAMETERS, 0, NTB_PARAMS_LEN
                );
            } else {
                setup_goto(net, STEP_DATA_ALT);
            }
            break;
        }
        case STEP_NTB_FORMAT: {
            /*
             * NTB-16. Forbidden for functions without NTB-32, and only valid
             * in alternate setting 0 (CDC NCM 1.0 Errata 1, 6.2.5).
             */
            if ((net->ntb_formats & NTB_FORMAT_32) != 0) {
                setup_request(net, REQ_OUT, SET_NTB_FORMAT, 0, 0);
            } else {
                setup_goto(net, STEP_NTB_INPUT_SIZE);
            }
            break;
        }
        case STEP_NTB_INPUT_SIZE: {
            /*
             * The 4-byte form is valid whatever D5 says, and the host picks
             * from 2048 to the function's maximum (CDC NCM 1.0 Errata 1,
             * 6.2.7, Table 6-4).
             */
            if (net->ntb_in_max > NTB_SIZE_LIMIT) {
                put_le32(net->ctrl_buf, NTB_SIZE_LIMIT);
                setup_request(net, REQ_OUT, SET_NTB_INPUT_SIZE, 0, 4);
            } else {
                setup_goto(net, STEP_MAX_DATAGRAM);
            }
            break;
        }
        case STEP_MAX_DATAGRAM: {
            /*
             * The host must read or set it (CDC NCM 1.0 Errata 1, 6.2.8 and
             * 7.2).
             */
            if ((net->ncm_caps & NCM_CAP_MAX_DATAGRAM) != 0) {
                setup_request(net, REQ_IN, GET_MAX_DATAGRAM_SIZE, 0, 2);
            } else {
                setup_goto(net, STEP_DATA_ALT);
            }
            break;
        }
        case STEP_DATA_ALT: {
            if (net->data_intf->cur_alt == net->data_alt_index) {
                setup_result(net, TENDRIL_OK, 0);
                break;
            }
            status = tendril_interface_set_alt(
                net->data_intf, net->data_alt, setup_async_done, net
            );
            if (status != TENDRIL_OK) {
                setup_result(net, status, 0);
            }
            break;
        }
        case STEP_FILTER: {
            /* CDC ECM 1.2, 6.2.4, Table 8 */
            if (net->kind == TENDRIL_NET_ECM
             || (net->ncm_caps & NCM_CAP_PACKET_FILTER) != 0) {
                setup_request(
                    net, REQ_OUT, SET_ETHERNET_PACKET_FILTER,
                    TENDRIL_NET_FILTER_DIRECTED | TENDRIL_NET_FILTER_BROADCAST
                        | TENDRIL_NET_FILTER_ALL_MULTICAST,
                    0
                );
            } else {
                setup_goto(net, STEP_DONE);
            }
            break;
        }
        case STEP_DONE: {
            setup_finish(net);
            break;
        }
        default: {
            break;
        }
    }
}

static bool net_probe(struct tendril_interface *intf) {
    struct tendril_device *dev = intf->dev;
    const struct tendril_interface_descriptor *id
        = tendril_interface_descriptor(intf);
    uint32_t quirks = tendril_id_quirks(
        net_quirks, ARRAY_SIZE(net_quirks), &dev->desc
    );
    int kind = TENDRIL_NET_ECM;
    bool vendor_ncm = false;
    struct tendril_interface *data_intf;
    size_t alt;

    if ((quirks & QUIRK_NO_UNION) != 0) {
        /*
         * One interface carries control and data, speaking ECM (clean-room
         * specification).
         */
        data_intf = intf;
    } else if (id != NULL && match_rndis(dev, id)) {
        kind = TENDRIL_NET_RNDIS;
        data_intf = find_rndis_data_interface(intf);
        if (data_intf == NULL || data_intf == intf) {
            return false;
        }
    } else {
        if (id == NULL || !match_class(dev, id, &kind, &vendor_ncm)) {
            return false;
        }
        data_intf = find_data_interface(intf);
        if (data_intf == NULL) {
            return false;
        }
    }
    if (!find_data_alt(data_intf, &alt)) {
        return false;
    }
    if (data_intf != intf
     && !tendril_interface_claim(data_intf, &net_driver)) {
        return false;
    }

    struct tendril_net *net = tendril_zalloc(sizeof(struct tendril_net));
    if (net == NULL) {
        LOG_WARN("net: out of memory");
        return false;
    }
    net->dev = dev;
    net->hc = tendril_device_hc(dev);
    net->ctrl_intf = intf;
    net->data_intf = data_intf;
    net->kind = kind;
    net->vendor_ncm = vendor_ncm;
    net->data_alt_index = alt;
    net->data_alt = data_intf->alts[alt]->alternate_setting;
    if (kind == TENDRIL_NET_RNDIS) {
        net->max_frame = DEFAULT_MAX_FRAME;
    } else {
        net->crc32 = (quirks & QUIRK_CRC32) != 0;
        read_functional(net);
    }
    list_init(&net->ops);
    list_init(&net->frames);
    for (size_t i = 0; i < MAX_XFERS; i++) {
        net->rx[i].net = net;
        net->tx[i].net = net;
        list_init(&net->tx[i].frames);
    }
    tendril_timer_init(&net->retry_timer, retry_timer_fn);
    tendril_interface_set_data(intf, net);

    if (kind == TENDRIL_NET_RNDIS) {
        rndis_setup(net);
    } else {
        net->step = STEP_MAC;
        setup_run(net);
    }
    return true;
}

static void clear_done(void *ctx, int status) {
    bool *clearing = ctx;

    (void)status;
    *clearing = false;
}

static void clear_halt(struct tendril_endpoint *ep, bool *clearing) {
    if (*clearing) {
        return;
    }
    *clearing = true;
    if (tendril_endpoint_clear_halt(ep, clear_done, clearing) != TENDRIL_OK) {
        *clearing = false;
    }
}

/*
 * Counts a failed receive or interrupt transfer; false once the endpoint is
 * given up on.
 */
static bool in_failed(
    struct tendril_endpoint *ep, size_t *errors, bool *gave_up, bool *clearing,
    int status
) {
    if (*gave_up) {
        return false;
    }
    if (++*errors > MAX_ERRORS) {
        *gave_up = true;
        LOG_WARN(
            "net: receiving keeps failing (%s), giving up",
            tendril_status_str(status)
        );
        return false;
    }
    if (status == TENDRIL_ERR_STALL) {
        clear_halt(ep, clearing);
    }
    return true;
}

static void retry_later(struct tendril_net *net) {
    hc_lock(net->hc);
    if (!tendril_timer_armed(&net->retry_timer)) {
        tendril_timer_arm(
            net->hc, &net->retry_timer, RETRY_DELAY_MS * NS_PER_MS
        );
    }
    hc_unlock(net->hc);
}

/*
 * Failures that leave a receive or interrupt transfer idle rather than count
 * as errors: the adapter stopped or suspended, the device gone, or the
 * transfer cancelled or refused while suspended.
 */
static bool in_idle(struct tendril_net *net, int status) {
    return net->stopped || net->suspended || status == TENDRIL_ERR_NO_DEVICE
        || status == TENDRIL_ERR_CANCELLED || status == TENDRIL_ERR_SUSPENDED;
}

static void deliver(
    struct tendril_net *net, const uint8_t *frame, size_t len
) {
    if (net_callbacks->receive != NULL) {
        net_callbacks->receive(net, frame, len);
    }
}

static void link_set(
    struct tendril_net *net, bool up, uint32_t rx_bps, uint32_t tx_bps
) {
    bool changed = up != net->link_up
        || (up && (rx_bps != net->rx_bps || tx_bps != net->tx_bps));

    net->link_up = up;
    net->rx_bps = rx_bps;
    net->tx_bps = tx_bps;
    if (changed && net->attached && net_callbacks->link != NULL) {
        net_callbacks->link(net, up, rx_bps, tx_bps);
    }
}

/* CDC 1.2, 6.3: an 8-byte header, then wLength bytes. */
static void notify_handle(
    struct tendril_net *net, const uint8_t *n, size_t len
) {
    if (n[0] != REQ_IN) {
        return;
    }
    switch (n[1]) {
        case NOTIFY_NETWORK_CONNECTION: {
            /* CDC 1.2, 6.3.1 */
            link_set(net, get_le16(n + 2) != 0, net->rx_bps, net->tx_bps);
            break;
        }
        case NOTIFY_SPEED_CHANGE: {
            /*
             * DLBitRate, what the host receives, then ULBitRate (CDC 1.2,
             * 6.3.3, Table 21).
             */
            if (len >= NOTIFY_HEADER_LEN + 8) {
                link_set(net, net->link_up, get_le32(n + 8), get_le32(n + 12));
            }
            break;
        }
        default: {
            break;
        }
    }
}

/*
 * Handles every complete notification in the buffer and keeps a partial one at
 * its start.
 */
static void notify_drain(struct tendril_net *net) {
    size_t pos = 0;

    while (net->notify_len - pos >= NOTIFY_HEADER_LEN && !net->stopped) {
        const uint8_t *n = net->notify_buf + pos;
        size_t total = NOTIFY_HEADER_LEN + (size_t)get_le16(n + 6);
        size_t have = net->notify_len - pos;
        if (total > NOTIFY_BUF_SIZE) {
            net->notify_skip = total - have;
            pos = net->notify_len;
            break;
        }
        if (have < total) {
            break;
        }
        notify_handle(net, n, total);
        pos += total;
    }
    if (net->stopped) {
        net->notify_len = 0;
        return;
    }
    memmove(net->notify_buf, net->notify_buf + pos, net->notify_len - pos);
    net->notify_len -= pos;
}

/* A notification may span transfers, and a transfer may carry several. */
static void notify_input(
    struct tendril_net *net, const uint8_t *data, size_t len
) {
    size_t off = 0;

    while (off < len && !net->stopped) {
        size_t n;
        if (net->notify_skip != 0) {
            n = MIN(net->notify_skip, len - off);
            net->notify_skip -= n;
            off += n;
            continue;
        }
        n = MIN(NOTIFY_BUF_SIZE - net->notify_len, len - off);
        memcpy(net->notify_buf + net->notify_len, data + off, n);
        net->notify_len += n;
        off += n;
        notify_drain(net);
    }
}

static void intr_done(struct tendril_transfer *xfer);

static void intr_error(struct tendril_net *net, int status) {
    net->notify_len = 0;
    net->notify_skip = 0;
    if (in_idle(net, status)) {
        return;
    }
    bool failed = in_failed(
        net->intr_ep, &net->intr_errors, &net->intr_gave_up,
        &net->intr_clearing, status
    );
    if (failed) {
        net->intr_waiting = true;
        retry_later(net);
    }
}

static void intr_submit(struct tendril_net *net) {
    struct tendril_transfer *xfer = &net->intr;
    size_t len = MIN(
        (size_t)net->intr_ep->max_packet, (size_t)INTR_MAX_PACKET
    );
    int status;

    if (len == 0) {
        len = INTR_MAX_PACKET;
    }
    net->intr_asked = len;
    tendril_transfer_init(xfer, net->intr_ep);
    xfer->buffer = net->intr_buf;
    xfer->length = len;
    xfer->callback = intr_done;
    xfer->context = net;
    status = tendril_transfer_submit(xfer);
    if (status == TENDRIL_OK) {
        net->intr_busy = true;
    } else {
        intr_error(net, status);
    }
}

static void intr_done(struct tendril_transfer *xfer) {
    struct tendril_net *net = xfer->context;

    net->intr_busy = false;
    if (xfer->status != TENDRIL_OK) {
        intr_error(net, xfer->status);
        return;
    }
    if (net->stopped) {
        return;
    }
    net->intr_errors = 0;
    if (net->kind == TENDRIL_NET_RNDIS) {
        /*
         * RESPONSE_AVAILABLE (CDC 1.2, 6.3.2), or the 8-byte record some
         * devices send instead: a message waits to be read.
         */
        if (xfer->actual != 0) {
            rndis_poll(net);
        }
    } else {
        notify_input(net, net->intr_buf, xfer->actual);
        /* A short transfer ends any partial notification. */
        if (xfer->actual < net->intr_asked) {
            net->notify_len = 0;
            net->notify_skip = 0;
        }
    }
    if (!net->stopped && !net->suspended && !net->intr_gave_up) {
        intr_submit(net);
    }
}

static void intr_start(struct tendril_net *net) {
    if (net->intr_ep != NULL && !net->stopped && !net->suspended
     && !net->intr_busy && !net->intr_waiting) {
        intr_submit(net);
    }
}

/*
 * One frame per transfer, without CRC unless the device adds one (CDC ECM 1.2,
 * 3.3.1 and 3.4).
 */
static void ecm_receive(
    struct tendril_net *net, const uint8_t *frame, size_t len
) {
    if (net->crc32) {
        if (len < ETH_HEADER_LEN + ETH_CRC_LEN) {
            return;
        }
        len -= ETH_CRC_LEN;
    }
    if (len >= ETH_HEADER_LEN) {
        deliver(net, frame, len);
    }
}

/* One NTB-16 (CDC NCM 1.0 Errata 1, 3). */
static void ncm_receive(
    struct tendril_net *net, const uint8_t *ntb, size_t len
) {
    if (len < NTH16_LEN || get_le32(ntb) != NTH16_SIGNATURE) {
        return;
    }

    /*
     * NTH16 (3.2.1, Table 3-1): a wBlockLength of 0 means a short packet ended
     * the block.
     */
    size_t block = get_le16(ntb + 8);
    if (block == 0) {
        block = len;
    } else if (block > len) {
        return;
    }
    size_t ndp = get_le16(ntb + 10);
    if (ndp < NTH16_LEN || ndp > block || block - ndp < NDP16_HEADER_LEN) {
        return;
    }

    /*
     * NDP16 (3.3.1, Table 3-3). wNextNdpIndex is reserved: only this NDP is
     * handled, as OpenBSD's sys/dev/usb/if_uncm.c (ISC license) also does.
     */
    const uint8_t *p = ntb + ndp;
    uint32_t signature = get_le32(p);
    size_t crc;
    if (signature == NDP16_SIGNATURE) {
        crc = 0;
    } else if (signature == NDP16_SIGNATURE_CRC) {
        /* Each datagram ends with its CRC (3.3.3, Table 3-5). */
        crc = ETH_CRC_LEN;
    } else {
        return;
    }
    size_t ndp_len = get_le16(p + 4);
    if (ndp_len < NDP16_MIN_LEN || ndp_len > block - ndp) {
        return;
    }

    for (size_t off = NDP16_HEADER_LEN;
         off + NDP16_ENTRY_LEN <= ndp_len && !net->stopped;
         off += NDP16_ENTRY_LEN) {
        size_t index = get_le16(p + off);
        size_t dlen = get_le16(p + off + 2);
        /* The first entry with a zero field ends the list (3.7). */
        if (index == 0 || dlen == 0) {
            break;
        }
        if (index > block || dlen > block - index
         || dlen < ETH_HEADER_LEN + crc) {
            continue;
        }
        deliver(net, ntb + index, dlen - crc);
    }
}

/*
 * REMOTE_NDIS_PACKET_MSG messages back to back ([MS-RNDIS] v20130118, 2.2.14
 * and 3.1.5.8). A tail shorter than a header is padding, which OpenBSD's
 * sys/dev/usb/if_urndis.c (ISC license) also tolerates. A malformed message
 * ends the walk, as its length cannot be trusted.
 */
static void rndis_receive(
    struct tendril_net *net, const uint8_t *buf, size_t len
) {
    size_t off = 0;

    while (len - off >= RNDIS_PACKET_HEADER_LEN && !net->stopped) {
        const uint8_t *msg = buf + off;
        uint32_t msg_len = get_le32(msg + 4);
        uint32_t data_off = get_le32(msg + 8);
        uint32_t data_len = get_le32(msg + 12);

        /* BEGIN FreeBSD sys/dev/usb/net/if_urndis.c */
        /*
         * DataOffset counts from its own field; DataLength's bounds are the
         * ones FreeBSD's sys/dev/usb/net/if_urndis.c (ISC license) accepts.
         */
        if (get_le32(msg) != RNDIS_PACKET_MSG
         || msg_len < RNDIS_PACKET_HEADER_LEN || msg_len > len - off
         || (data_off & 3) != 0 || data_off > msg_len - 8
         || data_len > msg_len - 8 - data_off || data_len < ETH_HEADER_LEN
         || data_len > RNDIS_MAX_FRAME) {
            net->rndis_rx_bad++;
            LOG_DEBUG(
                "net: interface %u: malformed RNDIS packet message (%zu so "
                "far)",
                (unsigned int)net->ctrl_intf->number, net->rndis_rx_bad
            );
            return;
        }
        /* END FreeBSD sys/dev/usb/net/if_urndis.c */
        deliver(net, msg + 8 + data_off, data_len);
        off += msg_len;
    }
}

static void rx_done(struct tendril_transfer *xfer);

static void rx_error(struct tendril_net *net, struct net_rx *rx, int status) {
    if (in_idle(net, status)) {
        return;
    }
    bool failed = in_failed(
        net->in_ep, &net->rx_errors, &net->rx_gave_up, &net->rx_clearing,
        status
    );
    if (failed) {
        rx->waiting = true;
        retry_later(net);
    }
}

static void rx_submit(struct tendril_net *net, struct net_rx *rx) {
    struct tendril_transfer *xfer = &rx->xfer;
    int status;

    tendril_transfer_init(xfer, net->in_ep);
    xfer->buffer = rx->buf;
    xfer->length = net->rx_buf_size;
    xfer->callback = rx_done;
    xfer->context = rx;
    status = tendril_transfer_submit(xfer);
    if (status == TENDRIL_OK) {
        rx->busy = true;
    } else {
        rx_error(net, rx, status);
    }
}

static void rx_done(struct tendril_transfer *xfer) {
    struct net_rx *rx = xfer->context;
    struct tendril_net *net = rx->net;

    rx->busy = false;
    if (xfer->status != TENDRIL_OK) {
        rx_error(net, rx, xfer->status);
        return;
    }
    if (net->stopped) {
        return;
    }
    net->rx_errors = 0;
    if (net->kind == TENDRIL_NET_NCM) {
        ncm_receive(net, rx->buf, xfer->actual);
    } else if (net->kind == TENDRIL_NET_RNDIS) {
        net->rndis_last_rx = tendril_now();
        rndis_receive(net, rx->buf, xfer->actual);
    } else {
        ecm_receive(net, rx->buf, xfer->actual);
    }
    if (!net->stopped && !net->suspended && !net->rx_gave_up) {
        rx_submit(net, rx);
    }
}

static void rx_start(struct tendril_net *net) {
    for (size_t i = 0; i < net->num_rx && !net->stopped && !net->suspended;
         i++) {
        if (!net->rx[i].busy && !net->rx[i].waiting) {
            rx_submit(net, &net->rx[i]);
        }
    }
}

static void retry_timer_fn(struct tendril_work *work) {
    struct tendril_net *net = CONTAINER_OF(
        work, struct tendril_net, retry_timer.work
    );

    if (net->suspended) {
        return;
    }
    for (size_t i = 0; i < net->num_rx && !net->stopped; i++) {
        if (net->rx[i].waiting) {
            net->rx[i].waiting = false;
            rx_submit(net, &net->rx[i]);
        }
    }
    if (net->intr_waiting && !net->stopped) {
        net->intr_waiting = false;
        intr_submit(net);
    }
}

/*
 * Moves every frame of `src` to the end of `dst`, to be completed with
 * `status`.
 */
static void frames_move(
    struct tendril_list *dst, struct tendril_list *src, int status
) {
    while (!list_empty(src)) {
        struct tendril_list *node = src->next;
        CONTAINER_OF(node, struct net_frame, node)->status = status;
        list_del(node);
        list_add_tail(dst, node);
    }
}

/*
 * Calls `done` for every frame of a list private to the caller, which
 * callbacks cannot reach.
 */
static void frames_complete(struct tendril_list *list) {
    while (!list_empty(list)) {
        struct net_frame *frame = CONTAINER_OF(
            list->next, struct net_frame, node
        );
        void (*done)(void *ctx, int status) = frame->done;
        void *ctx = frame->ctx;
        int status = frame->status;

        list_del(&frame->node);
        tendril_free(frame, sizeof(struct net_frame));
        if (done != NULL) {
            done(ctx, status);
        }
    }
}

/* Puts a slot's unsent frames back at the head of the queue, in order. */
static void frames_requeue(struct tendril_net *net, struct net_tx *tx) {
    while (!list_empty(&tx->frames)) {
        struct tendril_list *node = tx->frames.prev;
        list_del(node);
        list_add_head(&net->frames, node);
    }
}

static struct net_tx *tx_free_slot(struct tendril_net *net) {
    for (size_t i = 0; i < net->num_tx; i++) {
        if (!net->tx[i].busy) {
            return &net->tx[i];
        }
    }
    return NULL;
}

static bool tx_any_busy(struct tendril_net *net) {
    for (size_t i = 0; i < net->num_tx; i++) {
        if (net->tx[i].busy) {
            return true;
        }
    }
    return false;
}

/*
 * The lowest offset from `off` where (offset + 14) % divisor == remainder: the
 * remainder applies to the payload after the Ethernet header (CDC NCM 1.0
 * Errata 1, 3.3.4 and Table 6-3). OpenBSD's sys/dev/usb/if_uncm.c (ISC
 * license) applies it to the start of the datagram instead; this follows the
 * specification's text.
 */
static size_t datagram_offset(const struct tendril_net *net, size_t off) {
    size_t divisor = net->ndp_out_divisor;
    size_t at = (off + ETH_HEADER_LEN) % divisor;

    return off + (net->ndp_out_remainder + divisor - at) % divisor;
}

/*
 * Packs queued frames into an NTB-16 in tx->buf (CDC NCM 1.0 Errata 1, 3) and
 * returns its length, or 0 if the first frame does not fit even alone.
 */
static size_t ntb_build(struct tendril_net *net, struct net_tx *tx) {
    uint8_t *b = tx->buf;
    size_t align = net->ndp_out_align;
    size_t off = NTH16_LEN;
    uint16_t pos[NTB_MAX_DATAGRAMS];
    uint16_t lens[NTB_MAX_DATAGRAMS];
    size_t count = 0;

    while (!list_empty(&net->frames) && count < net->ntb_out_datagrams) {
        struct net_frame *frame = CONTAINER_OF(
            net->frames.next, struct net_frame, node
        );
        size_t at = datagram_offset(net, off);
        size_t end = at + frame->len;
        /*
         * The NDP16 after this datagram, with one entry more and the null
         * entry.
         */
        size_t need = ALIGN_UP(end, align) + NDP16_HEADER_LEN
            + NDP16_ENTRY_LEN * ((size_t)count + 2);
        if (need > net->ntb_out_size) {
            break;
        }
        memset(b + off, 0, at - off);
        memcpy(b + at, frame->data, frame->len);
        pos[count] = (uint16_t)at;
        lens[count] = (uint16_t)frame->len;
        count++;
        off = end;
        list_del(&frame->node);
        list_add_tail(&tx->frames, &frame->node);
    }
    if (count == 0) {
        struct tendril_list *node = net->frames.next;
        list_del(node);
        list_add_tail(&tx->frames, node);
        return 0;
    }

    size_t ndp = ALIGN_UP(off, align);
    size_t ndp_len = NDP16_HEADER_LEN + NDP16_ENTRY_LEN * ((size_t)count + 1);
    size_t total = ndp + ndp_len;
    memset(b + off, 0, ndp - off);

    /* NTH16 (3.2.1, Table 3-1) */
    put_le32(b, NTH16_SIGNATURE);
    put_le16(b + 4, NTH16_LEN);
    put_le16(b + 6, net->ntb_sequence);
    put_le16(b + 8, (uint16_t)total);
    put_le16(b + 10, (uint16_t)ndp);
    net->ntb_sequence = (uint16_t)(net->ntb_sequence + 1);

    /* NDP16 (3.3.1, Table 3-3), ended by a null entry (3.7) */
    uint8_t *p = b + ndp;
    put_le32(p, NDP16_SIGNATURE);
    put_le16(p + 4, (uint16_t)ndp_len);
    put_le16(p + 6, 0);
    for (size_t i = 0; i < count; i++) {
        put_le16(p + NDP16_HEADER_LEN + NDP16_ENTRY_LEN * i, pos[i]);
        put_le16(p + NDP16_HEADER_LEN + NDP16_ENTRY_LEN * i + 2, lens[i]);
    }
    memset(p + NDP16_HEADER_LEN + NDP16_ENTRY_LEN * count, 0, NDP16_ENTRY_LEN);
    return total;
}

/*
 * Wraps queued frames in REMOTE_NDIS_PACKET_MSG messages in tx->buf, as many
 * as MaxPacketsPerTransfer and the buffer, at most the device's
 * MaxTransferSize, allow; each but the last is padded with zeros to a multiple
 * of 2^PacketAlignmentFactor bytes ([MS-RNDIS] v20130118, 2.2.14). Returns the
 * transfer's length, or 0 if the first frame does not fit even alone.
 */
static size_t rndis_build(struct tendril_net *net, struct net_tx *tx) {
    uint8_t *b = tx->buf;
    size_t align = (size_t)1 << net->rndis_align;
    size_t off = 0;
    size_t prev = 0;
    size_t count = 0;

    while (!list_empty(&net->frames) && count < net->rndis_max_packets) {
        struct net_frame *frame = CONTAINER_OF(
            net->frames.next, struct net_frame, node
        );
        size_t start = 0;
        if (count != 0) {
            start = prev + ALIGN_UP(off - prev, align);
        }
        size_t end = start + RNDIS_PACKET_HEADER_LEN + frame->len;
        if (end > net->tx_buf_size) {
            break;
        }
        if (count != 0) {
            memset(b + off, 0, start - off);
            put_le32(b + prev + 4, (uint32_t)(start - prev));
        }
        /*
         * DataOffset 36 puts the frame right after the header; the other
         * fields stay 0.
         */
        memset(b + start, 0, RNDIS_PACKET_HEADER_LEN);
        put_le32(b + start, RNDIS_PACKET_MSG);
        put_le32(
            b + start + 4, (uint32_t)(RNDIS_PACKET_HEADER_LEN + frame->len)
        );
        put_le32(b + start + 8, RNDIS_PACKET_HEADER_LEN - 8);
        put_le32(b + start + 12, (uint32_t)frame->len);
        memcpy(b + start + RNDIS_PACKET_HEADER_LEN, frame->data, frame->len);
        prev = start;
        off = end;
        count++;
        list_del(&frame->node);
        list_add_tail(&tx->frames, &frame->node);
    }
    if (count == 0) {
        struct tendril_list *node = net->frames.next;
        list_del(node);
        list_add_tail(&tx->frames, node);
    }
    return off;
}

static void tx_done(struct tendril_transfer *xfer);

/*
 * Starts a transfer from the queue on a free slot; on failure the slot's
 * frames are not sent.
 */
static int tx_submit(struct tendril_net *net, struct net_tx *tx) {
    struct tendril_transfer *xfer = &tx->xfer;
    uint32_t flags = TENDRIL_XFER_ZERO_PACKET;
    void *buf;
    size_t len;
    int status;

    if (net->kind == TENDRIL_NET_NCM) {
        len = ntb_build(net, tx);
        if (len == 0) {
            return TENDRIL_ERR_INVALID;
        }
        buf = tx->buf;
        /*
         * Only an NTB of exactly the maximum size may end without a short
         * packet (CDC NCM 1.0 Errata 1, 3.2.1).
         */
        if (len == net->ntb_out_max) {
            flags = 0;
        }
    } else if (net->kind == TENDRIL_NET_RNDIS) {
        /*
         * A transfer that is a multiple of the packet size ends with a
         * zero-length packet, as OpenBSD's sys/dev/usb/if_urndis.c (ISC
         * license) forces short transfers.
         */
        len = rndis_build(net, tx);
        if (len == 0) {
            return TENDRIL_ERR_INVALID;
        }
        buf = tx->buf;
    } else {
        /*
         * One frame per transfer, ended by a short or zero-length packet (CDC
         * ECM 1.2, 3.3.1).
         */
        struct net_frame *frame = CONTAINER_OF(
            net->frames.next, struct net_frame, node
        );
        list_del(&frame->node);
        list_add_tail(&tx->frames, &frame->node);
        len = frame->len;
        if (net->crc32) {
            /* Followed by its CRC on the wire (clean-room specification). */
            memcpy(tx->buf, frame->data, len);
            put_le32(tx->buf + len, ether_crc32(frame->data, len));
            len += ETH_CRC_LEN;
            buf = tx->buf;
        } else {
            buf = (void *)(uintptr_t)frame->data;
        }
    }

    tendril_transfer_init(xfer, net->out_ep);
    xfer->buffer = buf;
    xfer->length = len;
    xfer->flags = flags;
    xfer->callback = tx_done;
    xfer->context = tx;
    status = tendril_transfer_submit(xfer);
    if (status == TENDRIL_OK) {
        tx->busy = true;
    }
    return status;
}

/*
 * Starts transfers while slots are free, moving frames that cannot go out to
 * `completed`, followed by the queue once the adapter has stopped and no
 * transfer is left.
 */
static void tx_start(struct tendril_net *net, struct tendril_list *completed) {
    while (net->ready && !net->stopped && !net->suspended && !net->tx_clearing
        && !list_empty(&net->frames)) {
        struct net_tx *tx = tx_free_slot(net);
        if (tx == NULL) {
            break;
        }
        int status = tx_submit(net, tx);
        if (status == TENDRIL_ERR_SUSPENDED) {
            /* Not sent: the frames wait for the resume. */
            frames_requeue(net, tx);
            break;
        }
        if (status != TENDRIL_OK) {
            if (status == TENDRIL_ERR_NO_DEVICE) {
                net->stopped = true;
            }
            frames_move(completed, &tx->frames, status);
        }
    }
    if (net->stopped && !tx_any_busy(net)) {
        frames_move(completed, &net->frames, TENDRIL_ERR_NO_DEVICE);
    }
}

static void tx_kick(struct tendril_net *net) {
    struct tendril_list completed;

    list_init(&completed);
    tx_start(net, &completed);
    frames_complete(&completed);
}

static void tx_clear_done(void *ctx, int status) {
    struct tendril_net *net = ctx;

    (void)status;
    net->tx_clearing = false;
    tx_kick(net);
}

static void tx_done(struct tendril_transfer *xfer) {
    struct net_tx *tx = xfer->context;
    struct tendril_net *net = tx->net;
    int status = xfer->status;
    struct tendril_list completed;

    list_init(&completed);
    tx->busy = false;
    if (status == TENDRIL_ERR_SUSPENDED) {
        /* Not sent: the frames wait for the resume. */
        frames_requeue(net, tx);
    } else {
        frames_move(&completed, &tx->frames, status);
    }
    if (status == TENDRIL_ERR_NO_DEVICE) {
        net->stopped = true;
    } else if (status == TENDRIL_ERR_STALL && !net->stopped
            && !net->tx_clearing) {
        net->tx_clearing = true;
        if (tendril_endpoint_clear_halt(net->out_ep, tx_clear_done, net)
         != TENDRIL_OK) {
            net->tx_clearing = false;
        }
    }
    /* Further transfers start before the completed frames' callbacks run. */
    tx_start(net, &completed);
    frames_complete(&completed);
}

int tendril_net_send(
    struct tendril_net *net, const void *frame, size_t len,
    void (*done)(void *ctx, int status), void *ctx
) {
    struct net_frame *entry;

    if (net->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (frame == NULL || len < ETH_HEADER_LEN || len > net->max_frame) {
        return TENDRIL_ERR_INVALID;
    }
    entry = tendril_zalloc(sizeof(struct net_frame));
    if (entry == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    entry->data = frame;
    entry->len = len;
    entry->done = done;
    entry->ctx = ctx;
    list_add_tail(&net->frames, &entry->node);
    tx_kick(net);
    return TENDRIL_OK;
}

static void op_done(struct tendril_transfer *xfer);

static void op_finish(
    struct tendril_net *net, struct net_op *op, int status, const uint8_t *msg,
    size_t len
) {
    void (*answer)(struct tendril_net *, int, const uint8_t *, size_t)
        = op->answer;
    void (*done)(void *, int) = op->done;
    void *ctx = op->ctx;

    tendril_free(op->data, op->length);
    tendril_free(op, sizeof(struct net_op));
    if (answer != NULL) {
        answer(net, status, msg, len);
    }
    if (done != NULL) {
        done(ctx, status);
    }
}

static void ops_fail(struct tendril_net *net) {
    struct tendril_list failed;

    list_init(&failed);
    while (!list_empty(&net->ops)) {
        struct tendril_list *node = net->ops.next;
        list_del(node);
        list_add_tail(&failed, node);
    }
    while (!list_empty(&failed)) {
        struct net_op *op = CONTAINER_OF(failed.next, struct net_op, node);
        list_del(&op->node);
        op_finish(net, op, TENDRIL_ERR_NO_DEVICE, NULL, 0);
    }
}

/*
 * Operations run one at a time, in queue order: RNDIS's from the start of
 * setup, as setup itself goes through them, the others once setup has
 * finished.
 */
static void op_kick(struct tendril_net *net) {
    while ((net->ready || net->kind == TENDRIL_NET_RNDIS) && !net->stopped
        && !net->suspended && net->op == NULL && !net->ctrl_busy
        && !list_empty(&net->ops)) {
        struct net_op *op = CONTAINER_OF(net->ops.next, struct net_op, node);
        int status;

        list_del(&op->node);
        if (net->kind == TENDRIL_NET_RNDIS) {
            net->op = op;
            status = rndis_send(net, op);
            if (status == TENDRIL_OK) {
                break;
            }
            net->op = NULL;
        } else {
            status = ctrl_submit(
                net, REQ_OUT, op->request, op->value, op->data, op->length,
                op_done
            );
            if (status == TENDRIL_OK) {
                net->op = op;
                break;
            }
        }
        if (status == TENDRIL_ERR_SUSPENDED) {
            /* Not sent: it waits for the resume. */
            list_add_head(&net->ops, &op->node);
            break;
        }
        if (status == TENDRIL_ERR_NO_DEVICE) {
            net->stopped = true;
        }
        op_finish(net, op, status, NULL, 0);
    }
    if (net->stopped && net->op == NULL) {
        ops_fail(net);
    }
}

static void op_done(struct tendril_transfer *xfer) {
    struct tendril_net *net = xfer->context;
    struct net_op *op = net->op;
    int status = xfer->status;

    net->op = NULL;
    if (status == TENDRIL_ERR_SUSPENDED && op != NULL) {
        /* Not sent: it waits for the resume. */
        list_add_head(&net->ops, &op->node);
        return;
    }
    if (status == TENDRIL_ERR_NO_DEVICE) {
        net->stopped = true;
    }
    if (op != NULL) {
        op_finish(net, op, status, NULL, 0);
    }
    op_kick(net);
}

static int op_queue(
    struct tendril_net *net, uint8_t request, uint16_t value,
    const uint8_t *data, size_t length, void (*done)(void *, int), void *ctx
) {
    struct net_op *op = tendril_zalloc(sizeof(struct net_op));

    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    if (length != 0) {
        op->data = tendril_zalloc(length);
        if (op->data == NULL) {
            tendril_free(op, sizeof(struct net_op));
            return TENDRIL_ERR_NO_MEMORY;
        }
        memcpy(op->data, data, length);
    }
    op->request = request;
    op->value = value;
    op->length = (uint16_t)length;
    op->done = done;
    op->ctx = ctx;
    list_add_tail(&net->ops, &op->node);
    op_kick(net);
    return TENDRIL_OK;
}

/*
 * RNDIS control messages go out by SEND_ENCAPSULATED_COMMAND and come back by
 * GET_ENCAPSULATED_RESPONSE (CDC 1.2, 6.2.1 and 6.2.2), one request at a time,
 * through the operation queue.
 */

static struct net_op *rndis_alloc(uint32_t type, size_t len) {
    struct net_op *op = tendril_zalloc(sizeof(struct net_op));

    if (op == NULL) {
        return NULL;
    }
    op->data = tendril_zalloc(len);
    if (op->data == NULL) {
        tendril_free(op, sizeof(struct net_op));
        return NULL;
    }
    op->length = (uint16_t)len;
    op->rndis_type = type;
    put_le32(op->data, type);
    put_le32(op->data + 4, (uint32_t)len);
    return op;
}

/* A request with the next RequestID, never 0 ([MS-RNDIS] v20130118, 2.2.2). */
static struct net_op *rndis_request(
    struct tendril_net *net, uint32_t type, size_t len,
    void (*answer)(struct tendril_net *, int, const uint8_t *, size_t)
) {
    struct net_op *op = rndis_alloc(type, len);

    if (op == NULL) {
        return NULL;
    }
    op->rndis_id = net->rndis_next_id;
    op->answer = answer;
    put_le32(op->data + 8, op->rndis_id);
    net->rndis_next_id++;
    if (net->rndis_next_id == 0) {
        net->rndis_next_id = 1;
    }
    return op;
}

static void rndis_queue(struct tendril_net *net, struct net_op *op) {
    list_add_tail(&net->ops, &op->node);
    op_kick(net);
}

/*
 * Ends the exchange, completing the outstanding request, if any; `msg` is its
 * completion.
 */
static void rndis_finish(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    struct net_op *op = net->op;

    net->op = NULL;
    net->ctrl_busy = false;
    net->rndis_timed_out = false;
    hc_lock(net->hc);
    tendril_timer_cancel(net->hc, &net->rndis_read_timer);
    tendril_timer_cancel(net->hc, &net->rndis_timeout_timer);
    hc_unlock(net->hc);
    if (status == TENDRIL_ERR_NO_DEVICE) {
        net->stopped = true;
    }
    if (op != NULL) {
        op_finish(net, op, status, msg, len);
    }
    op_kick(net);
}

static void rndis_read_done(struct tendril_transfer *xfer);

/*
 * Nothing to read: a request reads again 10 ms later, while unsolicited
 * reading ends.
 */
static void rndis_read_later(struct tendril_net *net) {
    if (net->op == NULL) {
        net->ctrl_busy = false;
        op_kick(net);
        return;
    }
    hc_lock(net->hc);
    tendril_timer_arm(
        net->hc, &net->rndis_read_timer, RNDIS_REREAD_MS * NS_PER_MS
    );
    hc_unlock(net->hc);
}

static void rndis_read(struct tendril_net *net) {
    int status = ctrl_submit(
        net, REQ_IN, GET_ENCAPSULATED_RESPONSE, 0, net->rndis_buf,
        RNDIS_RESPONSE_SIZE, rndis_read_done
    );

    if (status == TENDRIL_OK) {
        net->ctrl_pending = true;
    } else if (status == TENDRIL_ERR_NO_DEVICE) {
        rndis_finish(net, status, NULL, 0);
    } else {
        rndis_read_later(net);
    }
}

/*
 * Reads unsolicited messages until a read finds nothing; a request's exchange
 * reads them anyway.
 */
static void rndis_poll(struct tendril_net *net) {
    if (net->op != NULL || net->ctrl_busy || net->stopped || net->suspended) {
        return;
    }
    net->ctrl_busy = true;
    rndis_read(net);
}

static void rndis_send_done(struct tendril_transfer *xfer) {
    struct tendril_net *net = xfer->context;
    int status = xfer->status;

    net->ctrl_pending = false;
    if (net->rndis_timed_out) {
        status = TENDRIL_ERR_TIMEOUT;
    }
    if (status != TENDRIL_OK || net->op == NULL || net->op->no_reply) {
        rndis_finish(net, status, NULL, 0);
        return;
    }
    /* BEGIN OpenBSD sys/dev/usb/if_urndis.c, FreeBSD sys/dev/usb/net/if_urndis.c */
    /*
     * The answer is read at once, as OpenBSD's and FreeBSD's if_urndis.c (ISC
     * license) do.
     */
    rndis_read(net);
    /* END OpenBSD sys/dev/usb/if_urndis.c, FreeBSD sys/dev/usb/net/if_urndis.c */
}

/* Starts the exchange of net->op. */
static int rndis_send(struct tendril_net *net, struct net_op *op) {
    int status = ctrl_submit(
        net, REQ_OUT, SEND_ENCAPSULATED_COMMAND, 0, op->data, op->length,
        rndis_send_done
    );

    if (status == TENDRIL_OK) {
        net->ctrl_busy = true;
        net->ctrl_pending = true;
        net->rndis_timed_out = false;
        hc_lock(net->hc);
        tendril_timer_arm(
            net->hc, &net->rndis_timeout_timer, RNDIS_TIMEOUT_MS * NS_PER_MS
        );
        hc_unlock(net->hc);
    }
    return status;
}

static void rndis_read_timer_fn(struct tendril_work *work) {
    struct tendril_net *net = CONTAINER_OF(
        work, struct tendril_net, rndis_read_timer.work
    );

    if (net->op == NULL || !net->ctrl_busy || net->ctrl_pending) {
        return;
    }
    if (net->stopped) {
        rndis_finish(net, TENDRIL_ERR_NO_DEVICE, NULL, 0);
        return;
    }
    rndis_read(net);
}

/*
 * Windows' ControlTimeoutTimer ([MS-RNDIS] v20130118, 3.1.2, footnote <17>).
 */
static void rndis_timeout_fn(struct tendril_work *work) {
    struct tendril_net *net = CONTAINER_OF(
        work, struct tendril_net, rndis_timeout_timer.work
    );

    if (net->op == NULL || !net->ctrl_busy) {
        return;
    }
    net->rndis_timed_out = true;
    if (net->ctrl_pending) {
        tendril_transfer_cancel(&net->ctrl);
    } else {
        rndis_finish(net, TENDRIL_ERR_TIMEOUT, NULL, 0);
    }
}

/*
 * The shortest MessageLength accepted from the device, by message type
 * ([MS-RNDIS] v20130118, 2.2).
 */
static uint32_t rndis_min_len(uint32_t type) {
    switch (type) {
        case RNDIS_HALT_MSG:
        case RNDIS_KEEPALIVE_MSG: {
            return 12;
        }
        case RNDIS_INDICATE_STATUS_MSG: {
            return 20;
        }
        case RNDIS_QUERY_MSG | RNDIS_CMPLT: {
            return 24;
        }
        case RNDIS_INITIALIZE_MSG | RNDIS_CMPLT: {
            /*
             * The fields through PacketAlignmentFactor: 2.2.9 says 0x30 but
             * lists 52 bytes.
             */
            return 44;
        }
        case RNDIS_SET_MSG | RNDIS_CMPLT:
        case RNDIS_RESET_MSG | RNDIS_CMPLT:
        case RNDIS_KEEPALIVE_MSG | RNDIS_CMPLT: {
            return 16;
        }
        default: {
            return 8;
        }
    }
}

/* A reset carries no RequestID, so its completion matches by type alone. */
static bool rndis_answers(
    const struct net_op *op, uint32_t type, const uint8_t *msg
) {
    if (type != (op->rndis_type | RNDIS_CMPLT)) {
        return false;
    }
    return op->rndis_type == RNDIS_RESET_MSG
        || get_le32(msg + 8) == op->rndis_id;
}

static int rndis_status(uint32_t status) {
    if (status == RNDIS_STATUS_SUCCESS) {
        return TENDRIL_OK;
    }
    if (status == RNDIS_STATUS_NOT_SUPPORTED) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    return TENDRIL_ERR_IO;
}

/* REMOTE_NDIS_INDICATE_STATUS_MSG ([MS-RNDIS] v20130118, 2.2.7) */
static void rndis_indicate(struct tendril_net *net, const uint8_t *msg) {
    uint32_t status = get_le32(msg + 8);

    if (status == RNDIS_STATUS_MEDIA_CONNECT) {
        link_set(net, true, net->rx_bps, net->tx_bps);
    } else if (status == RNDIS_STATUS_MEDIA_DISCONNECT) {
        link_set(net, false, net->rx_bps, net->tx_bps);
    } else {
        LOG_DEBUG(
            "net: interface %u: RNDIS status %08x",
            (unsigned int)net->ctrl_intf->number, (unsigned int)status
        );
    }
}

/*
 * Answers the device's REMOTE_NDIS_KEEPALIVE_MSG ([MS-RNDIS] v20130118,
 * 3.1.5.9). It goes out once the current exchange ends.
 */
static void rndis_keepalive_reply(struct tendril_net *net, uint32_t id) {
    struct net_op *op = rndis_alloc(
        RNDIS_KEEPALIVE_MSG | RNDIS_CMPLT, RNDIS_KEEPALIVE_CMPLT_LEN
    );

    if (op == NULL) {
        return;
    }
    put_le32(op->data + 8, id);
    put_le32(op->data + 12, RNDIS_STATUS_SUCCESS);
    op->no_reply = true;
    list_add_tail(&net->ops, &op->node);
}

static void rndis_read_done(struct tendril_transfer *xfer) {
    struct tendril_net *net = xfer->context;
    struct net_op *op = net->op;
    const uint8_t *msg = net->rndis_buf;
    uint32_t type;
    uint32_t len;

    net->ctrl_pending = false;
    if (net->rndis_timed_out && op != NULL) {
        rndis_finish(net, TENDRIL_ERR_TIMEOUT, NULL, 0);
        return;
    }
    if (xfer->status == TENDRIL_ERR_NO_DEVICE || net->stopped) {
        rndis_finish(net, TENDRIL_ERR_NO_DEVICE, NULL, 0);
        return;
    }
    /* Nothing there yet. */
    if (xfer->status != TENDRIL_OK || xfer->actual < 8
     || get_le32(msg + 4) < 8) {
        rndis_read_later(net);
        return;
    }

    net->rndis_last_rx = tendril_now();
    type = get_le32(msg);
    len = get_le32(msg + 4);
    if (len <= xfer->actual && len >= rndis_min_len(type)) {
        switch (type) {
            case RNDIS_INDICATE_STATUS_MSG: {
                rndis_indicate(net, msg);
                break;
            }
            case RNDIS_KEEPALIVE_MSG: {
                rndis_keepalive_reply(net, get_le32(msg + 8));
                break;
            }
            case RNDIS_HALT_MSG: {
                /* The device halted ([MS-RNDIS] v20130118, 3.1.5.7). */
                LOG_WARN(
                    "net: interface %u: the RNDIS device halted",
                    (unsigned int)net->ctrl_intf->number
                );
                net->stopped = true;
                rndis_finish(net, TENDRIL_ERR_IO, NULL, 0);
                return;
            }
            default: {
                if (op != NULL && rndis_answers(op, type, msg)) {
                    size_t at = op->rndis_type == RNDIS_RESET_MSG ? 8 : 12;
                    rndis_finish(
                        net, rndis_status(get_le32(msg + at)), msg, len
                    );
                    return;
                }
                break;
            }
        }
    }

    /*
     * A callback above may have run the controller and ended this exchange
     * meanwhile.
     */
    if (net->op != op || !net->ctrl_busy || net->ctrl_pending) {
        return;
    }
    rndis_read(net);
}

/*
 * The data of a REMOTE_NDIS_QUERY_CMPLT: InformationBufferLength bytes at
 * InformationBufferOffset from the RequestID field ([MS-RNDIS] v20130118,
 * 2.2.10), within MessageLength.
 */
static bool rndis_query_data(
    const uint8_t *msg, size_t len, const uint8_t **data, size_t *data_len
) {
    uint32_t info_len;
    uint32_t info_off;

    if (msg == NULL || len < 24) {
        return false;
    }
    info_len = get_le32(msg + 16);
    info_off = get_le32(msg + 20);
    if (info_off > len - 8 || info_len > len - 8 - info_off) {
        return false;
    }
    *data = msg + 8 + info_off;
    *data_len = info_len;
    return true;
}

static bool rndis_answer_u32(
    int status, const uint8_t *msg, size_t len, uint32_t *value
) {
    const uint8_t *data;
    size_t data_len;

    if (status != TENDRIL_OK || !rndis_query_data(msg, len, &data, &data_len)
     || data_len < 4) {
        return false;
    }
    *value = get_le32(data);
    return true;
}

/*
 * REMOTE_NDIS_SET_MSG with its data right after the header ([MS-RNDIS]
 * v20130118, 2.2.5).
 */
static struct net_op *rndis_set_request(
    struct tendril_net *net, uint32_t oid, const uint8_t *data, size_t len,
    void (*answer)(struct tendril_net *, int, const uint8_t *, size_t)
) {
    struct net_op *op = rndis_request(
        net, RNDIS_SET_MSG, RNDIS_SET_LEN + len, answer
    );

    if (op == NULL) {
        return NULL;
    }
    put_le32(op->data + 12, oid);
    put_le32(op->data + 16, (uint32_t)len);
    if (len != 0) {
        /* Counted from RequestID; 0 when there is no data. */
        put_le32(op->data + 20, RNDIS_SET_LEN - 8);
        memcpy(op->data + RNDIS_SET_LEN, data, len);
    }
    return op;
}

/* Whether setup cannot go on: the device is gone, or the adapter stopped. */
static bool rndis_setup_over(struct tendril_net *net, int status) {
    if (status == TENDRIL_ERR_NO_DEVICE) {
        net->stopped = true;
    }
    return net->stopped;
}

/* REMOTE_NDIS_QUERY_MSG without input data ([MS-RNDIS] v20130118, 2.2.4). */
static void rndis_query_step(
    struct tendril_net *net, uint32_t oid,
    void (*answer)(struct tendril_net *, int, const uint8_t *, size_t)
) {
    struct net_op *op = rndis_request(
        net, RNDIS_QUERY_MSG, RNDIS_QUERY_LEN, answer
    );

    if (op == NULL) {
        setup_fail(net, "allocating an RNDIS message", TENDRIL_ERR_NO_MEMORY);
        return;
    }
    put_le32(op->data + 12, oid);
    rndis_queue(net, op);
}

static void rndis_filter_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    (void)msg;
    (void)len;
    if (rndis_setup_over(net, status)) {
        return;
    }
    if (status != TENDRIL_OK) {
        setup_fail(net, "setting the RNDIS packet filter", status);
        return;
    }
    /* RNDIS_DATA_INITIALIZED ([MS-RNDIS] v20130118, 3.1.3, step 4) */
    setup_finish(net);
}

/*
 * Directed, broadcast, and all multicast frames, as the attach callback
 * promises.
 */
static void rndis_filter_step(struct tendril_net *net) {
    uint8_t value[4];
    struct net_op *op;

    net->rndis_filter = NDIS_PACKET_TYPE_DIRECTED | NDIS_PACKET_TYPE_BROADCAST
        | NDIS_PACKET_TYPE_ALL_MULTICAST;
    put_le32(value, net->rndis_filter);
    op = rndis_set_request(
        net, OID_GEN_CURRENT_PACKET_FILTER, value, sizeof(value),
        rndis_filter_answer
    );
    if (op == NULL) {
        setup_fail(net, "allocating an RNDIS message", TENDRIL_ERR_NO_MEMORY);
        return;
    }
    rndis_queue(net, op);
}

/*
 * OID_GEN_LINK_SPEED, in units of 100 bit/s, for both directions; 0 if
 * unknown.
 */
static void rndis_speed_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    uint32_t speed;

    if (rndis_setup_over(net, status)) {
        return;
    }
    if (rndis_answer_u32(status, msg, len, &speed)) {
        uint32_t bps = speed > UINT32_MAX / 100 ? UINT32_MAX : speed * 100;
        net->rx_bps = bps;
        net->tx_bps = bps;
    }
    rndis_filter_step(net);
}

/* OID_GEN_MEDIA_CONNECT_STATUS: 0 connected, 1 disconnected. */
static void rndis_media_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    uint32_t state;

    if (rndis_setup_over(net, status)) {
        return;
    }
    /*
     * Without an answer the link counts as up: many devices, phones among
     * them, have no medium to lose.
     */
    net->link_up = !rndis_answer_u32(status, msg, len, &state) || state == 0;
    if (net->link_up) {
        rndis_query_step(net, OID_GEN_LINK_SPEED, rndis_speed_answer);
    } else {
        rndis_filter_step(net);
    }
}

/*
 * OID_802_3_MAXIMUM_LIST_SIZE: how many multicast addresses the device takes,
 * 0 if unknown.
 */
static void rndis_list_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    uint32_t size;

    if (rndis_setup_over(net, status)) {
        return;
    }
    net->mc_filters = 0;
    if (rndis_answer_u32(status, msg, len, &size)) {
        net->mc_filters = (uint16_t)MIN(size, (uint32_t)RNDIS_MAX_MC);
    }
    rndis_query_step(net, OID_GEN_MEDIA_CONNECT_STATUS, rndis_media_answer);
}

/*
 * OID_GEN_MAXIMUM_FRAME_SIZE: the largest payload, without the Ethernet
 * header.
 */
static void rndis_frame_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    size_t cap = MIN(
        (size_t)net->rndis_max_transfer, (size_t)RNDIS_MAX_TRANSFER
    ) - RNDIS_PACKET_HEADER_LEN;
    uint32_t payload;

    if (rndis_setup_over(net, status)) {
        return;
    }
    if (rndis_answer_u32(status, msg, len, &payload)) {
        if (payload > RNDIS_MAX_FRAME - ETH_HEADER_LEN) {
            net->max_frame = RNDIS_MAX_FRAME;
        } else {
            net->max_frame = MAX(
                (size_t)payload + ETH_HEADER_LEN, (size_t)RNDIS_MIN_FRAME
            );
        }
    }
    /* A frame must also fit into one transfer to the device. */
    net->max_frame = MIN(net->max_frame, cap);
    rndis_query_step(net, OID_802_3_MAXIMUM_LIST_SIZE, rndis_list_answer);
}

/* An answer of exactly 6 bytes is the station address. */
static bool rndis_take_mac(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    const uint8_t *data;
    size_t data_len;

    if (status != TENDRIL_OK || !rndis_query_data(msg, len, &data, &data_len)
     || data_len != MAC_LEN) {
        return false;
    }
    memcpy(net->mac, data, MAC_LEN);
    return true;
}

static void rndis_cur_addr_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    if (rndis_setup_over(net, status)) {
        return;
    }
    if (!rndis_take_mac(net, status, msg, len)) {
        setup_fail(
            net, "reading the station address",
            status == TENDRIL_OK ? TENDRIL_ERR_PROTOCOL : status
        );
        return;
    }
    rndis_query_step(net, OID_GEN_MAXIMUM_FRAME_SIZE, rndis_frame_answer);
}

static void rndis_perm_addr_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    if (rndis_setup_over(net, status)) {
        return;
    }
    if (rndis_take_mac(net, status, msg, len)) {
        rndis_query_step(net, OID_GEN_MAXIMUM_FRAME_SIZE, rndis_frame_answer);
    } else {
        rndis_query_step(
            net, OID_802_3_CURRENT_ADDRESS, rndis_cur_addr_answer
        );
    }
}

/* REMOTE_NDIS_INITIALIZE_CMPLT ([MS-RNDIS] v20130118, 2.2.9) */
static void rndis_init_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    (void)len;
    if (rndis_setup_over(net, status)) {
        return;
    }
    /* BEGIN OpenBSD sys/dev/usb/if_urndis.c */
    /*
     * 802.3 only, as OpenBSD's sys/dev/usb/if_urndis.c (ISC license) checks.
     */
    if (status == TENDRIL_OK && get_le32(msg + 28) != RNDIS_MEDIUM_802_3) {
        status = TENDRIL_ERR_UNSUPPORTED;
    }
    /* END OpenBSD sys/dev/usb/if_urndis.c */
    if (status != TENDRIL_OK) {
        setup_fail(net, "REMOTE_NDIS_INITIALIZE_MSG", status);
        return;
    }

    net->rndis_max_packets = get_le32(msg + 32);
    net->rndis_max_transfer = get_le32(msg + 36);
    net->rndis_align = get_le32(msg + 40);
    if (net->rndis_max_packets == 0) {
        net->rndis_max_packets = 1;
    }
    if (net->rndis_align > 7) {
        net->rndis_align = 0;
    }
    if (net->rndis_max_transfer < RNDIS_MIN_TRANSFER) {
        LOG_WARN(
            "net: interface %u: RNDIS MaxTransferSize %u is too small",
            (unsigned int)net->ctrl_intf->number,
            (unsigned int)net->rndis_max_transfer
        );
        net->stopped = true;
        return;
    }
    rndis_query_step(net, OID_802_3_PERMANENT_ADDRESS, rndis_perm_addr_answer);
}

static void rndis_alt_done(void *ctx, int status) {
    struct tendril_net *net = ctx;
    struct net_op *op;

    if (status != TENDRIL_OK) {
        setup_fail(net, "selecting the data alternate setting", status);
        return;
    }
    if (!find_endpoints(net)) {
        setup_fail(net, "finding the bulk endpoints", TENDRIL_ERR_UNSUPPORTED);
        return;
    }

    /*
     * Version 1.0, as Windows sends ([MS-RNDIS] v20130118, 2.2.2, footnotes
     * <1> to <3>).
     */
    op = rndis_request(
        net, RNDIS_INITIALIZE_MSG, RNDIS_INITIALIZE_LEN, rndis_init_answer
    );
    if (op == NULL) {
        setup_fail(net, "allocating an RNDIS message", TENDRIL_ERR_NO_MEMORY);
        return;
    }
    put_le32(op->data + 12, 1);
    put_le32(op->data + 16, 0);
    put_le32(op->data + 20, RNDIS_MAX_TRANSFER);
    rndis_queue(net, op);
}

static void rndis_readdress_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    (void)msg;
    (void)len;
    if (status != TENDRIL_OK && status != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN(
            "net: interface %u: restoring the RNDIS filters after a reset "
            "failed (%s)",
            (unsigned int)net->ctrl_intf->number, tendril_status_str(status)
        );
    }
}

/*
 * The reset lost the packet filter and multicast list: set them again
 * ([MS-RNDIS] v20130118, 3.1.5.5).
 */
static void rndis_readdress(struct tendril_net *net) {
    uint8_t value[4];
    struct net_op *op;

    put_le32(value, net->rndis_filter);
    op = rndis_set_request(
        net, OID_GEN_CURRENT_PACKET_FILTER, value, sizeof(value),
        rndis_readdress_answer
    );
    if (op == NULL) {
        rndis_readdress_answer(net, TENDRIL_ERR_NO_MEMORY, NULL, 0);
    } else {
        rndis_queue(net, op);
    }
    if (!net->rndis_mc_set) {
        return;
    }
    op = rndis_set_request(
        net, OID_802_3_MULTICAST_LIST, net->rndis_mc,
        net->rndis_mc_count * MAC_LEN, rndis_readdress_answer
    );
    if (op == NULL) {
        rndis_readdress_answer(net, TENDRIL_ERR_NO_MEMORY, NULL, 0);
    } else {
        rndis_queue(net, op);
    }
}

/* REMOTE_NDIS_RESET_CMPLT ([MS-RNDIS] v20130118, 2.2.12 and 3.1.5.5) */
static void rndis_reset_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    (void)len;
    if (status == TENDRIL_ERR_NO_DEVICE || net->stopped) {
        return;
    }
    if (status != TENDRIL_OK) {
        LOG_WARN(
            "net: interface %u: RNDIS reset failed (%s)",
            (unsigned int)net->ctrl_intf->number, tendril_status_str(status)
        );
        return;
    }
    /* AddressingReset */
    if (get_le32(msg + 12) == 1) {
        rndis_readdress(net);
    }
}

/*
 * REMOTE_NDIS_RESET_MSG, which carries no RequestID ([MS-RNDIS] v20130118,
 * 2.2.6).
 */
static void rndis_reset(struct tendril_net *net) {
    struct net_op *op = rndis_alloc(RNDIS_RESET_MSG, RNDIS_RESET_LEN);

    if (op == NULL) {
        rndis_reset_answer(net, TENDRIL_ERR_NO_MEMORY, NULL, 0);
        return;
    }
    op->answer = rndis_reset_answer;
    rndis_queue(net, op);
}

/*
 * A failed or unsuccessful keepalive resets the device ([MS-RNDIS] v20130118,
 * 3.1.5.4).
 */
static void rndis_keepalive_answer(
    struct tendril_net *net, int status, const uint8_t *msg, size_t len
) {
    (void)msg;
    (void)len;
    if (status != TENDRIL_OK && status != TENDRIL_ERR_NO_DEVICE
     && status != TENDRIL_ERR_SUSPENDED && !net->stopped) {
        rndis_reset(net);
    }
}

static void rndis_keepalive_arm(struct tendril_net *net, uint64_t delay) {
    hc_lock(net->hc);
    tendril_timer_arm(net->hc, &net->rndis_keepalive_timer, delay);
    hc_unlock(net->hc);
}

/*
 * KeepAliveTimer: a REMOTE_NDIS_KEEPALIVE_MSG once the device has been silent
 * for 5 seconds with no request outstanding, as long as Windows waits
 * ([MS-RNDIS] v20130118, 3.1.2 and 3.1.6, footnote <16>).
 */
static void rndis_keepalive_fn(struct tendril_work *work) {
    struct tendril_net *net = CONTAINER_OF(
        work, struct tendril_net, rndis_keepalive_timer.work
    );
    uint64_t period = RNDIS_KEEPALIVE_MS * NS_PER_MS;
    uint64_t now = tendril_now();
    uint64_t idle = now > net->rndis_last_rx ? now - net->rndis_last_rx : 0;

    if (net->stopped || net->suspended) {
        return;
    }
    if (idle < period) {
        rndis_keepalive_arm(net, period - idle);
        return;
    }
    if (net->op == NULL && !net->ctrl_busy) {
        struct net_op *op = rndis_request(
            net, RNDIS_KEEPALIVE_MSG, RNDIS_KEEPALIVE_LEN,
            rndis_keepalive_answer
        );
        if (op != NULL) {
            rndis_queue(net, op);
        }
    }
    rndis_keepalive_arm(net, period);
}

/*
 * Selects the data alternate setting, then initializes the device ([MS-RNDIS]
 * v20130118, 3.1.3).
 */
static void rndis_setup(struct tendril_net *net) {
    int status;

    tendril_timer_init(&net->rndis_read_timer, rndis_read_timer_fn);
    tendril_timer_init(&net->rndis_timeout_timer, rndis_timeout_fn);
    tendril_timer_init(&net->rndis_keepalive_timer, rndis_keepalive_fn);
    net->rndis_next_id = 1;
    if (net->data_intf->cur_alt == net->data_alt_index) {
        rndis_alt_done(net, TENDRIL_OK);
        return;
    }
    status = tendril_interface_set_alt(
        net->data_intf, net->data_alt, rndis_alt_done, net
    );
    if (status != TENDRIL_OK) {
        rndis_alt_done(net, status);
    }
}

/* NDIS packet filter bits for the TENDRIL_NET_FILTER_* ones. */
static uint32_t rndis_filter_bits(uint16_t filter) {
    uint32_t bits = 0;

    if ((filter & TENDRIL_NET_FILTER_PROMISCUOUS) != 0) {
        bits |= NDIS_PACKET_TYPE_PROMISCUOUS;
    }
    if ((filter & TENDRIL_NET_FILTER_ALL_MULTICAST) != 0) {
        bits |= NDIS_PACKET_TYPE_ALL_MULTICAST;
    }
    if ((filter & TENDRIL_NET_FILTER_DIRECTED) != 0) {
        bits |= NDIS_PACKET_TYPE_DIRECTED;
    }
    if ((filter & TENDRIL_NET_FILTER_BROADCAST) != 0) {
        bits |= NDIS_PACKET_TYPE_BROADCAST;
    }
    if ((filter & TENDRIL_NET_FILTER_MULTICAST) != 0) {
        bits |= NDIS_PACKET_TYPE_MULTICAST;
    }
    return bits;
}

static int rndis_set_oid(
    struct tendril_net *net, uint32_t oid, const uint8_t *data, size_t len,
    void (*done)(void *, int), void *ctx
) {
    struct net_op *op = rndis_set_request(net, oid, data, len, NULL);

    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->done = done;
    op->ctx = ctx;
    rndis_queue(net, op);
    return TENDRIL_OK;
}

/*
 * SET_ETHERNET_PACKET_FILTER (CDC ECM 1.2, 6.2.4, Table 8), or
 * OID_GEN_CURRENT_PACKET_FILTER for RNDIS.
 */
int tendril_net_set_filter(
    struct tendril_net *net, uint16_t filter,
    void (*done)(void *ctx, int status), void *ctx
) {
    if (net->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (net->kind == TENDRIL_NET_RNDIS) {
        uint8_t value[4];

        net->rndis_filter = rndis_filter_bits(filter);
        put_le32(value, net->rndis_filter);
        return rndis_set_oid(
            net, OID_GEN_CURRENT_PACKET_FILTER, value, sizeof(value), done, ctx
        );
    }
    if (net->kind == TENDRIL_NET_NCM
     && (net->ncm_caps & NCM_CAP_PACKET_FILTER) == 0) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    return op_queue(
        net, SET_ETHERNET_PACKET_FILTER, filter, NULL, 0, done, ctx
    );
}

/* SET_ETHERNET_MULTICAST_FILTERS (CDC ECM 1.2, 6.2.1) */
int tendril_net_set_multicast(
    struct tendril_net *net, const uint8_t *addrs, size_t count,
    void (*done)(void *ctx, int status), void *ctx
) {
    if (net->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (net->mc_filters == 0) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    if (count > (size_t)net->mc_filters || count * MAC_LEN > 65535
     || (addrs == NULL && count != 0)) {
        return TENDRIL_ERR_INVALID;
    }
    if (net->kind == TENDRIL_NET_RNDIS) {
        /* OID_802_3_MULTICAST_LIST, kept to set again after a reset. */
        if (count != 0) {
            memcpy(net->rndis_mc, addrs, count * MAC_LEN);
        }
        net->rndis_mc_count = count;
        net->rndis_mc_set = true;
        return rndis_set_oid(
            net, OID_802_3_MULTICAST_LIST, addrs, count * MAC_LEN, done, ctx
        );
    }
    return op_queue(
        net, SET_ETHERNET_MULTICAST_FILTERS, (uint16_t)count, addrs,
        count * MAC_LEN, done, ctx
    );
}

static void net_disconnect(struct tendril_interface *intf) {
    struct tendril_net *net = tendril_interface_data(intf);
    struct tendril_list completed;

    if (net == NULL) {
        return;
    }
    net->stopped = true;
    hc_lock(net->hc);
    tendril_timer_cancel(net->hc, &net->retry_timer);
    if (net->kind == TENDRIL_NET_RNDIS) {
        tendril_timer_cancel(net->hc, &net->rndis_read_timer);
        tendril_timer_cancel(net->hc, &net->rndis_timeout_timer);
        tendril_timer_cancel(net->hc, &net->rndis_keepalive_timer);
    }
    hc_unlock(net->hc);

    list_init(&completed);
    for (size_t i = 0; i < MAX_XFERS; i++) {
        frames_move(&completed, &net->tx[i].frames, TENDRIL_ERR_NO_DEVICE);
    }
    frames_move(&completed, &net->frames, TENDRIL_ERR_NO_DEVICE);
    frames_complete(&completed);
    if (net->op != NULL) {
        struct net_op *op = net->op;
        net->op = NULL;
        op_finish(net, op, TENDRIL_ERR_NO_DEVICE, NULL, 0);
    }
    ops_fail(net);

    if (net->attached && net_callbacks->detach != NULL) {
        net_callbacks->detach(net);
    }
    tendril_interface_set_data(intf, NULL);
    for (size_t i = 0; i < MAX_XFERS; i++) {
        tendril_free(net->rx[i].buf, net->rx_buf_size);
        tendril_free(net->tx[i].buf, net->tx_buf_size);
    }
    tendril_free(net, sizeof(struct tendril_net));
}

/*
 * Refuses while setup runs or anything is in flight that cannot simply be
 * stopped: a frame being sent or queued, an operation, or an RNDIS exchange.
 * The core then cancels the receive and interrupt transfers.
 */
static bool net_suspend(struct tendril_interface *intf) {
    struct tendril_net *net = tendril_interface_data(intf);

    if (net == NULL) {
        return true;
    }
    if (!net->stopped
     && (!net->ready || !list_empty(&net->frames) || tx_any_busy(net)
      || net->op != NULL || !list_empty(&net->ops) || net->ctrl_busy)) {
        return false;
    }
    net->suspended = true;
    hc_lock(net->hc);
    tendril_timer_cancel(net->hc, &net->retry_timer);
    if (net->kind == TENDRIL_NET_RNDIS) {
        tendril_timer_cancel(net->hc, &net->rndis_read_timer);
        tendril_timer_cancel(net->hc, &net->rndis_timeout_timer);
        tendril_timer_cancel(net->hc, &net->rndis_keepalive_timer);
    }
    hc_unlock(net->hc);
    return true;
}

/* The device resumed, or its suspend failed: everything starts again. */
static void net_resume(struct tendril_interface *intf) {
    struct tendril_net *net = tendril_interface_data(intf);

    if (net == NULL || !net->suspended) {
        return;
    }
    net->suspended = false;
    for (size_t i = 0; i < net->num_rx; i++) {
        net->rx[i].waiting = false;
    }
    net->rx_errors = 0;
    net->rx_gave_up = false;
    net->intr_waiting = false;
    net->intr_errors = 0;
    net->intr_gave_up = false;
    net->notify_len = 0;
    net->notify_skip = 0;
    rx_start(net);
    intr_start(net);
    if (net->kind == TENDRIL_NET_RNDIS && net->ready && !net->stopped) {
        /*
         * A device that lost its state answers the next keepalive badly, and
         * the reset restores it.
         */
        net->rndis_last_rx = tendril_now();
        hc_lock(net->hc);
        tendril_timer_arm(
            net->hc, &net->rndis_keepalive_timer,
            RNDIS_KEEPALIVE_MS * NS_PER_MS
        );
        hc_unlock(net->hc);
    }
    tx_kick(net);
    op_kick(net);
}

void tendril_net_register(const struct tendril_net_callbacks *callbacks) {
    net_callbacks = callbacks;
    net_driver.name = "net";
    net_driver.probe = net_probe;
    net_driver.disconnect = net_disconnect;
    net_driver.suspend = net_suspend;
    net_driver.resume = net_resume;
    tendril_driver_register(&net_driver);
}

struct send_waiter {
    volatile bool done;
    int status;
};

static void send_sync_done(void *ctx, int status) {
    struct send_waiter *waiter = ctx;

    waiter->status = status;
    waiter->done = true;
}

int tendril_net_send_sync(
    struct tendril_net *net, const void *frame, size_t len
) {
    struct send_waiter waiter = { false, TENDRIL_OK };
    struct tendril_hc *hc = tendril_device_hc(net->dev);
    int status = tendril_net_send(net, frame, len, send_sync_done, &waiter);

    if (status != TENDRIL_OK) {
        return status;
    }
    tendril_hc_wait(hc, &waiter.done);
    return waiter.status;
}

struct tendril_device *tendril_net_device(struct tendril_net *net) {
    return net->dev;
}

int tendril_net_kind(struct tendril_net *net) {
    return net->kind;
}

uint8_t tendril_net_interface(struct tendril_net *net) {
    return net->ctrl_intf->number;
}

const uint8_t *tendril_net_mac(struct tendril_net *net) {
    return net->mac;
}

size_t tendril_net_max_frame(struct tendril_net *net) {
    return net->max_frame;
}

size_t tendril_net_multicast_filters(struct tendril_net *net) {
    return net->mc_filters;
}

bool tendril_net_link_up(struct tendril_net *net) {
    return net->link_up;
}

void tendril_net_set_user(struct tendril_net *net, void *user) {
    net->user = user;
}

void *tendril_net_user(struct tendril_net *net) {
    return net->user;
}
