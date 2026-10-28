#include "../internal.h"

/* Enhanced Host Controller Interface, revision 1.0. */

#define CAP_LENGTH 0x00
#define CAP_HCSPARAMS 0x04
#define CAP_HCCPARAMS 0x08

#define OP_USBCMD 0x00
#define OP_USBSTS 0x04
#define OP_USBINTR 0x08
#define OP_FRINDEX 0x0c
#define OP_CTRLDSSEGMENT 0x10
#define OP_PERIODICLISTBASE 0x14
#define OP_ASYNCLISTADDR 0x18
#define OP_CONFIGFLAG 0x40
#define OP_PORTSC(n) (0x44 + 4 * ((n) - 1))

#define CMD_RS (1u << 0)
#define CMD_HCRESET (1u << 1)
#define CMD_PSE (1u << 4)
#define CMD_ASE (1u << 5)
#define CMD_IAAD (1u << 6)
#define CMD_ITC_SHIFT 16

#define STS_USBINT (1u << 0)
#define STS_ERRINT (1u << 1)
#define STS_PCD (1u << 2)
#define STS_FLR (1u << 3)
#define STS_HSE (1u << 4)
#define STS_IAA (1u << 5)
#define STS_HALTED (1u << 12)
#define STS_PSS (1u << 14)
#define STS_ASS (1u << 15)
#define STS_ACK (STS_USBINT | STS_ERRINT | STS_PCD | STS_FLR | STS_HSE | STS_IAA)

#define PORT_CCS (1u << 0)
#define PORT_CSC (1u << 1)
#define PORT_PE (1u << 2)
#define PORT_PEC (1u << 3)
#define PORT_OCA (1u << 4)
#define PORT_OCC (1u << 5)
#define PORT_FPR (1u << 6)
#define PORT_SUSPEND (1u << 7)
#define PORT_PR (1u << 8)
#define PORT_LS_MASK (3u << 10)
#define PORT_LS_K (1u << 10)
#define PORT_PP (1u << 12)
#define PORT_PO (1u << 13)
#define PORT_RWC (PORT_CSC | PORT_PEC | PORT_OCC)

/* Link pointers */
#define LINK_T 1u
#define LINK_ITD (0u << 1)
#define LINK_QH (1u << 1)
#define LINK_SITD (2u << 1)

/* qTD token */
#define TOK_ACTIVE (1u << 7)
#define TOK_HALTED (1u << 6)
#define TOK_DBE (1u << 5)
#define TOK_BABBLE (1u << 4)
#define TOK_XACT (1u << 3)
#define TOK_MMF (1u << 2)
#define TOK_PID_OUT (0u << 8)
#define TOK_PID_IN (1u << 8)
#define TOK_PID_SETUP (2u << 8)
#define TOK_PID_MASK (3u << 8)
#define TOK_CERR(n) ((uint32_t)(n) << 10)
#define TOK_IOC (1u << 15)
#define TOK_BYTES(n) ((uint32_t)(n) << 16)
#define TOK_GET_BYTES(t) (((t) >> 16) & 0x7fff)
#define TOK_DT (1u << 31)

/* QH endpoint characteristics and capabilities */
#define QH_EPS_FS (0u << 12)
#define QH_EPS_LS (1u << 12)
#define QH_EPS_HS (2u << 12)
#define QH_DTC (1u << 14)
#define QH_H (1u << 15)
#define QH_C (1u << 27)
#define QH_RL(n) ((uint32_t)(n) << 28)

/* iTD transaction and siTD state bits */
#define ITD_ACTIVE (1u << 31)
#define ITD_DBE (1u << 30)
#define ITD_BABBLE (1u << 29)
#define ITD_XACT (1u << 28)
#define ITD_IOC (1u << 15)
#define SITD_ACTIVE (1u << 7)
#define SITD_ERR (1u << 6)
#define SITD_DBE (1u << 5)
#define SITD_BABBLE (1u << 4)
#define SITD_XACT (1u << 3)
#define SITD_MMF (1u << 2)
#define SITD_IOC (1u << 31)

#define FRAMES 1024
#define LEVELS 9
#define QTD_MAX_BYTES (5 * 4096)
#define IAA_TIMEOUT_MS 50
#define ISO_LEAD_FRAMES 4
/* USB 2.0 7.1.7.7: TDRSMDN, the resume signalling software times (EHCI 1.0, 4.3.1). */
#define RESUME_SIGNAL_MS 20

struct ehci_qtd {
    uint32_t next;
    uint32_t alt_next;
    uint32_t token;
    uint32_t buf[5];
    uint32_t buf_hi[5];
    /* Holds a control transfer's SETUP data, which must be DMA-addressable. */
    uint8_t setup[8];
    uint32_t pad;
    uint64_t dma;
    struct ehci_qtd *sw_next;
    uint32_t length;
};

struct ehci_qh {
    uint32_t link;
    uint32_t ep_char;
    uint32_t ep_cap;
    uint32_t current;
    uint32_t next;
    uint32_t alt_next;
    uint32_t token;
    uint32_t buf[5];
    uint32_t buf_hi[5];
    uint64_t dma;
    struct tendril_endpoint *ep;
    struct ehci_qtd *dummy;
    struct ehci_qh *sw_next;
    struct ehci_qh *sw_prev;
    struct tendril_list unlink_node;
    unsigned level;
    unsigned unlink_frame;
    struct ehci_close *close;
    bool periodic;
    bool linked;
    bool unlinking;
    bool closing;
    bool iaa_again;
};

/* Waits for endpoints being closed, then completes an asynchronous operation. */
struct ehci_close {
    struct tendril_async *async;
    unsigned remaining;
    enum tendril_status status;
};

/* Isochronous descriptors are kept on per-frame software lists for unlinking. */
struct ehci_iso {
    struct ehci_iso *frame_next;
    uint32_t *hw_next;
    uint32_t link;
    unsigned frame;
    bool linked;
};

struct ehci_itd {
    uint32_t next;
    uint32_t trans[8];
    uint32_t page[7];
    uint32_t page_hi[7];
    uint64_t dma;
    struct ehci_iso iso;
    /* Isochronous packet index of each transaction, or -1. */
    int16_t packet[8];
};

struct ehci_sitd {
    uint32_t next;
    uint32_t endpoint;
    uint32_t schedule;
    uint32_t state;
    uint32_t page[2];
    uint32_t back;
    uint32_t page_hi[2];
    uint64_t dma;
    struct ehci_iso iso;
    unsigned packet;
    uint32_t length;
};

/* Per isochronous transfer: its descriptors, freed once the frames passed. */
struct ehci_iso_xfer {
    struct tendril_transfer *xfer;
    bool high_speed;
    unsigned count;
    void **descs;
    /* The frame of the last descriptor, or of the cancellation, as current_frame() counts. */
    uint32_t last_frame;
    struct tendril_list node;
};

struct ehci_iso_ep {
    /* While transfers are queued: the frame after the last one they take, as current_frame() counts. */
    uint32_t next_frame;
    bool streaming;
};

struct ehci {
    struct tendril_hc *hc;
    uint32_t op;
    unsigned num_ports;
    bool ppc;
    unsigned num_cc;
    bool ac64;
    uint32_t *frame_list;
    uint64_t frame_list_dma;
    struct ehci_iso *frame_iso[FRAMES];
    struct ehci_qh *skel[LEVELS];
    struct ehci_qh *async_head;
    struct ehci_qh *periodic_lists[LEVELS];
    struct tendril_dma_pool qtd_pool;
    struct tendril_dma_pool qh_pool;
    struct tendril_dma_pool itd_pool;
    struct tendril_dma_pool sitd_pool;
    /* QHs waiting on an async advance or a frame to pass. */
    struct tendril_list unlinking;
    bool iaa_pending;
    uint64_t iaa_start;
    struct tendril_list iso_active;
    struct tendril_timer timer;
    uint64_t reset_start[16];
    bool resetting[16];
    bool reset_changed[16];
    bool released[16];
    /* Resume signalling under way, and resumes finished but not yet reported. */
    uint64_t resume_start[16];
    bool resuming[16];
    bool resume_changed[16];
    struct tendril_timer port_timer;
    uint32_t intr;
    unsigned unlink_frame;
    uint32_t saved_cmd;
};

/* hcd_data slots in a transfer */
#define ED_QH 0
#define ED_FIRST 1
#define ED_LAST 2
#define ED_FLAGS 3
#define ED_ACTUAL 4
#define ED_ISO 5

#define EF_CANCEL (1u << 0)

static struct ehci *E(struct tendril_hc *hc) {
    return hc->hcd;
}

static uint32_t op_read(struct tendril_hc *hc, uint32_t reg) {
    return hc_read32(hc, E(hc)->op + reg);
}

static void op_write(struct tendril_hc *hc, uint32_t reg, uint32_t v) {
    hc_write32(hc, E(hc)->op + reg, v);
}

static uint32_t le(uint32_t v) {
    return tendril_le32(v);
}

static uint32_t lo32(uint64_t v) {
    return (uint32_t)v;
}

static uint32_t hi32(uint64_t v) {
    return (uint32_t)(v >> 32);
}

/* EHCI 1.0 2.3.4, Table 2-12: FRINDEX[13:3] is the frame number; its low bits index the frame list. */
static uint32_t current_frame(struct tendril_hc *hc) {
    return tendril_frame_update(hc, (op_read(hc, OP_FRINDEX) >> 3) & 0x7ff, 0x7ff);
}

/* qTDs */

static struct ehci_qtd *qtd_alloc(struct tendril_hc *hc) {
    uint64_t dma;
    struct ehci_qtd *qtd = tendril_dma_pool_alloc(&E(hc)->qtd_pool, &dma);
    if (qtd == NULL) {
        return NULL;
    }
    qtd->dma = dma;
    qtd->next = le(LINK_T);
    qtd->alt_next = le(LINK_T);
    qtd->token = 0;
    return qtd;
}

static void qtd_free(struct tendril_hc *hc, struct ehci_qtd *qtd) {
    tendril_dma_pool_free(&E(hc)->qtd_pool, qtd, qtd->dma);
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
 * Fills a qTD's buffer pointers with up to `want` bytes from the cursor,
 * keeping the size a multiple of `mps` unless it is the remainder. Returns the
 * bytes covered.
 */
static size_t fill_qtd_buffers(struct tendril_hc *hc, struct ehci_qtd *qtd, struct cursor *c, size_t want, uint16_t mps) {
    struct cursor probe = *c;
    size_t total = 0;
    uint64_t pages[5];
    unsigned n = 0;

    while (n < 5 && total < want) {
        uint64_t a = cursor_addr(&probe);
        size_t room = cursor_page_room(&probe);
        if (room == 0) {
            break;
        }
        /* After the first page, data has to start on a page boundary. */
        if (n != 0 && (a & 0xfff) != 0) {
            break;
        }
        pages[n++] = a;
        size_t take = MIN(room, want - total);
        total += take;
        cursor_advance(&probe, take);
        if (take < room) {
            break;
        }
    }
    if (total < want && total >= mps) {
        total -= total % mps;
    }
    for (unsigned i = 0; i < 5; i++) {
        uint64_t a = i < n ? pages[i] : 0;
        if (i != 0) {
            a &= ~0xfffull;
        }
        qtd->buf[i] = le(lo32(a));
        qtd->buf_hi[i] = E(hc)->ac64 ? le(hi32(a)) : 0;
    }
    cursor_advance(c, total);
    return total;
}

/* Queue heads */

static struct ehci_qh *qh_alloc(struct tendril_hc *hc) {
    uint64_t dma;
    struct ehci_qh *qh = tendril_dma_pool_alloc(&E(hc)->qh_pool, &dma);
    if (qh == NULL) {
        return NULL;
    }
    qh->dma = dma;
    qh->link = le(LINK_T);
    qh->next = le(LINK_T);
    qh->alt_next = le(LINK_T);
    list_init(&qh->unlink_node);
    return qh;
}

static void qh_free(struct tendril_hc *hc, struct ehci_qh *qh) {
    if (qh->dummy != NULL) {
        qtd_free(hc, qh->dummy);
    }
    tendril_dma_pool_free(&E(hc)->qh_pool, qh, qh->dma);
}

static uint32_t qh_link(struct ehci_qh *qh) {
    return lo32(qh->dma) | LINK_QH;
}

static void set_ep_fields(struct tendril_hc *hc, struct ehci_qh *qh, struct tendril_endpoint *ep) {
    struct tendril_device *dev = ep->dev;
    uint32_t ch = dev->address | (uint32_t)(ep->address & 0xf) << 8 | (uint32_t)ep->max_packet << 16;
    uint32_t cap = 0;
    (void)hc;

    switch (dev->speed) {
        case TENDRIL_SPEED_LOW: {
            ch |= QH_EPS_LS;
            break;
        }
        case TENDRIL_SPEED_FULL: {
            ch |= QH_EPS_FS;
            break;
        }
        default: {
            ch |= QH_EPS_HS;
            break;
        }
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_CONTROL) {
        ch |= QH_DTC;
        if (dev->speed != TENDRIL_SPEED_HIGH) {
            ch |= QH_C;
        }
    }
    if (ep->type != TENDRIL_ENDPOINT_XFER_INT) {
        /* EHCI 1.0, 4.9: reload the NAK counter on async endpoints. */
        ch |= QH_RL(dev->speed == TENDRIL_SPEED_HIGH ? 4 : 0);
    }
    cap |= (uint32_t)(ep->mult != 0 ? ep->mult : 1) << 30;
    if (dev->speed != TENDRIL_SPEED_HIGH && dev->tt_hub != NULL) {
        cap |= (uint32_t)dev->tt_hub->address << 16 | (uint32_t)dev->tt_port << 23;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_INT) {
        if (dev->speed == TENDRIL_SPEED_HIGH) {
            uint32_t smask;
            switch (ep->interval) {
                case 1: {
                    smask = 0xff;
                    break;
                }
                case 2: {
                    smask = 0x55;
                    break;
                }
                case 4: {
                    smask = 0x11;
                    break;
                }
                default: {
                    smask = 0x01;
                    break;
                }
            }
            cap |= smask;
        } else {
            /* EHCI 1.0, 4.12.2: start-split in microframe 0, complete-splits in 2 to 4. */
            cap |= 0x01 | 0x1c << 8;
        }
    }
    qh->ep_char = le(ch);
    qh->ep_cap = le(cap);
}

static unsigned periodic_level(struct tendril_endpoint *ep) {
    uint32_t frames = ep->interval / 8;
    unsigned level = 0;
    while (frames > 1 && level < LEVELS - 1) {
        frames >>= 1;
        level++;
    }
    return level;
}

static void link_async(struct tendril_hc *hc, struct ehci_qh *qh) {
    struct ehci_qh *head = E(hc)->async_head;
    qh->link = head->link;
    qh->sw_next = head->sw_next;
    qh->sw_prev = head;
    head->sw_next->sw_prev = qh;
    head->sw_next = qh;
    dma_barrier();
    head->link = le(qh_link(qh));
    qh->linked = true;
}

static void link_periodic(struct tendril_hc *hc, struct ehci_qh *qh) {
    struct ehci *e = E(hc);
    struct ehci_qh *skel = e->skel[qh->level];
    qh->link = skel->link;
    qh->sw_next = e->periodic_lists[qh->level];
    qh->sw_prev = NULL;
    if (qh->sw_next != NULL) {
        qh->sw_next->sw_prev = qh;
    }
    e->periodic_lists[qh->level] = qh;
    dma_barrier();
    skel->link = le(qh_link(qh));
    qh->linked = true;
}

static void unlink_qh_hw(struct tendril_hc *hc, struct ehci_qh *qh) {
    struct ehci *e = E(hc);
    if (!qh->periodic) {
        qh->sw_prev->link = qh->link;
        qh->sw_prev->sw_next = qh->sw_next;
        qh->sw_next->sw_prev = qh->sw_prev;
    } else {
        if (qh->sw_prev == NULL) {
            e->skel[qh->level]->link = qh->link;
            e->periodic_lists[qh->level] = qh->sw_next;
        } else {
            qh->sw_prev->link = qh->link;
            qh->sw_prev->sw_next = qh->sw_next;
        }
        if (qh->sw_next != NULL) {
            qh->sw_next->sw_prev = qh->sw_prev;
        }
    }
    dma_barrier();
    qh->linked = false;
}

/* Starts taking a QH off the schedule; it is safe to touch once the controller moved on. */
static void start_unlink(struct tendril_hc *hc, struct ehci_qh *qh) {
    struct ehci *e = E(hc);
    if (qh->unlinking) {
        return;
    }
    if (qh->linked) {
        unlink_qh_hw(hc, qh);
    }
    qh->unlinking = true;
    list_add_tail(&e->unlinking, &qh->unlink_node);
    qh->iaa_again = false;
    if (!qh->periodic && !e->iaa_pending && !hc->dead) {
        e->iaa_pending = true;
        e->iaa_start = tendril_now();
        op_write(hc, OP_USBCMD, op_read(hc, OP_USBCMD) | CMD_IAAD);
    } else if (!qh->periodic) {
        /* This one was unlinked after the pending doorbell; it needs another. */
        qh->iaa_again = true;
    }
    qh->unlink_frame = current_frame(hc);
    tendril_timer_arm(hc, &e->timer, 2 * NS_PER_MS);
}

/* Transfers on queue heads */

static struct ehci_qtd *xfer_first(struct tendril_transfer *xfer) {
    return (struct ehci_qtd *)xfer->internal.hcd_data[ED_FIRST];
}

static struct ehci_qtd *xfer_last(struct tendril_transfer *xfer) {
    return (struct ehci_qtd *)xfer->internal.hcd_data[ED_LAST];
}

static bool is_in(struct tendril_transfer *xfer) {
    if (xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL) {
        return (xfer->setup.request_type & TENDRIL_DIR_IN) != 0;
    }
    return (xfer->endpoint->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
}

/*
 * Builds a transfer's qTDs. The first goes into the queue head's current
 * dummy, which the controller is parked on, and is activated last; a fresh
 * dummy ends the chain.
 */
static enum tendril_status submit_qh(struct tendril_hc *hc, struct ehci_qh *qh, struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    bool control = ep->type == TENDRIL_ENDPOINT_XFER_CONTROL;
    bool in = is_in(xfer);
    size_t len = control ? tendril_le16(xfer->setup.length) : xfer->length;
    uint16_t mps = ep->max_packet;
    struct ehci_qtd *dummy = qtd_alloc(hc);
    struct ehci_qtd *list[64];
    unsigned n = 0;
    struct cursor c;

    if (dummy == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    struct ehci_qtd *first = qh->dummy;
    uint32_t first_token = 0;
    list[n++] = first;

    uint32_t data_pid = in ? TOK_PID_IN : TOK_PID_OUT;
    uint32_t cerr = ep->type == TENDRIL_ENDPOINT_XFER_ISOC ? 0 : TOK_CERR(3);
    struct ehci_qtd *status = NULL;
    size_t done = 0;

    cursor_init(&c, xfer);
    if (control) {
        memcpy(first->setup, &xfer->setup, 8);
        uint64_t a = first->dma + offsetof(struct ehci_qtd, setup);
        first->buf[0] = le(lo32(a));
        first->buf_hi[0] = E(hc)->ac64 ? le(hi32(a)) : 0;
        for (unsigned i = 1; i < 5; i++) {
            first->buf[i] = 0;
            first->buf_hi[i] = 0;
        }
        first->length = 8;
        first_token = TOK_PID_SETUP | cerr | TOK_BYTES(8) | TOK_ACTIVE;
    }

    /* Data stage, or the whole transfer for bulk and interrupt. */
    bool zlp = !in && (xfer->flags & TENDRIL_XFER_ZERO_PACKET) && len != 0 && len % mps == 0;
    bool any = false;
    bool toggle = true;
    while (done < len || (!control && !any) || zlp) {
        struct ehci_qtd *qtd;
        if (!control && !any) {
            qtd = first;
        } else if (n >= ARRAY_SIZE(list) - 2 || (qtd = qtd_alloc(hc)) == NULL) {
            for (unsigned i = 1; i < n; i++) {
                qtd_free(hc, list[i]);
            }
            qtd_free(hc, dummy);
            return n >= ARRAY_SIZE(list) - 2 && xfer->internal.bounce == NULL ? TENDRIL_ERR_BOUNCE
                                                                              : TENDRIL_ERR_NO_MEMORY;
        } else {
            list[n++] = qtd;
        }
        size_t chunk = 0;
        if (done < len) {
            chunk = fill_qtd_buffers(hc, qtd, &c, MIN(len - done, (size_t)QTD_MAX_BYTES), mps);
            if (chunk == 0) {
                for (unsigned i = 1; i < n; i++) {
                    qtd_free(hc, list[i]);
                }
                qtd_free(hc, dummy);
                return TENDRIL_ERR_BOUNCE;
            }
        } else {
            for (unsigned i = 0; i < 5; i++) {
                qtd->buf[i] = 0;
                qtd->buf_hi[i] = 0;
            }
            if (any) {
                zlp = false;
            }
        }
        qtd->length = (uint32_t)chunk;
        uint32_t token = data_pid | cerr | TOK_BYTES(chunk) | TOK_ACTIVE;
        if (control) {
            /* The queue head takes toggles from each qTD on control endpoints. */
            if (toggle) {
                token |= TOK_DT;
            }
            unsigned packets = chunk == 0 ? 1 : (unsigned)((chunk + mps - 1) / mps);
            toggle ^= packets & 1;
        }
        if (qtd == first) {
            first_token = token;
        } else {
            qtd->token = le(token);
        }
        done += chunk;
        any = true;
    }

    if (control) {
        status = qtd_alloc(hc);
        if (status == NULL) {
            for (unsigned i = 1; i < n; i++) {
                qtd_free(hc, list[i]);
            }
            qtd_free(hc, dummy);
            return TENDRIL_ERR_NO_MEMORY;
        }
        list[n++] = status;
        status->length = 0;
        for (unsigned i = 0; i < 5; i++) {
            status->buf[i] = 0;
            status->buf_hi[i] = 0;
        }
        status->token = le(((len == 0 || !in) ? TOK_PID_IN : TOK_PID_OUT) | cerr | TOK_DT | TOK_ACTIVE);
    }

    /* Links: a short read skips to the status stage, or past the transfer. */
    struct ehci_qtd *last = list[n - 1];
    for (unsigned i = 0; i < n; i++) {
        struct ehci_qtd *qtd = list[i];
        struct ehci_qtd *next = i + 1 < n ? list[i + 1] : dummy;
        qtd->sw_next = next;
        qtd->next = le(lo32(next->dma));
        if (in && qtd != status && !(control && i == 0)) {
            qtd->alt_next = le(lo32((control ? status : dummy)->dma));
        } else {
            qtd->alt_next = le(LINK_T);
        }
    }
    dummy->sw_next = NULL;
    dummy->next = le(LINK_T);
    dummy->alt_next = le(LINK_T);
    dummy->token = 0;

    if (last == first) {
        first_token |= TOK_IOC;
    } else {
        last->token = le(le(last->token) | TOK_IOC);
    }
    qh->dummy = dummy;

    uintptr_t *hd = xfer->internal.hcd_data;
    hd[ED_QH] = (uintptr_t)qh;
    hd[ED_FIRST] = (uintptr_t)first;
    hd[ED_LAST] = (uintptr_t)last;
    hd[ED_FLAGS] = 0;
    hd[ED_ACTUAL] = 0;

    dma_barrier();
    first->token = le(first_token);
    return TENDRIL_OK;
}

static enum tendril_status token_status(uint32_t token) {
    if (token & TOK_BABBLE) {
        return TENDRIL_ERR_BABBLE;
    }
    if (token & TOK_DBE) {
        return TENDRIL_ERR_OVERRUN;
    }
    if (token & (TOK_XACT | TOK_MMF)) {
        return TENDRIL_ERR_IO;
    }
    return TENDRIL_ERR_STALL;
}

static void free_xfer_qtds(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct ehci_qtd *qtd = xfer_first(xfer);
    struct ehci_qtd *last = xfer_last(xfer);
    while (qtd != NULL) {
        struct ehci_qtd *next = qtd->sw_next;
        qtd_free(hc, qtd);
        if (qtd == last) {
            break;
        }
        qtd = next;
    }
}

/*
 * Checks the oldest transfer of a queue head. Returns true if it completed,
 * so the caller can look at the next one.
 */
static bool scan_xfer(struct tendril_hc *hc, struct ehci_qh *qh, struct tendril_transfer *xfer) {
    bool control = xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL;
    struct ehci_qtd *last = xfer_last(xfer);
    size_t actual = 0;
    bool short_read = false;

    for (struct ehci_qtd *qtd = xfer_first(xfer); qtd != NULL; qtd = qtd->sw_next) {
        uint32_t token = le(qtd->token);
        if (token & TOK_ACTIVE) {
            if (short_read) {
                break;
            }
            return false;
        }
        dma_barrier();
        bool setup = (token & TOK_PID_MASK) == TOK_PID_SETUP;
        bool status_stage = control && qtd == last;
        size_t moved = qtd->length - MIN((size_t)TOK_GET_BYTES(token), (size_t)qtd->length);
        if (!setup && !status_stage) {
            actual += moved;
        }
        if (token & TOK_HALTED) {
            enum tendril_status st = token_status(token);
            free_xfer_qtds(hc, xfer);
            tendril_transfer_complete(hc, xfer, st, actual);
            /* Restart the halted queue at the next transfer or the dummy. */
            struct ehci_qtd *next = last->sw_next;
            qh->next = le(lo32((next != NULL ? next : qh->dummy)->dma));
            qh->alt_next = le(LINK_T);
            dma_barrier();
            qh->token = le(le(qh->token) & TOK_DT);
            return true;
        }
        if (qtd == last) {
            break;
        }
        if (!setup && moved < qtd->length && (token & TOK_PID_MASK) == TOK_PID_IN) {
            /* Short: the controller took the alternate link past the rest. */
            short_read = true;
            if (control) {
                continue;
            }
            break;
        }
    }
    if (short_read && control && (le(last->token) & TOK_ACTIVE)) {
        return false;
    }
    free_xfer_qtds(hc, xfer);
    tendril_transfer_complete(hc, xfer, TENDRIL_OK, actual);
    return true;
}

static void scan_qh(struct tendril_hc *hc, struct ehci_qh *qh) {
    struct tendril_endpoint *ep = qh->ep;
    while (!list_empty(&ep->transfers)) {
        struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        if (xfer->internal.hcd_data[ED_FLAGS] & EF_CANCEL) {
            break;
        }
        if (!scan_xfer(hc, qh, xfer)) {
            break;
        }
    }
}

/*
 * With the queue head off the schedule: drops cancelled transfers and
 * relinks the survivors' qTDs, then points the overlay at the right qTD.
 */
static void sweep_qh(struct tendril_hc *hc, struct ehci_qh *qh) {
    struct tendril_endpoint *ep = qh->ep;
    struct tendril_list *it;
    struct tendril_list *tmp;
    uint32_t current = le(qh->current);
    bool restart = false;

    /* Transfers that finished before the unlink are completed normally first. */
    scan_qh(hc, qh);

    LIST_FOR_EACH_SAFE(it, tmp, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        if (!(xfer->internal.hcd_data[ED_FLAGS] & EF_CANCEL)) {
            continue;
        }
        for (struct ehci_qtd *qtd = xfer_first(xfer); qtd != NULL; qtd = qtd->sw_next) {
            if (lo32(qtd->dma) == current) {
                restart = true;
            }
            if (qtd == xfer_last(xfer)) {
                break;
            }
        }
        size_t actual = 0;
        for (struct ehci_qtd *qtd = xfer_first(xfer); qtd != NULL; qtd = qtd->sw_next) {
            uint32_t token = le(qtd->token);
            if (!(token & TOK_ACTIVE) && (token & TOK_PID_MASK) != TOK_PID_SETUP) {
                actual += qtd->length - MIN((size_t)TOK_GET_BYTES(token), (size_t)qtd->length);
            }
            if (qtd == xfer_last(xfer)) {
                break;
            }
        }
        free_xfer_qtds(hc, xfer);
        tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, actual);
    }

    /* Rebuild the links between the transfers left, in order, ending at the dummy. */
    struct tendril_transfer *prev = NULL;
    LIST_FOR_EACH(it, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        if (prev != NULL) {
            struct ehci_qtd *plast = xfer_last(prev);
            plast->sw_next = xfer_first(xfer);
            plast->next = le(lo32(xfer_first(xfer)->dma));
        }
        prev = xfer;
    }
    struct tendril_transfer *head = list_empty(&ep->transfers) ? NULL
                                   : CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
    if (prev != NULL) {
        xfer_last(prev)->sw_next = qh->dummy;
        xfer_last(prev)->next = le(lo32(qh->dummy->dma));
    }
    LIST_FOR_EACH(it, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        bool control = xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL;
        struct ehci_qtd *after = xfer_last(xfer)->sw_next;
        for (struct ehci_qtd *qtd = xfer_first(xfer); qtd != NULL && qtd != after; qtd = qtd->sw_next) {
            if (!control && qtd->alt_next != le(LINK_T)) {
                qtd->alt_next = le(lo32(after->dma));
            }
        }
    }
    if (restart || (head == NULL && !(le(qh->token) & TOK_ACTIVE))) {
        struct ehci_qtd *next = head != NULL ? xfer_first(head) : qh->dummy;
        qh->current = 0;
        qh->next = le(lo32(next->dma));
        qh->alt_next = le(LINK_T);
        qh->token = le(le(qh->token) & TOK_DT);
    } else if (head != NULL && le(qh->alt_next) != LINK_T) {
        /* The transfer in flight may name a freed qTD as its alternate. */
        qh->alt_next = le(lo32(xfer_last(head)->sw_next->dma));
    }
    dma_barrier();
}

static void close_done(struct tendril_hc *hc, struct ehci_close *close) {
    if (--close->remaining == 0) {
        tendril_async_complete(hc, close->async, close->status);
        tendril_free(close, sizeof(*close));
    }
}

static void finish_unlink(struct tendril_hc *hc, struct ehci_qh *qh) {
    list_del(&qh->unlink_node);
    qh->unlinking = false;
    if (qh->closing) {
        struct tendril_endpoint *ep = qh->ep;
        while (!list_empty(&ep->transfers)) {
            struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
            free_xfer_qtds(hc, xfer);
            tendril_transfer_complete(hc, xfer, TENDRIL_ERR_NO_DEVICE, 0);
        }
        struct ehci_close *close = qh->close;
        qh_free(hc, qh);
        if (close != NULL) {
            close_done(hc, close);
        }
        return;
    }
    sweep_qh(hc, qh);
    if (qh->periodic) {
        link_periodic(hc, qh);
    } else {
        link_async(hc, qh);
    }
}

static void process_unlinks(struct tendril_hc *hc, bool iaa) {
    struct ehci *e = E(hc);
    struct tendril_list *it;
    struct tendril_list *tmp;
    unsigned frame = current_frame(hc);
    bool again = false;

    if (e->iaa_pending && (iaa || hc->dead || tendril_now() - e->iaa_start > IAA_TIMEOUT_MS * NS_PER_MS)) {
        e->iaa_pending = false;
        iaa = true;
    }
    LIST_FOR_EACH_SAFE(it, tmp, &e->unlinking) {
        struct ehci_qh *qh = CONTAINER_OF(it, struct ehci_qh, unlink_node);
        bool safe;
        if (qh->periodic) {
            safe = hc->dead || ((frame - qh->unlink_frame) & (FRAMES - 1)) >= 2;
        } else if (qh->iaa_again) {
            safe = false;
            again = true;
        } else {
            safe = iaa && !e->iaa_pending;
        }
        if (safe) {
            finish_unlink(hc, qh);
        }
    }
    if (again && !e->iaa_pending && !hc->dead) {
        LIST_FOR_EACH(it, &e->unlinking) {
            CONTAINER_OF(it, struct ehci_qh, unlink_node)->iaa_again = false;
        }
        e->iaa_pending = true;
        e->iaa_start = tendril_now();
        op_write(hc, OP_USBCMD, op_read(hc, OP_USBCMD) | CMD_IAAD);
    }
    if (!list_empty(&e->unlinking)) {
        tendril_timer_arm(hc, &e->timer, 2 * NS_PER_MS);
    }
}

/* Isochronous transfers */

static void iso_link(struct tendril_hc *hc, struct ehci_iso *iso, uint32_t *hw_next, uint32_t link, unsigned frame) {
    struct ehci *e = E(hc);
    iso->frame = frame;
    iso->hw_next = hw_next;
    iso->link = link;
    iso->frame_next = e->frame_iso[frame];
    *hw_next = e->frame_list[frame];
    e->frame_iso[frame] = iso;
    dma_barrier();
    e->frame_list[frame] = le(link);
    iso->linked = true;
}

static void iso_unlink(struct tendril_hc *hc, struct ehci_iso *iso) {
    struct ehci *e = E(hc);
    if (!iso->linked) {
        return;
    }
    struct ehci_iso **pp = &e->frame_iso[iso->frame];
    uint32_t *prev_hw = &e->frame_list[iso->frame];
    while (*pp != NULL && *pp != iso) {
        prev_hw = (*pp)->hw_next;
        pp = &(*pp)->frame_next;
    }
    if (*pp == iso) {
        *pp = iso->frame_next;
        *prev_hw = *iso->hw_next;
        dma_barrier();
    }
    iso->linked = false;
}

static struct ehci_iso_xfer *iso_state(struct tendril_transfer *xfer) {
    return (struct ehci_iso_xfer *)xfer->internal.hcd_data[ED_ISO];
}

static void iso_free_descs(struct tendril_hc *hc, struct ehci_iso_xfer *ix) {
    struct ehci *e = E(hc);
    for (unsigned i = 0; i < ix->count; i++) {
        if (ix->descs[i] == NULL) {
            continue;
        }
        if (ix->high_speed) {
            struct ehci_itd *itd = ix->descs[i];
            iso_unlink(hc, &itd->iso);
            tendril_dma_pool_free(&e->itd_pool, itd, itd->dma);
        } else {
            struct ehci_sitd *sitd = ix->descs[i];
            iso_unlink(hc, &sitd->iso);
            tendril_dma_pool_free(&e->sitd_pool, sitd, sitd->dma);
        }
    }
    tendril_free(ix->descs, ix->count * sizeof(void *));
    tendril_free(ix, sizeof(*ix));
}

static void set_page(uint32_t *lo, uint32_t *hi, bool ac64, uint64_t addr, uint32_t keep) {
    *lo = le((lo32(addr) & ~0xfffu) | keep);
    *hi = ac64 ? le(hi32(addr)) : 0;
}

static enum tendril_status build_itds(struct tendril_hc *hc, struct tendril_transfer *xfer, struct ehci_iso_xfer *ix,
                                     unsigned start) {
    struct ehci *e = E(hc);
    struct tendril_endpoint *ep = xfer->endpoint;
    bool in = (ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
    unsigned step_uf = ep->interval < 8 ? ep->interval : 8;
    unsigned per_frame = 8 / step_uf;
    unsigned frame_step = ep->interval < 8 ? 1 : ep->interval / 8;
    struct cursor c;

    ix->count = (xfer->num_iso_packets + per_frame - 1) / per_frame;
    ix->descs = tendril_zalloc(ix->count * sizeof(void *));
    if (ix->descs == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    cursor_init(&c, xfer);
    uint32_t p = 0;
    for (unsigned i = 0; i < ix->count; i++) {
        uint64_t dma;
        struct ehci_itd *itd = tendril_dma_pool_alloc(&e->itd_pool, &dma);
        if (itd == NULL) {
            return TENDRIL_ERR_NO_MEMORY;
        }
        itd->dma = dma;
        ix->descs[i] = itd;
        for (unsigned t = 0; t < 8; t++) {
            itd->packet[t] = -1;
        }
        uint64_t pages[7];
        unsigned npages = 0;
        for (unsigned k = 0; k < per_frame && p < xfer->num_iso_packets; k++, p++) {
            uint32_t len = xfer->iso_packets[p].length;
            uint64_t a = cursor_addr(&c);
            if (npages == 0 || (pages[npages - 1] & ~0xfffull) != (a & ~0xfffull)) {
                if (npages == 7) {
                    return TENDRIL_ERR_BOUNCE;
                }
                pages[npages++] = a & ~0xfffull;
            }
            unsigned pg = npages - 1;
            uint32_t offset = (uint32_t)(a & 0xfff);
            /* Pages after the packet's first one must follow on contiguously. */
            size_t left = len;
            while (left != 0) {
                size_t room = cursor_page_room(&c);
                if (room == 0) {
                    return TENDRIL_ERR_BOUNCE;
                }
                size_t take = MIN(room, left);
                cursor_advance(&c, take);
                left -= take;
                if (left != 0) {
                    uint64_t na = cursor_addr(&c);
                    if ((na & 0xfff) != 0 || npages == 7) {
                        return TENDRIL_ERR_BOUNCE;
                    }
                    pages[npages++] = na;
                }
            }
            unsigned slot = k * step_uf;
            itd->packet[slot] = (int16_t)p;
            uint32_t tr = ITD_ACTIVE | len << 16 | pg << 12 | offset;
            if (p + 1 == xfer->num_iso_packets) {
                tr |= ITD_IOC;
            }
            itd->trans[slot] = le(tr);
        }
        for (unsigned g = 0; g < 7; g++) {
            uint32_t keep = 0;
            if (g == 0) {
                keep = ep->dev->address | (uint32_t)(ep->address & 0xf) << 8;
            } else if (g == 1) {
                keep = ep->max_packet | (in ? 1u << 11 : 0);
            } else if (g == 2) {
                keep = ep->mult != 0 ? ep->mult : 1;
            }
            set_page(&itd->page[g], &itd->page_hi[g], e->ac64, g < npages ? pages[g] : 0, keep);
        }
        unsigned frame = (start + i * frame_step) & (FRAMES - 1);
        iso_link(hc, &itd->iso, &itd->next, lo32(itd->dma) | LINK_ITD, frame);
    }
    return TENDRIL_OK;
}

static enum tendril_status build_sitds(struct tendril_hc *hc, struct tendril_transfer *xfer, struct ehci_iso_xfer *ix,
                                      unsigned start) {
    struct ehci *e = E(hc);
    struct tendril_endpoint *ep = xfer->endpoint;
    struct tendril_device *dev = ep->dev;
    bool in = (ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
    unsigned frame_step = MAX(ep->interval / 8, 1u);
    struct cursor c;

    if (dev->tt_hub == NULL) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    ix->count = xfer->num_iso_packets;
    ix->descs = tendril_zalloc(ix->count * sizeof(void *));
    if (ix->descs == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    cursor_init(&c, xfer);
    for (unsigned i = 0; i < ix->count; i++) {
        uint32_t len = xfer->iso_packets[i].length;
        uint64_t dma;
        struct ehci_sitd *sitd = tendril_dma_pool_alloc(&e->sitd_pool, &dma);
        if (sitd == NULL) {
            return TENDRIL_ERR_NO_MEMORY;
        }
        sitd->dma = dma;
        sitd->packet = i;
        sitd->length = len;
        ix->descs[i] = sitd;
        uint64_t a0 = cursor_addr(&c);
        size_t room = cursor_page_room(&c);
        uint64_t a1 = a0;
        if (len > room) {
            cursor_advance(&c, room);
            a1 = cursor_addr(&c);
            if ((a1 & 0xfff) != 0 || len - room > cursor_page_room(&c)) {
                return TENDRIL_ERR_BOUNCE;
            }
            cursor_advance(&c, len - room);
        } else {
            cursor_advance(&c, len);
        }
        sitd->endpoint = le((in ? 1u << 31 : 0) | (uint32_t)dev->tt_port << 24 |
                            (uint32_t)dev->tt_hub->address << 16 | (uint32_t)(ep->address & 0xf) << 8 | dev->address);
        uint32_t tp_count = 0;
        if (in) {
            /* EHCI 1.0, 4.12.3: one start-split, complete-splits in microframes 2 to 7. */
            sitd->schedule = le(0x01 | 0xfc << 8);
        } else {
            unsigned splits = (len + 187) / 188;
            if (splits == 0) {
                splits = 1;
            }
            sitd->schedule = le((1u << splits) - 1);
            /* Transaction position: all for one split, else begin. */
            tp_count = (splits == 1 ? 0u : 1u) << 3 | splits;
        }
        sitd->state = le(SITD_ACTIVE | len << 16 | (i + 1 == ix->count ? SITD_IOC : 0));
        set_page(&sitd->page[0], &sitd->page_hi[0], e->ac64, a0, (uint32_t)(a0 & 0xfff));
        set_page(&sitd->page[1], &sitd->page_hi[1], e->ac64, a1, tp_count);
        sitd->back = le(LINK_T);
        unsigned frame = (start + i * frame_step) & (FRAMES - 1);
        iso_link(hc, &sitd->iso, &sitd->next, lo32(sitd->dma) | LINK_SITD, frame);
    }
    return TENDRIL_OK;
}

/* How build_itds() and build_sitds() lay a transfer out: `count` descriptors, `step` frames apart. */
static void iso_layout(struct tendril_transfer *xfer, bool high_speed, uint32_t *count, uint32_t *step) {
    struct tendril_endpoint *ep = xfer->endpoint;
    if (high_speed) {
        unsigned per_frame = 8 / (ep->interval < 8 ? ep->interval : 8);
        *count = (xfer->num_iso_packets + per_frame - 1) / per_frame;
        *step = ep->interval < 8 ? 1 : ep->interval / 8;
    } else {
        *count = xfer->num_iso_packets;
        *step = MAX(ep->interval / 8, 1u);
    }
}

static enum tendril_status submit_iso(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct ehci *e = E(hc);
    struct tendril_endpoint *ep = xfer->endpoint;
    struct ehci_iso_ep *iep = ep->hcd;
    bool high_speed = ep->dev->speed == TENDRIL_SPEED_HIGH;
    uint32_t now = current_frame(hc);
    uint32_t start;
    uint32_t count;
    uint32_t step;

    if (!(xfer->flags & TENDRIL_XFER_ISO_ASAP)) {
        start = xfer->start_frame;
    } else if (iep->streaming && (int32_t)(iep->next_frame - now) >= 2) {
        /* Back to back with what is queued. */
        start = iep->next_frame;
    } else {
        start = now + ISO_LEAD_FRAMES;
    }
    iso_layout(xfer, high_speed, &count, &step);
    /* All of it must fit before the controller comes round the frame list again. */
    if ((int32_t)(start - now) < 2 || start - now + (count - 1) * step + 1 > FRAMES) {
        return TENDRIL_ERR_MISSED;
    }
    struct ehci_iso_xfer *ix = tendril_zalloc(sizeof(*ix));
    if (ix == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    ix->xfer = xfer;
    ix->high_speed = high_speed;
    xfer->internal.hcd_data[ED_ISO] = (uintptr_t)ix;
    xfer->internal.hcd_data[ED_FLAGS] = 0;
    xfer->start_frame = start;
    unsigned index = start & (FRAMES - 1);
    enum tendril_status st = high_speed ? build_itds(hc, xfer, ix, index) : build_sitds(hc, xfer, ix, index);
    if (st != TENDRIL_OK) {
        iso_free_descs(hc, ix);
        return st;
    }
    ix->last_frame = start + (count - 1) * step;
    /* What is queued ends with whichever transfer ends last. */
    if (!iep->streaming || (int32_t)(start + count * step - iep->next_frame) > 0) {
        iep->next_frame = start + count * step;
    }
    iep->streaming = true;
    list_add_tail(&e->iso_active, &ix->node);
    return TENDRIL_OK;
}

/* Fills in packet results; returns whether every descriptor has finished. */
static bool iso_collect(struct tendril_hc *hc, struct ehci_iso_xfer *ix, bool force) {
    struct tendril_transfer *xfer = ix->xfer;
    bool in = (xfer->endpoint->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
    bool all = true;
    (void)hc;

    for (unsigned i = 0; i < ix->count; i++) {
        if (ix->high_speed) {
            struct ehci_itd *itd = ix->descs[i];
            for (unsigned t = 0; t < 8; t++) {
                if (itd->packet[t] < 0) {
                    continue;
                }
                uint32_t tr = le(itd->trans[t]);
                struct tendril_iso_packet *pkt = &xfer->iso_packets[itd->packet[t]];
                if (tr & ITD_ACTIVE) {
                    all = false;
                    pkt->status = TENDRIL_ERR_MISSED;
                    pkt->actual = 0;
                    continue;
                }
                pkt->actual = in ? (tr >> 16) & 0xfff : pkt->length;
                pkt->status = (tr & ITD_BABBLE) ? TENDRIL_ERR_BABBLE
                              : (tr & ITD_DBE) ? TENDRIL_ERR_OVERRUN
                              : (tr & ITD_XACT) ? TENDRIL_ERR_IO : TENDRIL_OK;
            }
        } else {
            struct ehci_sitd *sitd = ix->descs[i];
            uint32_t state = le(sitd->state);
            struct tendril_iso_packet *pkt = &xfer->iso_packets[sitd->packet];
            if (state & SITD_ACTIVE) {
                all = false;
                pkt->status = TENDRIL_ERR_MISSED;
                pkt->actual = 0;
                continue;
            }
            uint32_t left = (state >> 16) & 0x3ff;
            pkt->actual = in ? sitd->length - MIN(left, sitd->length) : sitd->length;
            pkt->status = (state & SITD_BABBLE) ? TENDRIL_ERR_BABBLE
                          : (state & SITD_DBE) ? TENDRIL_ERR_OVERRUN
                          : (state & (SITD_XACT | SITD_ERR)) ? TENDRIL_ERR_IO
                          : (state & SITD_MMF) ? TENDRIL_ERR_MISSED : TENDRIL_OK;
        }
    }
    return all || force;
}

static void scan_iso(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    struct tendril_list *it;
    struct tendril_list *tmp;
    uint32_t now = current_frame(hc);

    LIST_FOR_EACH_SAFE(it, tmp, &e->iso_active) {
        struct ehci_iso_xfer *ix = CONTAINER_OF(it, struct ehci_iso_xfer, node);
        struct tendril_transfer *xfer = ix->xfer;
        /* Past means at least 2 frames behind, the controller being done with it. */
        bool past = (int32_t)(now - ix->last_frame) >= 2;
        bool cancelled = (xfer->internal.hcd_data[ED_FLAGS] & EF_CANCEL) != 0;
        if (cancelled && !past) {
            continue;
        }
        if (!iso_collect(hc, ix, past || cancelled)) {
            continue;
        }
        if (!past) {
            /* The controller may still be in the last frame; finish next time. */
            continue;
        }
        list_del(&ix->node);
        size_t actual = 0;
        for (uint32_t p = 0; p < xfer->num_iso_packets; p++) {
            actual += xfer->iso_packets[p].actual;
        }
        struct tendril_endpoint *ep = xfer->endpoint;
        iso_free_descs(hc, ix);
        tendril_transfer_complete(hc, xfer, cancelled ? xfer->internal.cancel_status : TENDRIL_OK, actual);
        if (list_empty(&ep->transfers)) {
            ((struct ehci_iso_ep *)ep->hcd)->streaming = false;
        }
    }
}

static void cancel_iso(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct ehci_iso_xfer *ix = iso_state(xfer);
    xfer->internal.hcd_data[ED_FLAGS] |= EF_CANCEL;
    for (unsigned i = 0; i < ix->count; i++) {
        if (ix->descs[i] == NULL) {
            continue;
        }
        if (ix->high_speed) {
            struct ehci_itd *itd = ix->descs[i];
            iso_unlink(hc, &itd->iso);
        } else {
            struct ehci_sitd *sitd = ix->descs[i];
            iso_unlink(hc, &sitd->iso);
        }
    }
    ix->last_frame = current_frame(hc);
    tendril_timer_arm(hc, &E(hc)->timer, 3 * NS_PER_MS);
}

/* Transfers */

static enum tendril_status ehci_submit(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    if (ep->hcd == NULL) {
        return TENDRIL_ERR_INVALID;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        return submit_iso(hc, xfer);
    }
    return submit_qh(hc, ep->hcd, xfer);
}

static void ehci_cancel(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    if (hc->dead || ep->hcd == NULL) {
        if (ep->type != TENDRIL_ENDPOINT_XFER_ISOC && ep->hcd != NULL) {
            free_xfer_qtds(hc, xfer);
        }
        tendril_transfer_complete(hc, xfer, xfer->internal.cancel_status, 0);
        return;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        cancel_iso(hc, xfer);
        return;
    }
    xfer->internal.hcd_data[ED_FLAGS] |= EF_CANCEL;
    start_unlink(hc, ep->hcd);
}

/* Endpoints */

static enum tendril_status open_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep) {
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        if (ep->dev->speed != TENDRIL_SPEED_HIGH && ep->dev->tt_hub == NULL) {
            return TENDRIL_ERR_UNSUPPORTED;
        }
        struct ehci_iso_ep *iep = tendril_zalloc(sizeof(*iep));
        if (iep == NULL) {
            return TENDRIL_ERR_NO_MEMORY;
        }
        ep->hcd = iep;
        ep->open = true;
        return TENDRIL_OK;
    }
    struct ehci_qh *qh = qh_alloc(hc);
    if (qh == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    qh->dummy = qtd_alloc(hc);
    if (qh->dummy == NULL) {
        qh_free(hc, qh);
        return TENDRIL_ERR_NO_MEMORY;
    }
    qh->ep = ep;
    set_ep_fields(hc, qh, ep);
    qh->next = le(lo32(qh->dummy->dma));
    qh->token = 0;
    ep->hcd = qh;
    ep->open = true;
    if (ep->type == TENDRIL_ENDPOINT_XFER_INT) {
        qh->periodic = true;
        qh->level = periodic_level(ep);
        link_periodic(hc, qh);
    } else {
        link_async(hc, qh);
    }
    return TENDRIL_OK;
}

/* Closes an endpoint; returns whether the caller must wait on `close`. */
static bool close_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep, struct ehci_close *close) {
    void *hcd = ep->hcd;
    ep->open = false;
    ep->hcd = NULL;
    if (hcd == NULL) {
        return false;
    }
    if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        tendril_free(hcd, sizeof(struct ehci_iso_ep));
        return false;
    }
    struct ehci_qh *qh = hcd;
    qh->closing = true;
    qh->close = close;
    if (hc->dead) {
        if (qh->linked) {
            unlink_qh_hw(hc, qh);
        }
        qh->unlinking = true;
        list_add_tail(&E(hc)->unlinking, &qh->unlink_node);
        finish_unlink(hc, qh);
        return false;
    }
    if (close != NULL) {
        close->remaining++;
    }
    start_unlink(hc, qh);
    return true;
}

static enum tendril_status ehci_configure(struct tendril_hc *hc, struct tendril_device *dev,
                                         struct tendril_endpoint **drop, unsigned num_drop,
                                         struct tendril_endpoint **add, unsigned num_add,
                                         struct tendril_async *async) {
    (void)dev;
    struct ehci_close *close = tendril_zalloc(sizeof(*close));
    if (close == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    close->async = async;
    close->status = TENDRIL_OK;
    /* Held until everything is queued, so it cannot complete early. */
    close->remaining = 1;
    for (unsigned i = 0; i < num_drop; i++) {
        close_endpoint(hc, drop[i], close);
    }
    for (unsigned i = 0; i < num_add; i++) {
        enum tendril_status st = open_endpoint(hc, add[i]);
        if (st != TENDRIL_OK) {
            for (unsigned j = 0; j < i; j++) {
                close_endpoint(hc, add[j], close);
            }
            close->status = st;
            break;
        }
    }
    close_done(hc, close);
    return TENDRIL_OK;
}

static enum tendril_status ehci_device_init(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    enum tendril_status st = open_endpoint(hc, &dev->ep0);
    if (st == TENDRIL_OK) {
        tendril_async_complete(hc, async, TENDRIL_OK);
    }
    return st;
}

static enum tendril_status ehci_ep0_update(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    struct ehci_qh *qh = dev->ep0.hcd;
    /* The queue head is idle here, so its characteristics can change in place. */
    set_ep_fields(hc, qh, &dev->ep0);
    dma_barrier();
    tendril_async_complete(hc, async, TENDRIL_OK);
    return TENDRIL_OK;
}

static enum tendril_status ehci_endpoint_reset(struct tendril_hc *hc, struct tendril_endpoint *ep,
                                              struct tendril_async *async) {
    struct ehci_qh *qh = ep->hcd;
    if (qh != NULL && ep->type != TENDRIL_ENDPOINT_XFER_ISOC && list_empty(&ep->transfers)) {
        qh->token = le(le(qh->token) & ~TOK_DT);
        dma_barrier();
    }
    tendril_async_complete(hc, async, TENDRIL_OK);
    return TENDRIL_OK;
}

static void ehci_device_free(struct tendril_hc *hc, struct tendril_device *dev, struct tendril_async *async) {
    struct ehci_close *close = tendril_zalloc(sizeof(*close));
    if (close != NULL) {
        close->async = async;
        close->status = TENDRIL_OK;
        close->remaining = 1;
    }
    /* The device is going away with its interfaces' endpoints still open. */
    close_endpoint(hc, &dev->ep0, close);
    for (unsigned i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (unsigned e = 0; e < intf->num_endpoints; e++) {
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

/*
 * Root ports behave as a hub's would (USB 2.0 11.24.2.7.1.3 and 11.24.2.7.2.3):
 * software times the resume signalling of EHCI 1.0, 4.3.1, and the port reports
 * itself suspended until that is over.
 */
static uint32_t advance_resume(struct tendril_hc *hc, unsigned port, uint32_t sc) {
    struct ehci *e = E(hc);
    unsigned i = port - 1;

    if (!e->resuming[i] && (sc & PORT_SUSPEND) && (sc & PORT_FPR) && !(sc & PORT_PO)) {
        /* The device signalled a wakeup. */
        e->resuming[i] = true;
        e->resume_start[i] = tendril_now();
        tendril_timer_arm(hc, &e->port_timer, RESUME_SIGNAL_MS * NS_PER_MS);
    }
    if (e->resuming[i] && tendril_now() - e->resume_start[i] >= RESUME_SIGNAL_MS * NS_PER_MS) {
        /* The controller then clears Suspend, and has the port back within 2 ms. */
        op_write(hc, OP_PORTSC(port), sc & ~(PORT_RWC | PORT_FPR));
        tendril_hc_wait32(hc, e->op + OP_PORTSC(port), PORT_FPR, 0, 2000);
        e->resuming[i] = false;
        e->resume_changed[i] = true;
        sc = op_read(hc, OP_PORTSC(port));
    }
    return sc;
}

static void port_timer_work(struct tendril_work *work) {
    struct ehci *e = CONTAINER_OF(work, struct ehci, port_timer.work);
    struct tendril_hc *hc = e->hc;
    hc_lock(hc);
    bool again = false;
    for (unsigned p = 1; !hc->dead && p <= e->num_ports; p++) {
        if (!e->resuming[p - 1]) {
            continue;
        }
        advance_resume(hc, p, op_read(hc, OP_PORTSC(p)));
        if (e->resuming[p - 1]) {
            again = true;
        } else {
            tendril_roothub_changed(hc, p);
        }
    }
    if (again) {
        tendril_timer_arm(hc, &e->port_timer, 2 * NS_PER_MS);
    }
    hc_unlock(hc);
}

static void ehci_port_status(struct tendril_hc *hc, unsigned port, struct tendril_port_status *st) {
    struct ehci *e = E(hc);
    uint32_t sc = advance_resume(hc, port, op_read(hc, OP_PORTSC(port)));
    unsigned i = port - 1;

    if (e->resetting[i] && tendril_now() - e->reset_start[i] >= 50 * NS_PER_MS) {
        /* USB 2.0 7.1.7.5: root port resets last 50 ms, timed by software. */
        op_write(hc, OP_PORTSC(port), (sc & ~(PORT_RWC | PORT_PR)));
        tendril_hc_wait32(hc, e->op + OP_PORTSC(port), PORT_PR, 0, 2000);
        e->resetting[i] = false;
        e->reset_changed[i] = true;
        sc = op_read(hc, OP_PORTSC(port));
        if ((sc & PORT_CCS) && !(sc & PORT_PE) && e->num_cc != 0) {
            /* Not high-speed: hand it to a companion controller. */
            op_write(hc, OP_PORTSC(port), (sc & ~PORT_RWC) | PORT_PO);
            e->released[i] = true;
            sc = op_read(hc, OP_PORTSC(port));
        }
    }

    st->status = 0;
    st->change = 0;
    st->speed = TENDRIL_SPEED_HIGH;
    if (sc & PORT_PO) {
        /* Owned by a companion: absent here. */
        if (e->reset_changed[i]) {
            st->change |= TENDRIL_PORT_STAT_C_RESET;
        }
        if (sc & PORT_CSC) {
            st->change |= TENDRIL_PORT_STAT_C_CONNECTION;
        }
        return;
    }
    e->released[i] = false;
    if (sc & PORT_CCS) {
        st->status |= TENDRIL_PORT_STAT_CONNECTION;
    }
    if (sc & PORT_PE) {
        st->status |= TENDRIL_PORT_STAT_ENABLE | TENDRIL_PORT_STAT_HIGH_SPEED;
    }
    if (sc & PORT_SUSPEND) {
        st->status |= TENDRIL_PORT_STAT_SUSPEND;
    }
    if (sc & PORT_OCA) {
        st->status |= TENDRIL_PORT_STAT_OVER_CURRENT;
    }
    if ((sc & PORT_PR) || e->resetting[i]) {
        st->status |= TENDRIL_PORT_STAT_RESET;
    }
    if ((sc & PORT_PP) || !e->ppc) {
        st->status |= TENDRIL_PORT_STAT_POWER;
    }
    if (sc & PORT_CSC) {
        st->change |= TENDRIL_PORT_STAT_C_CONNECTION;
    }
    if (sc & PORT_PEC) {
        st->change |= TENDRIL_PORT_STAT_C_ENABLE;
    }
    if (sc & PORT_OCC) {
        st->change |= TENDRIL_PORT_STAT_C_OVER_CURRENT;
    }
    if (e->reset_changed[i]) {
        st->change |= TENDRIL_PORT_STAT_C_RESET;
    }
    if (e->resume_changed[i]) {
        st->change |= TENDRIL_PORT_STAT_C_SUSPEND;
    }
}

static enum tendril_status ehci_port_set_feature(struct tendril_hc *hc, unsigned port, unsigned feature) {
    struct ehci *e = E(hc);
    uint32_t sc = op_read(hc, OP_PORTSC(port)) & ~PORT_RWC;
    unsigned i = port - 1;

    switch (feature) {
        case TENDRIL_PORT_FEAT_RESET: {
            if ((sc & PORT_LS_MASK) == PORT_LS_K && !(sc & PORT_PE) && e->num_cc != 0) {
                /* EHCI 1.0, 4.2.2: a low-speed device shows a K state; it belongs to a companion. */
                op_write(hc, OP_PORTSC(port), sc | PORT_PO);
                e->released[i] = true;
                e->reset_changed[i] = true;
                return TENDRIL_OK;
            }
            op_write(hc, OP_PORTSC(port), (sc & ~PORT_PE) | PORT_PR);
            e->resetting[i] = true;
            e->reset_start[i] = tendril_now();
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_POWER: {
            op_write(hc, OP_PORTSC(port), sc | PORT_PP);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            op_write(hc, OP_PORTSC(port), sc | PORT_SUSPEND);
            return TENDRIL_OK;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
}

static enum tendril_status ehci_port_clear_feature(struct tendril_hc *hc, unsigned port, unsigned feature) {
    struct ehci *e = E(hc);
    uint32_t sc = op_read(hc, OP_PORTSC(port)) & ~PORT_RWC;

    switch (feature) {
        case TENDRIL_PORT_FEAT_ENABLE: {
            if (!(hc->quirks & TENDRIL_HC_QUIRK_NO_PORT_DISABLE)) {
                op_write(hc, OP_PORTSC(port), sc & ~PORT_PE);
            }
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_POWER: {
            op_write(hc, OP_PORTSC(port), sc & ~PORT_PP);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_SUSPEND: {
            unsigned i = port - 1;
            if ((sc & PORT_SUSPEND) && !e->resuming[i]) {
                op_write(hc, OP_PORTSC(port), sc | PORT_FPR);
                e->resuming[i] = true;
                e->resume_start[i] = tendril_now();
                tendril_timer_arm(hc, &e->port_timer, RESUME_SIGNAL_MS * NS_PER_MS);
            }
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_CONNECTION: {
            op_write(hc, OP_PORTSC(port), sc | PORT_CSC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_ENABLE: {
            op_write(hc, OP_PORTSC(port), sc | PORT_PEC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_OVER_CURRENT: {
            op_write(hc, OP_PORTSC(port), sc | PORT_OCC);
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_RESET: {
            e->reset_changed[port - 1] = false;
            return TENDRIL_OK;
        }
        case TENDRIL_PORT_FEAT_C_SUSPEND: {
            e->resume_changed[port - 1] = false;
            return TENDRIL_OK;
        }
        default: {
            return TENDRIL_ERR_UNSUPPORTED;
        }
    }
}

/* Controller */

static void bios_handoff(struct tendril_hc *hc, unsigned eecp) {
    unsigned guard = 0;
    while (eecp >= 0x40 && guard++ < 32) {
        uint32_t cap = tendril_glue_pci_read32(hc->ctx, (uint16_t)eecp);
        if ((cap & 0xff) == 1) {
            if (cap & (1u << 16)) {
                LOG_DEBUG("tendril: EHCI: taking over from firmware");
                tendril_glue_pci_write8(hc->ctx, (uint16_t)(eecp + 3), 1);
                uint64_t end = tendril_now() + 1000 * NS_PER_MS;
                while ((tendril_glue_pci_read32(hc->ctx, (uint16_t)eecp) & (1u << 16)) && tendril_now() < end) {
                }
                if (tendril_glue_pci_read32(hc->ctx, (uint16_t)eecp) & (1u << 16)) {
                    LOG_WARN("tendril: EHCI: firmware did not release the controller, forcing it");
                    tendril_glue_pci_write8(hc->ctx, (uint16_t)(eecp + 2), 0);
                }
            }
            /* Disable every SMI source and clear their status. */
            tendril_glue_pci_write32(hc->ctx, (uint16_t)(eecp + 4), 0xe0000000);
        }
        eecp = (cap >> 8) & 0xff;
    }
}

static void fail_everything(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    struct tendril_list *it;

    LIST_FOR_EACH(it, &hc->devices) {
        struct tendril_device *dev = CONTAINER_OF(it, struct tendril_device, node);
        struct tendril_endpoint *eps[32];
        unsigned n = 0;
        eps[n++] = &dev->ep0;
        for (unsigned i = 0; i < dev->num_interfaces; i++) {
            for (unsigned j = 0; j < dev->interfaces[i].num_endpoints && n < 32; j++) {
                eps[n++] = &dev->interfaces[i].endpoints[j];
            }
        }
        for (unsigned k = 0; k < n; k++) {
            struct tendril_endpoint *ep = eps[k];
            while (!list_empty(&ep->transfers)) {
                struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
                if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC) {
                    struct ehci_iso_xfer *ix = iso_state(xfer);
                    list_del(&ix->node);
                    iso_free_descs(hc, ix);
                } else if (ep->hcd != NULL) {
                    free_xfer_qtds(hc, xfer);
                }
                tendril_transfer_complete(hc, xfer, TENDRIL_ERR_NO_DEVICE, 0);
            }
        }
    }
    struct tendril_list *tmp;
    LIST_FOR_EACH_SAFE(it, tmp, &e->unlinking) {
        finish_unlink(hc, CONTAINER_OF(it, struct ehci_qh, unlink_node));
    }
}

static void controller_died(struct tendril_hc *hc, const char *why) {
    if (hc->dead) {
        return;
    }
    LOG_ERROR("tendril: EHCI: controller failed: %s", why);
    hc->dead = true;
    op_write(hc, OP_USBCMD, op_read(hc, OP_USBCMD) & ~CMD_RS);
    fail_everything(hc);
}

static void ehci_timer(struct tendril_work *work) {
    struct ehci *e = CONTAINER_OF(work, struct ehci, timer.work);
    struct tendril_hc *hc = e->hc;
    hc_lock(hc);
    if (!hc->dead) {
        process_unlinks(hc, false);
        scan_iso(hc);
        if (!list_empty(&e->iso_active)) {
            tendril_timer_arm(hc, &e->timer, 2 * NS_PER_MS);
        }
    }
    hc_unlock(hc);
}

static void scan_all(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    for (struct ehci_qh *qh = e->async_head->sw_next; qh != e->async_head; qh = qh->sw_next) {
        scan_qh(hc, qh);
    }
    for (unsigned l = 0; l < LEVELS; l++) {
        for (struct ehci_qh *qh = e->periodic_lists[l]; qh != NULL; qh = qh->sw_next) {
            scan_qh(hc, qh);
        }
    }
    scan_iso(hc);
}

static void ehci_poll(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    uint32_t sts = op_read(hc, OP_USBSTS);
    if (sts == 0xffffffff) {
        controller_died(hc, "controller removed");
        return;
    }
    uint32_t ack = sts & STS_ACK;
    if (ack != 0) {
        op_write(hc, OP_USBSTS, ack);
    }
    if (sts & STS_HSE) {
        controller_died(hc, "host system error");
        return;
    }
    scan_all(hc);
    if ((ack & STS_IAA) || !list_empty(&e->unlinking)) {
        process_unlinks(hc, (ack & STS_IAA) != 0);
    }
    if (ack & STS_PCD) {
        tendril_roothub_changed(hc, 0);
    }
    for (unsigned i = 0; i < e->num_ports; i++) {
        if (e->resetting[i] && tendril_now() - e->reset_start[i] >= 50 * NS_PER_MS) {
            tendril_roothub_changed(hc, i + 1);
        }
    }
    if (hc->config.irq) {
        op_write(hc, OP_USBINTR, e->intr);
    }
}

static bool ehci_irq(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    uint32_t sts = op_read(hc, OP_USBSTS);
    if (sts == 0xffffffff || !(sts & e->intr & STS_ACK)) {
        return false;
    }
    /* Masked until tendril_hc_poll() has handled the cause. */
    op_write(hc, OP_USBINTR, 0);
    return true;
}

static uint32_t ehci_frame_number(struct tendril_hc *hc) {
    return current_frame(hc);
}

static enum tendril_status halt_reset(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    op_write(hc, OP_USBCMD, op_read(hc, OP_USBCMD) & ~CMD_RS);
    if (!tendril_hc_wait32(hc, e->op + OP_USBSTS, STS_HALTED, STS_HALTED, 20000)) {
        LOG_WARN("tendril: EHCI: controller did not halt");
    }
    op_write(hc, OP_USBCMD, CMD_HCRESET);
    if (!tendril_hc_wait32(hc, e->op + OP_USBCMD, CMD_HCRESET, 0, 1000000)) {
        return TENDRIL_ERR_TIMEOUT;
    }
    return TENDRIL_OK;
}

static void free_structures(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    for (unsigned l = 0; l < LEVELS; l++) {
        if (e->skel[l] != NULL) {
            tendril_dma_pool_free(&e->qh_pool, e->skel[l], e->skel[l]->dma);
        }
    }
    if (e->async_head != NULL) {
        tendril_dma_pool_free(&e->qh_pool, e->async_head, e->async_head->dma);
    }
    tendril_dma_free(hc, e->frame_list, FRAMES * 4);
    tendril_dma_pool_destroy(&e->qtd_pool);
    tendril_dma_pool_destroy(&e->qh_pool);
    tendril_dma_pool_destroy(&e->itd_pool);
    tendril_dma_pool_destroy(&e->sitd_pool);
    tendril_free(e, sizeof(*e));
    hc->hcd = NULL;
}

static enum tendril_status ehci_start(struct tendril_hc *hc) {
    struct ehci *e = tendril_zalloc(sizeof(*e));
    if (e == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    hc->hcd = e;
    e->hc = hc;
    list_init(&e->unlinking);
    list_init(&e->iso_active);
    tendril_timer_init(&e->timer, ehci_timer);
    tendril_timer_init(&e->port_timer, port_timer_work);

    uint32_t cap = hc_read32(hc, CAP_LENGTH);
    if (cap == 0xffffffff) {
        free_structures(hc);
        return TENDRIL_ERR_NO_DEVICE;
    }
    e->op = cap & 0xff;
    uint32_t hcs = hc_read32(hc, CAP_HCSPARAMS);
    uint32_t hcc = hc_read32(hc, CAP_HCCPARAMS);
    e->num_ports = MIN(hcs & 0xf, 15u);
    e->ppc = (hcs & (1u << 4)) != 0;
    e->num_cc = (hcs >> 12) & 0xf;
    e->ac64 = (hcc & 1) && !(hc->quirks & TENDRIL_HC_QUIRK_NO_64BIT);
    /* Structures share one 4 GiB segment, the low one; only buffers may go above. */
    hc->dma32 = true;
    hc->data32 = !e->ac64;
    LOG_INFO("tendril: EHCI %x.%02x: %u ports, %u companions%s", cap >> 24, (cap >> 16) & 0xff, e->num_ports,
             e->num_cc, e->ac64 ? ", 64-bit" : "");

    if (hc->config.pci && !(hc->quirks & TENDRIL_HC_QUIRK_NO_HANDOFF)) {
        bios_handoff(hc, (hcc >> 8) & 0xff);
    }
    enum tendril_status st = halt_reset(hc);
    if (st != TENDRIL_OK) {
        LOG_ERROR("tendril: EHCI: reset failed");
        free_structures(hc);
        return st;
    }

    tendril_dma_pool_init(&e->qtd_pool, hc, sizeof(struct ehci_qtd), 32);
    tendril_dma_pool_init(&e->qh_pool, hc, sizeof(struct ehci_qh), 32);
    tendril_dma_pool_init(&e->itd_pool, hc, sizeof(struct ehci_itd), 32);
    tendril_dma_pool_init(&e->sitd_pool, hc, sizeof(struct ehci_sitd), 32);

    uint64_t dma;
    e->frame_list = tendril_dma_alloc(hc, FRAMES * 4, 4096, &e->frame_list_dma);
    e->async_head = tendril_dma_pool_alloc(&e->qh_pool, &dma);
    if (e->frame_list == NULL || e->async_head == NULL) {
        free_structures(hc);
        return TENDRIL_ERR_NO_MEMORY;
    }
    e->async_head->dma = dma;
    e->async_head->link = le(lo32(dma) | LINK_QH);
    e->async_head->ep_char = le(QH_H | QH_EPS_HS);
    e->async_head->next = le(LINK_T);
    e->async_head->alt_next = le(LINK_T);
    e->async_head->token = le(TOK_HALTED);
    e->async_head->sw_next = e->async_head;
    e->async_head->sw_prev = e->async_head;
    e->async_head->linked = true;

    /* Skeleton level k is visited every 2^k frames, in front of level k - 1. */
    for (unsigned l = 0; l < LEVELS; l++) {
        struct ehci_qh *s = tendril_dma_pool_alloc(&e->qh_pool, &dma);
        if (s == NULL) {
            free_structures(hc);
            return TENDRIL_ERR_NO_MEMORY;
        }
        s->dma = dma;
        s->ep_char = le(QH_EPS_HS);
        s->next = le(LINK_T);
        s->alt_next = le(LINK_T);
        s->token = le(TOK_HALTED);
        s->link = l == 0 ? le(LINK_T) : le(qh_link(e->skel[l - 1]));
        e->skel[l] = s;
    }
    for (unsigned f = 0; f < FRAMES; f++) {
        unsigned level = f == 0 ? LEVELS - 1 : MIN((unsigned)__builtin_ctz(f), LEVELS - 1u);
        e->frame_list[f] = le(qh_link(e->skel[level]));
    }

    if (e->ac64) {
        op_write(hc, OP_CTRLDSSEGMENT, 0);
    }
    op_write(hc, OP_PERIODICLISTBASE, lo32(e->frame_list_dma));
    op_write(hc, OP_ASYNCLISTADDR, lo32(e->async_head->dma));
    e->intr = STS_USBINT | STS_ERRINT | STS_PCD | STS_HSE | STS_IAA;
    op_write(hc, OP_USBINTR, hc->config.irq ? e->intr : 0);
    op_write(hc, OP_USBCMD, 1u << CMD_ITC_SHIFT | CMD_PSE | CMD_ASE | CMD_RS);
    if (!tendril_hc_wait32(hc, e->op + OP_USBSTS, STS_HALTED, 0, 20000)) {
        LOG_ERROR("tendril: EHCI: controller did not start");
        free_structures(hc);
        return TENDRIL_ERR_TIMEOUT;
    }
    op_write(hc, OP_CONFIGFLAG, 1);
    (void)op_read(hc, OP_CONFIGFLAG);

    st = tendril_roothub_init(hc, e->num_ports);
    if (st != TENDRIL_OK) {
        halt_reset(hc);
        free_structures(hc);
        return st;
    }
    if (e->ppc) {
        for (unsigned p = 1; p <= e->num_ports; p++) {
            uint32_t sc = op_read(hc, OP_PORTSC(p)) & ~PORT_RWC;
            op_write(hc, OP_PORTSC(p), sc | PORT_PP);
        }
    }
    /* USB 2.0 7.1.7.3: power settling before connects are trusted. */
    tendril_delay_us(20000);
    return TENDRIL_OK;
}

static void ehci_stop(struct tendril_hc *hc) {
    if (hc->hcd == NULL) {
        return;
    }
    hc->dead = true;
    tendril_timer_cancel(hc, &E(hc)->timer);
    tendril_timer_cancel(hc, &E(hc)->port_timer);
    op_write(hc, OP_USBINTR, 0);
    halt_reset(hc);
    /* Give the ports back to the companions. */
    op_write(hc, OP_CONFIGFLAG, 0);
    fail_everything(hc);
}

static void ehci_destroy(struct tendril_hc *hc) {
    if (hc->hcd != NULL) {
        free_structures(hc);
    }
}

/* EHCI 1.0, 4.3: with every port suspended, the controller stops. */
static enum tendril_status ehci_suspend(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    tendril_timer_cancel(hc, &e->port_timer);
    e->saved_cmd = op_read(hc, OP_USBCMD);
    op_write(hc, OP_USBCMD, e->saved_cmd & ~CMD_RS);
    if (!tendril_hc_wait32(hc, e->op + OP_USBSTS, STS_HALTED, STS_HALTED, 20000)) {
        op_write(hc, OP_USBCMD, e->saved_cmd);
        return TENDRIL_ERR_TIMEOUT;
    }
    return TENDRIL_OK;
}

static enum tendril_status ehci_resume(struct tendril_hc *hc) {
    struct ehci *e = E(hc);
    if (op_read(hc, OP_USBSTS) == 0xffffffff) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    /* Schedule addresses that no longer read back mean the controller lost power. */
    if (op_read(hc, OP_PERIODICLISTBASE) != lo32(e->frame_list_dma) ||
        op_read(hc, OP_ASYNCLISTADDR) != lo32(e->async_head->dma) || op_read(hc, OP_CONFIGFLAG) == 0) {
        return TENDRIL_ERR_IO;
    }
    op_write(hc, OP_USBINTR, hc->config.irq ? e->intr : 0);
    op_write(hc, OP_USBCMD, e->saved_cmd | CMD_RS);
    if (!tendril_hc_wait32(hc, e->op + OP_USBSTS, STS_HALTED, 0, 20000)) {
        return TENDRIL_ERR_IO;
    }
    return TENDRIL_OK;
}

const struct tendril_hcd_ops tendril_ehci_ops = {
    .name = "EHCI",
    .start = ehci_start,
    .stop = ehci_stop,
    .destroy = ehci_destroy,
    .irq = ehci_irq,
    .poll = ehci_poll,
    .frame_number = ehci_frame_number,
    .port_status = ehci_port_status,
    .port_set_feature = ehci_port_set_feature,
    .port_clear_feature = ehci_port_clear_feature,
    .device_init = ehci_device_init,
    .ep0_update = ehci_ep0_update,
    .configure = ehci_configure,
    .endpoint_reset = ehci_endpoint_reset,
    .device_free = ehci_device_free,
    .suspend = ehci_suspend,
    .resume = ehci_resume,
    .submit = ehci_submit,
    .cancel = ehci_cancel
};
