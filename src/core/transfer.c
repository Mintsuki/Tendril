#include "../internal.h"

enum xfer_state {
    XFER_IDLE,
    XFER_PENDING,
    XFER_CANCELLING,
    XFER_COMPLETING,
    XFER_DONE
};

static void xfer_done_work(struct tendril_work *work);
static void xfer_timeout_work(struct tendril_work *work);

static void reset_internal(struct tendril_transfer *xfer) {
    struct tendril_transfer_internal *in = &xfer->internal;
    list_init(&in->node);
    tendril_work_init(&in->done, xfer_done_work);
    tendril_timer_init(&in->timeout, xfer_timeout_work);
    in->hcd = NULL;
    in->segments = &in->segment;
    in->num_segments = 0;
    in->bounce = NULL;
    in->bounce_dma = 0;
    in->bounce_size = 0;
    in->mapped = 0;
    in->cancel_status = TENDRIL_OK;
}

void tendril_transfer_init(struct tendril_transfer *xfer, struct tendril_endpoint *ep) {
    memset(xfer, 0, sizeof(*xfer));
    xfer->endpoint = ep;
    reset_internal(xfer);
}

static size_t data_length(struct tendril_transfer *xfer) {
    if (xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL) {
        return tendril_le16(xfer->setup.length);
    }
    if (xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_ISOC) {
        size_t total = 0;
        for (uint32_t i = 0; i < xfer->num_iso_packets; i++) {
            total += xfer->iso_packets[i].length;
        }
        return total;
    }
    return xfer->length;
}

static bool is_in(struct tendril_transfer *xfer) {
    if (xfer->endpoint->type == TENDRIL_ENDPOINT_XFER_CONTROL) {
        return (xfer->setup.request_type & TENDRIL_DIR_IN) != 0;
    }
    return (xfer->endpoint->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
}

static enum tendril_dma_dir dma_dir(struct tendril_transfer *xfer) {
    return is_in(xfer) ? TENDRIL_DMA_FROM_DEVICE : TENDRIL_DMA_TO_DEVICE;
}

/* Undoes map_buffer() or bounce_buffer(), copying bounced IN data back. */
static void release_buffer(struct tendril_hc *hc, struct tendril_transfer *xfer, bool copy_back) {
    struct tendril_transfer_internal *in = &xfer->internal;

    if (in->bounce != NULL) {
        if (copy_back && is_in(xfer)) {
            memcpy(xfer->buffer, in->bounce, in->bounce_size);
        }
        tendril_dma_free(hc, in->bounce, in->bounce_size);
        in->bounce = NULL;
        in->bounce_size = 0;
    } else if (in->mapped != 0) {
        size_t off = 0;
        for (size_t i = 0; i < in->num_segments; i++) {
            struct tendril_dma_segment *seg = &in->segments[i];
            tendril_glue_dma_unmap(hc->ctx, (char *)xfer->buffer + off, seg->len, dma_dir(xfer), seg->addr);
            off += seg->len;
        }
    }
    if (in->segments != &in->segment) {
        tendril_free(in->segments, in->segment.len);
    }
    in->segments = &in->segment;
    in->num_segments = 0;
    in->mapped = 0;
}

/* Maps the buffer for DMA, one segment per mapping; false asks for a bounce. */
static bool map_buffer(struct tendril_hc *hc, struct tendril_transfer *xfer, size_t len) {
    struct tendril_transfer_internal *in = &xfer->internal;
    /* With an external array, segment.len holds its size in bytes. */
    size_t cap = 1;
    size_t off = 0;

    in->segments = &in->segment;
    in->num_segments = 0;
    while (off < len) {
        uint64_t dma;
        size_t n = tendril_glue_dma_map(hc->ctx, (char *)xfer->buffer + off, len - off, dma_dir(xfer), &dma);
        if (n == 0) {
            break;
        }
        if (n > len - off) {
            n = len - off;
        }
        bool fits = !hc->data32 || dma + n <= 0x100000000ull;
        bool aligned = off + n == len || ((dma + n) & 0xfff) == 0;
        if (fits && aligned && in->num_segments == cap) {
            size_t ncap = cap * 4;
            struct tendril_dma_segment *arr = tendril_zalloc(ncap * sizeof(*arr));
            if (arr == NULL) {
                fits = false;
            } else {
                memcpy(arr, in->segments, in->num_segments * sizeof(*arr));
                if (in->segments != &in->segment) {
                    tendril_free(in->segments, cap * sizeof(*arr));
                }
                in->segments = arr;
                cap = ncap;
            }
        }
        if (!fits || !aligned) {
            tendril_glue_dma_unmap(hc->ctx, (char *)xfer->buffer + off, n, dma_dir(xfer), dma);
            break;
        }
        in->segments[in->num_segments].addr = dma;
        in->segments[in->num_segments].len = n;
        in->num_segments++;
        off += n;
        in->mapped = off;
    }
    if (in->segments != &in->segment) {
        /* Remember the array size for freeing; the first segment lives in the array. */
        in->segment.len = cap * sizeof(struct tendril_dma_segment);
    }
    if (off == len) {
        return true;
    }
    release_buffer(hc, xfer, false);
    return false;
}

static enum tendril_status bounce_buffer(struct tendril_hc *hc, struct tendril_transfer *xfer, size_t len) {
    struct tendril_transfer_internal *in = &xfer->internal;
    uint64_t dma;
    void *b = tendril_dma_alloc_limit(hc, len, 64, hc->data32, &dma);
    if (b == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    if (!is_in(xfer)) {
        memcpy(b, xfer->buffer, len);
    }
    in->bounce = b;
    in->bounce_dma = dma;
    in->bounce_size = len;
    in->segments = &in->segment;
    in->segment.addr = dma;
    in->segment.len = len;
    in->num_segments = 1;
    return TENDRIL_OK;
}

static enum tendril_status prepare_buffer(struct tendril_hc *hc, struct tendril_transfer *xfer) {
    struct tendril_transfer_internal *in = &xfer->internal;
    size_t len = data_length(xfer);

    if (len == 0) {
        return TENDRIL_OK;
    }
    if (xfer->flags & TENDRIL_XFER_DMA) {
        if (hc->data32 && xfer->dma + len > 0x100000000ull) {
            return TENDRIL_ERR_INVALID;
        }
        in->segment.addr = xfer->dma;
        in->segment.len = len;
        in->num_segments = 1;
        return TENDRIL_OK;
    }
    if (map_buffer(hc, xfer, len)) {
        return TENDRIL_OK;
    }
    return bounce_buffer(hc, xfer, len);
}

static enum tendril_status validate(struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;

    switch (ep->type) {
        case TENDRIL_ENDPOINT_XFER_CONTROL: {
            if (xfer->length < tendril_le16(xfer->setup.length)) {
                return TENDRIL_ERR_INVALID;
            }
            break;
        }
        case TENDRIL_ENDPOINT_XFER_ISOC: {
            if (xfer->num_iso_packets == 0 || xfer->iso_packets == NULL || data_length(xfer) > xfer->length) {
                return TENDRIL_ERR_INVALID;
            }
            break;
        }
        case TENDRIL_ENDPOINT_XFER_BULK: {
            if (xfer->stream != 0 && xfer->stream > ep->streams) {
                return TENDRIL_ERR_INVALID;
            }
            break;
        }
        default: {
            break;
        }
    }
    if (xfer->stream != 0 && ep->type != TENDRIL_ENDPOINT_XFER_BULK) {
        return TENDRIL_ERR_INVALID;
    }
    if (data_length(xfer) != 0 && xfer->buffer == NULL && !(xfer->flags & TENDRIL_XFER_DMA)) {
        return TENDRIL_ERR_INVALID;
    }
    return TENDRIL_OK;
}

enum tendril_status tendril_transfer_submit(struct tendril_transfer *xfer) {
    struct tendril_endpoint *ep = xfer->endpoint;
    if (ep == NULL) {
        return TENDRIL_ERR_INVALID;
    }
    struct tendril_device *dev = ep->dev;
    struct tendril_hc *hc = dev->hc;
    struct tendril_transfer_internal *in = &xfer->internal;

    enum tendril_status st = validate(xfer);
    if (st != TENDRIL_OK) {
        return st;
    }

    hc_lock(hc);
    if (in->state == XFER_PENDING || in->state == XFER_CANCELLING || in->state == XFER_COMPLETING) {
        hc_unlock(hc);
        return TENDRIL_ERR_BUSY;
    }
    if (dev->gone || hc->dead || hc->removing) {
        hc_unlock(hc);
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->suspended) {
        hc_unlock(hc);
        return TENDRIL_ERR_SUSPENDED;
    }
    if (dev->config_busy && xfer != dev->config_xfer) {
        hc_unlock(hc);
        return TENDRIL_ERR_BUSY;
    }
    if (ep != &dev->ep0 && !ep->open) {
        hc_unlock(hc);
        return TENDRIL_ERR_INVALID;
    }

    reset_internal(xfer);
    in->hc = hc;
    xfer->status = TENDRIL_OK;
    xfer->actual = 0;
    for (uint32_t i = 0; ep->type == TENDRIL_ENDPOINT_XFER_ISOC && i < xfer->num_iso_packets; i++) {
        xfer->iso_packets[i].actual = 0;
        xfer->iso_packets[i].status = TENDRIL_OK;
    }

    st = prepare_buffer(hc, xfer);
    if (st != TENDRIL_OK) {
        hc_unlock(hc);
        return st;
    }

    in->state = XFER_PENDING;
    list_add_tail(&ep->transfers, &in->node);
    dev->pending++;

    st = hc->ops->submit(hc, xfer);
    if (st == TENDRIL_ERR_BOUNCE && in->bounce == NULL) {
        release_buffer(hc, xfer, false);
        st = bounce_buffer(hc, xfer, data_length(xfer));
        if (st == TENDRIL_OK) {
            st = hc->ops->submit(hc, xfer);
        }
    }
    if (st != TENDRIL_OK) {
        list_del(&in->node);
        dev->pending--;
        in->state = XFER_IDLE;
        release_buffer(hc, xfer, false);
        hc_unlock(hc);
        return st == TENDRIL_ERR_BOUNCE ? TENDRIL_ERR_NO_MEMORY : st;
    }
    if (xfer->timeout_ms != 0) {
        tendril_timer_arm(hc, &in->timeout, (uint64_t)xfer->timeout_ms * NS_PER_MS);
    }
    hc_unlock(hc);
    return TENDRIL_OK;
}

void tendril_transfer_complete(struct tendril_hc *hc, struct tendril_transfer *xfer,
                              enum tendril_status status, size_t actual) {
    struct tendril_transfer_internal *in = &xfer->internal;

    if (in->state != XFER_PENDING && in->state != XFER_CANCELLING) {
        return;
    }
    tendril_timer_cancel(hc, &in->timeout);
    list_del(&in->node);
    xfer->status = status;
    xfer->actual = actual;
    in->state = XFER_COMPLETING;
    tendril_queue_work(hc, &in->done);
}

void tendril_transfer_fail_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep, enum tendril_status status) {
    while (!list_empty(&ep->transfers)) {
        struct tendril_transfer *xfer = CONTAINER_OF(ep->transfers.next, struct tendril_transfer, internal.node);
        tendril_transfer_complete(hc, xfer, status, 0);
    }
}

static void xfer_done_work(struct tendril_work *work) {
    struct tendril_transfer *xfer = CONTAINER_OF(work, struct tendril_transfer, internal.done);
    struct tendril_transfer_internal *in = &xfer->internal;
    struct tendril_hc *hc = in->hc;
    struct tendril_device *dev = xfer->endpoint->dev;

    release_buffer(hc, xfer, true);
    if (xfer->status == TENDRIL_OK && (xfer->flags & TENDRIL_XFER_SHORT_NOT_OK) &&
        xfer->endpoint->type != TENDRIL_ENDPOINT_XFER_ISOC && xfer->actual < data_length(xfer)) {
        xfer->status = TENDRIL_ERR_SHORT;
    }

    /* The callback may resubmit or free the transfer; keep the device alive. */
    hc_lock(hc);
    in->state = XFER_DONE;
    dev->refs++;
    dev->pending--;
    bool idle = dev->gone && dev->pending == 0;
    hc_unlock(hc);

    if (xfer->callback != NULL) {
        xfer->callback(xfer);
    }
    if (idle) {
        tendril_device_transfers_idle(dev);
    }
    tendril_device_unref(dev);
}

void tendril_transfer_cancel_locked(struct tendril_hc *hc, struct tendril_transfer *xfer, enum tendril_status status) {
    struct tendril_transfer_internal *in = &xfer->internal;

    if (in->state != XFER_PENDING) {
        return;
    }
    in->state = XFER_CANCELLING;
    in->cancel_status = status;
    tendril_timer_cancel(hc, &in->timeout);
    hc->ops->cancel(hc, xfer);
}

void tendril_transfer_cancel(struct tendril_transfer *xfer) {
    struct tendril_hc *hc = xfer->internal.hc;
    if (hc == NULL) {
        return;
    }
    hc_lock(hc);
    tendril_transfer_cancel_locked(hc, xfer, TENDRIL_ERR_CANCELLED);
    hc_unlock(hc);
}

static void xfer_timeout_work(struct tendril_work *work) {
    struct tendril_transfer *xfer = CONTAINER_OF(work, struct tendril_transfer, internal.timeout.work);
    struct tendril_hc *hc = xfer->internal.hc;

    hc_lock(hc);
    tendril_transfer_cancel_locked(hc, xfer, TENDRIL_ERR_TIMEOUT);
    hc_unlock(hc);
}

bool tendril_transfer_done(struct tendril_transfer *xfer) {
    uint8_t state = xfer->internal.state;
    return state == XFER_DONE || state == XFER_IDLE;
}

enum tendril_status tendril_transfer_run(struct tendril_transfer *xfer) {
    enum tendril_status st = tendril_transfer_submit(xfer);
    if (st != TENDRIL_OK) {
        return st;
    }
    struct tendril_hc *hc = xfer->internal.hc;
    while (xfer->internal.state != XFER_DONE) {
        tendril_hc_poll(hc);
    }
    return xfer->status;
}

enum tendril_status tendril_control(
    struct tendril_device *dev, uint8_t request_type, uint8_t request,
    uint16_t value, uint16_t index, void *data, uint16_t length,
    size_t *actual, uint32_t timeout_ms) {
    struct tendril_transfer xfer;

    tendril_transfer_init(&xfer, &dev->ep0);
    xfer.setup.request_type = request_type;
    xfer.setup.request = request;
    xfer.setup.value = tendril_le16(value);
    xfer.setup.index = tendril_le16(index);
    xfer.setup.length = tendril_le16(length);
    xfer.buffer = data;
    xfer.length = length;
    xfer.timeout_ms = timeout_ms;
    enum tendril_status st = tendril_transfer_run(&xfer);
    if (actual != NULL) {
        *actual = xfer.actual;
    }
    return st;
}
