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

/* eXtensible Host Controller Interface, revision 1.2. */

/*
 * Parts of this file are derived from FreeBSD's
 * sys/dev/usb/controller/xhci_pci.c and sys/dev/usb/controller/xhci.c (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2010-2022 Hans Petter Selasky
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
 * sys/dev/usb/controller/xhcireg.h (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2010 Hans Petter Selasky. All rights reserved.
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../internal.h"
#include <tendril/glue.h>
#include <tendril/spec.h>
#include <tendril/tendril.h>

#define CAP_CAPLENGTH 0x00
#define CAP_HCSPARAMS1 0x04
#define CAP_HCSPARAMS2 0x08
#define CAP_HCCPARAMS1 0x10
#define CAP_DBOFF 0x14
#define CAP_RTSOFF 0x18
#define CAP_HCCPARAMS2 0x1c

#define OP_USBCMD 0x00
#define OP_USBSTS 0x04
#define OP_PAGESIZE 0x08
#define OP_DNCTRL 0x14
#define OP_CRCR 0x18
#define OP_DCBAAP 0x30
#define OP_CONFIG 0x38
#define OP_PORTSC(n) \
    ((uint32_t)0x400 + (uint32_t)0x10 * ((uint32_t)(n) - (uint32_t)1))

#define USBCMD_RS ((uint32_t)1 << 0)
#define USBCMD_HCRST ((uint32_t)1 << 1)
#define USBCMD_INTE ((uint32_t)1 << 2)
#define USBCMD_HSEE ((uint32_t)1 << 3)
#define USBCMD_CSS ((uint32_t)1 << 8)
#define USBCMD_CRS ((uint32_t)1 << 9)

#define USBSTS_HCH ((uint32_t)1 << 0)
#define USBSTS_HSE ((uint32_t)1 << 2)
#define USBSTS_EINT ((uint32_t)1 << 3)
#define USBSTS_PCD ((uint32_t)1 << 4)
#define USBSTS_SSS ((uint32_t)1 << 8)
#define USBSTS_RSS ((uint32_t)1 << 9)
#define USBSTS_SRE ((uint32_t)1 << 10)
#define USBSTS_CNR ((uint32_t)1 << 11)
#define USBSTS_HCE ((uint32_t)1 << 12)

#define HCC1_AC64 ((uint32_t)1 << 0)
#define HCC1_CSZ ((uint32_t)1 << 2)
#define HCC1_PPC ((uint32_t)1 << 3)
#define HCC1_MAXPSA(x) (((x) >> 12) & 0xf)
#define HCC1_XECP(x) ((x) >> 16)

#define PORTSC_CCS ((uint32_t)1 << 0)
#define PORTSC_PED ((uint32_t)1 << 1)
#define PORTSC_OCA ((uint32_t)1 << 3)
#define PORTSC_PR ((uint32_t)1 << 4)
#define PORTSC_PLS_SHIFT 5
#define PORTSC_PLS_MASK ((uint32_t)0xf << 5)
#define PORTSC_PP ((uint32_t)1 << 9)
#define PORTSC_SPEED(x) (((x) >> 10) & 0xf)
#define PORTSC_PIC_MASK ((uint32_t)3 << 14)
#define PORTSC_LWS ((uint32_t)1 << 16)
#define PORTSC_CSC ((uint32_t)1 << 17)
#define PORTSC_PEC ((uint32_t)1 << 18)
#define PORTSC_WRC ((uint32_t)1 << 19)
#define PORTSC_OCC ((uint32_t)1 << 20)
#define PORTSC_PRC ((uint32_t)1 << 21)
#define PORTSC_PLC ((uint32_t)1 << 22)
#define PORTSC_CEC ((uint32_t)1 << 23)
#define PORTSC_CAS ((uint32_t)1 << 24)
#define PORTSC_WCE ((uint32_t)1 << 25)
#define PORTSC_WDE ((uint32_t)1 << 26)
#define PORTSC_WOE ((uint32_t)1 << 27)
#define PORTSC_DR ((uint32_t)1 << 30)
#define PORTSC_WPR ((uint32_t)1 << 31)
/* Bits that keep their value when written back, leaving the rest untouched. */
#define PORTSC_KEEP \
    (PORTSC_PP | PORTSC_PIC_MASK | PORTSC_WCE | PORTSC_WDE | PORTSC_WOE)

/* Port link states as PLS reads and writes them (xHCI 1.2, Table 5-27). */
#define PLS_U0 ((uint32_t)0)
#define PLS_U3 ((uint32_t)3)
#define PLS_RESUME ((uint32_t)15)
/*
 * USB 2.0 7.1.7.7: TDRSMDN, the resume signalling software times on USB2
 * ports.
 */
#define RESUME_SIGNAL_MS 20

#define RT_MFINDEX 0x00
#define RT_IR0 0x20
#define IR_IMAN 0x00
#define IR_IMOD 0x04
#define IR_ERSTSZ 0x08
#define IR_ERSTBA 0x10
#define IR_ERDP 0x18
#define IMAN_IP ((uint32_t)1 << 0)
#define IMAN_IE ((uint32_t)1 << 1)
#define ERDP_EHB ((uint32_t)1 << 3)

#define CRCR_RCS ((uint32_t)1 << 0)
#define CRCR_CA ((uint32_t)1 << 2)
#define CRCR_CRR ((uint32_t)1 << 3)

/* TRB types */
#define TRB_NORMAL 1
#define TRB_SETUP 2
#define TRB_DATA 3
#define TRB_STATUS 4
#define TRB_ISOCH 5
#define TRB_LINK 6
#define TRB_NOOP 8
#define TRB_ENABLE_SLOT 9
#define TRB_DISABLE_SLOT 10
#define TRB_ADDRESS_DEVICE 11
#define TRB_CONFIGURE_ENDPOINT 12
#define TRB_EVALUATE_CONTEXT 13
#define TRB_RESET_ENDPOINT 14
#define TRB_STOP_ENDPOINT 15
#define TRB_SET_TR_DEQUEUE 16
#define TRB_TRANSFER_EVENT 32
#define TRB_COMMAND_COMPLETION 33
#define TRB_PORT_STATUS_CHANGE 34
#define TRB_HOST_CONTROLLER_EVENT 37

/* TRB control (dword 3) bits */
#define TRB_CYCLE ((uint32_t)1 << 0)
#define TRB_TC ((uint32_t)1 << 1)
#define TRB_ENT ((uint32_t)1 << 1)
#define TRB_ISP ((uint32_t)1 << 2)
#define TRB_CH ((uint32_t)1 << 4)
#define TRB_IOC ((uint32_t)1 << 5)
#define TRB_IDT ((uint32_t)1 << 6)
#define TRB_BEI ((uint32_t)1 << 9)
#define TRB_BSR ((uint32_t)1 << 9)
#define TRB_TSP ((uint32_t)1 << 9)
#define TRB_DIR_IN ((uint32_t)1 << 16)
#define TRB_SIA ((uint32_t)1 << 31)
#define TRB_TYPE(t) ((uint32_t)(t) << 10)
#define TRB_GET_TYPE(d3) (((d3) >> 10) & 0x3f)
#define TRB_SLOT(s) ((uint32_t)(s) << 24)
/* Stop Endpoint: the endpoint stops for a suspend (xHCI 1.2, Table 6-66). */
#define TRB_SP ((uint32_t)1 << 23)
#define TRB_EP(e) ((uint32_t)(e) << 16)
#define TRB_TD_SIZE(n) ((uint32_t)(n) << 17)

/* Completion codes */
#define CC_SUCCESS 1
#define CC_DATA_BUFFER 2
#define CC_BABBLE 3
#define CC_USB_TRANSACTION 4
#define CC_TRB 5
#define CC_STALL 6
#define CC_RESOURCE 7
#define CC_BANDWIDTH 8
#define CC_NO_SLOTS 9
#define CC_SHORT_PACKET 13
#define CC_RING_UNDERRUN 14
#define CC_RING_OVERRUN 15
#define CC_PARAMETER 17
#define CC_BANDWIDTH_OVERRUN 18
#define CC_CONTEXT_STATE 19
#define CC_EVENT_RING_FULL 21
#define CC_MISSED_SERVICE 23
#define CC_COMMAND_RING_STOPPED 24
#define CC_COMMAND_ABORTED 25
#define CC_STOPPED 26
#define CC_STOPPED_LENGTH_INVALID 27
#define CC_STOPPED_SHORT 28
#define CC_ISOCH_BUFFER_OVERRUN 31
#define CC_SPLIT_TRANSACTION 36

/* Endpoint context states and types */
#define EP_STATE_DISABLED 0
#define EP_STATE_RUNNING 1
#define EP_STATE_HALTED 2
#define EP_STATE_STOPPED 3
#define EP_STATE_ERROR 4

#define EP_TYPE_ISOCH_OUT 1
#define EP_TYPE_BULK_OUT 2
#define EP_TYPE_INT_OUT 3
#define EP_TYPE_CONTROL 4
#define EP_TYPE_ISOCH_IN 5
#define EP_TYPE_BULK_IN 6
#define EP_TYPE_INT_IN 7

#define EVENT_RING_TRBS 256
#define CMD_RING_TRBS 256
#define SMALL_RING_TRBS 256
#define LARGE_RING_TRBS 1024
#define STREAM_RING_TRBS 256
#define CMD_TIMEOUT_MS 5000
#define MAX_TRB_LEN 65536
/* BEGIN FreeBSD sys/dev/usb/controller/xhci_pci.c */
/*
 * As long as FreeBSD's sys/dev/usb/controller/xhci_pci.c (BSD-2-Clause,
 * xhci_pci_take_controller) waits.
 */
#define HANDOFF_MS 5000
/* END FreeBSD sys/dev/usb/controller/xhci_pci.c */
/* BEGIN FreeBSD sys/dev/usb/controller/xhcireg.h */
/*
 * FreeBSD's XHCI_IMOD_DEFAULT and XHCI_IMOD_DEFAULT_LP, from its
 * sys/dev/usb/controller/xhcireg.h (BSD-2-Clause): at most 8000 or 4000
 * interrupts a second, in 250 ns units.
 */
#define IMOD_DEFAULT 500
#define IMOD_LP 1016
/* END FreeBSD sys/dev/usb/controller/xhcireg.h */

struct xhci_trb {
    uint32_t d[4];
};

struct xhci_ring {
    struct xhci_trb *trbs;
    uint64_t dma;
    size_t size;
    size_t enq;
    size_t deq;
    uint32_t cycle;
};

struct xhci_cmd {
    struct tendril_list node;
    size_t index;
    void (*done)(
        struct tendril_hc *hc, struct xhci_cmd *cmd, int code,
        struct xhci_trb *event
    );
    struct tendril_async *async;
    struct tendril_device *dev;
    struct xhci_ep *xep;
    uint32_t arg;
};

struct xhci_ep {
    struct tendril_endpoint *ep;
    struct tendril_device *dev;
    uint8_t dci;
    struct xhci_ring ring;
    size_t num_streams;
    struct xhci_ring *streams;
    void *stream_ctx;
    uint64_t stream_ctx_dma;
    size_t stream_ctx_size;
    size_t pending_set_deq;
    bool stopping;
    bool recovering;
};

struct xhci_dev {
    uint8_t slot;
    uint8_t *out_ctx;
    uint64_t out_ctx_dma;
    uint8_t *in_ctx;
    uint64_t in_ctx_dma;
    struct xhci_ep *eps[32];
    uint8_t speed_id;
};

#define RESUME_IDLE 0
/* USB2 resume signalling, timed by software from resume_start. */
#define RESUME_SIGNALLING 1
/* U0 was written; the port has yet to leave the Resume state. */
#define RESUME_ENDING 2

struct xhci_port_proto {
    uint8_t major;
    uint8_t psic;
    const uint32_t *psi;
    int resume;
    uint64_t resume_start;
};

struct xhci {
    struct tendril_hc *hc;
    uint32_t op;
    uint32_t rt;
    uint32_t db;
    uint32_t hcc1;
    size_t max_slots;
    size_t max_ports;
    size_t ctx_size;
    bool ac64;
    uint64_t *dcbaa;
    uint64_t dcbaa_dma;
    uint64_t *scratch_array;
    uint64_t scratch_array_dma;
    void **scratch_bufs;
    size_t num_scratch;
    size_t page_size;
    struct xhci_ring cmd;
    size_t cmd_free;
    struct tendril_list cmds;
    struct tendril_timer cmd_timer;
    struct xhci_trb *evt;
    uint64_t evt_dma;
    size_t evt_deq;
    uint32_t evt_cycle;
    uint64_t *erst;
    uint64_t erst_dma;
    struct xhci_dev **devs;
    /* Per root port: protocol major revision and speed ID table. */
    struct xhci_port_proto *ports;
    uint32_t proto_psi[64];
    size_t proto_psi_used;
    /* Ends USB2 resume signalling a device started. */
    struct tendril_timer port_timer;
    /* Across system sleep. */
    uint32_t saved_cmd;
    uint32_t saved_config;
    bool save_failed;
};

/* hcd_data slots in a transfer */
#define XD_RING 0
#define XD_FIRST 1
#define XD_COUNT 2
#define XD_FLAGS 3
#define XD_ACTUAL 4
#define XD_STATUS_TRB 5

#define XF_CANCEL ((uint32_t)1 << 0)
#define XF_SHORT ((uint32_t)1 << 1)
#define XF_DATA_DONE ((uint32_t)1 << 2)
#define XF_STEP ((uint32_t)1 << 3)

static struct xhci *X(struct tendril_hc *hc) {
    return hc->hcd;
}

static uint32_t op_read(struct tendril_hc *hc, uint32_t reg) {
    return hc_read32(hc, X(hc)->op + reg);
}

static void op_write(struct tendril_hc *hc, uint32_t reg, uint32_t v) {
    hc_write32(hc, X(hc)->op + reg, v);
}

static void op_write64(struct tendril_hc *hc, uint32_t reg, uint64_t v) {
    hc_write32(hc, X(hc)->op + reg, (uint32_t)v);
    hc_write32(hc, X(hc)->op + reg + 4, (uint32_t)(v >> 32));
}

static uint32_t rt_read(struct tendril_hc *hc, uint32_t reg) {
    return hc_read32(hc, X(hc)->rt + reg);
}

static void rt_write(struct tendril_hc *hc, uint32_t reg, uint32_t v) {
    hc_write32(hc, X(hc)->rt + reg, v);
}

static void rt_write64(struct tendril_hc *hc, uint32_t reg, uint64_t v) {
    hc_write32(hc, X(hc)->rt + reg, (uint32_t)v);
    hc_write32(hc, X(hc)->rt + reg + 4, (uint32_t)(v >> 32));
}

static void ring_doorbell(
    struct tendril_hc *hc, uint8_t slot, uint8_t target, uint16_t stream
) {
    hc_write32(
        hc, X(hc)->db + (uint32_t)(4 * slot),
        (uint32_t)target | (uint32_t)stream << 16
    );
}

static void trb_write(
    struct xhci_trb *trb, uint64_t param, uint32_t status, uint32_t control
) {
    trb->d[0] = tendril_le32((uint32_t)param);
    trb->d[1] = tendril_le32((uint32_t)(param >> 32));
    trb->d[2] = tendril_le32(status);
    /* The cycle bit hands the TRB to the controller, so it goes last. */
    dma_barrier();
    trb->d[3] = tendril_le32(control);
}

static uint32_t trb_d(const struct xhci_trb *trb, size_t i) {
    return tendril_le32(trb->d[i]);
}

static uint64_t trb_param(const struct xhci_trb *trb) {
    return (uint64_t)trb_d(trb, 0) | (uint64_t)trb_d(trb, 1) << 32;
}

/* Contexts */

static uint32_t *ctx_at(struct tendril_hc *hc, uint8_t *base, size_t index) {
    return (uint32_t *)(void *)(base + index * X(hc)->ctx_size);
}

/*
 * Input context: index 0 is the control context, 1 the slot, 1 + DCI
 * endpoints.
 */
static uint32_t *in_ctrl(struct tendril_hc *hc, struct xhci_dev *xd) {
    return ctx_at(hc, xd->in_ctx, 0);
}

static uint32_t *in_slot(struct tendril_hc *hc, struct xhci_dev *xd) {
    return ctx_at(hc, xd->in_ctx, 1);
}

static uint32_t *in_ep(
    struct tendril_hc *hc, struct xhci_dev *xd, uint8_t dci
) {
    return ctx_at(hc, xd->in_ctx, 1 + dci);
}

static uint32_t *out_slot(struct tendril_hc *hc, struct xhci_dev *xd) {
    return ctx_at(hc, xd->out_ctx, 0);
}

static uint32_t *out_ep(
    struct tendril_hc *hc, struct xhci_dev *xd, uint8_t dci
) {
    return ctx_at(hc, xd->out_ctx, dci);
}

static void ctx_set(uint32_t *ctx, size_t dw, uint32_t v) {
    ctx[dw] = tendril_le32(v);
}

static uint32_t ctx_get(const uint32_t *ctx, size_t dw) {
    return tendril_le32(ctx[dw]);
}

static void clear_input(struct tendril_hc *hc, struct xhci_dev *xd) {
    memset(xd->in_ctx, 0, X(hc)->ctx_size * 33);
}

static uint8_t ep_dci(struct tendril_endpoint *ep) {
    uint8_t num = ep->address & TENDRIL_ENDPOINT_NUMBER_MASK;
    if (num == 0) {
        return 1;
    }
    return (uint8_t)(num * 2
        + ((ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0 ? 1 : 0));
}

/* Rings */

static bool ring_init(
    struct tendril_hc *hc, struct xhci_ring *ring, size_t size
) {
    size_t bytes = size * sizeof(struct xhci_trb);
    /* Aligning to the size keeps the segment inside a 64 KiB boundary. */
    ring->trbs = tendril_dma_alloc(hc, bytes, bytes, &ring->dma);
    if (ring->trbs == NULL) {
        return false;
    }
    ring->size = size;
    ring->enq = 0;
    ring->deq = 0;
    ring->cycle = 1;
    trb_write(
        &ring->trbs[size - 1], ring->dma, 0, TRB_TYPE(TRB_LINK) | TRB_TC
    );
    return true;
}

static void ring_free(struct tendril_hc *hc, struct xhci_ring *ring) {
    if (ring->trbs != NULL) {
        tendril_dma_free(hc, ring->trbs, ring->size * sizeof(struct xhci_trb));
        ring->trbs = NULL;
    }
}

static uint64_t ring_addr(struct xhci_ring *ring, size_t index) {
    return ring->dma + (uint64_t)index * sizeof(struct xhci_trb);
}

static size_t ring_next(struct xhci_ring *ring, size_t index) {
    index++;
    if (index == ring->size - 1) {
        index = 0;
    }
    return index;
}

static size_t ring_used(struct xhci_ring *ring) {
    size_t usable = ring->size - 1;
    return (ring->enq + usable - ring->deq) % usable;
}

static size_t ring_space(struct xhci_ring *ring) {
    /* One slot stays empty so a full ring never looks empty. */
    return ring->size - 2 - ring_used(ring);
}

/* Returns the index of a TRB address within the ring, or -1. */
static int ring_index(struct xhci_ring *ring, uint64_t addr) {
    if (addr < ring->dma
     || addr >= ring->dma + ring->size * sizeof(struct xhci_trb)
     || (addr & 0xf) != 0) {
        return -1;
    }
    return (int)((addr - ring->dma) / sizeof(struct xhci_trb));
}

/* Writes a TRB at the enqueue pointer; the cycle bit is applied here. */
static size_t ring_push(
    struct xhci_ring *ring, uint64_t param, uint32_t status, uint32_t control,
    bool valid
) {
    size_t index = ring->enq;
    uint32_t cycle = valid ? ring->cycle : ring->cycle ^ 1;
    trb_write(
        &ring->trbs[index], param, status, (control & ~TRB_CYCLE) | cycle
    );
    ring->enq++;
    if (ring->enq == ring->size - 1) {
        /* Hand over the link TRB, chaining it if the TD continues past it. */
        struct xhci_trb *link = &ring->trbs[ring->size - 1];
        uint32_t ctl = TRB_TYPE(TRB_LINK) | TRB_TC | (control & TRB_CH)
            | ring->cycle;
        trb_write(link, ring->dma, 0, ctl);
        ring->enq = 0;
        ring->cycle ^= 1;
    }
    return index;
}

/* Whether `index` lies in the TRB span [first, first + count) of a ring. */
static bool span_contains(
    struct xhci_ring *ring, size_t first, size_t count, size_t index
) {
    size_t usable = ring->size - 1;
    if (index >= usable) {
        return false;
    }
    return (index + usable - first) % usable < count;
}

static size_t span_last(struct xhci_ring *ring, size_t first, size_t count) {
    size_t usable = ring->size - 1;
    return (first + count - 1) % usable;
}

static size_t trb_len(const struct xhci_trb *trb) {
    return trb_d(trb, 2) & 0x1ffff;
}

static void give_first(struct xhci_ring *ring, size_t index, uint32_t cycle) {
    struct xhci_trb *trb = &ring->trbs[index];
    dma_barrier();
    trb->d[3] = tendril_le32((tendril_le32(trb->d[3]) & ~TRB_CYCLE) | cycle);
}

static int cc_status(int code) {
    switch (code) {
        case CC_SUCCESS:
        case CC_SHORT_PACKET: {
            return TENDRIL_OK;
        }
        case CC_STALL: {
            return TENDRIL_ERR_STALL;
        }
        case CC_BABBLE: {
            return TENDRIL_ERR_BABBLE;
        }
        case CC_DATA_BUFFER:
        case CC_ISOCH_BUFFER_OVERRUN:
        case CC_BANDWIDTH_OVERRUN: {
            return TENDRIL_ERR_OVERRUN;
        }
        case CC_MISSED_SERVICE: {
            return TENDRIL_ERR_MISSED;
        }
        case CC_BANDWIDTH:
        case CC_RESOURCE: {
            return TENDRIL_ERR_BANDWIDTH;
        }
        case CC_NO_SLOTS: {
            return TENDRIL_ERR_NO_MEMORY;
        }
        case CC_TRB:
        case CC_PARAMETER:
        case CC_CONTEXT_STATE: {
            return TENDRIL_ERR_CONTROLLER;
        }
        default: {
            return TENDRIL_ERR_IO;
        }
    }
}

static void controller_died(struct tendril_hc *hc, const char *why);
static void port_timer_work(struct tendril_work *work);

/* Commands */


static int cmd_submit(
    struct tendril_hc *hc, uint64_t param, uint32_t status, uint32_t control,
    void (*done)(
        struct tendril_hc *, struct xhci_cmd *, int, struct xhci_trb *
    ),
    struct tendril_async *async, struct tendril_device *dev,
    struct xhci_ep *xep, uint32_t arg
) {
    struct xhci *x = X(hc);
    if (hc->dead) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (x->cmd_free == 0) {
        return TENDRIL_ERR_BUSY;
    }
    struct xhci_cmd *cmd = tendril_zalloc(sizeof(struct xhci_cmd));
    if (cmd == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    cmd->done = done;
    cmd->async = async;
    cmd->dev = dev;
    cmd->xep = xep;
    cmd->arg = arg;
    cmd->index = ring_push(&x->cmd, param, status, control, true);
    x->cmd_free--;
    if (list_empty(&x->cmds)) {
        tendril_timer_arm(hc, &x->cmd_timer, CMD_TIMEOUT_MS * NS_PER_MS);
    }
    list_add_tail(&x->cmds, &cmd->node);
    ring_doorbell(hc, 0, 0, 0);
    return TENDRIL_OK;
}

static void handle_command_completion(
    struct tendril_hc *hc, struct xhci_trb *ev
) {
    struct xhci *x = X(hc);
    int index = ring_index(&x->cmd, trb_param(ev));
    int code = (int)(trb_d(ev, 2) >> 24);
    struct tendril_list *it;

    if (index < 0) {
        LOG_WARN("xHCI: completion for an unknown command");
        return;
    }
    LIST_FOR_EACH(it, &x->cmds) {
        struct xhci_cmd *cmd = CONTAINER_OF(it, struct xhci_cmd, node);
        if (cmd->index != (size_t)index) {
            continue;
        }
        list_del(&cmd->node);
        x->cmd_free++;
        x->cmd.deq = ring_next(&x->cmd, cmd->index);
        if (list_empty(&x->cmds)) {
            tendril_timer_cancel(hc, &x->cmd_timer);
        } else {
            tendril_timer_arm(hc, &x->cmd_timer, CMD_TIMEOUT_MS * NS_PER_MS);
        }
        cmd->done(hc, cmd, code, ev);
        tendril_free(cmd, sizeof(struct xhci_cmd));
        return;
    }
}

/* Completes an async-backed command with its translated completion code. */
static void cmd_async_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    (void)ev;
    tendril_async_complete(
        hc, cmd->async, code == CC_SUCCESS ? TENDRIL_OK : cc_status(code)
    );
}

/* Transfer bookkeeping */

static struct xhci_dev *xdev(struct tendril_device *dev) {
    return dev->hcd;
}

static struct xhci_ep *xep_of(struct tendril_endpoint *ep) {
    return ep->hcd;
}

static struct xhci_ring *xfer_ring(struct tendril_transfer *xfer) {
    return (struct xhci_ring *)xfer->internal.hcd_data[XD_RING];
}

static size_t xfer_first(struct tendril_transfer *xfer) {
    return (size_t)xfer->internal.hcd_data[XD_FIRST];
}

static size_t xfer_count(struct tendril_transfer *xfer) {
    return (size_t)xfer->internal.hcd_data[XD_COUNT];
}

static bool trb_is_data(const struct xhci_trb *trb) {
    int type = (int)TRB_GET_TYPE(trb_d(trb, 3));
    return type == TRB_NORMAL || type == TRB_DATA || type == TRB_ISOCH;
}

/*
 * Bytes moved by the TRBs of a transfer before `index`, plus the TRB at
 * `index`.
 */
static size_t bytes_through(
    struct xhci_ring *ring, size_t first, size_t index, size_t residual
) {
    size_t total = 0;
    for (size_t i = first; i != index; i = ring_next(ring, i)) {
        if (trb_is_data(&ring->trbs[i])) {
            total += trb_len(&ring->trbs[i]);
        }
    }
    struct xhci_trb *trb = &ring->trbs[index];
    if (trb_is_data(trb)) {
        size_t len = trb_len(trb);
        total += residual < len ? len - residual : 0;
    }
    return total;
}

/* Marks the ring space of a finished transfer free, if it was the oldest. */
static void retire_span(
    struct xhci_ring *ring, struct tendril_transfer *xfer
) {
    size_t first = xfer_first(xfer);
    if (ring->deq == first
     || span_contains(ring, ring->deq, ring_used(ring), first)) {
        ring->deq = ring_next(ring, span_last(ring, first, xfer_count(xfer)));
    }
}

static void complete_xfer(
    struct tendril_hc *hc, struct tendril_transfer *xfer, int st, size_t actual
) {
    retire_span(xfer_ring(xfer), xfer);
    tendril_transfer_complete(hc, xfer, st, actual);
}

static struct tendril_transfer *find_xfer(
    struct xhci_ep *xep, struct xhci_ring *ring, size_t index
) {
    struct tendril_list *it;
    LIST_FOR_EACH(it, &xep->ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(
            it, struct tendril_transfer, internal.node
        );
        if (xfer_ring(xfer) == ring
         && span_contains(ring, xfer_first(xfer), xfer_count(xfer), index)) {
            return xfer;
        }
    }
    return NULL;
}

static bool has_pending(struct xhci_ep *xep) {
    return !list_empty(&xep->ep->transfers);
}

static void doorbell_ep(struct tendril_hc *hc, struct xhci_ep *xep) {
    struct xhci_dev *xd = xdev(xep->dev);
    if (xep->num_streams == 0) {
        ring_doorbell(hc, xd->slot, xep->dci, 0);
        return;
    }
    for (size_t s = 1; s <= xep->num_streams; s++) {
        if (xep->streams[s].enq != xep->streams[s].deq) {
            ring_doorbell(hc, xd->slot, xep->dci, (uint16_t)s);
        }
    }
}

/* Halt recovery and cancellation */

static void start_stop(struct tendril_hc *hc, struct xhci_ep *xep);
static void start_recovery(struct tendril_hc *hc, struct xhci_ep *xep);

static int ep_state(struct tendril_hc *hc, struct xhci_ep *xep) {
    return ctx_get(out_ep(hc, xdev(xep->dev), xep->dci), 0) & 0x7;
}

static bool any_cancelled(struct xhci_ep *xep) {
    struct tendril_list *it;
    LIST_FOR_EACH(it, &xep->ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(
            it, struct tendril_transfer, internal.node
        );
        if ((xfer->internal.hcd_data[XD_FLAGS] & XF_CANCEL) != 0) {
            return true;
        }
    }
    return false;
}

static void turn_noop(struct xhci_ring *ring, size_t first, size_t count) {
    size_t i = first;
    for (size_t n = 0; n < count; n++, i = ring_next(ring, i)) {
        struct xhci_trb *trb = &ring->trbs[i];
        uint32_t cycle = trb_d(trb, 3) & TRB_CYCLE;
        trb->d[0] = 0;
        trb->d[1] = 0;
        trb->d[2] = 0;
        trb->d[3] = tendril_le32(TRB_TYPE(TRB_NOOP) | cycle);
    }
    /* A link TRB the span crossed must no longer chain into it. */
    struct xhci_trb *link = &ring->trbs[ring->size - 1];
    if (span_contains(ring, first, count, ring->size - 2) && count > 1) {
        link->d[3] = tendril_le32(trb_d(link, 3) & ~TRB_CH);
    }
}

static uint32_t dcs_at(struct xhci_ring *ring, size_t index) {
    if (index == ring->enq) {
        return ring->cycle;
    }
    return trb_d(&ring->trbs[index], 3) & TRB_CYCLE;
}

static uint64_t ring_hw_dequeue(
    struct tendril_hc *hc, struct xhci_ep *xep, uint16_t stream
) {
    if (stream == 0) {
        uint32_t *ctx = out_ep(hc, xdev(xep->dev), xep->dci);
        return ((uint64_t)ctx_get(ctx, 2) | (uint64_t)ctx_get(ctx, 3) << 32)
            & ~(uint64_t)0xf;
    }
    uint32_t *sc = (uint32_t *)xep->stream_ctx + stream * 4;
    return ((uint64_t)tendril_le32(sc[0])
        | (uint64_t)tendril_le32(sc[1]) << 32) & ~(uint64_t)0xf;
}

static void set_deq_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
);

/*
 * With the endpoint stopped, completes cancelled and failed transfers and
 * moves the dequeue pointer of each ring past them. Returns the number of Set
 * TR Dequeue Pointer commands issued.
 */
static size_t sweep_ring(
    struct tendril_hc *hc, struct xhci_ep *xep, struct xhci_ring *ring,
    uint16_t stream
) {
    int hw = ring_index(ring, ring_hw_dequeue(hc, xep, stream));
    size_t hw_index = hw < 0 ? ring->deq : (size_t)hw;
    size_t new_index = hw_index;
    bool need_set = false;
    struct tendril_list *it;
    struct tendril_list *tmp;

    LIST_FOR_EACH_SAFE(it, tmp, &xep->ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(
            it, struct tendril_transfer, internal.node
        );
        if (xfer_ring(xfer) != ring
         || (xfer->internal.hcd_data[XD_FLAGS] & XF_CANCEL) == 0) {
            continue;
        }
        size_t first = xfer_first(xfer);
        size_t count = xfer_count(xfer);
        if (span_contains(ring, first, count, new_index)) {
            new_index = ring_next(ring, span_last(ring, first, count));
            need_set = true;
        } else {
            turn_noop(ring, first, count);
        }
        tendril_transfer_complete(
            hc, xfer, xfer->internal.cancel_status,
            xfer->internal.hcd_data[XD_ACTUAL]
        );
    }

    ring->deq = hw_index;
    if (!need_set) {
        return 0;
    }
    uint64_t ptr = ring_addr(ring, new_index) | dcs_at(ring, new_index);
    if (stream != 0) {
        /* Stream Context Type 1: primary Transfer Ring. */
        ptr |= (uint32_t)1 << 1;
    }
    struct xhci_dev *xd = xdev(xep->dev);
    int st = cmd_submit(
        hc, ptr, (uint32_t)stream << 16,
        TRB_TYPE(TRB_SET_TR_DEQUEUE) | TRB_SLOT(xd->slot) | TRB_EP(xep->dci),
        set_deq_done, NULL, xep->dev, xep,
        (uint32_t)new_index | (uint32_t)stream << 16
    );
    if (st != TENDRIL_OK) {
        controller_died(hc, "command ring full");
        return 0;
    }
    return 1;
}

static void sweep(struct tendril_hc *hc, struct xhci_ep *xep) {
    size_t issued = 0;
    if (xep->num_streams == 0) {
        issued += sweep_ring(hc, xep, &xep->ring, 0);
    } else {
        for (size_t s = 1; s <= xep->num_streams; s++) {
            issued += sweep_ring(hc, xep, &xep->streams[s], (uint16_t)s);
        }
    }
    xep->pending_set_deq = issued;
    if (issued == 0) {
        xep->recovering = false;
        if (any_cancelled(xep)) {
            start_stop(hc, xep);
        } else if (has_pending(xep)) {
            doorbell_ep(hc, xep);
        }
    }
}

static void set_deq_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    struct xhci_ep *xep = cmd->xep;
    (void)ev;
    uint16_t stream = (uint16_t)(cmd->arg >> 16);
    struct xhci_ring *ring = stream == 0 ? &xep->ring : &xep->streams[stream];
    if (code == CC_SUCCESS) {
        ring->deq = cmd->arg & 0xffff;
    } else {
        LOG_WARN("xHCI: Set TR Dequeue Pointer failed with code %u", code);
    }
    if (xep->pending_set_deq != 0 && --xep->pending_set_deq != 0) {
        return;
    }
    xep->recovering = false;
    if (any_cancelled(xep)) {
        start_stop(hc, xep);
    } else if (has_pending(xep)) {
        doorbell_ep(hc, xep);
    }
}

static void stop_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    struct xhci_ep *xep = cmd->xep;
    (void)ev;
    xep->stopping = false;
    if (code != CC_SUCCESS && code != CC_CONTEXT_STATE) {
        LOG_WARN("xHCI: Stop Endpoint failed with code %u", code);
    }
    if (ep_state(hc, xep) == EP_STATE_HALTED) {
        start_recovery(hc, xep);
        return;
    }
    xep->recovering = true;
    sweep(hc, xep);
}

static void start_stop(struct tendril_hc *hc, struct xhci_ep *xep) {
    struct xhci_dev *xd = xdev(xep->dev);
    xep->stopping = true;
    int st = cmd_submit(
        hc, 0, 0,
        TRB_TYPE(TRB_STOP_ENDPOINT) | TRB_SLOT(xd->slot) | TRB_EP(xep->dci),
        stop_done, NULL, xep->dev, xep, 0
    );
    if (st != TENDRIL_OK) {
        xep->stopping = false;
        controller_died(hc, "command ring full");
    }
}

static void reset_ep_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    struct xhci_ep *xep = cmd->xep;
    (void)ev;
    if (code != CC_SUCCESS && code != CC_CONTEXT_STATE) {
        LOG_WARN("xHCI: Reset Endpoint failed with code %u", code);
    }
    sweep(hc, xep);
}

/* The endpoint halted: reset it, then skip whatever failed. */
static void start_recovery(struct tendril_hc *hc, struct xhci_ep *xep) {
    struct xhci_dev *xd = xdev(xep->dev);
    xep->recovering = true;
    int st = cmd_submit(
        hc, 0, 0,
        TRB_TYPE(TRB_RESET_ENDPOINT) | TRB_SLOT(xd->slot) | TRB_EP(xep->dci),
        reset_ep_done, NULL, xep->dev, xep, 0
    );
    if (st != TENDRIL_OK) {
        xep->recovering = false;
        controller_died(hc, "command ring full");
    }
}

/* Transfer events */

static bool halting_code(int code) {
    return code == CC_STALL || code == CC_BABBLE || code == CC_USB_TRANSACTION
        || code == CC_SPLIT_TRANSACTION;
}

static void finish_control(
    struct tendril_hc *hc, struct xhci_ep *xep, struct tendril_transfer *xfer,
    struct xhci_ring *ring, size_t index, int code, size_t residual
) {
    uintptr_t *hd = xfer->internal.hcd_data;
    size_t first = xfer_first(xfer);
    size_t status_index = span_last(ring, first, xfer_count(xfer));
    size_t data_len = tendril_le16(xfer->setup.length);

    if (code == CC_SUCCESS || code == CC_SHORT_PACKET) {
        if (index == status_index) {
            size_t actual = (hd[XD_FLAGS] & XF_DATA_DONE) != 0 ? hd[XD_ACTUAL]
                : data_len;
            complete_xfer(hc, xfer, TENDRIL_OK, actual);
            return;
        }
        if (index != first) {
            hd[XD_ACTUAL] = bytes_through(
                ring, ring_next(ring, first), index, residual
            );
            hd[XD_FLAGS] |= XF_DATA_DONE;
            /* BEGIN FreeBSD sys/dev/usb/controller/xhci.c */
            if ((hd[XD_FLAGS] & XF_STEP) != 0) {
                /*
                 * The status stage was held back until the data stage
                 * finished.
                 */
                hd[XD_FLAGS] &= ~XF_STEP;
                give_first(
                    ring, hd[XD_STATUS_TRB] & 0xffff,
                    (uint32_t)(hd[XD_STATUS_TRB] >> 16) & 1
                );
                doorbell_ep(hc, xep);
            }
            /* END FreeBSD sys/dev/usb/controller/xhci.c */
        }
        return;
    }

    size_t actual = index == first ? 0
        : bytes_through(ring, ring_next(ring, first), index, residual);
    if (index == status_index && (hd[XD_FLAGS] & XF_DATA_DONE) != 0) {
        actual = hd[XD_ACTUAL];
    }
    xfer->internal.hcd_data[XD_FLAGS] |= XF_CANCEL;
    xfer->internal.cancel_status = cc_status(code);
    hd[XD_ACTUAL] = actual;
}

static void handle_transfer_event(struct tendril_hc *hc, struct xhci_trb *ev) {
    struct xhci *x = X(hc);
    uint32_t d2 = trb_d(ev, 2);
    uint32_t d3 = trb_d(ev, 3);
    uint8_t slot = (uint8_t)(d3 >> 24);
    uint8_t dci = (uint8_t)((d3 >> 16) & 0x1f);
    int code = (int)(d2 >> 24);
    size_t residual = d2 & 0xffffff;
    uint64_t ptr = trb_param(ev);

    if (slot == 0 || slot > x->max_slots || x->devs[slot] == NULL) {
        return;
    }
    struct xhci_ep *xep = x->devs[slot]->eps[dci];
    if (xep == NULL || code == CC_RING_UNDERRUN || code == CC_RING_OVERRUN) {
        return;
    }

    struct xhci_ring *ring = &xep->ring;
    int index = -1;
    if (xep->num_streams == 0) {
        index = ring_index(ring, ptr);
    } else {
        for (size_t s = 1; s <= xep->num_streams && index < 0; s++) {
            index = ring_index(&xep->streams[s], ptr);
            if (index >= 0) {
                ring = &xep->streams[s];
            }
        }
    }
    struct tendril_transfer *xfer = index < 0 ? NULL
        : find_xfer(xep, ring, (size_t)index);

    if (code == CC_STOPPED || code == CC_STOPPED_LENGTH_INVALID
     || code == CC_STOPPED_SHORT) {
        if (xfer != NULL && code != CC_STOPPED_LENGTH_INVALID
         && xep->ep->type != TENDRIL_ENDPOINT_XFER_CONTROL) {
            xfer->internal.hcd_data[XD_ACTUAL] = bytes_through(
                ring, xfer_first(xfer), (size_t)index, residual
            );
        }
        return;
    }
    if (xfer == NULL) {
        if (halting_code(code) && !xep->recovering) {
            start_recovery(hc, xep);
        }
        return;
    }

    uintptr_t *hd = xfer->internal.hcd_data;
    size_t first = xfer_first(xfer);
    size_t last = span_last(ring, first, xfer_count(xfer));

    switch (xep->ep->type) {
        case TENDRIL_ENDPOINT_XFER_CONTROL: {
            finish_control(hc, xep, xfer, ring, (size_t)index, code, residual);
            break;
        }
        case TENDRIL_ENDPOINT_XFER_ISOC: {
            /* Each packet is one TD; count TD ends before the event TRB. */
            size_t packet = 0;
            size_t td_first = first;
            for (size_t i = first; i != (size_t)index;
                 i = ring_next(ring, i)) {
                if ((trb_d(&ring->trbs[i], 3) & TRB_CH) == 0) {
                    packet++;
                    td_first = ring_next(ring, i);
                }
            }
            if (packet < xfer->num_iso_packets) {
                struct tendril_iso_packet *pkt = &xfer->iso_packets[packet];
                pkt->actual = (uint32_t)bytes_through(
                    ring, td_first, (size_t)index, residual
                );
                pkt->status = cc_status(code);
                hd[XD_ACTUAL] += pkt->actual;
            }
            size_t td_last = (size_t)index;
            while ((trb_d(&ring->trbs[td_last], 3) & TRB_CH) != 0) {
                td_last = ring_next(ring, td_last);
            }
            if (td_last == last || packet + 1 >= xfer->num_iso_packets) {
                complete_xfer(hc, xfer, TENDRIL_OK, hd[XD_ACTUAL]);
            }
            break;
        }
        default: {
            size_t actual = bytes_through(
                ring, first, (size_t)index, residual
            );
            if (code == CC_SUCCESS || code == CC_SHORT_PACKET) {
                if (code == CC_SUCCESS && (size_t)index != last) {
                    /*
                     * An IOC is only on the last TRB; anything else is stale.
                     */
                    break;
                }
                complete_xfer(hc, xfer, TENDRIL_OK, actual);
                break;
            }
            hd[XD_FLAGS] |= XF_CANCEL;
            hd[XD_ACTUAL] = actual;
            xfer->internal.cancel_status = cc_status(code);
            break;
        }
    }

    if (code != CC_SUCCESS && code != CC_SHORT_PACKET
     && code != CC_MISSED_SERVICE
     && xep->ep->type != TENDRIL_ENDPOINT_XFER_ISOC) {
        /*
         * The failed transfer is completed once the endpoint is stopped or
         * reset.
         */
        if (halting_code(code) || ep_state(hc, xep) == EP_STATE_HALTED) {
            if (!xep->recovering) {
                start_recovery(hc, xep);
            }
        } else if (!xep->stopping && !xep->recovering) {
            start_stop(hc, xep);
        }
    }
}

/* Events */

static void handle_port_event(struct tendril_hc *hc, struct xhci_trb *ev) {
    uint8_t port = (uint8_t)(trb_d(ev, 0) >> 24);
    tendril_roothub_changed(hc, port);
}

static void process_events(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    bool any = false;

    for (;;) {
        struct xhci_trb *ev = &x->evt[x->evt_deq];
        uint32_t d3 = trb_d(ev, 3);
        if ((d3 & TRB_CYCLE) != x->evt_cycle) {
            break;
        }
        /* The rest of the TRB is only valid once the cycle bit matches. */
        dma_barrier();
        any = true;
        switch (TRB_GET_TYPE(trb_d(ev, 3))) {
            case TRB_TRANSFER_EVENT: {
                handle_transfer_event(hc, ev);
                break;
            }
            case TRB_COMMAND_COMPLETION: {
                handle_command_completion(hc, ev);
                break;
            }
            case TRB_PORT_STATUS_CHANGE: {
                handle_port_event(hc, ev);
                break;
            }
            case TRB_HOST_CONTROLLER_EVENT: {
                int code = (int)(trb_d(ev, 2) >> 24);
                if (code == CC_EVENT_RING_FULL) {
                    LOG_WARN("xHCI: event ring full");
                } else {
                    LOG_WARN("xHCI: host controller event %u", code);
                }
                break;
            }
            default: {
                break;
            }
        }
        if (hc->dead) {
            return;
        }
        x->evt_deq++;
        if (x->evt_deq == EVENT_RING_TRBS) {
            x->evt_deq = 0;
            x->evt_cycle ^= 1;
        }
    }
    if (any) {
        rt_write64(
            hc, RT_IR0 + IR_ERDP,
            (x->evt_dma + x->evt_deq * sizeof(struct xhci_trb)) | ERDP_EHB
        );
    }
}

static uint32_t xhci_frame_number(struct tendril_hc *hc);

static void xhci_poll(struct tendril_hc *hc) {
    uint32_t sts = op_read(hc, OP_USBSTS);
    if (sts == 0xffffffff) {
        controller_died(hc, "controller removed");
        return;
    }
    if ((sts & (USBSTS_HSE | USBSTS_HCE)) != 0) {
        controller_died(
            hc,
            (sts & USBSTS_HCE) != 0 ? "host controller error"
                : "host system error"
        );
        return;
    }
    if ((sts & (USBSTS_EINT | USBSTS_PCD)) != 0) {
        op_write(hc, OP_USBSTS, sts & (USBSTS_EINT | USBSTS_PCD));
    }
    /*
     * Keeps the frame count going, as nothing else may look at MFINDEX for a
     * whole wrap.
     */
    (void)xhci_frame_number(hc);
    process_events(hc);
}

static bool xhci_irq(struct tendril_hc *hc) {
    uint32_t sts = op_read(hc, OP_USBSTS);
    if (sts == 0xffffffff) {
        return false;
    }
    uint32_t iman = rt_read(hc, RT_IR0 + IR_IMAN);
    if ((sts & (USBSTS_EINT | USBSTS_HSE | USBSTS_HCE)) == 0
     && (iman & IMAN_IP) == 0) {
        return false;
    }
    /*
     * Clearing IP and EINT deasserts the interrupt; EHB holds off another
     * until poll.
     */
    op_write(hc, OP_USBSTS, sts & USBSTS_EINT);
    rt_write(hc, RT_IR0 + IR_IMAN, iman | IMAN_IP);
    return true;
}

/* xHCI 1.2 5.5.1, Table 5-36: MFINDEX[13:3] is the frame number. */
static uint32_t xhci_frame_number(struct tendril_hc *hc) {
    return tendril_frame_update(
        hc, (rt_read(hc, RT_MFINDEX) >> 3) & 0x7ff, 0x7ff
    );
}

/* Fails everything outstanding; with the lock held. */
static void fail_all(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    hc->dead = true;
    tendril_timer_cancel(hc, &x->cmd_timer);
    tendril_timer_cancel(hc, &x->port_timer);

    while (!list_empty(&x->cmds)) {
        struct xhci_cmd *cmd = CONTAINER_OF(
            x->cmds.next, struct xhci_cmd, node
        );
        list_del(&cmd->node);
        if (cmd->async != NULL) {
            tendril_async_complete(hc, cmd->async, TENDRIL_ERR_CONTROLLER);
        }
        tendril_free(cmd, sizeof(struct xhci_cmd));
    }
    struct tendril_list *it;
    LIST_FOR_EACH(it, &hc->devices) {
        struct tendril_device *dev = CONTAINER_OF(
            it, struct tendril_device, node
        );
        tendril_transfer_fail_endpoint(hc, &dev->ep0, TENDRIL_ERR_CONTROLLER);
        for (size_t i = 0; i < dev->num_interfaces; i++) {
            struct tendril_interface *intf = &dev->interfaces[i];
            for (size_t e = 0; e < intf->num_endpoints; e++) {
                tendril_transfer_fail_endpoint(
                    hc, &intf->endpoints[e], TENDRIL_ERR_CONTROLLER
                );
            }
        }
    }
    uint32_t cmd = op_read(hc, OP_USBCMD);
    if (cmd != 0xffffffff) {
        op_write(hc, OP_USBCMD, cmd & ~(USBCMD_RS | USBCMD_INTE));
    }
}

static void controller_died(struct tendril_hc *hc, const char *why) {
    if (hc->dead) {
        return;
    }
    LOG_ERROR("xHCI: controller failed: %s", why);
    fail_all(hc);
}

static void cmd_timeout(struct tendril_work *work) {
    struct xhci *x = CONTAINER_OF(work, struct xhci, cmd_timer.work);
    struct tendril_hc *hc = x->hc;
    hc_lock(hc);
    if (!list_empty(&x->cmds)) {
        controller_died(hc, "command timed out");
    }
    hc_unlock(hc);
}

/* Initialization */

static void bios_handoff(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    uint32_t off = HCC1_XECP(x->hcc1) << 2;

    while (off != 0) {
        uint32_t cap = hc_read32(hc, off);
        if ((cap & 0xff) == 1) {
            if ((cap & ((uint32_t)1 << 16)) != 0) {
                LOG_DEBUG("xHCI: taking over from firmware");
                hc_write32(hc, off, cap | ((uint32_t)1 << 24));
                bool released = tendril_hc_wait32(
                    hc, off, (uint32_t)1 << 16, 0, HANDOFF_MS * 1000
                );
                if (!released) {
                    LOG_WARN(
                        "xHCI: firmware did not release the controller, "
                        "forcing it"
                    );
                    hc_write32(
                        hc, off,
                        (hc_read32(hc, off) & ~((uint32_t)1 << 16))
                            | ((uint32_t)1 << 24)
                    );
                }
            }
            /* Disable SMIs and clear their status. */
            uint32_t ctl = hc_read32(hc, off + 4);
            uint32_t enables = (uint32_t)1 << 0 | (uint32_t)1 << 4
                | (uint32_t)1 << 13 | (uint32_t)1 << 14 | (uint32_t)1 << 15;
            ctl &= ~enables;
            ctl |= (uint32_t)1 << 29 | (uint32_t)1 << 30 | (uint32_t)1 << 31;
            hc_write32(hc, off + 4, ctl);
            return;
        }
        uint32_t next = (cap >> 8) & 0xff;
        off = next == 0 ? 0 : off + (next << 2);
    }
}

static void parse_protocols(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    uint32_t off = HCC1_XECP(x->hcc1) << 2;

    while (off != 0) {
        uint32_t cap = hc_read32(hc, off);
        if ((cap & 0xff) == 2) {
            uint8_t major = (uint8_t)(cap >> 24);
            uint32_t ports = hc_read32(hc, off + 8);
            size_t offset = ports & 0xff;
            size_t count = (ports >> 8) & 0xff;
            size_t psic = ports >> 28;
            const uint32_t *psi = NULL;
            if (psic != 0
             && x->proto_psi_used + psic <= ARRAY_SIZE(x->proto_psi)) {
                for (size_t i = 0; i < psic; i++) {
                    x->proto_psi[x->proto_psi_used + i] = hc_read32(
                        hc, off + 16 + 4 * (uint32_t)i
                    );
                }
                psi = &x->proto_psi[x->proto_psi_used];
                x->proto_psi_used += psic;
            } else {
                psic = 0;
            }
            for (size_t p = offset; p < offset + count && p <= x->max_ports;
                 p++) {
                if (p == 0) {
                    continue;
                }
                x->ports[p - 1].major = major;
                x->ports[p - 1].psic = (uint8_t)psic;
                x->ports[p - 1].psi = psi;
            }
        }
        uint32_t next = (cap >> 8) & 0xff;
        off = next == 0 ? 0 : off + (next << 2);
    }
}

/*
 * Maps a PORTSC speed ID to a speed, through the protocol's PSI table if any.
 */
static int port_speed(struct tendril_hc *hc, uint8_t port, int id) {
    struct xhci_port_proto *proto = &X(hc)->ports[port - 1];
    if (proto->psic == 0) {
        switch (id) {
            case 1: {
                return TENDRIL_SPEED_FULL;
            }
            case 2: {
                return TENDRIL_SPEED_LOW;
            }
            case 3: {
                return TENDRIL_SPEED_HIGH;
            }
            case 4: {
                return TENDRIL_SPEED_SUPER;
            }
            case 5: {
                return TENDRIL_SPEED_SUPER_PLUS;
            }
            default: {
                return id > 5 ? TENDRIL_SPEED_SUPER_PLUS_X2
                    : TENDRIL_SPEED_FULL;
            }
        }
    }
    for (size_t i = 0; i < proto->psic; i++) {
        uint32_t psi = proto->psi[i];
        if ((int)(psi & 0xf) != id) {
            continue;
        }
        uint64_t mantissa = psi >> 16;
        unsigned int exponent = (psi >> 4) & 3;
        uint64_t rate = mantissa;
        for (size_t e = 0; e < exponent; e++) {
            rate *= 1000;
        }
        if (proto->major < 3) {
            if (rate <= 1500000) {
                return TENDRIL_SPEED_LOW;
            }
            return rate <= 12000000 ? TENDRIL_SPEED_FULL : TENDRIL_SPEED_HIGH;
        }
        if (rate <= (uint64_t)5000000000) {
            return TENDRIL_SPEED_SUPER;
        }
        return rate <= (uint64_t)10000000000 ? TENDRIL_SPEED_SUPER_PLUS
            : TENDRIL_SPEED_SUPER_PLUS_X2;
    }
    return proto->major >= 3 ? TENDRIL_SPEED_SUPER : TENDRIL_SPEED_FULL;
}

static void free_scratchpads(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    if (x->scratch_bufs != NULL) {
        for (size_t i = 0; i < x->num_scratch; i++) {
            tendril_dma_free(hc, x->scratch_bufs[i], x->page_size);
        }
        tendril_free(x->scratch_bufs, x->num_scratch * sizeof(void *));
        x->scratch_bufs = NULL;
    }
    tendril_dma_free(hc, x->scratch_array, x->num_scratch * sizeof(uint64_t));
    x->scratch_array = NULL;
}

static void free_all(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    free_scratchpads(hc);
    tendril_dma_free(hc, x->dcbaa, 2048);
    tendril_dma_free(hc, x->evt, EVENT_RING_TRBS * sizeof(struct xhci_trb));
    tendril_dma_free(hc, x->erst, 64);
    ring_free(hc, &x->cmd);
    tendril_free(x->devs, (x->max_slots + 1) * sizeof(struct xhci_dev *));
    tendril_free(x->ports, x->max_ports * sizeof(struct xhci_port_proto));
    tendril_free(x, sizeof(struct xhci));
    hc->hcd = NULL;
}

static uint32_t imod(struct tendril_hc *hc) {
    return (hc->quirks & TENDRIL_HC_QUIRK_XHCI_IMOD_LP) != 0 ? IMOD_LP
        : IMOD_DEFAULT;
}

static int halt_and_reset(struct tendril_hc *hc) {
    bool ready = tendril_hc_wait32(
        hc, X(hc)->op + OP_USBSTS, USBSTS_CNR, 0, 1000000
    );
    if (!ready) {
        return TENDRIL_ERR_TIMEOUT;
    }
    uint32_t cmd = op_read(hc, OP_USBCMD);
    if ((cmd & USBCMD_RS) != 0) {
        op_write(hc, OP_USBCMD, cmd & ~(USBCMD_RS | USBCMD_INTE));
    }
    /* xHCI 1.2, 5.4.1.1: halting completes within 16 microframes. */
    bool halted = tendril_hc_wait32(
        hc, X(hc)->op + OP_USBSTS, USBSTS_HCH, USBSTS_HCH, 20000
    );
    if (!halted) {
        LOG_WARN("xHCI: controller did not halt");
    }
    op_write(hc, OP_USBCMD, USBCMD_HCRST);
    /* BEGIN FreeBSD sys/dev/usb/controller/xhci.c */
    /*
     * FreeBSD's sys/dev/usb/controller/xhci.c (BSD-2-Clause,
     * xhci_reset_controller) waits 10 ms before looking.
     */
    tendril_delay_us(10000);
    /* END FreeBSD sys/dev/usb/controller/xhci.c */
    bool reset = tendril_hc_wait32(
        hc, X(hc)->op + OP_USBCMD, USBCMD_HCRST, 0, 1000000
    );
    if (!reset) {
        return TENDRIL_ERR_TIMEOUT;
    }
    ready = tendril_hc_wait32(
        hc, X(hc)->op + OP_USBSTS, USBSTS_CNR, 0, 1000000
    );
    return ready ? TENDRIL_OK : TENDRIL_ERR_TIMEOUT;
}

/* BEGIN FreeBSD sys/dev/usb/controller/xhci_pci.c */
/*
 * Switches every port that can be switched over to the xHCI: USB3_PSSEN from
 * USB3PRM and XUSB2PR from XUSB2PRM (Intel 9 Series Chipset Family PCH
 * datasheet 330550-002, 16.1.36 to 16.1.39), as FreeBSD's
 * sys/dev/usb/controller/xhci_pci.c (BSD-2-Clause) does for the controllers it
 * lists.
 */
static void intel_port_routing(struct tendril_hc *hc) {
    void *ctx = hc->ctx;
    uint32_t usb3 = tendril_glue_pci_read32(ctx, 0xdc);
    uint32_t usb2 = tendril_glue_pci_read32(ctx, 0xd4);
    tendril_glue_pci_write32(ctx, 0xd8, usb3);
    tendril_glue_pci_write32(ctx, 0xd0, usb2);
    LOG_DEBUG("xHCI: routed ports to xHCI (USB3 0x%x, USB2 0x%x)", usb3, usb2);
}
/* END FreeBSD sys/dev/usb/controller/xhci_pci.c */

static int xhci_start(struct tendril_hc *hc) {
    struct xhci *x = tendril_zalloc(sizeof(struct xhci));
    if (x == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    hc->hcd = x;
    x->hc = hc;
    list_init(&x->cmds);
    tendril_timer_init(&x->cmd_timer, cmd_timeout);
    tendril_timer_init(&x->port_timer, port_timer_work);

    uint32_t cap0 = hc_read32(hc, CAP_CAPLENGTH);
    if (cap0 == 0xffffffff) {
        free_all(hc);
        return TENDRIL_ERR_NO_DEVICE;
    }
    x->op = cap0 & 0xff;
    x->rt = hc_read32(hc, CAP_RTSOFF) & ~(uint32_t)0x1f;
    x->db = hc_read32(hc, CAP_DBOFF) & ~(uint32_t)0x3;
    uint32_t hcs1 = hc_read32(hc, CAP_HCSPARAMS1);
    uint32_t hcs2 = hc_read32(hc, CAP_HCSPARAMS2);
    x->hcc1 = hc_read32(hc, CAP_HCCPARAMS1);
    x->max_slots = hcs1 & 0xff;
    x->max_ports = hcs1 >> 24;
    x->ctx_size = (x->hcc1 & HCC1_CSZ) != 0 ? 64 : 32;
    x->ac64 = (x->hcc1 & HCC1_AC64) != 0
        && (hc->quirks & TENDRIL_HC_QUIRK_NO_64BIT) == 0;
    hc->dma32 = !x->ac64;
    hc->data32 = !x->ac64;
    LOG_INFO(
        "xHCI %x.%02x: %zu slots, %zu ports, %zu-byte contexts%s", cap0 >> 24,
        (cap0 >> 16) & 0xff, x->max_slots, x->max_ports, x->ctx_size,
        x->ac64 ? ", 64-bit" : ""
    );

    x->devs = tendril_zalloc((x->max_slots + 1) * sizeof(struct xhci_dev *));
    x->ports = tendril_zalloc(x->max_ports * sizeof(struct xhci_port_proto));
    if (x->devs == NULL || x->ports == NULL) {
        free_all(hc);
        return TENDRIL_ERR_NO_MEMORY;
    }

    if (hc->config.pci && (hc->quirks & TENDRIL_HC_QUIRK_NO_HANDOFF) == 0) {
        bios_handoff(hc);
    }
    int st = halt_and_reset(hc);
    if (st != TENDRIL_OK) {
        LOG_ERROR("xHCI: reset failed");
        free_all(hc);
        return st;
    }

    uint32_t pagesize = op_read(hc, OP_PAGESIZE) & 0xffff;
    if ((pagesize & 1) == 0) {
        LOG_ERROR("xHCI: 4 KiB pages unsupported (PAGESIZE 0x%x)", pagesize);
        free_all(hc);
        return TENDRIL_ERR_UNSUPPORTED;
    }
    x->page_size = 4096;

    x->dcbaa = tendril_dma_alloc(hc, 2048, 4096, &x->dcbaa_dma);
    if (x->dcbaa == NULL) {
        free_all(hc);
        return TENDRIL_ERR_NO_MEMORY;
    }

    x->num_scratch = ((hcs2 >> 27) & 0x1f) | (((hcs2 >> 21) & 0x1f) << 5);
    if (x->num_scratch != 0) {
        x->scratch_array = tendril_dma_alloc(
            hc, x->num_scratch * sizeof(uint64_t), 64, &x->scratch_array_dma
        );
        x->scratch_bufs = tendril_zalloc(x->num_scratch * sizeof(void *));
        if (x->scratch_array == NULL || x->scratch_bufs == NULL) {
            free_all(hc);
            return TENDRIL_ERR_NO_MEMORY;
        }
        for (size_t i = 0; i < x->num_scratch; i++) {
            uint64_t dma;
            x->scratch_bufs[i] = tendril_dma_alloc(
                hc, x->page_size, x->page_size, &dma
            );
            if (x->scratch_bufs[i] == NULL) {
                free_all(hc);
                return TENDRIL_ERR_NO_MEMORY;
            }
            x->scratch_array[i] = tendril_le64(dma);
        }
        x->dcbaa[0] = tendril_le64(x->scratch_array_dma);
    }

    if (!ring_init(hc, &x->cmd, CMD_RING_TRBS)) {
        free_all(hc);
        return TENDRIL_ERR_NO_MEMORY;
    }
    x->cmd_free = CMD_RING_TRBS - 2;

    x->evt = tendril_dma_alloc(
        hc, EVENT_RING_TRBS * sizeof(struct xhci_trb), 4096, &x->evt_dma
    );
    x->erst = tendril_dma_alloc(hc, 64, 64, &x->erst_dma);
    if (x->evt == NULL || x->erst == NULL) {
        free_all(hc);
        return TENDRIL_ERR_NO_MEMORY;
    }
    x->erst[0] = tendril_le64(x->evt_dma);
    x->erst[1] = tendril_le64(EVENT_RING_TRBS);
    x->evt_cycle = 1;

    op_write(
        hc, OP_CONFIG,
        (op_read(hc, OP_CONFIG) & ~(uint32_t)0xff) | (uint32_t)x->max_slots
    );
    op_write(hc, OP_DNCTRL, 0);
    op_write64(hc, OP_DCBAAP, x->dcbaa_dma);
    op_write64(hc, OP_CRCR, x->cmd.dma | CRCR_RCS);

    rt_write(hc, RT_IR0 + IR_ERSTSZ, 1);
    rt_write64(hc, RT_IR0 + IR_ERDP, x->evt_dma);
    rt_write64(hc, RT_IR0 + IR_ERSTBA, x->erst_dma);
    rt_write(hc, RT_IR0 + IR_IMOD, imod(hc));
    uint32_t cmd = USBCMD_RS | USBCMD_HSEE;
    if (hc->config.irq) {
        rt_write(hc, RT_IR0 + IR_IMAN, IMAN_IE | IMAN_IP);
        cmd |= USBCMD_INTE;
    } else {
        rt_write(hc, RT_IR0 + IR_IMAN, IMAN_IP);
    }

    if (hc->config.pci
     && (hc->quirks & TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING) != 0) {
        intel_port_routing(hc);
    }

    op_write(hc, OP_USBCMD, cmd);
    if (!tendril_hc_wait32(hc, x->op + OP_USBSTS, USBSTS_HCH, 0, 20000)) {
        LOG_ERROR("xHCI: controller did not start");
        free_all(hc);
        return TENDRIL_ERR_TIMEOUT;
    }

    parse_protocols(hc);
    st = tendril_roothub_init(hc, x->max_ports);
    if (st != TENDRIL_OK) {
        op_write(hc, OP_USBCMD, 0);
        free_all(hc);
        return st;
    }
    bool powered = false;
    for (size_t p = 1; p <= x->max_ports; p++) {
        hc->root.ports[p - 1].superspeed = x->ports[p - 1].major >= 3;
        uint32_t sc = op_read(hc, OP_PORTSC(p));
        if ((x->hcc1 & HCC1_PPC) != 0 && (sc & PORTSC_PP) == 0) {
            op_write(hc, OP_PORTSC(p), (sc & PORTSC_KEEP) | PORTSC_PP);
            tendril_roothub_powered(hc, (uint8_t)p, 20);
            powered = true;
        }
    }
    /*
     * The reset left the USB2 ports disconnected (xHCI 1.2, 4.19.1.1.2), and
     * seeing a device again takes up to 2 ms (TDCNN, USB 2.0 Table 7-13);
     * ports switched on just now first need their power to settle (USB 2.0
     * 7.1.7.3).
     */
    tendril_delay_us(powered ? 20000 : 2000);
    return TENDRIL_OK;
}

static void xhci_stop(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    if (x == NULL) {
        return;
    }
    fail_all(hc);
    halt_and_reset(hc);
    for (size_t s = 1; s <= x->max_slots; s++) {
        struct xhci_dev *xd = x->devs[s];
        if (xd == NULL) {
            continue;
        }
        for (size_t i = 0; i < 32; i++) {
            if (xd->eps[i] != NULL) {
                xd->eps[i]->ep->hcd = NULL;
            }
        }
    }
}

static void xhci_destroy(struct tendril_hc *hc) {
    if (hc->hcd != NULL) {
        free_all(hc);
    }
}

/* Root hub */

static void portsc_write(struct tendril_hc *hc, uint8_t port, uint32_t set);

static void set_link_state(
    struct tendril_hc *hc, uint8_t port, uint32_t pls, uint32_t extra
) {
    portsc_write(hc, port, PORTSC_LWS | (pls << PORTSC_PLS_SHIFT) | extra);
}

/*
 * Root ports behave as a hub's would (USB 2.0 11.24.2.7.1.3 and
 * 11.24.2.7.2.3): resume signalling is timed here, and the port reports itself
 * suspended until it is back in U0. xHCI 1.2, 4.15.2 leaves that timing to
 * software on USB2 ports, and a USB3 port that a device woke waits for
 * software to allow U0.
 */
static uint32_t advance_resume(
    struct tendril_hc *hc, uint8_t port, uint32_t sc
) {
    struct xhci *x = X(hc);
    struct xhci_port_proto *p = &x->ports[port - 1];
    uint32_t pls = (sc & PORTSC_PLS_MASK) >> PORTSC_PLS_SHIFT;

    if (p->resume == RESUME_ENDING && pls != PLS_RESUME) {
        p->resume = RESUME_IDLE;
    }
    if (p->resume == RESUME_IDLE && pls == PLS_RESUME
     && (sc & PORTSC_PED) != 0) {
        /*
         * The device signalled a wakeup. Its PLC is consumed here; U0 raises
         * another.
         */
        if (p->major >= 3) {
            set_link_state(hc, port, PLS_U0, PORTSC_PLC);
            p->resume = RESUME_ENDING;
        } else {
            portsc_write(hc, port, PORTSC_PLC);
            p->resume = RESUME_SIGNALLING;
            p->resume_start = tendril_now();
            tendril_timer_arm(
                hc, &x->port_timer, RESUME_SIGNAL_MS * NS_PER_MS
            );
        }
        return op_read(hc, OP_PORTSC(port));
    }
    if (p->resume == RESUME_SIGNALLING
     && tendril_now() - p->resume_start >= RESUME_SIGNAL_MS * NS_PER_MS) {
        set_link_state(hc, port, PLS_U0, 0);
        p->resume = RESUME_ENDING;
        return op_read(hc, OP_PORTSC(port));
    }
    return sc;
}

static void port_timer_work(struct tendril_work *work) {
    struct xhci *x = CONTAINER_OF(work, struct xhci, port_timer.work);
    struct tendril_hc *hc = x->hc;
    hc_lock(hc);
    bool again = false;
    for (size_t p = 1; !hc->dead && p <= x->max_ports; p++) {
        if (x->ports[p - 1].resume == RESUME_SIGNALLING) {
            advance_resume(hc, (uint8_t)p, op_read(hc, OP_PORTSC(p)));
            again |= x->ports[p - 1].resume == RESUME_SIGNALLING;
        }
    }
    if (again) {
        tendril_timer_arm(hc, &x->port_timer, 2 * NS_PER_MS);
    }
    hc_unlock(hc);
}

static void xhci_port_status(
    struct tendril_hc *hc, uint8_t port, struct tendril_port_status *st
) {
    struct xhci *x = X(hc);
    uint32_t sc = advance_resume(hc, port, op_read(hc, OP_PORTSC(port)));
    bool ss = x->ports[port - 1].major >= 3;
    uint32_t pls = (sc & PORTSC_PLS_MASK) >> PORTSC_PLS_SHIFT;
    bool resuming = pls == PLS_RESUME
        || x->ports[port - 1].resume != RESUME_IDLE;

    st->status = 0;
    st->change = 0;
    if ((sc & PORTSC_CCS) != 0) {
        st->status |= TENDRIL_PORT_STAT_CONNECTION;
    }
    if ((sc & PORTSC_PED) != 0) {
        st->status |= TENDRIL_PORT_STAT_ENABLE;
    }
    if ((sc & PORTSC_OCA) != 0) {
        st->status |= TENDRIL_PORT_STAT_OVER_CURRENT;
    }
    if ((sc & PORTSC_PR) != 0) {
        st->status |= TENDRIL_PORT_STAT_RESET;
    }
    st->speed = port_speed(hc, port, PORTSC_SPEED(sc));
    if (ss) {
        /*
         * Hubs have no Resume link state: the link is in U3 until it reaches
         * U0.
         */
        uint32_t shown = pls == PLS_RESUME ? PLS_U3 : pls;
        st->status |= (uint16_t)(shown
            << TENDRIL_SS_PORT_STAT_LINK_STATE_SHIFT);
        if ((sc & PORTSC_PP) != 0) {
            st->status |= TENDRIL_SS_PORT_STAT_POWER;
        }
    } else {
        if ((sc & PORTSC_PP) != 0) {
            st->status |= TENDRIL_PORT_STAT_POWER;
        }
        if (pls == PLS_U3 || resuming) {
            st->status |= TENDRIL_PORT_STAT_SUSPEND;
        }
        if ((sc & PORTSC_CCS) != 0 && st->speed == TENDRIL_SPEED_LOW) {
            st->status |= TENDRIL_PORT_STAT_LOW_SPEED;
        } else if ((sc & PORTSC_CCS) != 0 && st->speed == TENDRIL_SPEED_HIGH) {
            st->status |= TENDRIL_PORT_STAT_HIGH_SPEED;
        }
    }
    if ((sc & PORTSC_CSC) != 0) {
        st->change |= TENDRIL_PORT_STAT_C_CONNECTION;
    }
    if ((sc & PORTSC_PEC) != 0) {
        st->change |= TENDRIL_PORT_STAT_C_ENABLE;
    }
    if ((sc & PORTSC_OCC) != 0) {
        st->change |= TENDRIL_PORT_STAT_C_OVER_CURRENT;
    }
    if ((sc & PORTSC_PRC) != 0) {
        st->change |= TENDRIL_PORT_STAT_C_RESET;
    }
    if ((sc & PORTSC_WRC) != 0) {
        st->change |= TENDRIL_PORT_STAT_C_BH_RESET;
    }
    if ((sc & PORTSC_PLC) != 0) {
        st->change |= ss ? TENDRIL_PORT_STAT_C_LINK_STATE
            : TENDRIL_PORT_STAT_C_SUSPEND;
    }
    if ((sc & PORTSC_CEC) != 0) {
        st->change |= TENDRIL_PORT_STAT_C_CONFIG_ERROR;
    }
}

static void portsc_write(struct tendril_hc *hc, uint8_t port, uint32_t set) {
    uint32_t sc = op_read(hc, OP_PORTSC(port));
    op_write(hc, OP_PORTSC(port), (sc & PORTSC_KEEP) | set);
}

static int xhci_port_set_feature(
    struct tendril_hc *hc, uint8_t port, int feature
) {
    switch (PORT_FEATURE_SELECTOR(feature)) {
        case TENDRIL_PORT_FEAT_RESET: {
            portsc_write(hc, port, PORTSC_PR);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_BH_RESET: {
            portsc_write(hc, port, PORTSC_WPR);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_POWER: {
            portsc_write(hc, port, PORTSC_PP);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            set_link_state(hc, port, PLS_U3, 0);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_LINK_STATE: {
            /*
             * USB3 ports: U3 suspends, U0 resumes (xHCI 1.2, 4.15.1 and
             * 4.15.2.2).
             */
            uint32_t target = (uint32_t)PORT_FEATURE_PARAM(feature);
            if (target != PLS_U0 && target != PLS_U3) {
                return TENDRIL_ERR_UNSUPPORTED;
            }
            set_link_state(hc, port, target, 0);
            return TENDRIL_OK;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
}

static int xhci_port_clear_feature(
    struct tendril_hc *hc, uint8_t port, int feature
) {
    struct xhci *x = X(hc);
    switch (feature) {
        case TENDRIL_PORT_FEAT_ENABLE: {
            /* BEGIN FreeBSD sys/dev/usb/controller/xhci.c */
            /*
             * FreeBSD's XHCI_QUIRK_DISABLE_PORT_PED
             * (sys/dev/usb/controller/xhci.c, BSD-2-Clause).
             */
            if ((hc->quirks & TENDRIL_HC_QUIRK_NO_PORT_DISABLE) == 0) {
                portsc_write(hc, port, PORTSC_PED);
            }
            /* END FreeBSD sys/dev/usb/controller/xhci.c */
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_POWER: {
            uint32_t sc = op_read(hc, OP_PORTSC(port));
            op_write(hc, OP_PORTSC(port), sc & PORTSC_KEEP & ~PORTSC_PP);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            /*
             * USB2 ports: drive resume, ended once advance_resume() has timed
             * it.
             */
            struct xhci_port_proto *p = &x->ports[port - 1];
            uint32_t sc = op_read(hc, OP_PORTSC(port));
            if (((sc & PORTSC_PLS_MASK) >> PORTSC_PLS_SHIFT) == PLS_U3
             && p->resume == RESUME_IDLE) {
                set_link_state(hc, port, PLS_RESUME, 0);
                p->resume = RESUME_SIGNALLING;
                p->resume_start = tendril_now();
                tendril_timer_arm(
                    hc, &x->port_timer, RESUME_SIGNAL_MS * NS_PER_MS
                );
            }
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_CONNECTION: {
            portsc_write(hc, port, PORTSC_CSC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_ENABLE: {
            portsc_write(hc, port, PORTSC_PEC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_OVER_CURRENT: {
            portsc_write(hc, port, PORTSC_OCC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_RESET: {
            portsc_write(hc, port, PORTSC_PRC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_BH_RESET: {
            portsc_write(hc, port, PORTSC_WRC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_LINK_STATE:
        case TENDRIL_PORT_FEAT_C_SUSPEND: {
            portsc_write(hc, port, PORTSC_PLC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_CONFIG_ERROR: {
            portsc_write(hc, port, PORTSC_CEC);
            return TENDRIL_OK;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
}

/* Devices */

static uint32_t speed_id(struct tendril_hc *hc, struct tendril_device *dev) {
    /*
     * Slot contexts want the root port's PORTSC speed ID; walk up to the root.
     */
    if (dev->parent == NULL) {
        return PORTSC_SPEED(op_read(hc, OP_PORTSC(dev->root_port)));
    }
    switch (dev->speed) {
        case TENDRIL_SPEED_FULL: {
            return 1;
        }
        case TENDRIL_SPEED_LOW: {
            return 2;
        }
        case TENDRIL_SPEED_HIGH: {
            return 3;
        }
        case TENDRIL_SPEED_SUPER: {
            return 4;
        }
        default: {
            return 5;
        }
    }
}

static struct xhci_ep *ep_alloc(
    struct tendril_hc *hc, struct tendril_endpoint *ep, size_t size
) {
    struct xhci_ep *xep = tendril_zalloc(sizeof(struct xhci_ep));
    if (xep == NULL) {
        return NULL;
    }
    xep->ep = ep;
    xep->dev = ep->dev;
    xep->dci = ep_dci(ep);
    if (!ring_init(hc, &xep->ring, size)) {
        tendril_free(xep, sizeof(struct xhci_ep));
        return NULL;
    }
    return xep;
}

static void free_streams(struct tendril_hc *hc, struct xhci_ep *xep) {
    if (xep->streams != NULL) {
        for (size_t s = 1; s <= xep->num_streams; s++) {
            ring_free(hc, &xep->streams[s]);
        }
        tendril_free(
            xep->streams, (xep->num_streams + 1) * sizeof(struct xhci_ring)
        );
        xep->streams = NULL;
    }
    tendril_dma_free(hc, xep->stream_ctx, xep->stream_ctx_size);
    xep->stream_ctx = NULL;
    xep->num_streams = 0;
}

static void ep_free(struct tendril_hc *hc, struct xhci_ep *xep) {
    if (xep == NULL) {
        return;
    }
    free_streams(hc, xep);
    ring_free(hc, &xep->ring);
    if (xep->ep->hcd == xep) {
        xep->ep->hcd = NULL;
    }
    tendril_free(xep, sizeof(struct xhci_ep));
}

static void dev_free_mem(struct tendril_hc *hc, struct xhci_dev *xd) {
    for (size_t i = 0; i < 32; i++) {
        ep_free(hc, xd->eps[i]);
    }
    tendril_dma_free(hc, xd->out_ctx, 4096);
    tendril_dma_free(hc, xd->in_ctx, 4096);
    tendril_free(xd, sizeof(struct xhci_dev));
}

static void enable_slot_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    struct xhci *x = X(hc);
    struct tendril_device *dev = cmd->dev;
    struct xhci_dev *xd = xdev(dev);

    if (code != CC_SUCCESS) {
        tendril_async_complete(hc, cmd->async, cc_status(code));
        return;
    }
    uint8_t slot = (uint8_t)(trb_d(ev, 3) >> 24);
    if (slot == 0 || slot > x->max_slots || x->devs[slot] != NULL) {
        tendril_async_complete(hc, cmd->async, TENDRIL_ERR_CONTROLLER);
        return;
    }
    xd->slot = slot;
    x->devs[slot] = xd;
    x->dcbaa[slot] = tendril_le64(xd->out_ctx_dma);
    tendril_async_complete(hc, cmd->async, TENDRIL_OK);
}

static int xhci_device_init(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async
) {
    struct xhci_dev *xd = tendril_zalloc(sizeof(struct xhci_dev));
    if (xd == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    xd->out_ctx = tendril_dma_alloc(hc, 4096, 4096, &xd->out_ctx_dma);
    xd->in_ctx = tendril_dma_alloc(hc, 4096, 4096, &xd->in_ctx_dma);
    xd->eps[1] = ep_alloc(hc, &dev->ep0, SMALL_RING_TRBS);
    if (xd->out_ctx == NULL || xd->in_ctx == NULL || xd->eps[1] == NULL) {
        dev_free_mem(hc, xd);
        return TENDRIL_ERR_NO_MEMORY;
    }
    dev->ep0.hcd = xd->eps[1];
    dev->hcd = xd;
    xd->speed_id = (uint8_t)speed_id(hc, dev);

    int st = cmd_submit(
        hc, 0, 0, TRB_TYPE(TRB_ENABLE_SLOT), enable_slot_done, async, dev,
        NULL, 0
    );
    if (st != TENDRIL_OK) {
        dev->ep0.hcd = NULL;
        dev->hcd = NULL;
        dev_free_mem(hc, xd);
    }
    return st;
}

static void fill_ep_ctx(
    struct tendril_hc *hc, struct xhci_dev *xd, struct xhci_ep *xep
) {
    struct tendril_endpoint *ep = xep->ep;
    uint32_t *c = in_ep(hc, xd, xep->dci);
    bool in = (ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
    unsigned int type;
    unsigned int avg;

    switch (ep->type) {
        case TENDRIL_ENDPOINT_XFER_CONTROL: {
            type = EP_TYPE_CONTROL;
            avg = 8;
            break;
        }
        case TENDRIL_ENDPOINT_XFER_ISOC: {
            type = in ? EP_TYPE_ISOCH_IN : EP_TYPE_ISOCH_OUT;
            avg = 3072;
            break;
        }
        case TENDRIL_ENDPOINT_XFER_BULK: {
            type = in ? EP_TYPE_BULK_IN : EP_TYPE_BULK_OUT;
            avg = 3072;
            break;
        }
        default: {
            type = in ? EP_TYPE_INT_IN : EP_TYPE_INT_OUT;
            avg = 1024;
            break;
        }
    }

    bool ss = tendril_device_is_ss(ep->dev);
    bool periodic = ep->type == TENDRIL_ENDPOINT_XFER_ISOC
        || ep->type == TENDRIL_ENDPOINT_XFER_INT;
    unsigned int burst = 0;
    unsigned int mult = 0;
    if (ss) {
        burst = ep->max_burst;
        if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
            mult = ep->mult - (uint32_t)1;
        }
    } else if (ep->dev->speed == TENDRIL_SPEED_HIGH && periodic) {
        burst = ep->mult - (uint32_t)1;
    }
    unsigned int interval = periodic || ep->dev->speed == TENDRIL_SPEED_HIGH
        ? ep->interval_log2
        : 0;
    uint32_t esit = periodic ? (uint32_t)ep->max_esit_payload : 0;
    unsigned int cerr = ep->type == TENDRIL_ENDPOINT_XFER_ISOC ? 0 : 3;

    ctx_set(c, 0, (mult << 8) | (interval << 16) | ((esit >> 16) << 24));
    ctx_set(
        c, 1,
        (cerr << 1) | (type << 3) | (burst << 8)
            | ((uint32_t)ep->max_packet << 16)
    );
    uint64_t deq = xep->ring.dma | xep->ring.cycle;
    ctx_set(c, 2, (uint32_t)deq);
    ctx_set(c, 3, (uint32_t)(deq >> 32));
    ctx_set(c, 4, avg | ((esit & 0xffff) << 16));
}

static void fill_slot_ctx(
    struct tendril_hc *hc, struct tendril_device *dev, size_t entries
) {
    struct xhci_dev *xd = xdev(dev);
    uint32_t *s = in_slot(hc, xd);
    uint32_t dw0 = (dev->route & 0xfffff) | ((uint32_t)xd->speed_id << 20)
        | ((uint32_t)entries << 27);
    uint32_t dw2 = 0;

    if (dev->tt_hub != NULL && dev->tt_hub->hcd != NULL) {
        dw2 |= (uint32_t)xdev(dev->tt_hub)->slot
            | ((uint32_t)dev->tt_port << 8);
        if (dev->tt_hub->hub != NULL && dev->tt_hub->hub->multi_tt) {
            dw0 |= (uint32_t)1 << 25;
        }
    }
    if (dev->hub != NULL) {
        dw0 |= (uint32_t)1 << 26;
        if (dev->hub->multi_tt) {
            dw0 |= (uint32_t)1 << 25;
        }
        if (dev->speed == TENDRIL_SPEED_HIGH) {
            dw2 |= (uint32_t)dev->hub->think_time << 16;
        }
    }
    ctx_set(s, 0, dw0);
    uint32_t ports = dev->hub != NULL ? (uint32_t)dev->hub->num_ports : 0;
    ctx_set(s, 1, (uint32_t)dev->root_port << 16 | ports << 24);
    ctx_set(s, 2, dw2);
    ctx_set(s, 3, 0);
}

static void address_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    struct tendril_device *dev = cmd->dev;
    (void)ev;
    if (code == CC_SUCCESS) {
        dev->address = (uint8_t)(ctx_get(out_slot(hc, xdev(dev)), 3) & 0xff);
    }
    tendril_async_complete(
        hc, cmd->async, code == CC_SUCCESS ? TENDRIL_OK : cc_status(code)
    );
}

static int xhci_device_address(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async
) {
    struct xhci_dev *xd = xdev(dev);
    clear_input(hc, xd);
    ctx_set(in_ctrl(hc, xd), 1, ((uint32_t)1 << 0) | ((uint32_t)1 << 1));
    fill_slot_ctx(hc, dev, 1);
    fill_ep_ctx(hc, xd, xd->eps[1]);
    return cmd_submit(
        hc, xd->in_ctx_dma, 0,
        TRB_TYPE(TRB_ADDRESS_DEVICE) | TRB_SLOT(xd->slot), address_done, async,
        dev, NULL, 0
    );
}

static int xhci_ep0_update(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async
) {
    struct xhci_dev *xd = xdev(dev);
    uint32_t *out = out_ep(hc, xd, 1);
    if (((ctx_get(out, 1) >> 16) & 0xffff) == dev->ep0.max_packet) {
        tendril_async_complete(hc, async, TENDRIL_OK);
        return TENDRIL_OK;
    }
    clear_input(hc, xd);
    ctx_set(in_ctrl(hc, xd), 1, (uint32_t)1 << 1);
    uint32_t *c = in_ep(hc, xd, 1);
    memcpy(c, out, X(hc)->ctx_size);
    ctx_set(
        c, 1,
        (ctx_get(out, 1) & 0xffff) | ((uint32_t)dev->ep0.max_packet << 16)
    );
    return cmd_submit(
        hc, xd->in_ctx_dma, 0,
        TRB_TYPE(TRB_EVALUATE_CONTEXT) | TRB_SLOT(xd->slot), cmd_async_done,
        async, dev, NULL, 0
    );
}

static size_t context_entries(struct xhci_dev *xd) {
    size_t last = 1;
    for (size_t i = 2; i < 32; i++) {
        if (xd->eps[i] != NULL) {
            last = i;
        }
    }
    return last;
}

static void copy_out_slot(struct tendril_hc *hc, struct xhci_dev *xd) {
    memcpy(in_slot(hc, xd), out_slot(hc, xd), X(hc)->ctx_size);
    /* Slot state and address are output-only. */
    ctx_set(in_slot(hc, xd), 3, 0);
}

static int xhci_hub_update(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async
) {
    struct xhci_dev *xd = xdev(dev);
    clear_input(hc, xd);
    ctx_set(in_ctrl(hc, xd), 1, (uint32_t)1 << 0);
    fill_slot_ctx(hc, dev, context_entries(xd));
    return cmd_submit(
        hc, xd->in_ctx_dma, 0,
        TRB_TYPE(TRB_CONFIGURE_ENDPOINT) | TRB_SLOT(xd->slot), cmd_async_done,
        async, dev, NULL, 0
    );
}

/* Endpoints being swapped by an outstanding Configure Endpoint command. */
struct config_change {
    struct xhci_ep *add[31];
    size_t num_add;
    struct xhci_ep *drop[31];
    size_t num_drop;
};

static void configure_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    struct tendril_device *dev = cmd->dev;
    struct xhci_dev *xd = xdev(dev);
    struct config_change *cc = (struct config_change *)(uintptr_t)cmd->xep;
    (void)ev;

    if (code == CC_SUCCESS) {
        for (size_t i = 0; i < cc->num_drop; i++) {
            struct xhci_ep *old = cc->drop[i];
            old->ep->open = false;
            if (xd->eps[old->dci] == old) {
                xd->eps[old->dci] = NULL;
            }
            ep_free(hc, old);
        }
        for (size_t i = 0; i < cc->num_add; i++) {
            cc->add[i]->ep->open = true;
        }
    } else {
        for (size_t i = 0; i < cc->num_add; i++) {
            struct xhci_ep *nep = cc->add[i];
            if (xd->eps[nep->dci] == nep) {
                xd->eps[nep->dci] = NULL;
            }
            ep_free(hc, nep);
        }
        /* The dropped endpoints stay as they were. */
        for (size_t i = 0; i < cc->num_drop; i++) {
            xd->eps[cc->drop[i]->dci] = cc->drop[i];
        }
    }
    tendril_free(cc, sizeof(struct config_change));
    tendril_async_complete(
        hc, cmd->async, code == CC_SUCCESS ? TENDRIL_OK : cc_status(code)
    );
}

static int xhci_configure(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_endpoint **drop, size_t num_drop,
    struct tendril_endpoint **add, size_t num_add, struct tendril_async *async
) {
    struct xhci_dev *xd = xdev(dev);
    uint32_t drop_flags = 0;
    uint32_t add_flags = (uint32_t)1 << 0;

    if (num_drop == 0 && num_add == 0) {
        tendril_async_complete(hc, async, TENDRIL_OK);
        return TENDRIL_OK;
    }
    struct config_change *cc = tendril_zalloc(sizeof(struct config_change));
    if (cc == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }

    clear_input(hc, xd);
    for (size_t i = 0; i < num_drop; i++) {
        struct xhci_ep *old = xep_of(drop[i]);
        if (old == NULL) {
            continue;
        }
        drop_flags |= (uint32_t)1 << old->dci;
        cc->drop[cc->num_drop++] = old;
        xd->eps[old->dci] = NULL;
    }
    for (size_t i = 0; i < num_add; i++) {
        struct tendril_endpoint *ep = add[i];
        bool large = ep->type == TENDRIL_ENDPOINT_XFER_BULK
            || ep->type == TENDRIL_ENDPOINT_XFER_ISOC;
        size_t size = large ? LARGE_RING_TRBS : SMALL_RING_TRBS;
        struct xhci_ep *nep = ep_alloc(hc, ep, size);
        if (nep == NULL) {
            for (size_t j = 0; j < cc->num_add; j++) {
                xd->eps[cc->add[j]->dci] = NULL;
                ep_free(hc, cc->add[j]);
            }
            for (size_t j = 0; j < cc->num_drop; j++) {
                xd->eps[cc->drop[j]->dci] = cc->drop[j];
            }
            tendril_free(cc, sizeof(struct config_change));
            return TENDRIL_ERR_NO_MEMORY;
        }
        ep->hcd = nep;
        xd->eps[nep->dci] = nep;
        add_flags |= (uint32_t)1 << nep->dci;
        cc->add[cc->num_add++] = nep;
        fill_ep_ctx(hc, xd, nep);
    }
    ctx_set(in_ctrl(hc, xd), 0, drop_flags & ~(uint32_t)3);
    ctx_set(in_ctrl(hc, xd), 1, add_flags);
    copy_out_slot(hc, xd);
    uint32_t *s = in_slot(hc, xd);
    ctx_set(
        s, 0,
        (ctx_get(s, 0) & ~((uint32_t)0x1f << 27))
            | ((uint32_t)context_entries(xd) << 27)
    );

    int st = cmd_submit(
        hc, xd->in_ctx_dma, 0,
        TRB_TYPE(TRB_CONFIGURE_ENDPOINT) | TRB_SLOT(xd->slot), configure_done,
        async, dev, (struct xhci_ep *)(uintptr_t)cc, 0
    );
    if (st != TENDRIL_OK) {
        for (size_t j = 0; j < cc->num_add; j++) {
            xd->eps[cc->add[j]->dci] = NULL;
            ep_free(hc, cc->add[j]);
        }
        for (size_t j = 0; j < cc->num_drop; j++) {
            xd->eps[cc->drop[j]->dci] = cc->drop[j];
        }
        tendril_free(cc, sizeof(struct config_change));
    }
    return st;
}

/*
 * Resets the endpoint's sequence state after a halt was cleared. Recovery
 * already reset it with Reset Endpoint after a halt; otherwise, dropping and
 * re-adding the endpoint does.
 */
static int xhci_endpoint_reset(
    struct tendril_hc *hc, struct tendril_endpoint *ep,
    struct tendril_async *async
) {
    struct xhci_ep *xep = xep_of(ep);
    if (xep == NULL || ep->type == TENDRIL_ENDPOINT_XFER_CONTROL
     || xep->num_streams != 0 || has_pending(xep) || xep->stopping
     || xep->recovering) {
        tendril_async_complete(hc, async, TENDRIL_OK);
        return TENDRIL_OK;
    }
    struct xhci_dev *xd = xdev(ep->dev);
    clear_input(hc, xd);
    xep->ring.enq = 0;
    xep->ring.deq = 0;
    xep->ring.cycle = 1;
    memset(xep->ring.trbs, 0, (xep->ring.size - 1) * sizeof(struct xhci_trb));
    trb_write(
        &xep->ring.trbs[xep->ring.size - 1], xep->ring.dma, 0,
        TRB_TYPE(TRB_LINK) | TRB_TC
    );
    fill_ep_ctx(hc, xd, xep);
    ctx_set(in_ctrl(hc, xd), 0, (uint32_t)1 << xep->dci);
    ctx_set(
        in_ctrl(hc, xd), 1, ((uint32_t)1 << 0) | ((uint32_t)1 << xep->dci)
    );
    copy_out_slot(hc, xd);
    return cmd_submit(
        hc, xd->in_ctx_dma, 0,
        TRB_TYPE(TRB_CONFIGURE_ENDPOINT) | TRB_SLOT(xd->slot), cmd_async_done,
        async, ep->dev, xep, 0
    );
}

static int xhci_alloc_streams(
    struct tendril_hc *hc, struct tendril_endpoint **eps, size_t count,
    size_t streams, struct tendril_async *async
) {
    struct xhci *x = X(hc);
    unsigned int max_psa = HCC1_MAXPSA(x->hcc1);
    if (max_psa == 0) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    /*
     * A linear primary array of 2^(MaxPStreams + 1) entries, stream 0
     * reserved.
     */
    unsigned int order = 1;
    while (((uint32_t)1 << order) < streams + 1) {
        order++;
    }
    if (order > max_psa + 1) {
        order = max_psa + 1;
    }
    unsigned int entries = (uint32_t)1 << order;
    if (streams > entries - 1) {
        streams = entries - 1;
    }

    struct tendril_device *dev = eps[0]->dev;
    struct xhci_dev *xd = xdev(dev);
    clear_input(hc, xd);
    uint32_t flags = (uint32_t)1 << 0;
    for (size_t i = 0; i < count; i++) {
        struct xhci_ep *xep = xep_of(eps[i]);
        if (xep == NULL || has_pending(xep) || xep->num_streams != 0) {
            return TENDRIL_ERR_BUSY;
        }
    }
    int st = TENDRIL_ERR_NO_MEMORY;
    for (size_t i = 0; i < count; i++) {
        struct xhci_ep *xep = xep_of(eps[i]);
        xep->stream_ctx_size = entries * 16;
        xep->stream_ctx = tendril_dma_alloc(
            hc, xep->stream_ctx_size, xep->stream_ctx_size,
            &xep->stream_ctx_dma
        );
        xep->streams = tendril_zalloc(
            (streams + 1) * sizeof(struct xhci_ring)
        );
        if (xep->stream_ctx == NULL || xep->streams == NULL) {
            goto fail;
        }
        xep->num_streams = streams;
        uint32_t *sc = xep->stream_ctx;
        for (size_t s = 1; s <= streams; s++) {
            if (!ring_init(hc, &xep->streams[s], STREAM_RING_TRBS)) {
                goto fail;
            }
            uint64_t deq = xep->streams[s].dma | (uint32_t)1
                | ((uint32_t)1 << 1);
            sc[s * 4 + 0] = tendril_le32((uint32_t)deq);
            sc[s * 4 + 1] = tendril_le32((uint32_t)(deq >> 32));
        }
        eps[i]->streams = streams;
        fill_ep_ctx(hc, xd, xep);
        uint32_t *c = in_ep(hc, xd, xep->dci);
        /* MaxPStreams and LSA; the dequeue pointer names the stream array. */
        ctx_set(
            c, 0, ctx_get(c, 0) | ((order - 1) << 10) | ((uint32_t)1 << 15)
        );
        ctx_set(c, 2, (uint32_t)xep->stream_ctx_dma);
        ctx_set(c, 3, (uint32_t)(xep->stream_ctx_dma >> 32));
        flags |= (uint32_t)1 << xep->dci;
    }
    ctx_set(in_ctrl(hc, xd), 0, flags & ~(uint32_t)3);
    ctx_set(in_ctrl(hc, xd), 1, flags);
    copy_out_slot(hc, xd);
    st = cmd_submit(
        hc, xd->in_ctx_dma, 0,
        TRB_TYPE(TRB_CONFIGURE_ENDPOINT) | TRB_SLOT(xd->slot), cmd_async_done,
        async, dev, NULL, 0
    );
    if (st == TENDRIL_OK) {
        return TENDRIL_OK;
    }

fail:
    /* Nothing was configured, so whatever streams were set up go again. */
    for (size_t i = 0; i < count; i++) {
        free_streams(hc, xep_of(eps[i]));
        eps[i]->streams = 0;
    }
    return st;
}

static void disable_slot_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    struct xhci *x = X(hc);
    struct tendril_device *dev = cmd->dev;
    struct xhci_dev *xd = xdev(dev);
    (void)ev;
    (void)code;
    x->devs[xd->slot] = NULL;
    x->dcbaa[xd->slot] = 0;
    dev->hcd = NULL;
    dev_free_mem(hc, xd);
    tendril_async_complete(hc, cmd->async, TENDRIL_OK);
}

static void xhci_device_free(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async
) {
    struct xhci *x = X(hc);
    struct xhci_dev *xd = xdev(dev);

    if (xd == NULL) {
        tendril_async_complete(hc, async, TENDRIL_OK);
        return;
    }
    if (xd->slot != 0 && !hc->dead) {
        int st = cmd_submit(
            hc, 0, 0, TRB_TYPE(TRB_DISABLE_SLOT) | TRB_SLOT(xd->slot),
            disable_slot_done, async, dev, NULL, 0
        );
        if (st == TENDRIL_OK) {
            return;
        }
    }
    if (xd->slot != 0) {
        x->devs[xd->slot] = NULL;
        x->dcbaa[xd->slot] = 0;
    }
    dev->hcd = NULL;
    dev_free_mem(hc, xd);
    tendril_async_complete(hc, async, TENDRIL_OK);
}

/* Transfers */

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

/* Takes up to `want` contiguous bytes, never crossing a 64 KiB boundary. */
static size_t cursor_take(struct cursor *c, size_t want, uint64_t *addr) {
    while (c->idx < c->num && c->off == c->segs[c->idx].len) {
        c->idx++;
        c->off = 0;
    }
    if (c->idx >= c->num || want == 0) {
        *addr = 0;
        return 0;
    }
    uint64_t a = c->segs[c->idx].addr + c->off;
    size_t n = MIN(want, c->segs[c->idx].len - c->off);
    n = MIN(n, (size_t)(0x10000 - (a & 0xffff)));
    c->off += n;
    *addr = a;
    return n;
}

static size_t count_td(struct cursor *c, size_t len) {
    size_t n = 0;
    size_t done = 0;
    if (len == 0) {
        return 1;
    }
    while (done < len) {
        uint64_t addr;
        size_t got = cursor_take(c, len - done, &addr);
        if (got == 0) {
            return 0;
        }
        done += got;
        n++;
    }
    return n;
}

struct td_builder {
    struct xhci_ring *ring;
    size_t first;
    uint32_t first_cycle;
    size_t count;
};

static void tb_push(
    struct td_builder *tb, uint64_t param, uint32_t status, uint32_t control
) {
    bool first = tb->count == 0;
    if (first) {
        tb->first = tb->ring->enq;
        tb->first_cycle = tb->ring->cycle;
    }
    /* The first TRB is handed over last, so the controller sees whole TDs. */
    ring_push(tb->ring, param, status, control, !first);
    tb->count++;
}

static void push_td(
    struct td_builder *tb, struct cursor *c, size_t len, size_t mps,
    uint32_t first_ctl, uint32_t all_ctl, uint32_t last_ctl
) {
    size_t tdpc = len == 0 ? 0 : (len + mps - 1) / mps;
    size_t done = 0;
    bool first = true;

    do {
        uint64_t addr = 0;
        size_t n = cursor_take(c, len - done, &addr);
        done += n;
        bool last = done >= len;
        size_t remaining = done / mps;
        uint32_t td_size = last ? 0
            : (uint32_t)MIN((size_t)31, tdpc - remaining);
        uint32_t ctl = (first ? first_ctl : TRB_TYPE(TRB_NORMAL)) | all_ctl;
        ctl |= last ? last_ctl : TRB_CH;
        tb_push(tb, addr, (uint32_t)n | TRB_TD_SIZE(td_size), ctl);
        first = false;
        if (n == 0) {
            break;
        }
    } while (done < len);
}

static uint64_t setup_param(const struct tendril_setup *setup) {
    const uint8_t *b = (const uint8_t *)setup;
    uint64_t v = 0;
    for (size_t i = 0; i < 8; i++) {
        v |= (uint64_t)b[i] << (8 * i);
    }
    return v;
}

static int submit_control(
    struct tendril_hc *hc, struct xhci_ep *xep, struct tendril_transfer *xfer
) {
    struct xhci_ring *ring = &xep->ring;
    uintptr_t *hd = xfer->internal.hcd_data;
    size_t len = tendril_le16(xfer->setup.length);
    bool in = (xfer->setup.request_type & TENDRIL_DIR_IN) != 0;
    /* BEGIN FreeBSD sys/dev/usb/controller/xhci.c */
    /*
     * FreeBSD's ctlstep (sys/dev/usb/controller/xhci.c, BSD-2-Clause): the
     * status stage waits for the data stage.
     */
    bool step = len != 0
        && (hc->quirks & TENDRIL_HC_QUIRK_XHCI_CTRL_STEP) != 0;
    /* END FreeBSD sys/dev/usb/controller/xhci.c */
    struct cursor c;

    cursor_init(&c, xfer);
    size_t need = 2 + (len != 0 ? count_td(&c, len) : 0);
    if (len != 0 && need == 2) {
        return TENDRIL_ERR_INVALID;
    }
    if (need > ring_space(ring)) {
        return TENDRIL_ERR_BUSY;
    }

    struct td_builder tb = { ring, 0, 0, 0 };
    uint32_t trt = len == 0 ? 0 : in ? (uint32_t)3 << 16 : (uint32_t)2 << 16;
    tb_push(
        &tb, setup_param(&xfer->setup), 8, TRB_TYPE(TRB_SETUP) | TRB_IDT | trt
    );
    if (len != 0) {
        cursor_init(&c, xfer);
        push_td(
            &tb, &c, len, xep->ep->max_packet,
            TRB_TYPE(TRB_DATA) | (in ? TRB_DIR_IN : 0), in ? TRB_ISP : 0,
            step ? TRB_IOC : 0
        );
    }
    /* The status stage runs opposite to the data, or IN without data. */
    uint32_t status_ctl = TRB_TYPE(TRB_STATUS) | TRB_IOC
        | ((len == 0 || !in) ? TRB_DIR_IN : 0);
    /* BEGIN FreeBSD sys/dev/usb/controller/xhci.c */
    if (step) {
        size_t index = ring->enq;
        uint32_t cycle = ring->cycle;
        ring_push(ring, 0, 0, status_ctl, false);
        tb.count++;
        hd[XD_STATUS_TRB] = index | (uintptr_t)cycle << 16;
        hd[XD_FLAGS] |= XF_STEP;
    } else {
        tb_push(&tb, 0, 0, status_ctl);
    }
    /* END FreeBSD sys/dev/usb/controller/xhci.c */
    hd[XD_FIRST] = tb.first;
    hd[XD_COUNT] = tb.count;
    give_first(ring, tb.first, tb.first_cycle);
    return TENDRIL_OK;
}

static int submit_bulk(
    struct tendril_hc *hc, struct xhci_ep *xep, struct xhci_ring *ring,
    struct tendril_transfer *xfer
) {
    uintptr_t *hd = xfer->internal.hcd_data;
    struct tendril_endpoint *ep = xep->ep;
    bool in = (ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
    size_t len = xfer->length;
    bool zlp = !in && (xfer->flags & TENDRIL_XFER_ZERO_PACKET) != 0 && len != 0
        && len % ep->max_packet == 0;
    struct cursor c;
    (void)hc;

    cursor_init(&c, xfer);
    size_t need = count_td(&c, len);
    if (need == 0) {
        return TENDRIL_ERR_INVALID;
    }
    need += zlp ? 1 : 0;
    if (need > ring->size - 2) {
        return xfer->internal.bounce == NULL ? TENDRIL_ERR_BOUNCE
            : TENDRIL_ERR_INVALID;
    }
    if (need > ring_space(ring)) {
        return TENDRIL_ERR_BUSY;
    }

    struct td_builder tb = { ring, 0, 0, 0 };
    cursor_init(&c, xfer);
    push_td(
        &tb, &c, len, ep->max_packet, TRB_TYPE(TRB_NORMAL), in ? TRB_ISP : 0,
        zlp ? 0 : TRB_IOC
    );
    if (zlp) {
        tb_push(&tb, 0, 0, TRB_TYPE(TRB_NORMAL) | TRB_IOC);
    }
    hd[XD_FIRST] = tb.first;
    hd[XD_COUNT] = tb.count;
    give_first(ring, tb.first, tb.first_cycle);
    return TENDRIL_OK;
}

static int submit_isoc(
    struct tendril_hc *hc, struct xhci_ep *xep, struct tendril_transfer *xfer
) {
    struct xhci_ring *ring = &xep->ring;
    uintptr_t *hd = xfer->internal.hcd_data;
    struct tendril_endpoint *ep = xep->ep;
    bool in = (ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
    bool ss = tendril_device_is_ss(ep->dev);
    unsigned int burst = ss ? (unsigned int)ep->max_burst + 1
        : ep->dev->speed == TENDRIL_SPEED_HIGH ? (unsigned int)ep->mult
        : 1;
    struct cursor c;

    cursor_init(&c, xfer);
    size_t need = 0;
    for (size_t i = 0; i < xfer->num_iso_packets; i++) {
        size_t n = count_td(&c, xfer->iso_packets[i].length);
        if (n == 0) {
            return TENDRIL_ERR_INVALID;
        }
        need += n;
    }
    if (need > ring->size - 2) {
        return xfer->internal.bounce == NULL ? TENDRIL_ERR_BOUNCE
            : TENDRIL_ERR_INVALID;
    }
    if (need > ring_space(ring)) {
        return TENDRIL_ERR_BUSY;
    }

    uint32_t frame = 0;
    bool asap = (xfer->flags & TENDRIL_XFER_ISO_ASAP) != 0;
    if (!asap) {
        frame = xfer->start_frame;
    } else {
        xfer->start_frame = xhci_frame_number(hc);
    }

    struct td_builder tb = { ring, 0, 0, 0 };
    cursor_init(&c, xfer);
    for (size_t i = 0; i < xfer->num_iso_packets; i++) {
        size_t plen = xfer->iso_packets[i].length;
        size_t tdpc = plen == 0 ? 1
            : (plen + ep->max_packet - 1) / ep->max_packet;
        unsigned int tbc = (unsigned int)((tdpc + burst - 1) / burst - 1);
        unsigned int residue = (unsigned int)(tdpc % burst);
        unsigned int tlbpc = residue == 0 ? burst - 1 : residue - 1;
        uint32_t first = TRB_TYPE(TRB_ISOCH) | ((uint32_t)(tbc & 3) << 7)
            | ((uint32_t)(tlbpc & 0xf) << 16);
        if (asap) {
            first |= TRB_SIA;
        } else {
            uint32_t f = (frame + (uint32_t)(((uint64_t)i * ep->interval) / 8))
                & 0x7ff;
            first |= f << 20;
        }
        bool last = i + 1 == xfer->num_iso_packets;
        push_td(
            &tb, &c, plen, ep->max_packet, first, in ? TRB_ISP : 0,
            TRB_IOC | (last ? 0 : TRB_BEI)
        );
    }
    hd[XD_FIRST] = tb.first;
    hd[XD_COUNT] = tb.count;
    give_first(ring, tb.first, tb.first_cycle);
    return TENDRIL_OK;
}

static int xhci_submit(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct xhci_ep *xep = xep_of(xfer->endpoint);
    if (xep == NULL) {
        return TENDRIL_ERR_INVALID;
    }
    struct xhci_ring *ring = &xep->ring;
    if (xfer->stream != 0) {
        if (xfer->stream > xep->num_streams) {
            return TENDRIL_ERR_INVALID;
        }
        ring = &xep->streams[xfer->stream];
    }
    uintptr_t *hd = xfer->internal.hcd_data;
    memset(hd, 0, sizeof(xfer->internal.hcd_data));
    hd[XD_RING] = (uintptr_t)ring;

    int st;
    switch (xfer->endpoint->type) {
        case TENDRIL_ENDPOINT_XFER_CONTROL: {
            st = submit_control(hc, xep, xfer);
            break;
        }
        case TENDRIL_ENDPOINT_XFER_ISOC: {
            st = submit_isoc(hc, xep, xfer);
            break;
        }
        default: {
            st = submit_bulk(hc, xep, ring, xfer);
            break;
        }
    }
    if (st != TENDRIL_OK) {
        return st;
    }
    if (!xep->stopping && !xep->recovering) {
        ring_doorbell(hc, xdev(xep->dev)->slot, xep->dci, xfer->stream);
    }
    return TENDRIL_OK;
}

static void xhci_cancel(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct xhci_ep *xep = xep_of(xfer->endpoint);
    if (xep == NULL || hc->dead) {
        tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, 0);
        return;
    }
    xfer->internal.hcd_data[XD_FLAGS] |= XF_CANCEL;
    if (!xep->stopping && !xep->recovering) {
        start_stop(hc, xep);
    }
}

/* Power management */

static void suspend_next(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async, uint8_t dci
);

static void suspend_stop_done(
    struct tendril_hc *hc, struct xhci_cmd *cmd, int code, struct xhci_trb *ev
) {
    (void)ev;
    if (code != CC_SUCCESS && code != CC_CONTEXT_STATE) {
        LOG_DEBUG(
            "xHCI: Stop Endpoint for a suspend failed with code %u", code
        );
    }
    suspend_next(hc, cmd->dev, cmd->async, (uint8_t)(cmd->arg + 1));
}

/*
 * xHCI 1.2, 4.15.1.1: endpoints stop, flagged as suspending, before the link
 * is suspended.
 */
static void suspend_next(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async, uint8_t dci
) {
    struct xhci_dev *xd = xdev(dev);
    for (; xd != NULL && dci < 32; dci++) {
        struct xhci_ep *xep = xd->eps[dci];
        if (xep == NULL || xep->stopping || xep->recovering
         || ep_state(hc, xep) != EP_STATE_RUNNING) {
            continue;
        }
        int st = cmd_submit(
            hc, 0, 0,
            TRB_TYPE(TRB_STOP_ENDPOINT) | TRB_SLOT(xd->slot) | TRB_EP(dci)
                | TRB_SP,
            suspend_stop_done, async, dev, xep, (uint32_t)dci
        );
        if (st != TENDRIL_OK) {
            tendril_async_complete(hc, async, st);
        }
        return;
    }
    tendril_async_complete(hc, async, TENDRIL_OK);
}

static int xhci_device_suspend(
    struct tendril_hc *hc, struct tendril_device *dev,
    struct tendril_async *async
) {
    /* Stopped endpoints start again when their doorbells next ring. */
    suspend_next(hc, dev, async, 1);
    return TENDRIL_OK;
}

static uint64_t event_dequeue(struct xhci *x) {
    return x->evt_dma + x->evt_deq * sizeof(struct xhci_trb);
}

/* xHCI 1.2, 4.23.2: stop, then have the controller save its internal state. */
static int xhci_suspend(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    if (!list_empty(&x->cmds)) {
        return TENDRIL_ERR_BUSY;
    }
    tendril_timer_cancel(hc, &x->port_timer);
    uint32_t cmd = op_read(hc, OP_USBCMD) & ~USBCMD_RS;
    op_write(hc, OP_USBCMD, cmd);
    bool halted = tendril_hc_wait32(
        hc, x->op + OP_USBSTS, USBSTS_HCH, USBSTS_HCH, 20000
    );
    if (!halted) {
        LOG_ERROR("xHCI: controller did not halt for suspend");
        op_write(hc, OP_USBCMD, cmd | USBCMD_RS);
        return TENDRIL_ERR_TIMEOUT;
    }
    x->saved_cmd = cmd;
    x->saved_config = op_read(hc, OP_CONFIG);
    op_write(hc, OP_USBCMD, cmd | USBCMD_CSS);
    x->save_failed = !tendril_hc_wait32(
        hc, x->op + OP_USBSTS, USBSTS_SSS, 0, 1000000
    ) || (op_read(hc, OP_USBSTS) & USBSTS_SRE) != 0;
    if (x->save_failed) {
        LOG_WARN("xHCI: saving the controller state failed");
        op_write(hc, OP_USBSTS, USBSTS_SRE);
    }
    return TENDRIL_OK;
}

static int xhci_resume(struct tendril_hc *hc) {
    struct xhci *x = X(hc);
    if (op_read(hc, OP_USBSTS) == 0xffffffff) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (!tendril_hc_wait32(hc, x->op + OP_USBSTS, USBSTS_CNR, 0, 1000000)) {
        return TENDRIL_ERR_IO;
    }
    /*
     * Registers that kept their values mean the controller kept power, and its
     * state with it.
     */
    bool kept = (op_read(hc, OP_CONFIG) & 0xff) == x->max_slots
        && op_read(hc, OP_DCBAAP) == (uint32_t)x->dcbaa_dma
        && op_read(hc, OP_DCBAAP + 4) == (uint32_t)(x->dcbaa_dma >> 32);
    if (!kept) {
        if (x->save_failed) {
            return TENDRIL_ERR_IO;
        }
        if (hc->config.pci
         && (hc->quirks & TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING) != 0) {
            intel_port_routing(hc);
        }
        op_write(hc, OP_DNCTRL, 0);
        op_write64(hc, OP_DCBAAP, x->dcbaa_dma);
        op_write(hc, OP_CONFIG, x->saved_config);
        rt_write(hc, RT_IR0 + IR_ERSTSZ, 1);
        rt_write64(hc, RT_IR0 + IR_ERSTBA, x->erst_dma);
        rt_write64(hc, RT_IR0 + IR_ERDP, event_dequeue(x));
        rt_write(
            hc, RT_IR0 + IR_IMAN, hc->config.irq ? IMAN_IE | IMAN_IP : IMAN_IP
        );
        rt_write(hc, RT_IR0 + IR_IMOD, imod(hc));
        op_write(hc, OP_USBCMD, x->saved_cmd | USBCMD_CRS);
        if (!tendril_hc_wait32(hc, x->op + OP_USBSTS, USBSTS_RSS, 0, 1000000)
         || (op_read(hc, OP_USBSTS) & USBSTS_SRE) != 0) {
            LOG_WARN("xHCI: restoring the controller state failed");
            return TENDRIL_ERR_IO;
        }
        /*
         * The command ring starts over, empty, at the address CRCR is given.
         */
        memset(x->cmd.trbs, 0, x->cmd.size * sizeof(struct xhci_trb));
        x->cmd.enq = 0;
        x->cmd.deq = 0;
        x->cmd.cycle = 1;
        x->cmd_free = CMD_RING_TRBS - 2;
        trb_write(
            &x->cmd.trbs[x->cmd.size - 1], x->cmd.dma, 0,
            TRB_TYPE(TRB_LINK) | TRB_TC
        );
        op_write64(hc, OP_CRCR, x->cmd.dma | CRCR_RCS);
    }
    op_write(hc, OP_USBCMD, x->saved_cmd | USBCMD_RS);
    if (!tendril_hc_wait32(hc, x->op + OP_USBSTS, USBSTS_HCH, 0, 20000)) {
        LOG_ERROR("xHCI: controller did not restart after suspend");
        return TENDRIL_ERR_IO;
    }
    return TENDRIL_OK;
}

const struct tendril_hcd_ops tendril_xhci_ops = {
    .name = "xHCI",
    .start = xhci_start,
    .stop = xhci_stop,
    .destroy = xhci_destroy,
    .irq = xhci_irq,
    .poll = xhci_poll,
    .frame_number = xhci_frame_number,
    .port_status = xhci_port_status,
    .port_set_feature = xhci_port_set_feature,
    .port_clear_feature = xhci_port_clear_feature,
    .device_init = xhci_device_init,
    .device_address = xhci_device_address,
    .ep0_update = xhci_ep0_update,
    .hub_update = xhci_hub_update,
    .configure = xhci_configure,
    .endpoint_reset = xhci_endpoint_reset,
    .alloc_streams = xhci_alloc_streams,
    .device_free = xhci_device_free,
    .device_suspend = xhci_device_suspend,
    .suspend = xhci_suspend,
    .resume = xhci_resume,
    .submit = xhci_submit,
    .cancel = xhci_cancel
};
