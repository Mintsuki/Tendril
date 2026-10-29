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

/* OpenHCI 1.0a. Registers, descriptors, and the HCCA are little-endian and use 32-bit addresses. */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/controller/ohci.c
 * and sys/dev/usb/usb.h (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
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
 * Parts of this file are derived from OpenBSD's sys/dev/usb/ohci.c (commit
 * fba0a8da167d48e1d94a300389b91b0a48708155):
 *
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
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

#include "../internal.h"

#define REG_REVISION 0x00
#define REG_CONTROL 0x04
#define REG_COMMAND_STATUS 0x08
#define REG_INTR_STATUS 0x0c
#define REG_INTR_ENABLE 0x10
#define REG_INTR_DISABLE 0x14
#define REG_HCCA 0x18
#define REG_CONTROL_HEAD_ED 0x20
#define REG_CONTROL_CURRENT_ED 0x24
#define REG_BULK_HEAD_ED 0x28
#define REG_BULK_CURRENT_ED 0x2c
#define REG_FM_INTERVAL 0x34
#define REG_FM_NUMBER 0x3c
#define REG_PERIODIC_START 0x40
#define REG_RH_DESC_A 0x48
#define REG_RH_STATUS 0x50
#define REG_RH_PORT(n) ((uint32_t)(0x54 + 4 * ((n) - 1)))

#define CTL_CBSR_MASK ((uint32_t)3)
#define CTL_RATIO_4_1 ((uint32_t)3)
#define CTL_PLE ((uint32_t)1 << 2)
#define CTL_IE ((uint32_t)1 << 3)
#define CTL_CLE ((uint32_t)1 << 4)
#define CTL_BLE ((uint32_t)1 << 5)
#define CTL_HCFS_MASK ((uint32_t)3 << 6)
#define CTL_HCFS_RESET ((uint32_t)0 << 6)
#define CTL_HCFS_RESUME ((uint32_t)1 << 6)
#define CTL_HCFS_OPERATIONAL ((uint32_t)2 << 6)
#define CTL_IR ((uint32_t)1 << 8)

#define CMD_HCR ((uint32_t)1 << 0)
#define CMD_CLF ((uint32_t)1 << 1)
#define CMD_BLF ((uint32_t)1 << 2)
#define CMD_OCR ((uint32_t)1 << 3)

#define INT_SO ((uint32_t)1 << 0)
#define INT_WDH ((uint32_t)1 << 1)
#define INT_SF ((uint32_t)1 << 2)
#define INT_RD ((uint32_t)1 << 3)
#define INT_UE ((uint32_t)1 << 4)
#define INT_FNO ((uint32_t)1 << 5)
#define INT_RHSC ((uint32_t)1 << 6)
#define INT_OC ((uint32_t)1 << 30)
#define INT_MIE ((uint32_t)1 << 31)
#define INT_ALL (INT_SO | INT_WDH | INT_SF | INT_RD | INT_UE | INT_FNO | INT_RHSC | INT_OC)

#define FI_MASK ((uint32_t)0x3fff)
#define FI_DEFAULT ((uint32_t)11999)
#define FIT ((uint32_t)1 << 31)
/* OHCI 1.0a, 5.4: bit times of transaction and EOF overhead, taken off before the 6/7 for bit stuffing. */
#define MAX_OVERHEAD ((uint32_t)210)

#define RHA_NDP_MASK ((uint32_t)0xff)
#define RHA_PSM ((uint32_t)1 << 8)
#define RHA_NPS ((uint32_t)1 << 9)
#define RHA_NOCP ((uint32_t)1 << 12)
#define RHS_LPSC ((uint32_t)1 << 16)

/* Port status: each read bit doubles as a write-one command, as noted. */
#define PS_CCS ((uint32_t)1 << 0)
#define PS_CLEAR_ENABLE ((uint32_t)1 << 0)
#define PS_PES ((uint32_t)1 << 1)
#define PS_SET_ENABLE ((uint32_t)1 << 1)
#define PS_PSS ((uint32_t)1 << 2)
#define PS_SET_SUSPEND ((uint32_t)1 << 2)
#define PS_POCI ((uint32_t)1 << 3)
#define PS_CLEAR_SUSPEND ((uint32_t)1 << 3)
#define PS_PRS ((uint32_t)1 << 4)
#define PS_SET_RESET ((uint32_t)1 << 4)
#define PS_PPS ((uint32_t)1 << 8)
#define PS_SET_POWER ((uint32_t)1 << 8)
#define PS_LSDA ((uint32_t)1 << 9)
#define PS_CLEAR_POWER ((uint32_t)1 << 9)
#define PS_CSC ((uint32_t)1 << 16)
#define PS_PESC ((uint32_t)1 << 17)
#define PS_PSSC ((uint32_t)1 << 18)
#define PS_OCIC ((uint32_t)1 << 19)
#define PS_PRSC ((uint32_t)1 << 20)

#define ED_DIR_OUT ((uint32_t)1 << 11)
#define ED_DIR_IN ((uint32_t)2 << 11)
#define ED_LOW_SPEED ((uint32_t)1 << 13)
#define ED_SKIP ((uint32_t)1 << 14)
#define ED_ISO ((uint32_t)1 << 15)
#define ED_HALTED ((uint32_t)1)
#define ED_CARRY ((uint32_t)2)
#define ED_PTR_MASK ((uint32_t)0xfffffff0)

#define TD_ROUNDING ((uint32_t)1 << 18)
#define TD_DP_SETUP ((uint32_t)0 << 19)
#define TD_DP_OUT ((uint32_t)1 << 19)
#define TD_DP_IN ((uint32_t)2 << 19)
#define TD_DP_MASK ((uint32_t)3 << 19)
#define TD_DI(n) ((uint32_t)(n) << 21)
#define TD_TOGGLE_CARRY ((uint32_t)0 << 24)
#define TD_DATA0 ((uint32_t)2 << 24)
#define TD_DATA1 ((uint32_t)3 << 24)
#define TD_TOGGLE_MASK ((uint32_t)3 << 24)
#define TD_CC_SHIFT 28
#define TD_NOT_ACCESSED ((uint32_t)15 << 28)

#define ITD_FC(n) ((uint32_t)(n) << 24)
/* An untouched offset carries condition code 111x; x selects the page. */
#define ITD_OFFSET_FRESH ((uint32_t)0xe000)

#define CC_NOERROR 0
#define CC_STALL 4
#define CC_DATA_OVERRUN 8
#define CC_DATA_UNDERRUN 9
#define CC_BUFFER_OVERRUN 12
#define CC_BUFFER_UNDERRUN 13
#define CC_NOT_ACCESSED 14

#define INTR_LEVELS 6
/* BEGIN FreeBSD sys/dev/usb/controller/ohci.c */
/*
 * 4 KiB per TD, as FreeBSD's sys/dev/usb/controller/ohci.c (BSD-2-Clause) has
 * it, though OHCI 1.0a allows two pages.
 */
#define TD_MAX_BYTES 4096
/* END FreeBSD sys/dev/usb/controller/ohci.c */
#define ITD_PACKETS 8
#define ISO_LEAD_FRAMES 4
/* BEGIN FreeBSD sys/dev/usb/usb.h, sys/dev/usb/controller/ohci.c */
/*
 * FreeBSD's USB_BUS_RESET_DELAY and USB_PORT_ROOT_RESET_DELAY, from its
 * sys/dev/usb/usb.h (BSD-2-Clause), as its sys/dev/usb/controller/ohci.c
 * (BSD-2-Clause, ohci_controller_init and ohci_roothub_exec) uses them, and its
 * wait for SMM to let go.
 */
#define BUS_RESET_MS 100
#define ROOT_RESET_MS 200
#define SMM_WAIT_MS 100
/* END FreeBSD sys/dev/usb/usb.h, sys/dev/usb/controller/ohci.c */

struct ohci_hcca {
    uint32_t intr_table[32];
    uint16_t frame_number;
    uint16_t pad;
    uint32_t done_head;
    uint8_t reserved[116];
};

struct ohci_td {
    uint32_t flags;
    uint32_t cbp;
    uint32_t next;
    uint32_t be;
    uint64_t dma;
    struct ohci_td *sw_next;
    uint32_t length;
    bool skipped;
    /* Holds a control transfer's SETUP data, which must be DMA-addressable. */
    uint8_t setup[8];
};

struct ohci_itd {
    uint32_t flags;
    uint32_t bp0;
    uint32_t next;
    uint32_t be;
    uint16_t offset[ITD_PACKETS];
    uint64_t dma;
    struct ohci_itd *sw_next;
    uint32_t first_packet;
    uint32_t count;
};

#define KIND_CONTROL 0
#define KIND_BULK 1
#define KIND_INTR 2
#define KIND_ISO 3
#define KIND_SKELETON 4

/* Waits for endpoints being closed, then completes an asynchronous operation. */
struct ohci_close {
    struct tendril_async *async;
    unsigned int remaining;
    int status;
};

struct ohci_ed {
    uint32_t flags;
    uint32_t tail;
    uint32_t head;
    uint32_t next;
    uint64_t dma;
    struct tendril_endpoint *ep;
    /* The empty descriptor TailP names, a struct ohci_td or, for isochronous, a struct ohci_itd. */
    void *dummy;
    uint64_t dummy_dma;
    /* Hardware list order, around a skeleton descriptor. */
    struct ohci_ed *sw_next;
    struct ohci_ed *sw_prev;
    struct tendril_list node;
    struct tendril_list wait_node;
    int kind;
    size_t level;
    uint32_t wait_frame;
    struct ohci_close *close;
    bool linked;
    bool waiting;
    bool closing;
    /* Isochronous streaming: the frame after the last one scheduled. */
    uint32_t next_frame;
    bool streaming;
};

struct ohci {
    struct tendril_hc *hc;
    size_t num_ports;
    bool per_port_power;
    bool no_power_switching;
    struct ohci_hcca *hcca;
    uint64_t hcca_dma;
    struct ohci_ed *control_head;
    struct ohci_ed *bulk_head;
    struct ohci_ed *skel[INTR_LEVELS];
    struct ohci_ed *iso_head;
    struct tendril_dma_pool ed_pool;
    struct tendril_dma_pool td_pool;
    struct tendril_dma_pool itd_pool;
    /* Every endpoint descriptor in use, for scanning. */
    struct tendril_list eds;
    /* Descriptors waiting for frames to pass: closing ones off the lists, paused ones skipped. */
    struct tendril_list waiting;
    /* List enables held off while control or bulk descriptors are removed. */
    unsigned int control_removals;
    unsigned int bulk_removals;
    uint32_t control;
    uint32_t intr;
    struct tendril_timer timer;
    uint64_t reset_start[15];
    bool resetting[15];
};

/* hcd_data slots in a transfer */
#define XD_FIRST 0
#define XD_LAST 1
#define XD_FLAGS 2
#define XD_ACTUAL 3

#define XF_CANCEL ((uint32_t)1 << 0)
#define XF_SHORT ((uint32_t)1 << 1)

static struct ohci *O(struct tendril_hc *hc) {
    return hc->hcd;
}

static uint32_t reg_read(struct tendril_hc *hc, uint32_t reg) {
    return hc_read32(hc, reg);
}

static void reg_write(struct tendril_hc *hc, uint32_t reg, uint32_t v) {
    hc_write32(hc, reg, v);
}

static uint32_t le(uint32_t v) {
    return tendril_le32(v);
}

/* The 32-bit frame number, extended from the 16-bit counter. */
static uint32_t current_frame(struct tendril_hc *hc) {
    return tendril_frame_update(hc, reg_read(hc, REG_FM_NUMBER) & 0xffff, 0xffff);
}

static struct ohci_td *td_alloc(struct tendril_hc *hc) {
    uint64_t dma;
    struct ohci_td *td = tendril_dma_pool_alloc(&O(hc)->td_pool, &dma);
    if (td != NULL) {
        td->dma = dma;
    }
    return td;
}

static void td_free(struct tendril_hc *hc, struct ohci_td *td) {
    tendril_dma_pool_free(&O(hc)->td_pool, td, td->dma);
}

static struct ohci_itd *itd_alloc(struct tendril_hc *hc) {
    uint64_t dma;
    struct ohci_itd *itd = tendril_dma_pool_alloc(&O(hc)->itd_pool, &dma);
    if (itd != NULL) {
        itd->dma = dma;
    }
    return itd;
}

static void itd_free(struct tendril_hc *hc, struct ohci_itd *itd) {
    tendril_dma_pool_free(&O(hc)->itd_pool, itd, itd->dma);
}

static struct ohci_ed *ed_alloc(struct tendril_hc *hc) {
    uint64_t dma;
    struct ohci_ed *ed = tendril_dma_pool_alloc(&O(hc)->ed_pool, &dma);
    if (ed != NULL) {
        ed->dma = dma;
        list_init(&ed->node);
        list_init(&ed->wait_node);
    }
    return ed;
}

static void ed_free(struct tendril_hc *hc, struct ohci_ed *ed) {
    tendril_dma_pool_free(&O(hc)->ed_pool, ed, ed->dma);
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

static uint64_t cursor_addr(struct cursor *c) {
    while (c->idx < c->num && c->off == c->segs[c->idx].len) {
        c->idx++;
        c->off = 0;
    }
    return c->idx < c->num ? c->segs[c->idx].addr + c->off : 0;
}

/* Bytes contiguous at the cursor up to the end of its 4 KiB page. */
static size_t cursor_page_room(struct cursor *c) {
    uint64_t a = cursor_addr(c);
    if (c->idx >= c->num) {
        return 0;
    }
    size_t seg = c->segs[c->idx].len - c->off;
    size_t page = 4096 - (size_t)(a & 0xfff);
    return MIN(seg, page);
}

static void cursor_advance(struct cursor *c, size_t n) {
    while (n != 0 && c->idx < c->num) {
        size_t room = c->segs[c->idx].len - c->off;
        size_t take = MIN(room, n);
        c->off += take;
        n -= take;
        if (c->off == c->segs[c->idx].len) {
            c->idx++;
            c->off = 0;
        }
    }
}

/*
 * Takes up to `want` bytes from the cursor into one buffer a TD can describe:
 * the rest of one page, then the start of any other (OHCI 1.0a, 4.3.1.3.1). Unless
 * that is all of `want`, the size is kept a multiple of `mps`. Returns the
 * bytes taken, or 0 if the buffer cannot be described.
 */
static size_t take_buffer(struct cursor *c, size_t want, uint16_t mps, uint32_t *cbp, uint32_t *be) {
    struct cursor probe = *c;
    bool last = want <= TD_MAX_BYTES;
    want = MIN(want, (size_t)TD_MAX_BYTES);
    uint64_t start = cursor_addr(&probe);
    size_t first = MIN(cursor_page_room(&probe), want);
    uint64_t second = 0;
    size_t total = first;

    if (first == 0) {
        return 0;
    }
    cursor_advance(&probe, first);
    if (total < want && ((start + first) & 0xfff) == 0) {
        second = cursor_addr(&probe);
        if ((second & 0xfff) == 0) {
            total += MIN(cursor_page_room(&probe), want - total);
        }
    }
    if (total < want || !last) {
        total -= total % mps;
        if (total == 0) {
            return 0;
        }
    }
    *cbp = (uint32_t)start;
    *be = total <= first ? (uint32_t)(start + total - 1) : (uint32_t)(second + (total - first) - 1);
    cursor_advance(c, total);
    return total;
}

/* Bytes a retired TD moved, from what is left between CurrentBufferPointer and BufferEnd. */
static size_t td_moved(const struct ohci_td *td) {
    uint32_t cbp = le(td->cbp);
    uint32_t be = le(td->be);
    if (cbp == 0) {
        return td->length;
    }
    size_t pages = ((be ^ cbp) & ~(uint32_t)0xfff) != 0 ? 0x1000 : 0;
    size_t left = pages + (be & 0xfff) - (cbp & 0xfff) + 1;
    return td->length - MIN(left, (size_t)td->length);
}

static int cc_status(unsigned int cc) {
    switch (cc) {
        case CC_STALL: {
            return TENDRIL_ERR_STALL;
        }
        case CC_DATA_OVERRUN: {
            return TENDRIL_ERR_BABBLE;
        }
        case CC_BUFFER_OVERRUN:
        case CC_BUFFER_UNDERRUN: {
            return TENDRIL_ERR_OVERRUN;
        }
        default: {
            return TENDRIL_ERR_IO;
        }
    }
}

/* Endpoint descriptors */

static uint32_t ed_flags(struct tendril_endpoint *ep) {
    struct tendril_device *dev = ep->dev;
    uint32_t f = (uint32_t)(dev->address & 0x7f) | (uint32_t)(ep->address & TENDRIL_ENDPOINT_NUMBER_MASK) << 7
        | (uint32_t)(ep->max_packet & 0x7ff) << 16;
    if (dev->speed == TENDRIL_SPEED_LOW) {
        f |= ED_LOW_SPEED;
    }
    if (ep->type != TENDRIL_ENDPOINT_XFER_CONTROL) {
        f |= (ep->address & TENDRIL_ENDPOINT_DIR_MASK) ? ED_DIR_IN : ED_DIR_OUT;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        f |= ED_ISO;
    }
    return f;
}

static struct ohci_ed *list_head(struct tendril_hc *hc, struct ohci_ed *ed) {
    struct ohci *o = O(hc);
    switch (ed->kind) {
        case KIND_CONTROL: {
            return o->control_head;
        }
        case KIND_BULK: {
            return o->bulk_head;
        }
        case KIND_INTR: {
            return o->skel[ed->level];
        }
        default: {
            return o->iso_head;
        }
    }
}

/* Puts a descriptor right after its list's skeleton, where the controller picks it up next time round. */
static void link_ed(struct tendril_hc *hc, struct ohci_ed *ed) {
    struct ohci_ed *head = list_head(hc, ed);
    ed->next = head->next;
    ed->sw_next = head->sw_next;
    ed->sw_prev = head;
    head->sw_next->sw_prev = ed;
    head->sw_next = ed;
    dma_barrier();
    head->next = le((uint32_t)ed->dma);
    ed->linked = true;
}

static void unlink_ed(struct ohci_ed *ed) {
    struct ohci_ed *prev = ed->sw_prev;
    prev->next = ed->next;
    prev->sw_next = ed->sw_next;
    ed->sw_next->sw_prev = prev;
    ed->sw_next = ed;
    ed->sw_prev = ed;
    ed->linked = false;
    dma_barrier();
}

static size_t intr_level(struct tendril_endpoint *ep) {
    size_t log2 = ep->interval_log2 > 3 ? ep->interval_log2 - (uint32_t)3 : (uint32_t)0;
    return MIN(log2, (size_t)INTR_LEVELS - 1);
}

static void set_list_enables(struct tendril_hc *hc) {
    struct ohci *o = O(hc);
    uint32_t ctl = o->control | CTL_PLE | CTL_IE | CTL_CLE | CTL_BLE;
    if (o->control_removals != 0) {
        ctl &= ~CTL_CLE;
    }
    if (o->bulk_removals != 0) {
        ctl &= ~CTL_BLE;
    }
    reg_write(hc, REG_CONTROL, ctl);
}

/*
 * Waits two frame starts before touching a descriptor the controller may be
 * using (OHCI 1.0a, 5.2.7.1.2-3): a removed one may still be in hand, and a skipped
 * one may be mid-transaction.
 */
static void wait_frames(struct tendril_hc *hc, struct ohci_ed *ed) {
    struct ohci *o = O(hc);
    if (ed->waiting) {
        return;
    }
    ed->waiting = true;
    ed->wait_frame = current_frame(hc);
    list_add_tail(&o->waiting, &ed->wait_node);
    tendril_timer_arm(hc, &o->timer, NS_PER_MS);
}

/* Transfers */

static struct ohci_td *xfer_first(struct tendril_transfer *xfer) {
    return (struct ohci_td *)xfer->internal.hcd_data[XD_FIRST];
}

static struct ohci_td *xfer_last(struct tendril_transfer *xfer) {
    return (struct ohci_td *)xfer->internal.hcd_data[XD_LAST];
}

static bool is_in(struct tendril_transfer *xfer) {
    if (xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL) {
        return (xfer->setup.request_type & TENDRIL_DIR_IN) != 0;
    }
    return (xfer->endpoint->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
}

static void free_chain(struct tendril_hc *hc, struct ohci_td *td, struct ohci_td *stop) {
    while (td != NULL && td != stop) {
        struct ohci_td *next = td->sw_next;
        td_free(hc, td);
        td = next;
    }
}

static void free_xfer_tds(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    free_chain(hc, xfer_first(xfer), xfer_last(xfer)->sw_next);
}

static void fill_td(struct ohci_td *td, uint32_t flags, uint32_t cbp, uint32_t be, uint32_t length) {
    td->flags = le(flags | TD_NOT_ACCESSED | TD_DI(0));
    td->cbp = le(cbp);
    td->be = le(length != 0 ? be : 0);
    td->length = length;
    td->skipped = false;
}

/* Queues a control, bulk, or interrupt transfer behind the descriptor's dummy TD. */
static void list_filled(struct tendril_hc *hc, struct ohci_ed *ed);

static int submit_general(struct tendril_hc *hc, struct ohci_ed *ed, struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    bool control = ep->type == TENDRIL_ENDPOINT_XFER_CONTROL;
    bool in = is_in(xfer);
    uint16_t mps = ep->max_packet;
    size_t len = control ? tendril_le16(xfer->setup.length) : xfer->length;
    bool zlp = !in && (xfer->flags & TENDRIL_XFER_ZERO_PACKET) && len != 0 && len % mps == 0;
    struct ohci_td *first = ed->dummy;
    struct ohci_td *dummy = td_alloc(hc);
    struct ohci_td *td = first;
    struct cursor c;

    if (dummy == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    cursor_init(&c, xfer);
    td->sw_next = NULL;

    if (control) {
        memcpy(td->setup, &xfer->setup, 8);
        uint32_t a = (uint32_t)(td->dma + offsetof(struct ohci_td, setup));
        fill_td(td, TD_DP_SETUP | TD_DATA0, a, a + 7, 8);
    }

    /* Data: control toggles start at DATA1, bulk and interrupt carry theirs in the ED. */
    uint32_t dir = in ? TD_DP_IN : TD_DP_OUT;
    unsigned int packets = 1;
    size_t done = 0;
    bool any = false;
    while (done < len || (!control && !any) || zlp) {
        if (control || any) {
            struct ohci_td *n = td_alloc(hc);
            if (n == NULL) {
                free_chain(hc, first->sw_next, NULL);
                first->sw_next = NULL;
                td_free(hc, dummy);
                return TENDRIL_ERR_NO_MEMORY;
            }
            n->sw_next = NULL;
            td->sw_next = n;
            td->next = le((uint32_t)n->dma);
            td = n;
        }
        uint32_t cbp = 0;
        uint32_t be = 0;
        size_t chunk = 0;
        if (done < len) {
            chunk = take_buffer(&c, len - done, mps, &cbp, &be);
            if (chunk == 0) {
                free_chain(hc, first->sw_next, NULL);
                first->sw_next = NULL;
                td_free(hc, dummy);
                return TENDRIL_ERR_BOUNCE;
            }
        } else if (any) {
            zlp = false;
        }
        uint32_t toggle = control ? ((packets & 1) ? TD_DATA1 : TD_DATA0) : TD_TOGGLE_CARRY;
        bool last_data = done + chunk == len;
        /* Only the last IN TD may end short without an error; earlier ones halt on underrun. */
        fill_td(td, dir | toggle | (in && last_data ? TD_ROUNDING : 0), cbp, be, (uint32_t)chunk);
        packets += (unsigned int)((chunk + mps - 1) / mps);
        done += chunk;
        any = true;
    }

    if (control) {
        struct ohci_td *n = td_alloc(hc);
        if (n == NULL) {
            free_chain(hc, first->sw_next, NULL);
            first->sw_next = NULL;
            td_free(hc, dummy);
            return TENDRIL_ERR_NO_MEMORY;
        }
        n->sw_next = NULL;
        td->sw_next = n;
        td->next = le((uint32_t)n->dma);
        td = n;
        fill_td(td, (in && len != 0 ? TD_DP_OUT : TD_DP_IN) | TD_DATA1, 0, 0, 0);
    }

    td->sw_next = dummy;
    td->next = le((uint32_t)dummy->dma);
    dummy->sw_next = NULL;
    xfer->internal.hcd_data[XD_FIRST] = (uintptr_t)first;
    xfer->internal.hcd_data[XD_LAST] = (uintptr_t)td;
    xfer->internal.hcd_data[XD_FLAGS] = 0;
    xfer->internal.hcd_data[XD_ACTUAL] = 0;
    ed->dummy = dummy;
    ed->dummy_dma = dummy->dma;
    dma_barrier();
    ed->tail = le((uint32_t)dummy->dma);
    dma_barrier();
    list_filled(hc, ed);
    return TENDRIL_OK;
}

/* The TD the controller works on next, with the halt and carry bits dropped. */
static uint32_t ed_head(struct ohci_ed *ed) {
    return le(ed->head) & ED_PTR_MASK;
}

/*
 * Tells the controller a control or bulk list has TDs to run (OHCI 1.0a,
 * HcCommandStatus CLF and BLF); once it has walked a list and found nothing,
 * it does not look again until told.
 */
static void list_filled(struct tendril_hc *hc, struct ohci_ed *ed) {
    if (ed->kind == KIND_CONTROL) {
        reg_write(hc, REG_COMMAND_STATUS, CMD_CLF);
    } else if (ed->kind == KIND_BULK) {
        reg_write(hc, REG_COMMAND_STATUS, CMD_BLF);
    }
}

/* Points the halted or skipped descriptor at `td`, keeping the toggle carry, and has the controller look again. */
static void ed_restart(struct tendril_hc *hc, struct ohci_ed *ed, uint64_t td_dma, uint32_t carry) {
    ed->head = le((uint32_t)td_dma | carry);
    dma_barrier();
    list_filled(hc, ed);
}

/*
 * Checks the oldest transfer of a descriptor. TDs retire in order, so those
 * ahead of HeadP are done. Returns true if the transfer completed.
 */
static bool scan_general(struct tendril_hc *hc, struct ohci_ed *ed, struct tendril_transfer *xfer) {
    bool control = xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL;
    struct ohci_td *first = xfer_first(xfer);
    struct ohci_td *last = xfer_last(xfer);
    uint32_t hp = ed_head(ed);
    size_t actual = 0;

    for (struct ohci_td *td = first;; td = td->sw_next) {
        if (!td->skipped) {
            if ((uint32_t)td->dma == hp) {
                return false;
            }
            dma_barrier();
            uint32_t flags = le(td->flags);
            unsigned int cc = flags >> TD_CC_SHIFT;
            bool data = !control || (td != first && td != last);
            if (data) {
                actual += td_moved(td);
            }
            if (cc == CC_DATA_UNDERRUN && data && (flags & TD_DP_MASK) == TD_DP_IN) {
                /* A short packet ended the data early; the rest of the data stage never runs. */
                struct ohci_td *resume = control ? last : last->sw_next;
                for (struct ohci_td *s = td->sw_next; s != resume; s = s->sw_next) {
                    s->skipped = true;
                }
                if (!(xfer->internal.hcd_data[XD_FLAGS] & XF_SHORT)) {
                    xfer->internal.hcd_data[XD_FLAGS] |= XF_SHORT;
                    ed_restart(hc, ed, resume->dma, le(ed->head) & ED_CARRY);
                }
                if (!control) {
                    break;
                }
            } else if (cc != CC_NOERROR) {
                /* The descriptor halted: drop the rest of the transfer and go on with the next. */
                struct ohci_td *resume = last->sw_next;
                uint32_t carry = le(ed->head) & ED_CARRY;
                free_xfer_tds(hc, xfer);
                ed_restart(hc, ed, resume->dma, carry);
                tendril_transfer_complete(hc, xfer, cc_status(cc), actual);
                return true;
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

/* Isochronous transfers */

static struct ohci_itd *xfer_first_itd(struct tendril_transfer *xfer) {
    return (struct ohci_itd *)xfer->internal.hcd_data[XD_FIRST];
}

static struct ohci_itd *xfer_last_itd(struct tendril_transfer *xfer) {
    return (struct ohci_itd *)xfer->internal.hcd_data[XD_LAST];
}

static void free_itd_chain(struct tendril_hc *hc, struct ohci_itd *itd, struct ohci_itd *stop) {
    while (itd != NULL && itd != stop) {
        struct ohci_itd *next = itd->sw_next;
        itd_free(hc, itd);
        itd = next;
    }
}

/*
 * Packs as many of the packets starting at `p` as fit one ITD: up to eight,
 * back to back in the rest of one page and the start of another, the same
 * two-page buffer a general TD has (OHCI 1.0a, 4.3.2.3.1). Returns the count.
 */
static uint32_t fill_itd(
    struct ohci_itd *itd, struct tendril_transfer *xfer, uint32_t p, struct cursor *c, uint32_t frame
) {
    uint64_t start = cursor_addr(c);
    size_t first = cursor_page_room(c);
    size_t room = first;
    uint64_t page1 = 0;
    size_t used = 0;
    uint32_t n = 0;

    if ((start + first) % 4096 == 0) {
        struct cursor probe = *c;
        cursor_advance(&probe, first);
        page1 = cursor_addr(&probe);
        if (page1 != 0 && page1 % 4096 == 0) {
            room += cursor_page_room(&probe);
        } else {
            page1 = 0;
        }
    }
    while (p + n < xfer->num_iso_packets && n < ITD_PACKETS) {
        uint32_t len = xfer->iso_packets[p + n].length;
        if (used + len > room) {
            break;
        }
        uint32_t off = used < first ? (uint32_t)((start + used) & 0xfff) : (uint32_t)(0x1000 | (used - first));
        itd->offset[n] = tendril_le16((uint16_t)(ITD_OFFSET_FRESH | off));
        used += len;
        n++;
    }
    if (n == 0) {
        return 0;
    }
    /* BufferEnd is the last byte; with no data, the byte before the start. */
    uint64_t be = used <= first ? start + used - 1 : page1 + (used - first) - 1;
    itd->flags = le((frame & 0xffff) | TD_DI(0) | ITD_FC(n - 1) | TD_NOT_ACCESSED);
    itd->bp0 = le((uint32_t)(start & ~(uint64_t)0xfff));
    itd->be = le((uint32_t)be);
    itd->first_packet = p;
    itd->count = n;
    cursor_advance(c, used);
    return n;
}

static int submit_iso(struct tendril_hc *hc, struct ohci_ed *ed, struct tendril_transfer *xfer) {
    uint32_t now = current_frame(hc);
    uint32_t frame;
    struct cursor c;

    if (!(xfer->flags & TENDRIL_XFER_ISO_ASAP)) {
        frame = xfer->start_frame;
    } else if (ed->streaming && (int32_t)(ed->next_frame - now) > 0) {
        /* Back to back with what is queued. */
        frame = ed->next_frame;
    } else {
        frame = now + ISO_LEAD_FRAMES;
    }
    if ((int32_t)(frame - now) <= 0 || (int32_t)(frame - now) >= 0x8000) {
        return TENDRIL_ERR_MISSED;
    }

    struct ohci_itd *first = ed->dummy;
    struct ohci_itd *itd = first;
    uint32_t p = 0;
    cursor_init(&c, xfer);
    itd->sw_next = NULL;
    for (;;) {
        uint32_t n = fill_itd(itd, xfer, p, &c, frame + p);
        struct ohci_itd *next = n != 0 ? itd_alloc(hc) : NULL;
        if (next == NULL) {
            free_itd_chain(hc, first->sw_next, NULL);
            first->sw_next = NULL;
            return n == 0 ? TENDRIL_ERR_BOUNCE : TENDRIL_ERR_NO_MEMORY;
        }
        p += n;
        next->sw_next = NULL;
        itd->sw_next = next;
        itd->next = le((uint32_t)next->dma);
        if (p == xfer->num_iso_packets) {
            /* The last ITD allocated becomes the dummy. */
            xfer->internal.hcd_data[XD_FIRST] = (uintptr_t)first;
            xfer->internal.hcd_data[XD_LAST] = (uintptr_t)itd;
            xfer->internal.hcd_data[XD_FLAGS] = 0;
            xfer->start_frame = frame;
            ed->dummy = next;
            ed->dummy_dma = next->dma;
            /* What is queued ends with whichever transfer ends last. */
            if (!ed->streaming || (int32_t)(frame + p - ed->next_frame) > 0) {
                ed->next_frame = frame + p;
            }
            ed->streaming = true;
            dma_barrier();
            ed->tail = le((uint32_t)next->dma);
            dma_barrier();
            return TENDRIL_OK;
        }
        itd = next;
    }
}

static int psw_status(unsigned int cc) {
    switch (cc) {
        case CC_NOERROR:
        case CC_DATA_UNDERRUN: {
            return TENDRIL_OK;
        }
        case CC_NOT_ACCESSED:
        case CC_NOT_ACCESSED + 1: {
            return TENDRIL_ERR_MISSED;
        }
        default: {
            return cc_status(cc);
        }
    }
}

static bool scan_iso(struct tendril_hc *hc, struct ohci_ed *ed, struct tendril_transfer *xfer) {
    struct ohci_itd *first = xfer_first_itd(xfer);
    struct ohci_itd *last = xfer_last_itd(xfer);
    uint32_t hp = ed_head(ed);
    bool in = is_in(xfer);
    size_t actual = 0;
    int st = TENDRIL_OK;

    for (struct ohci_itd *itd = first;; itd = itd->sw_next) {
        if ((uint32_t)itd->dma == hp) {
            return false;
        }
        if (itd == last) {
            break;
        }
    }
    dma_barrier();
    for (struct ohci_itd *itd = first;; itd = itd->sw_next) {
        for (uint32_t i = 0; i < itd->count; i++) {
            struct tendril_iso_packet *pkt = &xfer->iso_packets[itd->first_packet + i];
            uint16_t psw = tendril_le16(itd->offset[i]);
            unsigned int cc = psw >> (uint32_t)12;
            pkt->status = psw_status(cc);
            pkt->actual = pkt->status == TENDRIL_OK ? (in ? psw & (uint32_t)0x7ff : pkt->length) : 0;
            actual += pkt->actual;
            if (pkt->status != TENDRIL_OK && st == TENDRIL_OK) {
                st = pkt->status;
            }
        }
        if (itd == last) {
            break;
        }
    }
    free_itd_chain(hc, first, last->sw_next);
    /* Packet errors are in the packets; the transfer itself completed. */
    (void)st;
    tendril_transfer_complete(hc, xfer, TENDRIL_OK, actual);
    return true;
}

static void scan_ed(struct tendril_hc *hc, struct ohci_ed *ed) {
    struct tendril_endpoint *ep = ed->ep;
    while (!list_empty(&ep->transfers)) {
        struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        if (xfer->internal.hcd_data[XD_FLAGS] & XF_CANCEL) {
            break;
        }
        bool done = ed->kind == KIND_ISO ? scan_iso(hc, ed, xfer) : scan_general(hc, ed, xfer);
        if (!done) {
            break;
        }
    }
    if (ed->kind == KIND_ISO && list_empty(&ep->transfers)) {
        ed->streaming = false;
    }
}

static void scan_all(struct tendril_hc *hc) {
    struct ohci *o = O(hc);
    struct tendril_list *it;
    LIST_FOR_EACH(it, &o->eds) {
        struct ohci_ed *ed = CONTAINER_OF(it, struct ohci_ed, node);
        if (!ed->closing) {
            scan_ed(hc, ed);
        }
    }
}

/*
 * With the descriptor skipped and idle: completes cancelled transfers, then
 * relinks the rest behind HeadP, keeping the data toggle.
 */
static void sweep_ed(struct tendril_hc *hc, struct ohci_ed *ed) {
    struct tendril_endpoint *ep = ed->ep;
    struct tendril_list *it;
    struct tendril_list *tmp;
    bool iso = ed->kind == KIND_ISO;

    /* Whatever finished before the pause completes normally. */
    while (!list_empty(&ep->transfers)) {
        struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        if (!(iso ? scan_iso(hc, ed, xfer) : scan_general(hc, ed, xfer))) {
            break;
        }
    }

    uint32_t head = le(ed->head);
    uint32_t carry = head & ED_CARRY;
    struct tendril_transfer *oldest = list_empty(&ep->transfers) ? NULL
        : CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
    bool restart = false;

    LIST_FOR_EACH_SAFE(it, tmp, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        if (!(xfer->internal.hcd_data[XD_FLAGS] & XF_CANCEL)) {
            continue;
        }
        size_t actual = 0;
        if (xfer == oldest) {
            /* HeadP is inside this transfer, so it moves to the next. */
            restart = true;
            bool control = xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL;
            for (struct ohci_td *td = xfer_first(xfer); !iso && td != xfer_last(xfer)->sw_next; td = td->sw_next) {
                if ((uint32_t)td->dma == (head & ED_PTR_MASK)) {
                    /* A TD part way through holds a toggle the carry has not caught up with. */
                    uint32_t t = le(td->flags) & TD_TOGGLE_MASK;
                    if (!control && (t & TD_DATA0) == TD_DATA0) {
                        carry = (t & ((uint32_t)1 << 24)) ? ED_CARRY : 0;
                    }
                    break;
                }
                if (!td->skipped && !(control && (td == xfer_first(xfer) || td == xfer_last(xfer)))) {
                    actual += td_moved(td);
                }
            }
        }
        if (iso) {
            free_itd_chain(hc, xfer_first_itd(xfer), xfer_last_itd(xfer)->sw_next);
        } else {
            free_xfer_tds(hc, xfer);
        }
        tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, actual);
    }

    /* Rebuild the links between the transfers left, in order, ending at the dummy. */
    struct tendril_transfer *prev = NULL;
    LIST_FOR_EACH(it, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        if (prev != NULL) {
            if (iso) {
                xfer_last_itd(prev)->sw_next = xfer_first_itd(xfer);
                xfer_last_itd(prev)->next = le((uint32_t)xfer_first_itd(xfer)->dma);
            } else {
                xfer_last(prev)->sw_next = xfer_first(xfer);
                xfer_last(prev)->next = le((uint32_t)xfer_first(xfer)->dma);
            }
        }
        prev = xfer;
    }
    if (prev != NULL) {
        if (iso) {
            xfer_last_itd(prev)->sw_next = ed->dummy;
            xfer_last_itd(prev)->next = le((uint32_t)ed->dummy_dma);
        } else {
            xfer_last(prev)->sw_next = ed->dummy;
            xfer_last(prev)->next = le((uint32_t)ed->dummy_dma);
        }
    }
    if (restart) {
        uint64_t next = ed->dummy_dma;
        if (!list_empty(&ep->transfers)) {
            struct tendril_transfer *h = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
            next = iso ? xfer_first_itd(h)->dma : xfer_first(h)->dma;
        }
        ed_restart(hc, ed, next, iso ? 0 : carry);
    }
    if (iso && list_empty(&ep->transfers)) {
        ed->streaming = false;
    }
}

/* Descriptors waiting on frames */

static void close_done(struct tendril_hc *hc, struct ohci_close *close) {
    if (close != NULL && --close->remaining == 0) {
        tendril_async_complete(hc, close->async, close->status);
        tendril_free(close, sizeof(struct ohci_close));
    }
}

static void fail_transfers(struct tendril_hc *hc, struct ohci_ed *ed, int status) {
    struct tendril_endpoint *ep = ed->ep;
    while (!list_empty(&ep->transfers)) {
        struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        if (ed->kind == KIND_ISO) {
            free_itd_chain(hc, xfer_first_itd(xfer), xfer_last_itd(xfer)->sw_next);
        } else {
            free_xfer_tds(hc, xfer);
        }
        tendril_transfer_complete(hc, xfer, status, 0);
    }
}

/* Frees a descriptor the controller can no longer see. */
static void free_ed(struct tendril_hc *hc, struct ohci_ed *ed) {
    struct ohci_close *close = ed->close;
    fail_transfers(hc, ed, TENDRIL_ERR_NO_DEVICE);
    list_del(&ed->node);
    if (ed->kind == KIND_ISO) {
        itd_free(hc, ed->dummy);
    } else {
        td_free(hc, ed->dummy);
    }
    ed_free(hc, ed);
    close_done(hc, close);
}

static void finish_wait(struct tendril_hc *hc, struct ohci_ed *ed) {
    struct ohci *o = O(hc);
    list_del(&ed->wait_node);
    ed->waiting = false;
    if (ed->closing) {
        if (ed->kind == KIND_CONTROL) {
            o->control_removals--;
        } else if (ed->kind == KIND_BULK) {
            o->bulk_removals--;
        }
        free_ed(hc, ed);
        return;
    }
    sweep_ed(hc, ed);
    ed->flags = le(le(ed->flags) & ~ED_SKIP);
    dma_barrier();
    if (!list_empty(&ed->ep->transfers)) {
        list_filled(hc, ed);
    }
}

static void process_waiting(struct tendril_hc *hc) {
    struct ohci *o = O(hc);
    struct tendril_list *it;
    struct tendril_list *tmp;
    uint32_t frame = current_frame(hc);
    bool control = false;
    bool bulk = false;

    LIST_FOR_EACH(it, &o->waiting) {
        struct ohci_ed *ed = CONTAINER_OF(it, struct ohci_ed, wait_node);
        if (ed->closing && (hc->dead || frame - ed->wait_frame >= 2)) {
            control |= ed->kind == KIND_CONTROL;
            bulk |= ed->kind == KIND_BULK;
        }
    }
    /* OHCI 1.0a, 5.2.7.1.2: with the list stopped, its current pointer may still name a removed descriptor. */
    if (control) {
        reg_write(hc, REG_CONTROL_CURRENT_ED, 0);
    }
    if (bulk) {
        reg_write(hc, REG_BULK_CURRENT_ED, 0);
    }
    LIST_FOR_EACH_SAFE(it, tmp, &o->waiting) {
        struct ohci_ed *ed = CONTAINER_OF(it, struct ohci_ed, wait_node);
        if (hc->dead || frame - ed->wait_frame >= 2) {
            finish_wait(hc, ed);
        }
    }
    if (!hc->dead && (control || bulk)) {
        set_list_enables(hc);
    }
    if (!list_empty(&o->waiting) && !hc->dead) {
        tendril_timer_arm(hc, &o->timer, NS_PER_MS);
    }
}

static void ohci_timer(struct tendril_work *work) {
    struct ohci *o = CONTAINER_OF(work, struct ohci, timer.work);
    struct tendril_hc *hc = o->hc;
    hc_lock(hc);
    if (!hc->dead) {
        process_waiting(hc);
    }
    hc_unlock(hc);
}

/* Transfers */

static int ohci_submit(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct ohci_ed *ed = xfer->endpoint->hcd;
    if (ed == NULL || ed->closing) {
        return TENDRIL_ERR_INVALID;
    }
    if (ed->kind == KIND_ISO) {
        return submit_iso(hc, ed, xfer);
    }
    return submit_general(hc, ed, xfer);
}

static void ohci_cancel(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct ohci_ed *ed = xfer->endpoint->hcd;
    if (hc->dead || ed == NULL) {
        if (ed != NULL) {
            if (ed->kind == KIND_ISO) {
                free_itd_chain(hc, xfer_first_itd(xfer), xfer_last_itd(xfer)->sw_next);
            } else {
                free_xfer_tds(hc, xfer);
            }
        }
        tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, 0);
        return;
    }
    xfer->internal.hcd_data[XD_FLAGS] |= XF_CANCEL;
    /* OHCI 1.0a, 5.2.7.1.3: skip the descriptor, then change its queue once a frame has passed. */
    if (!ed->waiting) {
        ed->flags = le(le(ed->flags) | ED_SKIP);
        dma_barrier();
        wait_frames(hc, ed);
    }
}

/* Endpoints */

static int open_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep) {
    struct ohci *o = O(hc);
    bool iso = ep->type == TENDRIL_ENDPOINT_XFER_ISOC;

    if (ep->dev->speed > TENDRIL_SPEED_FULL || (iso && ep->dev->speed == TENDRIL_SPEED_LOW)) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    struct ohci_ed *ed = ed_alloc(hc);
    if (ed == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    if (iso) {
        struct ohci_itd *itd = itd_alloc(hc);
        ed->dummy = itd;
        ed->dummy_dma = itd != NULL ? itd->dma : 0;
    } else {
        struct ohci_td *td = td_alloc(hc);
        ed->dummy = td;
        ed->dummy_dma = td != NULL ? td->dma : 0;
    }
    if (ed->dummy == NULL) {
        ed_free(hc, ed);
        return TENDRIL_ERR_NO_MEMORY;
    }
    switch (ep->type) {
        case TENDRIL_ENDPOINT_XFER_CONTROL: {
            ed->kind = KIND_CONTROL;
            break;
        }
        case TENDRIL_ENDPOINT_XFER_BULK: {
            ed->kind = KIND_BULK;
            break;
        }
        case TENDRIL_ENDPOINT_XFER_INT: {
            ed->kind = KIND_INTR;
            ed->level = intr_level(ep);
            break;
        }
        default: {
            ed->kind = KIND_ISO;
            break;
        }
    }
    ed->ep = ep;
    ed->flags = le(ed_flags(ep));
    ed->tail = le((uint32_t)ed->dummy_dma);
    ed->head = le((uint32_t)ed->dummy_dma);
    ed->sw_next = ed;
    ed->sw_prev = ed;
    list_add_tail(&o->eds, &ed->node);
    link_ed(hc, ed);
    ep->hcd = ed;
    ep->open = true;
    return TENDRIL_OK;
}

static void close_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep, struct ohci_close *close) {
    struct ohci *o = O(hc);
    struct ohci_ed *ed = ep->hcd;

    ep->open = false;
    ep->hcd = NULL;
    if (ed == NULL) {
        return;
    }
    ed->closing = true;
    ed->close = close;
    if (close != NULL) {
        close->remaining++;
    }
    if (ed->linked) {
        ed->flags = le(le(ed->flags) | ED_SKIP);
        unlink_ed(ed);
        if (!hc->dead && ed->kind == KIND_CONTROL) {
            o->control_removals++;
            set_list_enables(hc);
        } else if (!hc->dead && ed->kind == KIND_BULK) {
            o->bulk_removals++;
            set_list_enables(hc);
        }
    }
    if (hc->dead) {
        if (ed->waiting) {
            list_del(&ed->wait_node);
            ed->waiting = false;
        }
        free_ed(hc, ed);
        return;
    }
    if (ed->waiting) {
        /* Paused already; the removal needs its own frames, counted from now. */
        ed->wait_frame = current_frame(hc);
    } else {
        wait_frames(hc, ed);
    }
}

static int ohci_configure(
    struct tendril_hc *hc, struct tendril_device *dev, struct tendril_endpoint **drop, size_t num_drop,
    struct tendril_endpoint **add, size_t num_add, struct tendril_async *async
) {
    (void)dev;
    struct ohci_close *close = tendril_zalloc(sizeof(struct ohci_close));
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

static int ohci_device_init(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    int st = open_endpoint(hc, &dev->ep0);
    if (st == TENDRIL_OK) {
        tendril_async_complete(hc, async, TENDRIL_OK);
    }
    return st;
}

static int ohci_ep0_update(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    struct ohci_ed *ed = dev->ep0.hcd;
    /* Nothing is queued on endpoint 0 here, so its address and packet size can change in place. */
    ed->flags = le(ed_flags(&dev->ep0) | (le(ed->flags) & ED_SKIP));
    dma_barrier();
    tendril_async_complete(hc, async, TENDRIL_OK);
    return TENDRIL_OK;
}

static int ohci_endpoint_reset(struct tendril_hc *hc, struct tendril_endpoint *ep, struct tendril_async *async) {
    struct ohci_ed *ed = ep->hcd;
    /* With no TDs queued the controller leaves HeadP alone, so the carry can go back to DATA0. */
    if (ed != NULL && ed->kind != KIND_ISO && list_empty(&ep->transfers)) {
        ed->head = le(le(ed->head) & ~(ED_CARRY | ED_HALTED));
        dma_barrier();
    }
    tendril_async_complete(hc, async, TENDRIL_OK);
    return TENDRIL_OK;
}

static void ohci_device_free(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    struct ohci_close *close = tendril_zalloc(sizeof(struct ohci_close));
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

static void ohci_port_status(struct tendril_hc *hc, size_t port, struct tendril_port_status *st) {
    struct ohci *o = O(hc);
    size_t i = port - 1;
    uint32_t ps = reg_read(hc, REG_RH_PORT(port));

    /* BEGIN FreeBSD sys/dev/usb/controller/ohci.c */
    /* The controller resets for 10 ms; as with FreeBSD, the reset is over after ROOT_RESET_MS. */
    if (o->resetting[i] && !(ps & PS_PRS) && tendril_now() - o->reset_start[i] >= ROOT_RESET_MS * NS_PER_MS) {
        o->resetting[i] = false;
    }
    /* END FreeBSD sys/dev/usb/controller/ohci.c */

    st->status = 0;
    st->change = 0;
    st->speed = (ps & PS_LSDA) ? TENDRIL_SPEED_LOW : TENDRIL_SPEED_FULL;
    if (ps & PS_CCS) {
        st->status |= TENDRIL_PORT_STAT_CONNECTION;
    }
    if (ps & PS_PES) {
        st->status |= TENDRIL_PORT_STAT_ENABLE;
    }
    if (ps & PS_PSS) {
        st->status |= TENDRIL_PORT_STAT_SUSPEND;
    }
    if (ps & PS_POCI) {
        st->status |= TENDRIL_PORT_STAT_OVER_CURRENT;
    }
    if ((ps & PS_PRS) || o->resetting[i]) {
        st->status |= TENDRIL_PORT_STAT_RESET;
    }
    if ((ps & PS_PPS) || o->no_power_switching) {
        st->status |= TENDRIL_PORT_STAT_POWER;
    }
    if (ps & PS_LSDA) {
        st->status |= TENDRIL_PORT_STAT_LOW_SPEED;
    }
    if (ps & PS_CSC) {
        st->change |= TENDRIL_PORT_STAT_C_CONNECTION;
    }
    if (ps & PS_PESC) {
        st->change |= TENDRIL_PORT_STAT_C_ENABLE;
    }
    if (ps & PS_PSSC) {
        st->change |= TENDRIL_PORT_STAT_C_SUSPEND;
    }
    if (ps & PS_OCIC) {
        st->change |= TENDRIL_PORT_STAT_C_OVER_CURRENT;
    }
    if ((ps & PS_PRSC) && !o->resetting[i]) {
        st->change |= TENDRIL_PORT_STAT_C_RESET;
    }
}

static int ohci_port_set_feature(struct tendril_hc *hc, size_t port, unsigned int feature) {
    struct ohci *o = O(hc);
    switch (feature) {
        case TENDRIL_PORT_FEAT_RESET: {
            reg_write(hc, REG_RH_PORT(port), PS_SET_RESET);
            o->resetting[port - 1] = true;
            o->reset_start[port - 1] = tendril_now();
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_POWER: {
            if (o->per_port_power) {
                reg_write(hc, REG_RH_PORT(port), PS_SET_POWER);
            } else {
                reg_write(hc, REG_RH_STATUS, RHS_LPSC);
            }
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            reg_write(hc, REG_RH_PORT(port), PS_SET_SUSPEND);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_ENABLE: {
            reg_write(hc, REG_RH_PORT(port), PS_SET_ENABLE);
            return TENDRIL_OK;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
}

static int ohci_port_clear_feature(struct tendril_hc *hc, size_t port, unsigned int feature) {
    uint32_t bit;
    switch (feature) {
        case TENDRIL_PORT_FEAT_ENABLE: {
            if (hc->quirks & TENDRIL_HC_QUIRK_NO_PORT_DISABLE) {
                return TENDRIL_OK;
            }
            bit = PS_CLEAR_ENABLE;
            break;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            /* The controller times the resume and reports it with C_PORT_SUSPEND. */
            bit = PS_CLEAR_SUSPEND;
            break;
        }
        case TENDRIL_PORT_FEAT_POWER: {
            if (!O(hc)->per_port_power) {
                return TENDRIL_OK;
            }
            bit = PS_CLEAR_POWER;
            break;
        }
        case TENDRIL_PORT_FEAT_C_CONNECTION: {
            bit = PS_CSC;
            break;
        }
        case TENDRIL_PORT_FEAT_C_ENABLE: {
            bit = PS_PESC;
            break;
        }
        case TENDRIL_PORT_FEAT_C_SUSPEND: {
            bit = PS_PSSC;
            break;
        }
        case TENDRIL_PORT_FEAT_C_OVER_CURRENT: {
            bit = PS_OCIC;
            break;
        }
        case TENDRIL_PORT_FEAT_C_RESET: {
            bit = PS_PRSC;
            break;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
    reg_write(hc, REG_RH_PORT(port), bit);
    return TENDRIL_OK;
}

/* Controller */

static void fail_everything(struct tendril_hc *hc) {
    struct ohci *o = O(hc);
    struct tendril_list *it;
    struct tendril_list *tmp;

    LIST_FOR_EACH_SAFE(it, tmp, &o->waiting) {
        finish_wait(hc, CONTAINER_OF(it, struct ohci_ed, wait_node));
    }
    LIST_FOR_EACH(it, &o->eds) {
        struct ohci_ed *ed = CONTAINER_OF(it, struct ohci_ed, node);
        fail_transfers(hc, ed, TENDRIL_ERR_NO_DEVICE);
    }
}

static void controller_died(struct tendril_hc *hc, const char *why) {
    if (hc->dead) {
        return;
    }
    LOG_ERROR("OHCI: controller failed: %s", why);
    hc->dead = true;
    reg_write(hc, REG_INTR_DISABLE, INT_ALL | INT_MIE);
    reg_write(hc, REG_CONTROL, CTL_HCFS_RESET);
    fail_everything(hc);
}

static void ohci_poll(struct tendril_hc *hc) {
    struct ohci *o = O(hc);
    uint32_t sts = reg_read(hc, REG_INTR_STATUS);
    if (sts == 0xffffffff) {
        controller_died(hc, "controller removed");
        return;
    }
    /* Completions are found by scanning, so clearing WDH just lets the next done head be written. */
    sts &= INT_ALL;
    if (sts != 0) {
        reg_write(hc, REG_INTR_STATUS, sts);
    }
    if (sts & INT_UE) {
        controller_died(hc, "unrecoverable error");
        return;
    }
    (void)current_frame(hc);
    scan_all(hc);
    if (!list_empty(&o->waiting)) {
        process_waiting(hc);
    }
    if (sts & INT_RHSC) {
        tendril_roothub_changed(hc, 0);
    }
    if (hc->config.irq) {
        reg_write(hc, REG_INTR_ENABLE, INT_MIE);
    }
}

static bool ohci_irq(struct tendril_hc *hc) {
    struct ohci *o = O(hc);
    uint32_t sts = reg_read(hc, REG_INTR_STATUS);
    if (sts == 0xffffffff || !(sts & o->intr)) {
        return false;
    }
    /* Masked until tendril_hc_poll() has handled the cause. */
    reg_write(hc, REG_INTR_DISABLE, INT_MIE);
    return true;
}

static uint32_t ohci_frame_number(struct tendril_hc *hc) {
    return current_frame(hc);
}

/* OHCI 1.0a, 5.1.1.3: takes the controller from SMM or BIOS firmware, then resets the bus. */
static void take_ownership(struct tendril_hc *hc) {
    uint32_t ctl = reg_read(hc, REG_CONTROL);
    if ((ctl & CTL_IR) && !(hc->quirks & TENDRIL_HC_QUIRK_NO_HANDOFF)) {
        LOG_DEBUG("OHCI: taking over from SMM");
        reg_write(hc, REG_COMMAND_STATUS, CMD_OCR);
        /* BEGIN FreeBSD sys/dev/usb/controller/ohci.c, OpenBSD sys/dev/usb/ohci.c */
        if (!tendril_hc_wait32(hc, REG_CONTROL, CTL_IR, 0, SMM_WAIT_MS * 1000)) {
            LOG_WARN("OHCI: SMM did not release the controller, resetting it");
        }
        /* END FreeBSD sys/dev/usb/controller/ohci.c, OpenBSD sys/dev/usb/ohci.c */
    } else if ((ctl & CTL_HCFS_MASK) != CTL_HCFS_RESET && (ctl & CTL_HCFS_MASK) != CTL_HCFS_OPERATIONAL) {
        /* Firmware left it suspended: resume the bus before taking it down. */
        reg_write(hc, REG_CONTROL, (ctl & ~CTL_HCFS_MASK) | CTL_HCFS_RESUME);
        tendril_delay_us(20000);
    }
    /* BEGIN FreeBSD sys/dev/usb/controller/ohci.c, OpenBSD sys/dev/usb/ohci.c */
    /*
     * Some controllers only start after this reset, which the specification does
     * not ask for, as FreeBSD's sys/dev/usb/controller/ohci.c (BSD-2-Clause,
     * ohci_controller_init) notes.
     */
    reg_write(hc, REG_CONTROL, CTL_HCFS_RESET);
    tendril_delay_us(BUS_RESET_MS * 1000);
    /* END FreeBSD sys/dev/usb/controller/ohci.c, OpenBSD sys/dev/usb/ohci.c */
}

static void free_structures(struct tendril_hc *hc) {
    struct ohci *o = O(hc);
    struct ohci_ed *skels[INTR_LEVELS + 3];
    size_t n = 0;
    skels[n++] = o->control_head;
    skels[n++] = o->bulk_head;
    skels[n++] = o->iso_head;
    for (size_t l = 0; l < INTR_LEVELS; l++) {
        skels[n++] = o->skel[l];
    }
    for (size_t i = 0; i < n; i++) {
        if (skels[i] != NULL) {
            ed_free(hc, skels[i]);
        }
    }
    tendril_dma_free(hc, o->hcca, sizeof(struct ohci_hcca));
    tendril_dma_pool_destroy(&o->td_pool);
    tendril_dma_pool_destroy(&o->itd_pool);
    tendril_dma_pool_destroy(&o->ed_pool);
    tendril_free(o, sizeof(struct ohci));
    hc->hcd = NULL;
}

static struct ohci_ed *skeleton(struct tendril_hc *hc) {
    struct ohci_ed *ed = ed_alloc(hc);
    if (ed != NULL) {
        ed->flags = le(ED_SKIP);
        ed->kind = KIND_SKELETON;
        ed->sw_next = ed;
        ed->sw_prev = ed;
    }
    return ed;
}

static int ohci_start(struct tendril_hc *hc) {
    struct ohci *o = tendril_zalloc(sizeof(struct ohci));
    if (o == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    hc->hcd = o;
    o->hc = hc;
    list_init(&o->eds);
    list_init(&o->waiting);
    tendril_timer_init(&o->timer, ohci_timer);

    uint32_t rev = reg_read(hc, REG_REVISION);
    if (rev == 0xffffffff) {
        free_structures(hc);
        return TENDRIL_ERR_NO_DEVICE;
    }
    /* Every address the controller handles is 32 bits wide. */
    hc->dma32 = true;
    hc->data32 = true;

    tendril_dma_pool_init(&o->ed_pool, hc, sizeof(struct ohci_ed), 16);
    tendril_dma_pool_init(&o->td_pool, hc, sizeof(struct ohci_td), 16);
    tendril_dma_pool_init(&o->itd_pool, hc, sizeof(struct ohci_itd), 32);
    o->hcca = tendril_dma_alloc(hc, sizeof(struct ohci_hcca), 256, &o->hcca_dma);
    o->control_head = skeleton(hc);
    o->bulk_head = skeleton(hc);
    o->iso_head = skeleton(hc);
    for (size_t l = 0; l < INTR_LEVELS; l++) {
        o->skel[l] = skeleton(hc);
    }
    bool ok = o->hcca != NULL && o->control_head != NULL && o->bulk_head != NULL && o->iso_head != NULL;
    for (size_t l = 0; l < INTR_LEVELS; l++) {
        ok = ok && o->skel[l] != NULL;
    }
    if (!ok) {
        free_structures(hc);
        return TENDRIL_ERR_NO_MEMORY;
    }
    /* Level k is visited every 2^k frames, ahead of level k - 1; isochronous descriptors come last. */
    for (size_t l = 1; l < INTR_LEVELS; l++) {
        o->skel[l]->next = le((uint32_t)o->skel[l - 1]->dma);
    }
    o->skel[0]->next = le((uint32_t)o->iso_head->dma);
    for (size_t i = 0; i < 32; i++) {
        size_t level = i == 0 ? INTR_LEVELS - 1 : MIN(lowest_bit(i), (size_t)INTR_LEVELS - 1);
        o->hcca->intr_table[i] = le((uint32_t)o->skel[level]->dma);
    }

    take_ownership(hc);
    uint32_t fi = reg_read(hc, REG_FM_INTERVAL) & FI_MASK;
    if (fi == 0) {
        fi = FI_DEFAULT;
    }
    reg_write(hc, REG_COMMAND_STATUS, CMD_HCR);
    if (!tendril_hc_wait32(hc, REG_COMMAND_STATUS, CMD_HCR, 0, 1000)) {
        LOG_ERROR("OHCI: reset failed");
        free_structures(hc);
        return TENDRIL_ERR_TIMEOUT;
    }

    /* OHCI 1.0a, 5.1.1.4: now suspended, with at most 2 ms to become operational. */
    dma_barrier();
    reg_write(hc, REG_HCCA, (uint32_t)o->hcca_dma);
    reg_write(hc, REG_CONTROL_HEAD_ED, (uint32_t)o->control_head->dma);
    reg_write(hc, REG_BULK_HEAD_ED, (uint32_t)o->bulk_head->dma);
    reg_write(hc, REG_INTR_DISABLE, INT_ALL | INT_MIE);
    reg_write(hc, REG_INTR_STATUS, INT_ALL);
    o->intr = INT_WDH | INT_RHSC | INT_UE;
    if (hc->config.irq) {
        reg_write(hc, REG_INTR_ENABLE, o->intr | INT_MIE);
    }
    /* BEGIN FreeBSD sys/dev/usb/controller/ohci.c, OpenBSD sys/dev/usb/ohci.c */
    uint32_t mask = CTL_CBSR_MASK | CTL_HCFS_MASK | CTL_IR | CTL_PLE | CTL_IE | CTL_CLE | CTL_BLE;
    uint32_t keep = reg_read(hc, REG_CONTROL) & ~mask;
    o->control = keep | CTL_RATIO_4_1 | CTL_HCFS_OPERATIONAL;
    set_list_enables(hc);

    /* FreeBSD's ohci_controller_init sets these only now: the controller ignores them while suspended. */
    uint32_t fit = (reg_read(hc, REG_FM_INTERVAL) & FIT) ^ FIT;
    reg_write(hc, REG_FM_INTERVAL, fit | ((fi - MAX_OVERHEAD) * 6 / 7) << 16 | fi);
    reg_write(hc, REG_PERIODIC_START, fi * 9 / 10);

    /* Ports on: global power, with NOCP set meanwhile against a chip bug, as FreeBSD's ohci_controller_init does. */
    uint32_t desca = reg_read(hc, REG_RH_DESC_A);
    reg_write(hc, REG_RH_DESC_A, desca | RHA_NOCP);
    reg_write(hc, REG_RH_STATUS, RHS_LPSC);
    tendril_delay_us(5000);
    reg_write(hc, REG_RH_DESC_A, desca);
    o->no_power_switching = (desca & RHA_NPS) != 0;
    o->per_port_power = !o->no_power_switching && (desca & RHA_PSM) != 0;

    /* The AMD-756 may report no ports at first; FreeBSD's ohci_controller_init rereads the same way. */
    for (size_t i = 0; i < 10 && (desca & RHA_NDP_MASK) == 0; i++) {
        tendril_delay_us(5000);
        desca = reg_read(hc, REG_RH_DESC_A);
    }
    /* END FreeBSD sys/dev/usb/controller/ohci.c, OpenBSD sys/dev/usb/ohci.c */
    o->num_ports = MIN(desca & RHA_NDP_MASK, (uint32_t)15);
    if (o->per_port_power) {
        for (size_t p = 1; p <= o->num_ports; p++) {
            reg_write(hc, REG_RH_PORT(p), PS_SET_POWER);
        }
    }
    LOG_INFO("OHCI %u.%u: %zu ports", (rev >> 4) & 0xf, rev & 0xf, o->num_ports);

    int st = tendril_roothub_init(hc, o->num_ports);
    if (st != TENDRIL_OK) {
        reg_write(hc, REG_CONTROL, CTL_HCFS_RESET);
        free_structures(hc);
        return st;
    }
    /* PowerOnToPowerGoodTime, in 2 ms units, and at least the USB 2.0 7.1.7.3 settling time. */
    tendril_delay_us(MAX((desca >> 24) * (uint32_t)2000, (uint32_t)20000));
    return TENDRIL_OK;
}

static void ohci_stop(struct tendril_hc *hc) {
    if (hc->hcd == NULL) {
        return;
    }
    hc->dead = true;
    reg_write(hc, REG_INTR_DISABLE, INT_ALL | INT_MIE);
    reg_write(hc, REG_CONTROL, CTL_HCFS_RESET);
    fail_everything(hc);
}

static void ohci_destroy(struct tendril_hc *hc) {
    if (hc->hcd != NULL) {
        free_structures(hc);
    }
}

/*
 * No suspend step: entering USBRESUME would clear every port's suspend (OHCI
 * 1.0a, HcRhPortStatus PSS), so the controller stays operational while its
 * ports are suspended.
 */
static int ohci_resume(struct tendril_hc *hc) {
    uint32_t ctl = reg_read(hc, REG_CONTROL);
    if (ctl == 0xffffffff) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    /* An HCCA that no longer reads back means the controller lost power. */
    if (reg_read(hc, REG_HCCA) != (uint32_t)O(hc)->hcca_dma || (ctl & CTL_HCFS_MASK) != CTL_HCFS_OPERATIONAL) {
        return TENDRIL_ERR_IO;
    }
    return TENDRIL_OK;
}

const struct tendril_hcd_ops tendril_ohci_ops = {
    .name = "OHCI",
    .start = ohci_start,
    .stop = ohci_stop,
    .destroy = ohci_destroy,
    .irq = ohci_irq,
    .poll = ohci_poll,
    .frame_number = ohci_frame_number,
    .port_status = ohci_port_status,
    .port_set_feature = ohci_port_set_feature,
    .port_clear_feature = ohci_port_clear_feature,
    .device_init = ohci_device_init,
    .ep0_update = ohci_ep0_update,
    .configure = ohci_configure,
    .endpoint_reset = ohci_endpoint_reset,
    .device_free = ohci_device_free,
    .resume = ohci_resume,
    .submit = ohci_submit,
    .cancel = ohci_cancel
};
