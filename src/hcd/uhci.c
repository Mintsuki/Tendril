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
 * UHCI 1.1. Registers sit in I/O space; descriptors are little-endian with
 * 32-bit addresses.
 */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb.h and
 * sys/dev/usb/controller/uhci.c (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
 * Copyright (c) 1998 The NetBSD Foundation, Inc. All rights reserved.
 * Copyright (c) 1998 Lennart Augustsson. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Parts of this file are derived from FreeBSD's
 * sys/dev/usb/controller/uhci_pci.c (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (augustss@carlstedt.se) at
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../../include/tendril/glue.h"
#include "../../include/tendril/spec.h"
#include "../../include/tendril/tendril.h"
#include "../internal.h"

#define REG_USBCMD 0x00
#define REG_USBSTS 0x02
#define REG_USBINTR 0x04
#define REG_FRNUM 0x06
#define REG_FLBASEADD 0x08
#define REG_PORTSC(n) ((uint32_t)(0x10 + 2 * ((n) - 1)))

#define CMD_RS ((uint32_t)1 << 0)
#define CMD_HCRESET ((uint32_t)1 << 1)
#define CMD_GRESET ((uint32_t)1 << 2)
#define CMD_CF ((uint32_t)1 << 6)
#define CMD_MAXP ((uint32_t)1 << 7)

#define STS_USBINT ((uint32_t)1 << 0)
#define STS_ERROR ((uint32_t)1 << 1)
#define STS_RESUME ((uint32_t)1 << 2)
#define STS_HSE ((uint32_t)1 << 3)
#define STS_HCPE ((uint32_t)1 << 4)
#define STS_HALTED ((uint32_t)1 << 5)
#define STS_ACK (STS_USBINT | STS_ERROR | STS_RESUME | STS_HSE | STS_HCPE)

#define INTR_TIMEOUT_CRC ((uint32_t)1 << 0)
#define INTR_IOC ((uint32_t)1 << 2)
#define INTR_SHORT ((uint32_t)1 << 3)

#define PORT_CCS ((uint32_t)1 << 0)
#define PORT_CSC ((uint32_t)1 << 1)
#define PORT_PE ((uint32_t)1 << 2)
#define PORT_PEC ((uint32_t)1 << 3)
#define PORT_RD ((uint32_t)1 << 6)
#define PORT_ALWAYS_ONE ((uint32_t)1 << 7)
#define PORT_LSDA ((uint32_t)1 << 8)
#define PORT_PR ((uint32_t)1 << 9)
#define PORT_SUSP ((uint32_t)1 << 12)
/* Change bits, cleared by writing one, so never written back by accident. */
#define PORT_RWC (PORT_CSC | PORT_PEC)

#define LINK_T ((uint32_t)1)
#define LINK_QH ((uint32_t)1 << 1)
#define LINK_VF ((uint32_t)1 << 2)

#define TD_STATUS_BITSTUFF ((uint32_t)1 << 17)
#define TD_STATUS_CRC_TIMEOUT ((uint32_t)1 << 18)
#define TD_STATUS_BABBLE ((uint32_t)1 << 20)
#define TD_STATUS_DBE ((uint32_t)1 << 21)
#define TD_STATUS_STALLED ((uint32_t)1 << 22)
#define TD_ACTIVE ((uint32_t)1 << 23)
#define TD_IOC ((uint32_t)1 << 24)
#define TD_IOS ((uint32_t)1 << 25)
#define TD_LS ((uint32_t)1 << 26)
#define TD_CERR(n) ((uint32_t)(n) << 27)
#define TD_SPD ((uint32_t)1 << 29)
#define TD_ERRORS (TD_STATUS_BITSTUFF | TD_STATUS_CRC_TIMEOUT | TD_STATUS_BABBLE | TD_STATUS_DBE | TD_STATUS_STALLED)
/* Lengths are stored minus one, so 7FFh means none. */
#define TD_ACTLEN(s) (((s) + 1) & (uint32_t)0x7ff)

#define PID_IN ((uint32_t)0x69)
#define PID_OUT ((uint32_t)0xe1)
#define PID_SETUP ((uint32_t)0x2d)
#define TOKEN_ADDR(a) ((uint32_t)(a) << 8)
#define TOKEN_ENDPT(e) ((uint32_t)(e) << 15)
#define TOKEN_D ((uint32_t)1 << 19)
#define TOKEN_MAXLEN(n) (((uint32_t)(n) - (uint32_t)1) << 21)
#define TOKEN_GET_MAXLEN(t) ((((t) >> 21) + 1) & (uint32_t)0x7ff)

#define PCI_LEGSUP 0xc0
/* Clears the trap status bits and turns off every trap and SMI source. */
#define LEGSUP_CLEAR 0x8f00
#define LEGSUP_PIRQ 0x2000

#define FRAMES 1024
#define LEVELS 8
#define MAX_PORTS 8
/* BEGIN FreeBSD sys/dev/usb/usb.h, sys/dev/usb/controller/uhci.c */
/*
 * FreeBSD's USB_BUS_RESET_DELAY, USB_PORT_ROOT_RESET_DELAY, and
 * USB_PORT_RESET_DELAY, from its sys/dev/usb/usb.h (BSD-2-Clause), as its
 * sys/dev/usb/controller/uhci.c (BSD-2-Clause, uhci_reset and uhci_portreset)
 * uses them; USB 2.0 7.1.7.5 asks for at least 50 ms of root port reset.
 */
#define GLOBAL_RESET_MS 100
#define ROOT_RESET_MS 200
#define ENABLE_POLL_MS 50
#define ENABLE_TRIES 12
/* END FreeBSD sys/dev/usb/usb.h, sys/dev/usb/controller/uhci.c */
/* USB 2.0 7.1.7.7: TDRSMDN, the resume signalling software times. */
#define RESUME_SIGNAL_MS 20
#define PORT_POLL_MS 100
#define ISO_LEAD_FRAMES 4

struct uhci_td {
    uint32_t link;
    uint32_t status;
    uint32_t token;
    uint32_t buffer;
    uint64_t dma;
    struct uhci_td *sw_next;
    /* Isochronous TDs sit on the frame list, chained per frame. */
    struct uhci_td *frame_next;
    uint32_t frame;
    bool skipped;
    /* Holds a control transfer's SETUP data, which must be DMA-addressable. */
    uint8_t setup[8];
};

#define KIND_CONTROL 0
#define KIND_BULK 1
#define KIND_INTR 2
#define KIND_SKELETON 3

/*
 * Waits for endpoints being closed, then completes an asynchronous operation.
 */
struct uhci_close {
    struct tendril_async *async;
    size_t remaining;
    int status;
};

struct uhci_qh {
    uint32_t link;
    uint32_t element;
    uint64_t dma;
    struct tendril_endpoint *ep;
    struct uhci_td *dummy;
    struct uhci_qh *sw_next;
    struct uhci_qh *sw_prev;
    struct tendril_list node;
    struct tendril_list wait_node;
    int kind;
    size_t level;
    uint32_t wait_frame;
    struct uhci_close *close;
    bool linked;
    bool waiting;
    bool closing;
    /*
     * The data toggle of the next TD queued on a bulk or interrupt endpoint.
     */
    uint32_t toggle;
};

/* Per isochronous endpoint: where its stream of frames stands. */
struct uhci_iso_ep {
    uint32_t next_frame;
    bool streaming;
};

#define PHASE_IDLE 0
#define PHASE_RESET 1
#define PHASE_ENABLE 2

struct uhci {
    struct tendril_hc *hc;
    size_t num_ports;
    uint32_t *frame_list;
    uint64_t frame_list_dma;
    struct uhci_td *frame_iso[FRAMES];
    struct uhci_qh *skel[LEVELS];
    struct uhci_qh *control_skel;
    struct uhci_qh *bulk_skel;
    struct tendril_dma_pool td_pool;
    struct tendril_dma_pool qh_pool;
    struct tendril_list qhs;
    /* Queue heads off the schedule, waiting for frames to pass. */
    struct tendril_list waiting;
    /* Isochronous transfers in flight. */
    struct tendril_list iso_active;
    uint16_t intr;
    uint64_t port_check;
    struct tendril_timer timer;
    int phase[MAX_PORTS];
    uint64_t phase_start[MAX_PORTS];
    size_t enable_tries[MAX_PORTS];
    bool reset_changed[MAX_PORTS];
    /*
     * Resume signalling under way, and resumes finished but not yet reported.
     */
    uint64_t resume_start[MAX_PORTS];
    bool resuming[MAX_PORTS];
    bool resume_changed[MAX_PORTS];
    struct tendril_timer port_timer;
    uint16_t saved_cmd;
};

/* hcd_data slots in a transfer */
#define XD_FIRST 0
#define XD_LAST 1
#define XD_FLAGS 2
#define XD_COUNT 3
#define XD_NODE 4

#define XF_CANCEL ((uint32_t)1 << 0)
#define XF_SHORT ((uint32_t)1 << 1)

/*
 * Isochronous transfers are tracked through a list node kept beside the
 * transfer.
 */
struct uhci_iso_xfer {
    struct tendril_list node;
    struct tendril_transfer *xfer;
    uint32_t last_frame;
    uint32_t cancel_frame;
    bool cancelled;
};

static struct uhci *U(struct tendril_hc *hc) {
    return hc->hcd;
}

static uint16_t reg_read16(struct tendril_hc *hc, uint32_t reg) {
    return hc_read16(hc, reg);
}

static void reg_write16(struct tendril_hc *hc, uint32_t reg, uint16_t v) {
    hc_write16(hc, reg, v);
}

static uint32_t le(uint32_t v) {
    return tendril_le32(v);
}

/* The 32-bit frame number, extended from the 11-bit counter. */
static uint32_t current_frame(struct tendril_hc *hc) {
    return tendril_frame_update(hc, reg_read16(hc, REG_FRNUM) & (uint32_t)0x7ff, 0x7ff);
}

static struct uhci_td *td_alloc(struct tendril_hc *hc) {
    uint64_t dma;
    struct uhci_td *td = tendril_dma_pool_alloc(&U(hc)->td_pool, &dma);
    if (td != NULL) {
        td->dma = dma;
    }
    return td;
}

static void td_free(struct tendril_hc *hc, struct uhci_td *td) {
    tendril_dma_pool_free(&U(hc)->td_pool, td, td->dma);
}

static struct uhci_qh *qh_alloc(struct tendril_hc *hc) {
    uint64_t dma;
    struct uhci_qh *qh = tendril_dma_pool_alloc(&U(hc)->qh_pool, &dma);
    if (qh != NULL) {
        qh->dma = dma;
        list_init(&qh->node);
        list_init(&qh->wait_node);
        qh->sw_next = qh;
        qh->sw_prev = qh;
    }
    return qh;
}

static void qh_free(struct tendril_hc *hc, struct uhci_qh *qh) {
    tendril_dma_pool_free(&U(hc)->qh_pool, qh, qh->dma);
}

/* Data cursor over a transfer's DMA segments. */
struct cursor {
    const struct tendril_dma_segment *segs;
    size_t num;
    size_t idx;
    size_t off;
};

static void cursor_init(struct cursor *c, struct tendril_transfer *xfer) {
    c->segs = xfer->internal.segments;
    c->num = xfer->internal.num_segments;
    c->idx = 0;
    c->off = 0;
}

/* The address at the cursor, and how many bytes follow it contiguously. */
static uint64_t cursor_take(struct cursor *c, size_t want, size_t *got) {
    while (c->idx < c->num && c->off == c->segs[c->idx].len) {
        c->idx++;
        c->off = 0;
    }
    if (c->idx >= c->num) {
        *got = 0;
        return 0;
    }
    uint64_t a = c->segs[c->idx].addr + c->off;
    *got = MIN(want, c->segs[c->idx].len - c->off);
    c->off += *got;
    return a;
}

static int td_status(uint32_t s) {
    if ((s & TD_STATUS_BABBLE) != 0) {
        return TENDRIL_ERR_BABBLE;
    }
    if ((s & TD_STATUS_DBE) != 0) {
        return TENDRIL_ERR_OVERRUN;
    }
    if ((s & (TD_STATUS_CRC_TIMEOUT | TD_STATUS_BITSTUFF)) != 0) {
        return TENDRIL_ERR_IO;
    }
    return TENDRIL_ERR_STALL;
}

/* Queue heads */

static struct uhci_qh *skeleton_of(struct tendril_hc *hc, struct uhci_qh *qh) {
    struct uhci *u = U(hc);
    switch (qh->kind) {
        case KIND_CONTROL: {
            return u->control_skel;
        }
        case KIND_BULK: {
            return u->bulk_skel;
        }
        default: {
            return u->skel[qh->level];
        }
    }
}

/* Puts a queue head right after its skeleton. */
static void link_qh(struct tendril_hc *hc, struct uhci_qh *qh) {
    struct uhci_qh *head = skeleton_of(hc, qh);
    qh->link = head->link;
    qh->sw_next = head->sw_next;
    qh->sw_prev = head;
    head->sw_next->sw_prev = qh;
    head->sw_next = qh;
    dma_barrier();
    head->link = le((uint32_t)qh->dma | LINK_QH);
    qh->linked = true;
}

static void unlink_qh(struct uhci_qh *qh) {
    struct uhci_qh *prev = qh->sw_prev;
    prev->link = qh->link;
    prev->sw_next = qh->sw_next;
    qh->sw_next->sw_prev = prev;
    qh->sw_next = qh;
    qh->sw_prev = qh;
    qh->linked = false;
    dma_barrier();
}

static size_t intr_level(struct tendril_endpoint *ep) {
    size_t log2 = ep->interval_log2 > 3 ? ep->interval_log2 - (uint32_t)3 : (uint32_t)0;
    return MIN(log2, (size_t)LEVELS - 1);
}

/*
 * Off the schedule, a queue head is safe to change once two frames have
 * started.
 */
static void wait_frames(struct tendril_hc *hc, struct uhci_qh *qh) {
    struct uhci *u = U(hc);
    if (qh->waiting) {
        return;
    }
    qh->waiting = true;
    qh->wait_frame = current_frame(hc);
    list_add_tail(&u->waiting, &qh->wait_node);
    tendril_timer_arm(hc, &u->timer, NS_PER_MS);
}

/* Transfers on queues */

static struct uhci_td *xfer_first(struct tendril_transfer *xfer) {
    return (struct uhci_td *)xfer->internal.hcd_data[XD_FIRST];
}

static struct uhci_td *xfer_last(struct tendril_transfer *xfer) {
    return (struct uhci_td *)xfer->internal.hcd_data[XD_LAST];
}

static bool is_in(struct tendril_transfer *xfer) {
    if (xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL) {
        return (xfer->setup.request_type & TENDRIL_DIR_IN) != 0;
    }
    return (xfer->endpoint->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
}

static void free_chain(struct tendril_hc *hc, struct uhci_td *td, struct uhci_td *stop) {
    while (td != NULL && td != stop) {
        struct uhci_td *next = td->sw_next;
        td_free(hc, td);
        td = next;
    }
}

static void free_xfer_tds(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    free_chain(hc, xfer_first(xfer), xfer_last(xfer)->sw_next);
}

static uint32_t td_token(struct tendril_endpoint *ep, uint32_t pid, size_t len, uint32_t toggle) {
    return TOKEN_MAXLEN(len) | (toggle != 0 ? TOKEN_D : 0) | TOKEN_ENDPT(ep->address & TENDRIL_ENDPOINT_NUMBER_MASK)
        | TOKEN_ADDR(ep->dev->address & 0x7f) | pid;
}

/*
 * An inactive TD ending a queue; it holds a valid token in case the controller
 * looks.
 */
static void make_dummy(struct uhci_td *td) {
    td->link = le(LINK_T);
    td->status = 0;
    td->token = le(TOKEN_MAXLEN(0) | PID_IN);
    td->buffer = 0;
    td->sw_next = NULL;
    td->skipped = false;
}

/*
 * The first TD of a new transfer is the queue's old dummy, which the
 * controller can already see: it stays inactive until the whole chain behind
 * it is built.
 */
static void set_status(struct uhci_td *td, struct uhci_td *first, uint32_t status) {
    td->status = le(td == first ? status & ~TD_ACTIVE : status);
}

static int submit_queue(struct tendril_hc *hc, struct uhci_qh *qh, struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    bool control = ep->type == TENDRIL_ENDPOINT_XFER_CONTROL;
    bool in = is_in(xfer);
    size_t mps = ep->max_packet;
    size_t len = control ? tendril_le16(xfer->setup.length) : xfer->length;
    bool zlp = !in && (xfer->flags & TENDRIL_XFER_ZERO_PACKET) != 0 && len != 0 && len % mps == 0;
    uint32_t base = TD_CERR(3) | (ep->dev->speed == TENDRIL_SPEED_LOW ? TD_LS : 0);
    struct uhci_td *first = qh->dummy;
    struct uhci_td *dummy = td_alloc(hc);
    struct uhci_td *td = NULL;
    uint32_t toggle = control ? 1 : qh->toggle;
    struct cursor c;

    if (dummy == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    make_dummy(dummy);
    cursor_init(&c, xfer);
    first->sw_next = NULL;
    first->skipped = false;

    /*
     * Appends a TD to the chain, the old dummy first; its status is set by the
     * caller.
     */
#define NEXT_TD() \
    do { \
        struct uhci_td *n_ = td == NULL ? first : td_alloc(hc); \
        if (n_ == NULL) { \
            free_chain(hc, first->sw_next, NULL); \
            make_dummy(first); \
            td_free(hc, dummy); \
            return TENDRIL_ERR_NO_MEMORY; \
        } \
        if (td != NULL) { \
            n_->sw_next = NULL; \
            n_->skipped = false; \
            td->sw_next = n_; \
            td->link = le((uint32_t)n_->dma | LINK_VF); \
        } \
        td = n_; \
    } while (0)

    if (control) {
        NEXT_TD();
        memcpy(td->setup, &xfer->setup, 8);
        td->buffer = le((uint32_t)(td->dma + offsetof(struct uhci_td, setup)));
        td->token = le(td_token(ep, PID_SETUP, 8, 0));
        set_status(td, first, base | TD_ACTIVE);
    }
    size_t done = 0;
    bool any = false;
    while (done < len || (!control && !any) || zlp) {
        size_t chunk = 0;
        uint64_t addr = 0;
        if (done < len) {
            size_t want = MIN((size_t)mps, len - done);
            addr = cursor_take(&c, want, &chunk);
            if (chunk != want) {
                /* A packet's buffer must be contiguous. */
                free_chain(hc, first->sw_next, NULL);
                make_dummy(first);
                td_free(hc, dummy);
                return TENDRIL_ERR_BOUNCE;
            }
        } else if (any) {
            zlp = false;
        }
        NEXT_TD();
        td->buffer = le((uint32_t)addr);
        td->token = le(td_token(ep, in ? PID_IN : PID_OUT, chunk, toggle));
        /*
         * Short packet detection stops the queue on a short IN packet, for the
         * driver to sort out.
         */
        set_status(td, first, base | TD_ACTIVE | (in ? TD_SPD : 0));
        toggle ^= 1;
        done += chunk;
        any = true;
    }
    if (control) {
        NEXT_TD();
        td->buffer = 0;
        td->token = le(td_token(ep, in && len != 0 ? PID_OUT : PID_IN, 0, 1));
        set_status(td, first, base | TD_ACTIVE);
    } else {
        qh->toggle = toggle;
    }
#undef NEXT_TD

    td->status = le(le(td->status) | TD_IOC);
    td->sw_next = dummy;
    td->link = le((uint32_t)dummy->dma | LINK_VF);
    xfer->internal.hcd_data[XD_FIRST] = (uintptr_t)first;
    xfer->internal.hcd_data[XD_LAST] = (uintptr_t)td;
    xfer->internal.hcd_data[XD_FLAGS] = 0;
    qh->dummy = dummy;

    /* The old dummy goes live last, once everything behind it is in place. */
    dma_barrier();
    first->status = le(le(first->status) | TD_ACTIVE);
    dma_barrier();
    return TENDRIL_OK;
}

/*
 * Rewrites the data toggles from `td` on, after packets queued earlier did not
 * all happen.
 */
static void retoggle(struct uhci_qh *qh, struct uhci_td *td, uint32_t toggle) {
    for (; td != NULL && td != qh->dummy; td = td->sw_next) {
        if (td->skipped) {
            continue;
        }
        uint32_t token = le(td->token) & ~TOKEN_D;
        td->token = le(token | (toggle != 0 ? TOKEN_D : 0));
        toggle ^= 1;
    }
    qh->toggle = toggle;
}

/* Restarts a stopped queue at `td`, which the controller has not yet seen. */
static void queue_restart(struct uhci_qh *qh, struct uhci_td *td) {
    dma_barrier();
    qh->element = le((uint32_t)td->dma);
    dma_barrier();
}

/*
 * Checks the oldest transfer of a queue. A short IN packet or an error leaves
 * the queue stopped on that TD, which is restarted past the transfer here.
 */
static bool scan_queue_xfer(struct tendril_hc *hc, struct uhci_qh *qh, struct tendril_transfer *xfer) {
    bool control = xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL;
    struct uhci_td *first = xfer_first(xfer);
    struct uhci_td *last = xfer_last(xfer);
    size_t actual = 0;

    for (struct uhci_td *td = first; ; td = td->sw_next) {
        if (!td->skipped) {
            uint32_t s = le(td->status);
            if ((s & TD_ACTIVE) != 0) {
                return false;
            }
            dma_barrier();
            uint32_t token = le(td->token);
            size_t moved = TD_ACTLEN(s);
            bool data = !control || (td != first && td != last);
            if (data) {
                actual += moved;
            }
            if ((s & TD_ERRORS) != 0) {
                struct uhci_td *next = last->sw_next;
                if (!control) {
                    /* The failed packet did not toggle. */
                    retoggle(qh, next, (token & TOKEN_D) != 0);
                }
                /*
                 * The element pointer leaves the failed TDs before they are
                 * freed.
                 */
                queue_restart(qh, next);
                free_xfer_tds(hc, xfer);
                tendril_transfer_complete(hc, xfer, td_status(s), actual);
                return true;
            }
            if (data && (token & 0xff) == PID_IN && moved < TOKEN_GET_MAXLEN(token)) {
                /* Short: the rest of the data stage never runs. */
                struct uhci_td *resume = control ? last : last->sw_next;
                for (struct uhci_td *k = td->sw_next; k != resume; k = k->sw_next) {
                    k->skipped = true;
                }
                if ((xfer->internal.hcd_data[XD_FLAGS] & XF_SHORT) == 0) {
                    xfer->internal.hcd_data[XD_FLAGS] |= XF_SHORT;
                    if (!control) {
                        retoggle(qh, resume, (token & TOKEN_D) == 0);
                    }
                    queue_restart(qh, resume);
                }
                if (!control) {
                    break;
                }
            }
        }
        if (td == last) {
            break;
        }
    }
    free_xfer_tds(hc, xfer);
    tendril_transfer_complete(hc, xfer, TENDRIL_OK, actual);
    return true;
}

static void scan_qh(struct tendril_hc *hc, struct uhci_qh *qh) {
    struct tendril_endpoint *ep = qh->ep;
    while (!list_empty(&ep->transfers)) {
        struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        if ((xfer->internal.hcd_data[XD_FLAGS] & XF_CANCEL) != 0) {
            break;
        }
        if (!scan_queue_xfer(hc, qh, xfer)) {
            break;
        }
    }
}

/*
 * With the queue head off the schedule: completes cancelled transfers, then
 * relinks the rest and restarts the queue at the right TD and data toggle.
 */
static void sweep_qh(struct tendril_hc *hc, struct uhci_qh *qh) {
    struct tendril_endpoint *ep = qh->ep;
    bool control = ep->type == TENDRIL_ENDPOINT_XFER_CONTROL;
    struct tendril_list *it;
    struct tendril_list *tmp;

    scan_qh(hc, qh);
    struct tendril_transfer *oldest = list_empty(&ep->transfers) ? NULL
        : CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
    bool restart = false;
    int toggle = -1;

    LIST_FOR_EACH_SAFE(it, tmp, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        if ((xfer->internal.hcd_data[XD_FLAGS] & XF_CANCEL) == 0) {
            continue;
        }
        size_t actual = 0;
        if (xfer == oldest) {
            restart = true;
            for (struct uhci_td *td = xfer_first(xfer); td != xfer_last(xfer)->sw_next; td = td->sw_next) {
                uint32_t s = le(td->status);
                uint32_t token = le(td->token);
                if (td->skipped) {
                    continue;
                }
                if ((s & TD_ACTIVE) != 0) {
                    if (toggle < 0) {
                        toggle = (token & TOKEN_D) != 0;
                    }
                    break;
                }
                /* A completed packet toggled; one that failed did not. */
                toggle = (int)(((token & TOKEN_D) != 0) ^ ((s & TD_ERRORS) == 0));
                if (!control || (td != xfer_first(xfer) && td != xfer_last(xfer))) {
                    actual += TD_ACTLEN(s);
                }
            }
        }
        free_xfer_tds(hc, xfer);
        tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, actual);
    }

    /*
     * Rebuild the links between the transfers left, in order, ending at the
     * dummy.
     */
    struct tendril_transfer *prev = NULL;
    LIST_FOR_EACH(it, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        if (prev != NULL) {
            xfer_last(prev)->sw_next = xfer_first(xfer);
            xfer_last(prev)->link = le((uint32_t)xfer_first(xfer)->dma | LINK_VF);
        }
        prev = xfer;
    }
    if (prev != NULL) {
        xfer_last(prev)->sw_next = qh->dummy;
        xfer_last(prev)->link = le((uint32_t)qh->dummy->dma | LINK_VF);
    }
    struct uhci_td *head = qh->dummy;
    if (!list_empty(&ep->transfers)) {
        head = xfer_first(CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node));
    }
    if (!control && restart) {
        retoggle(qh, head, toggle < 0 ? qh->toggle : (unsigned int)toggle);
    } else if (!control && !list_empty(&ep->transfers)) {
        /*
         * The transfer in flight keeps its toggles; whatever was queued behind
         * it moves up.
         */
        struct tendril_transfer *h = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        retoggle(qh, xfer_last(h)->sw_next, ((le(xfer_last(h)->token) & TOKEN_D) != 0) ^ (uint32_t)1);
    }
    if (restart) {
        queue_restart(qh, head);
    }
}

/* Isochronous transfers */

static struct uhci_iso_xfer *iso_state(struct tendril_transfer *xfer) {
    return (struct uhci_iso_xfer *)xfer->internal.hcd_data[XD_NODE];
}

static void iso_unlink_td(struct tendril_hc *hc, struct uhci_td *td) {
    struct uhci *u = U(hc);
    size_t idx = td->frame % FRAMES;
    if (u->frame_iso[idx] == td) {
        u->frame_iso[idx] = td->frame_next;
        u->frame_list[idx] = td->link;
    } else {
        for (struct uhci_td *p = u->frame_iso[idx]; p != NULL; p = p->frame_next) {
            if (p->frame_next == td) {
                p->frame_next = td->frame_next;
                p->link = td->link;
                break;
            }
        }
    }
    dma_barrier();
}

static void iso_free(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct uhci_iso_xfer *ix = iso_state(xfer);
    struct uhci_td *td = xfer_first(xfer);
    while (td != NULL) {
        struct uhci_td *next = td->sw_next;
        iso_unlink_td(hc, td);
        td_free(hc, td);
        td = next;
    }
    list_del(&ix->node);
    tendril_free(ix, sizeof(struct uhci_iso_xfer));
}

static int submit_iso(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct uhci *u = U(hc);
    struct tendril_endpoint *ep = xfer->endpoint;
    struct uhci_iso_ep *iep = ep->hcd;
    bool in = is_in(xfer);
    uint32_t now = current_frame(hc);
    size_t n = xfer->num_iso_packets;
    uint32_t frame;
    struct cursor c;

    if ((xfer->flags & TENDRIL_XFER_ISO_ASAP) == 0) {
        frame = xfer->start_frame;
    } else if (iep->streaming && (int32_t)(iep->next_frame - now) > 0) {
        /* Back to back with what is queued. */
        frame = iep->next_frame;
    } else {
        frame = now + ISO_LEAD_FRAMES;
    }
    /* Everything must land within the frame list, and in the future. */
    if ((int32_t)(frame - now) <= 0 || frame + n - now >= FRAMES) {
        return TENDRIL_ERR_MISSED;
    }
    struct uhci_iso_xfer *ix = tendril_zalloc(sizeof(struct uhci_iso_xfer));
    if (ix == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }

    struct uhci_td *first = NULL;
    struct uhci_td *prev = NULL;
    cursor_init(&c, xfer);
    for (size_t i = 0; i < n; i++) {
        size_t len = xfer->iso_packets[i].length;
        size_t got = 0;
        uint64_t addr = len != 0 ? cursor_take(&c, len, &got) : 0;
        struct uhci_td *td = got == len ? td_alloc(hc) : NULL;
        if (td == NULL) {
            free_chain(hc, first, NULL);
            tendril_free(ix, sizeof(struct uhci_iso_xfer));
            return got == len ? TENDRIL_ERR_NO_MEMORY : TENDRIL_ERR_BOUNCE;
        }
        td->sw_next = NULL;
        td->frame = frame + (uint32_t)i;
        td->buffer = le((uint32_t)addr);
        td->token = le(td_token(ep, in ? PID_IN : PID_OUT, len, 0));
        td->status = le(TD_ACTIVE | TD_IOS | (i == n - 1 ? TD_IOC : 0));
        if (prev != NULL) {
            prev->sw_next = td;
        } else {
            first = td;
        }
        prev = td;
    }
    /* Each TD goes in front of whatever its frame already runs. */
    for (struct uhci_td *td = first; td != NULL; td = td->sw_next) {
        size_t idx = td->frame % FRAMES;
        td->link = u->frame_list[idx];
        td->frame_next = u->frame_iso[idx];
        u->frame_iso[idx] = td;
        dma_barrier();
        u->frame_list[idx] = le((uint32_t)td->dma);
    }
    ix->xfer = xfer;
    ix->last_frame = frame + (uint32_t)n - 1;
    list_add_tail(&u->iso_active, &ix->node);
    xfer->internal.hcd_data[XD_FIRST] = (uintptr_t)first;
    xfer->internal.hcd_data[XD_NODE] = (uintptr_t)ix;
    xfer->internal.hcd_data[XD_FLAGS] = 0;
    xfer->start_frame = frame;
    /* What is queued ends with whichever transfer ends last. */
    if (!iep->streaming || (int32_t)(frame + (uint32_t)n - iep->next_frame) > 0) {
        iep->next_frame = frame + (uint32_t)n;
    }
    iep->streaming = true;
    tendril_timer_arm(hc, &u->timer, 2 * NS_PER_MS);
    return TENDRIL_OK;
}

/*
 * Whether none of the transfer's frames is close enough to now for the
 * controller to be on it.
 */
static bool iso_frames_clear(struct tendril_transfer *xfer, uint32_t now) {
    for (struct uhci_td *td = xfer_first(xfer); td != NULL; td = td->sw_next) {
        int32_t d = (int32_t)(td->frame - now);
        if (d >= -1 && d <= 1) {
            return false;
        }
    }
    return true;
}

/*
 * Completes isochronous transfers once their frames are over, or cancelled
 * ones once they are clear.
 */
static void scan_iso(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    struct tendril_list *it;
    struct tendril_list *tmp;
    uint32_t now = current_frame(hc);

    LIST_FOR_EACH_SAFE(it, tmp, &u->iso_active) {
        struct uhci_iso_xfer *ix = CONTAINER_OF(it, struct uhci_iso_xfer, node);
        struct tendril_transfer *xfer = ix->xfer;
        if (ix->cancelled ? (int32_t)(now - ix->cancel_frame) < 2 || !iso_frames_clear(xfer, now)
          : (int32_t)(now - ix->last_frame) < 2) {
            continue;
        }
        size_t actual = 0;
        uint32_t i = 0;
        bool in = is_in(xfer);
        for (struct uhci_td *td = xfer_first(xfer); td != NULL; td = td->sw_next, i++) {
            struct tendril_iso_packet *pkt = &xfer->iso_packets[i];
            uint32_t s = le(td->status);
            if ((s & TD_ACTIVE) != 0 || (ix->cancelled && TD_ACTLEN(s) == 0 && (s & TD_ERRORS) == 0)) {
                pkt->status = TENDRIL_ERR_MISSED;
                pkt->actual = 0;
            } else if ((s & TD_ERRORS) != 0) {
                pkt->status = td_status(s);
                pkt->actual = 0;
            } else {
                pkt->status = TENDRIL_OK;
                pkt->actual = in ? TD_ACTLEN(s) : pkt->length;
            }
            actual += pkt->actual;
        }
        bool cancelled = ix->cancelled;
        struct tendril_endpoint *ep = xfer->endpoint;
        iso_free(hc, xfer);
        tendril_transfer_complete(hc, xfer, cancelled ? xfer->internal.cancel_status : TENDRIL_OK, actual);
        if (list_empty(&ep->transfers) && ep->hcd != NULL) {
            ((struct uhci_iso_ep *)ep->hcd)->streaming = false;
        }
    }
    if (!list_empty(&u->iso_active)) {
        tendril_timer_arm(hc, &u->timer, 2 * NS_PER_MS);
    }
}

static void scan_all(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    struct tendril_list *it;
    LIST_FOR_EACH(it, &u->qhs) {
        struct uhci_qh *qh = CONTAINER_OF(it, struct uhci_qh, node);
        if (!qh->closing && !qh->waiting) {
            scan_qh(hc, qh);
        }
    }
    scan_iso(hc);
}

/* Queue heads waiting on frames */

static void close_done(struct tendril_hc *hc, struct uhci_close *close) {
    if (close != NULL && --close->remaining == 0) {
        tendril_async_complete(hc, close->async, close->status);
        tendril_free(close, sizeof(struct uhci_close));
    }
}

static void fail_queue(struct tendril_hc *hc, struct uhci_qh *qh, int status) {
    struct tendril_endpoint *ep = qh->ep;
    while (!list_empty(&ep->transfers)) {
        struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        free_xfer_tds(hc, xfer);
        tendril_transfer_complete(hc, xfer, status, 0);
    }
}

static void free_qh(struct tendril_hc *hc, struct uhci_qh *qh) {
    struct uhci_close *close = qh->close;
    fail_queue(hc, qh, TENDRIL_ERR_NO_DEVICE);
    list_del(&qh->node);
    td_free(hc, qh->dummy);
    qh_free(hc, qh);
    close_done(hc, close);
}

static void finish_wait(struct tendril_hc *hc, struct uhci_qh *qh) {
    list_del(&qh->wait_node);
    qh->waiting = false;
    if (qh->closing) {
        free_qh(hc, qh);
        return;
    }
    sweep_qh(hc, qh);
    link_qh(hc, qh);
}

static void process_waiting(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    struct tendril_list *it;
    struct tendril_list *tmp;
    uint32_t frame = current_frame(hc);

    LIST_FOR_EACH_SAFE(it, tmp, &u->waiting) {
        struct uhci_qh *qh = CONTAINER_OF(it, struct uhci_qh, wait_node);
        if (hc->dead || frame - qh->wait_frame >= 2) {
            finish_wait(hc, qh);
        }
    }
    if (!list_empty(&u->waiting) && !hc->dead) {
        tendril_timer_arm(hc, &u->timer, NS_PER_MS);
    }
}

/* Root ports raise no interrupt, so they are looked at regularly. */
static void check_ports(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    u->port_check = tendril_now();
    for (size_t p = 1; p <= u->num_ports; p++) {
        uint16_t v = reg_read16(hc, REG_PORTSC(p));
        bool woke = (v & PORT_SUSP) != 0 && (v & PORT_RD) != 0 && !u->resuming[p - 1];
        if ((v & PORT_RWC) != 0 || u->phase[p - 1] != PHASE_IDLE || u->reset_changed[p - 1] || woke
         || u->resume_changed[p - 1]) {
            tendril_roothub_changed(hc, (uint8_t)p);
        }
    }
}

static void uhci_timer(struct tendril_work *work) {
    struct uhci *u = CONTAINER_OF(work, struct uhci, timer.work);
    struct tendril_hc *hc = u->hc;
    hc_lock(hc);
    if (!hc->dead) {
        process_waiting(hc);
        scan_iso(hc);
        if (tendril_now() - u->port_check >= PORT_POLL_MS * NS_PER_MS) {
            check_ports(hc);
        }
        uint64_t next = PORT_POLL_MS * NS_PER_MS;
        if (!list_empty(&u->waiting) || !list_empty(&u->iso_active)) {
            next = NS_PER_MS;
        }
        if (!tendril_timer_armed(&u->timer)) {
            tendril_timer_arm(hc, &u->timer, next);
        }
    }
    hc_unlock(hc);
}

/* Transfers */

static int uhci_submit(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    if (ep->hcd == NULL) {
        return TENDRIL_ERR_INVALID;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        return submit_iso(hc, xfer);
    }
    struct uhci_qh *qh = ep->hcd;
    if (qh->closing) {
        return TENDRIL_ERR_INVALID;
    }
    return submit_queue(hc, qh, xfer);
}

static void uhci_cancel(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        struct uhci_iso_xfer *ix = iso_state(xfer);
        if (hc->dead) {
            iso_free(hc, xfer);
            tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, 0);
            return;
        }
        /*
         * Inactive TDs are passed over; they come off the frame list once the
         * controller cannot be on them.
         */
        for (struct uhci_td *td = xfer_first(xfer); td != NULL; td = td->sw_next) {
            td->status = le(le(td->status) & ~TD_ACTIVE);
        }
        dma_barrier();
        ix->cancelled = true;
        ix->cancel_frame = current_frame(hc);
        tendril_timer_arm(hc, &U(hc)->timer, NS_PER_MS);
        return;
    }
    struct uhci_qh *qh = ep->hcd;
    if (hc->dead || qh == NULL) {
        if (qh != NULL) {
            free_xfer_tds(hc, xfer);
        }
        tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, 0);
        return;
    }
    xfer->internal.hcd_data[XD_FLAGS] |= XF_CANCEL;
    if (!qh->waiting) {
        if (qh->linked) {
            unlink_qh(qh);
        }
        wait_frames(hc, qh);
    }
}

/* Endpoints */

static int open_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep) {
    struct uhci *u = U(hc);

    if (ep->dev->speed > TENDRIL_SPEED_FULL) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        if (ep->dev->speed == TENDRIL_SPEED_LOW) {
            return TENDRIL_ERR_UNSUPPORTED;
        }
        struct uhci_iso_ep *iep = tendril_zalloc(sizeof(struct uhci_iso_ep));
        if (iep == NULL) {
            return TENDRIL_ERR_NO_MEMORY;
        }
        ep->hcd = iep;
        ep->open = true;
        return TENDRIL_OK;
    }
    struct uhci_qh *qh = qh_alloc(hc);
    if (qh == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    qh->dummy = td_alloc(hc);
    if (qh->dummy == NULL) {
        qh_free(hc, qh);
        return TENDRIL_ERR_NO_MEMORY;
    }
    make_dummy(qh->dummy);
    switch (ep->type) {
        case TENDRIL_ENDPOINT_XFER_CONTROL: {
            qh->kind = KIND_CONTROL;
            break;
        }
        case TENDRIL_ENDPOINT_XFER_BULK: {
            qh->kind = KIND_BULK;
            break;
        }
        default: {
            qh->kind = KIND_INTR;
            qh->level = intr_level(ep);
            break;
        }
    }
    qh->ep = ep;
    qh->toggle = 0;
    qh->element = le((uint32_t)qh->dummy->dma);
    list_add_tail(&u->qhs, &qh->node);
    link_qh(hc, qh);
    ep->hcd = qh;
    ep->open = true;
    return TENDRIL_OK;
}

static void close_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep, struct uhci_close *close) {
    void *hcd = ep->hcd;
    ep->open = false;
    ep->hcd = NULL;
    if (hcd == NULL) {
        return;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        tendril_free(hcd, sizeof(struct uhci_iso_ep));
        return;
    }
    struct uhci_qh *qh = hcd;
    qh->closing = true;
    qh->close = close;
    if (close != NULL) {
        close->remaining++;
    }
    if (qh->linked) {
        unlink_qh(qh);
    }
    if (hc->dead) {
        if (qh->waiting) {
            list_del(&qh->wait_node);
            qh->waiting = false;
        }
        free_qh(hc, qh);
        return;
    }
    if (qh->waiting) {
        qh->wait_frame = current_frame(hc);
    } else {
        wait_frames(hc, qh);
    }
}

static int uhci_configure(
    struct tendril_hc *hc, struct tendril_device *dev, struct tendril_endpoint **drop, size_t num_drop,
    struct tendril_endpoint **add, size_t num_add, struct tendril_async *async
) {
    (void)dev;
    struct uhci_close *close = tendril_zalloc(sizeof(struct uhci_close));
    if (close == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    close->async = async;
    close->status = TENDRIL_OK;
    /* Held until everything is queued, so it cannot complete early. */
    close->remaining = 1;
    for (size_t i = 0; i < num_drop; i++) {
        close_endpoint(hc, drop[i], close);
    }
    for (size_t i = 0; i < num_add; i++) {
        int st = open_endpoint(hc, add[i]);
        if (st != TENDRIL_OK) {
            for (size_t j = 0; j < i; j++) {
                close_endpoint(hc, add[j], close);
            }
            close->status = st;
            break;
        }
    }
    close_done(hc, close);
    return TENDRIL_OK;
}

static int uhci_device_init(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    int st = open_endpoint(hc, &dev->ep0);
    if (st == TENDRIL_OK) {
        tendril_async_complete(hc, async, TENDRIL_OK);
    }
    return st;
}

static int uhci_ep0_update(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    /*
     * The address and packet size go into each TD as it is built; nothing is
     * stored.
     */
    (void)dev;
    tendril_async_complete(hc, async, TENDRIL_OK);
    return TENDRIL_OK;
}

static int uhci_endpoint_reset(struct tendril_hc *hc, struct tendril_endpoint *ep, struct tendril_async *async) {
    struct uhci_qh *qh = ep->hcd;
    if (qh != NULL && ep->type != TENDRIL_ENDPOINT_XFER_ISOC && list_empty(&ep->transfers)) {
        qh->toggle = 0;
    }
    tendril_async_complete(hc, async, TENDRIL_OK);
    return TENDRIL_OK;
}

static void uhci_device_free(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    struct uhci_close *close = tendril_zalloc(sizeof(struct uhci_close));
    if (close != NULL) {
        close->async = async;
        close->status = TENDRIL_OK;
        close->remaining = 1;
    }
    /* The device is going away with its interfaces' endpoints still open. */
    close_endpoint(hc, &dev->ep0, close);
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (size_t e = 0; e < intf->num_endpoints; e++) {
            close_endpoint(hc, &intf->endpoints[e], close);
        }
    }
    if (close != NULL) {
        close_done(hc, close);
    } else {
        tendril_async_complete(hc, async, TENDRIL_OK);
    }
}

/* Root hub */

static void port_write(struct tendril_hc *hc, uint8_t port, uint16_t v) {
    reg_write16(hc, REG_PORTSC(port), v);
}

static uint16_t port_read_rw(struct tendril_hc *hc, uint8_t port) {
    return (uint16_t)(reg_read16(hc, REG_PORTSC(port)) & ~PORT_RWC);
}

/* BEGIN FreeBSD sys/dev/usb/controller/uhci.c */
/*
 * Moves a port through its reset, then enabling it, which UHCI leaves to
 * software, in the steps FreeBSD's sys/dev/usb/controller/uhci.c
 * (BSD-2-Clause, uhci_portreset) takes.
 */
static void port_advance(struct tendril_hc *hc, uint8_t port) {
    struct uhci *u = U(hc);
    size_t i = port - 1;
    uint64_t elapsed = tendril_now() - u->phase_start[i];

    if (u->phase[i] == PHASE_RESET && elapsed >= ROOT_RESET_MS * NS_PER_MS) {
        port_write(hc, port, (uint16_t)(port_read_rw(hc, port) & ~PORT_PR));
        /*
         * FreeBSD's uhci_portreset found that some devices need exactly this
         * pause before the port is enabled.
         */
        tendril_delay_us(100);
        port_write(hc, port, (uint16_t)(port_read_rw(hc, port) | PORT_PE));
        u->phase[i] = PHASE_ENABLE;
        u->phase_start[i] = tendril_now();
        u->enable_tries[i] = 0;
        return;
    }
    if (u->phase[i] != PHASE_ENABLE || elapsed < (u->enable_tries[i] + 1) * ENABLE_POLL_MS * NS_PER_MS) {
        return;
    }
    u->enable_tries[i]++;
    uint16_t v = reg_read16(hc, REG_PORTSC(port));
    bool done = false;
    if ((v & PORT_CCS) == 0 || (v & PORT_PE) != 0 || u->enable_tries[i] >= ENABLE_TRIES) {
        done = true;
    } else if ((v & PORT_RWC) != 0) {
        /*
         * Connection and enable changes from the reset itself: clear them and
         * look again.
         */
        port_write(hc, port, (uint16_t)((v & ~PORT_RWC) | (v & PORT_RWC)));
    } else {
        port_write(hc, port, (uint16_t)((v & ~PORT_RWC) | PORT_PE));
    }
    if (done) {
        u->phase[i] = PHASE_IDLE;
        u->reset_changed[i] = true;
    }
}
/* END FreeBSD sys/dev/usb/controller/uhci.c */

/*
 * Root ports behave as a hub's would (USB 2.0 11.24.2.7.1.3 and
 * 11.24.2.7.2.3): software times resume signalling, ending it as UHCI 1.1,
 * 2.1.7.1 describes, and the port reports itself suspended until then.
 */
static void advance_resume(struct tendril_hc *hc, uint8_t port) {
    struct uhci *u = U(hc);
    size_t i = port - 1;
    uint16_t v = reg_read16(hc, REG_PORTSC(port));

    if (!u->resuming[i] && (v & PORT_SUSP) != 0 && (v & PORT_RD) != 0) {
        /*
         * The device signalled a wakeup; the controller drives resume while RD
         * stays set.
         */
        u->resuming[i] = true;
        u->resume_start[i] = tendril_now();
        tendril_timer_arm(hc, &u->port_timer, RESUME_SIGNAL_MS * NS_PER_MS);
    }
    if (u->resuming[i] && tendril_now() - u->resume_start[i] >= RESUME_SIGNAL_MS * NS_PER_MS) {
        /*
         * Resume Detect and Suspend clear together, then RD reads one until
         * the EOP is out.
         */
        port_write(hc, port, (uint16_t)(port_read_rw(hc, port) & ~(PORT_RD | PORT_SUSP)));
        tendril_hc_wait16(hc, REG_PORTSC(port), PORT_RD, 0, 1000);
        u->resuming[i] = false;
        u->resume_changed[i] = true;
    }
}

static void port_timer_work(struct tendril_work *work) {
    struct uhci *u = CONTAINER_OF(work, struct uhci, port_timer.work);
    struct tendril_hc *hc = u->hc;
    hc_lock(hc);
    bool again = false;
    for (size_t p = 1; !hc->dead && p <= u->num_ports; p++) {
        if (!u->resuming[p - 1]) {
            continue;
        }
        advance_resume(hc, (uint8_t)p);
        if (u->resuming[p - 1]) {
            again = true;
        } else {
            tendril_roothub_changed(hc, (uint8_t)p);
        }
    }
    if (again) {
        tendril_timer_arm(hc, &u->port_timer, 2 * NS_PER_MS);
    }
    hc_unlock(hc);
}

static void uhci_port_status(struct tendril_hc *hc, uint8_t port, struct tendril_port_status *st) {
    struct uhci *u = U(hc);
    size_t i = port - 1;

    if (u->phase[i] != PHASE_IDLE) {
        port_advance(hc, port);
    }
    advance_resume(hc, port);
    uint16_t v = reg_read16(hc, REG_PORTSC(port));
    st->status = TENDRIL_PORT_STAT_POWER;
    st->change = 0;
    st->speed = (v & PORT_LSDA) != 0 ? TENDRIL_SPEED_LOW : TENDRIL_SPEED_FULL;
    if ((v & PORT_CCS) != 0) {
        st->status |= TENDRIL_PORT_STAT_CONNECTION;
    }
    if ((v & PORT_PE) != 0 && u->phase[i] == PHASE_IDLE) {
        st->status |= TENDRIL_PORT_STAT_ENABLE;
    }
    if ((v & PORT_SUSP) != 0) {
        st->status |= TENDRIL_PORT_STAT_SUSPEND;
    }
    if ((v & PORT_PR) != 0 || u->phase[i] != PHASE_IDLE) {
        st->status |= TENDRIL_PORT_STAT_RESET;
    }
    if ((v & PORT_LSDA) != 0) {
        st->status |= TENDRIL_PORT_STAT_LOW_SPEED;
    }
    if (u->phase[i] == PHASE_IDLE) {
        if ((v & PORT_CSC) != 0) {
            st->change |= TENDRIL_PORT_STAT_C_CONNECTION;
        }
        if ((v & PORT_PEC) != 0) {
            st->change |= TENDRIL_PORT_STAT_C_ENABLE;
        }
    }
    if (u->reset_changed[i]) {
        st->change |= TENDRIL_PORT_STAT_C_RESET;
    }
    if (u->resume_changed[i]) {
        st->change |= TENDRIL_PORT_STAT_C_SUSPEND;
    }
}

static int uhci_port_set_feature(struct tendril_hc *hc, uint8_t port, int feature) {
    struct uhci *u = U(hc);
    switch (feature) {
        case TENDRIL_PORT_FEAT_RESET: {
            port_write(hc, port, (uint16_t)(port_read_rw(hc, port) | PORT_PR));
            u->phase[port - 1] = PHASE_RESET;
            u->phase_start[port - 1] = tendril_now();
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_POWER: {
            /* Root ports are always powered. */
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            port_write(hc, port, (uint16_t)(port_read_rw(hc, port) | PORT_SUSP));
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_ENABLE: {
            port_write(hc, port, (uint16_t)(port_read_rw(hc, port) | PORT_PE));
            return TENDRIL_OK;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
}

static int uhci_port_clear_feature(struct tendril_hc *hc, uint8_t port, int feature) {
    struct uhci *u = U(hc);
    uint16_t v = port_read_rw(hc, port);
    switch (feature) {
        case TENDRIL_PORT_FEAT_ENABLE: {
            if ((hc->quirks & TENDRIL_HC_QUIRK_NO_PORT_DISABLE) == 0) {
                port_write(hc, port, (uint16_t)(v & ~PORT_PE));
            }
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            size_t i = port - 1;
            if ((v & PORT_SUSP) != 0 && !u->resuming[i]) {
                /*
                 * UHCI 1.1, 2.1.7.1: drive resume, ended by advance_resume()
                 * once timed.
                 */
                port_write(hc, port, (uint16_t)(v | PORT_RD));
                u->resuming[i] = true;
                u->resume_start[i] = tendril_now();
                tendril_timer_arm(hc, &u->port_timer, RESUME_SIGNAL_MS * NS_PER_MS);
            }
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_SUSPEND: {
            u->resume_changed[port - 1] = false;
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_POWER:
        case TENDRIL_PORT_FEAT_C_OVER_CURRENT: {
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_CONNECTION: {
            port_write(hc, port, (uint16_t)(v | PORT_CSC));
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_ENABLE: {
            port_write(hc, port, (uint16_t)(v | PORT_PEC));
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_RESET: {
            u->reset_changed[port - 1] = false;
            return TENDRIL_OK;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
}

/* Controller */

static void fail_everything(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    struct tendril_list *it;
    struct tendril_list *tmp;

    LIST_FOR_EACH_SAFE(it, tmp, &u->waiting) {
        finish_wait(hc, CONTAINER_OF(it, struct uhci_qh, wait_node));
    }
    LIST_FOR_EACH(it, &u->qhs) {
        fail_queue(hc, CONTAINER_OF(it, struct uhci_qh, node), TENDRIL_ERR_NO_DEVICE);
    }
    while (!list_empty(&u->iso_active)) {
        struct uhci_iso_xfer *ix = CONTAINER_OF(u->iso_active.next, struct uhci_iso_xfer, node);
        struct tendril_transfer *xfer = ix->xfer;
        iso_free(hc, xfer);
        tendril_transfer_complete(hc, xfer, TENDRIL_ERR_NO_DEVICE, 0);
    }
}

static void halt(struct tendril_hc *hc) {
    reg_write16(hc, REG_USBCMD, 0);
    if (!tendril_hc_wait16(hc, REG_USBSTS, STS_HALTED, STS_HALTED, 2000)) {
        LOG_WARN("UHCI: controller did not halt");
    }
}

static void controller_died(struct tendril_hc *hc, const char *why) {
    if (hc->dead) {
        return;
    }
    LOG_ERROR("UHCI: controller failed: %s", why);
    hc->dead = true;
    reg_write16(hc, REG_USBINTR, 0);
    reg_write16(hc, REG_USBCMD, 0);
    fail_everything(hc);
}

static void uhci_poll(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    uint16_t sts = reg_read16(hc, REG_USBSTS);
    if (sts == 0xffff) {
        controller_died(hc, "controller removed");
        return;
    }
    uint16_t ack = sts & STS_ACK;
    if (ack != 0) {
        reg_write16(hc, REG_USBSTS, ack);
    }
    if ((sts & STS_HSE) != 0) {
        controller_died(hc, "host system error");
        return;
    }
    if ((sts & STS_HCPE) != 0) {
        controller_died(hc, "host controller process error");
        return;
    }
    scan_all(hc);
    if (!list_empty(&u->waiting)) {
        process_waiting(hc);
    }
    if (tendril_now() - u->port_check >= PORT_POLL_MS * NS_PER_MS) {
        check_ports(hc);
    }
    if (hc->config.irq) {
        reg_write16(hc, REG_USBINTR, u->intr);
    }
}

static bool uhci_irq(struct tendril_hc *hc) {
    uint16_t sts = reg_read16(hc, REG_USBSTS);
    if (sts == 0xffff || (sts & STS_ACK) == 0) {
        return false;
    }
    /* Masked until tendril_hc_poll() has handled the cause. */
    reg_write16(hc, REG_USBINTR, 0);
    return true;
}

static uint32_t uhci_frame_number(struct tendril_hc *hc) {
    return current_frame(hc);
}

static void free_structures(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    for (size_t l = 0; l < LEVELS; l++) {
        if (u->skel[l] != NULL) {
            qh_free(hc, u->skel[l]);
        }
    }
    if (u->control_skel != NULL) {
        qh_free(hc, u->control_skel);
    }
    if (u->bulk_skel != NULL) {
        qh_free(hc, u->bulk_skel);
    }
    tendril_dma_free(hc, u->frame_list, FRAMES * 4);
    tendril_dma_pool_destroy(&u->td_pool);
    tendril_dma_pool_destroy(&u->qh_pool);
    tendril_free(u, sizeof(struct uhci));
    hc->hcd = NULL;
}

static struct uhci_qh *skeleton(struct tendril_hc *hc) {
    struct uhci_qh *qh = qh_alloc(hc);
    if (qh != NULL) {
        qh->kind = KIND_SKELETON;
        qh->element = le(LINK_T);
        qh->link = le(LINK_T);
    }
    return qh;
}

/*
 * Ports are counted by PORTSC bit 7, which always reads as one (UHCI 1.1,
 * 2.1.7); UHCI does not say how many there are.
 */
static size_t count_ports(struct tendril_hc *hc) {
    size_t n = 0;
    while (n < MAX_PORTS) {
        uint16_t v = reg_read16(hc, REG_PORTSC(n + 1));
        if (v == 0xffff || (v & PORT_ALWAYS_ONE) == 0) {
            break;
        }
        n++;
    }
    return n;
}

static int uhci_start(struct tendril_hc *hc) {
    struct uhci *u = tendril_zalloc(sizeof(struct uhci));
    if (u == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    hc->hcd = u;
    u->hc = hc;
    list_init(&u->qhs);
    list_init(&u->waiting);
    list_init(&u->iso_active);
    tendril_timer_init(&u->timer, uhci_timer);
    tendril_timer_init(&u->port_timer, port_timer_work);
    if (reg_read16(hc, REG_USBSTS) == 0xffff) {
        free_structures(hc);
        return TENDRIL_ERR_NO_DEVICE;
    }
    /* Every address the controller handles is 32 bits wide. */
    hc->dma32 = true;
    hc->data32 = true;

    if (hc->config.pci && (hc->quirks & TENDRIL_HC_QUIRK_NO_HANDOFF) == 0) {
        /*
         * Clear the trap status bits and every SMI enable (Intel 82371AB PIIX4
         * datasheet 290562, 6.1.12).
         */
        tendril_glue_pci_write16(hc->ctx, PCI_LEGSUP, LEGSUP_CLEAR);
    }
    halt(hc);
    /* UHCI 1.1, 2.1.1 asks for at least 10 ms of global reset. */
    reg_write16(hc, REG_USBCMD, CMD_GRESET);
    tendril_delay_us(GLOBAL_RESET_MS * 1000);
    reg_write16(hc, REG_USBCMD, 0);
    tendril_delay_us(10000);
    reg_write16(hc, REG_USBCMD, CMD_HCRESET);
    if (!tendril_hc_wait16(hc, REG_USBCMD, CMD_HCRESET, 0, 10000)) {
        LOG_ERROR("UHCI: reset failed");
        free_structures(hc);
        return TENDRIL_ERR_TIMEOUT;
    }
    u->num_ports = count_ports(hc);

    tendril_dma_pool_init(&u->td_pool, hc, sizeof(struct uhci_td), 16);
    tendril_dma_pool_init(&u->qh_pool, hc, sizeof(struct uhci_qh), 16);
    u->frame_list = tendril_dma_alloc(hc, FRAMES * 4, 4096, &u->frame_list_dma);
    u->control_skel = skeleton(hc);
    u->bulk_skel = skeleton(hc);
    bool ok = u->frame_list != NULL && u->control_skel != NULL && u->bulk_skel != NULL;
    for (size_t l = 0; l < LEVELS; l++) {
        u->skel[l] = skeleton(hc);
        ok = ok && u->skel[l] != NULL;
    }
    if (!ok) {
        free_structures(hc);
        return TENDRIL_ERR_NO_MEMORY;
    }
    /*
     * Level k runs every 2^k frames ahead of level k - 1, then control, then
     * bulk.
     */
    u->bulk_skel->link = le(LINK_T);
    u->control_skel->link = le((uint32_t)u->bulk_skel->dma | LINK_QH);
    u->skel[0]->link = le((uint32_t)u->control_skel->dma | LINK_QH);
    for (size_t l = 1; l < LEVELS; l++) {
        u->skel[l]->link = le((uint32_t)u->skel[l - 1]->dma | LINK_QH);
    }
    for (size_t f = 0; f < FRAMES; f++) {
        size_t level = f == 0 ? LEVELS - 1 : MIN(lowest_bit(f), (size_t)LEVELS - 1);
        u->frame_list[f] = le((uint32_t)u->skel[level]->dma | LINK_QH);
    }
    dma_barrier();

    u->intr = INTR_TIMEOUT_CRC | INTR_IOC | INTR_SHORT;
    reg_write16(hc, REG_USBINTR, hc->config.irq ? u->intr : 0);
    reg_write16(hc, REG_FRNUM, 0);
    hc_write32(hc, REG_FLBASEADD, (uint32_t)u->frame_list_dma);
    reg_write16(hc, REG_USBSTS, STS_ACK);
    reg_write16(hc, REG_USBCMD, CMD_RS | CMD_CF | CMD_MAXP);
    if (!tendril_hc_wait16(hc, REG_USBSTS, STS_HALTED, 0, 10000)) {
        LOG_ERROR("UHCI: controller did not start");
        free_structures(hc);
        return TENDRIL_ERR_TIMEOUT;
    }
    if (hc->config.pci && (hc->quirks & TENDRIL_HC_QUIRK_NO_HANDOFF) == 0) {
        /* BEGIN FreeBSD sys/dev/usb/controller/uhci_pci.c */
        /*
         * Interrupts back on PIRQD, as FreeBSD's
         * sys/dev/usb/controller/uhci_pci.c (BSD-2-Clause) leaves it.
         */
        tendril_glue_pci_write16(hc->ctx, PCI_LEGSUP, LEGSUP_PIRQ);
        /* END FreeBSD sys/dev/usb/controller/uhci_pci.c */
    }
    LOG_INFO("UHCI: %zu ports", u->num_ports);

    int st = tendril_roothub_init(hc, u->num_ports);
    if (st != TENDRIL_OK) {
        halt(hc);
        free_structures(hc);
        return st;
    }
    u->port_check = tendril_now();
    tendril_timer_arm(hc, &u->timer, PORT_POLL_MS * NS_PER_MS);
    return TENDRIL_OK;
}

static void uhci_stop(struct tendril_hc *hc) {
    if (hc->hcd == NULL) {
        return;
    }
    hc->dead = true;
    reg_write16(hc, REG_USBINTR, 0);
    halt(hc);
    tendril_timer_cancel(hc, &U(hc)->timer);
    tendril_timer_cancel(hc, &U(hc)->port_timer);
    fail_everything(hc);
}

static int uhci_suspend(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    tendril_timer_cancel(hc, &u->port_timer);
    u->saved_cmd = reg_read16(hc, REG_USBCMD);
    reg_write16(hc, REG_USBCMD, (uint16_t)(u->saved_cmd & ~CMD_RS));
    if (!tendril_hc_wait16(hc, REG_USBSTS, STS_HALTED, STS_HALTED, 2000)) {
        reg_write16(hc, REG_USBCMD, u->saved_cmd);
        return TENDRIL_ERR_TIMEOUT;
    }
    return TENDRIL_OK;
}

static int uhci_resume(struct tendril_hc *hc) {
    struct uhci *u = U(hc);
    if (reg_read16(hc, REG_USBSTS) == 0xffff) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    /*
     * A frame list address that no longer reads back means the controller lost
     * power.
     */
    if (hc_read32(hc, REG_FLBASEADD) != (uint32_t)u->frame_list_dma || (reg_read16(hc, REG_USBCMD) & CMD_CF) == 0) {
        return TENDRIL_ERR_IO;
    }
    reg_write16(hc, REG_USBINTR, hc->config.irq ? u->intr : 0);
    reg_write16(hc, REG_USBCMD, (uint16_t)(u->saved_cmd | CMD_RS));
    if (!tendril_hc_wait16(hc, REG_USBSTS, STS_HALTED, 0, 10000)) {
        return TENDRIL_ERR_IO;
    }
    return TENDRIL_OK;
}

static void uhci_destroy(struct tendril_hc *hc) {
    if (hc->hcd != NULL) {
        free_structures(hc);
    }
}

const struct tendril_hcd_ops tendril_uhci_ops = {
    .name = "UHCI",
    .start = uhci_start,
    .stop = uhci_stop,
    .destroy = uhci_destroy,
    .irq = uhci_irq,
    .poll = uhci_poll,
    .frame_number = uhci_frame_number,
    .port_status = uhci_port_status,
    .port_set_feature = uhci_port_set_feature,
    .port_clear_feature = uhci_port_clear_feature,
    .device_init = uhci_device_init,
    .ep0_update = uhci_ep0_update,
    .configure = uhci_configure,
    .endpoint_reset = uhci_endpoint_reset,
    .device_free = uhci_device_free,
    .suspend = uhci_suspend,
    .resume = uhci_resume,
    .submit = uhci_submit,
    .cancel = uhci_cancel
};
