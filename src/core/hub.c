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

/* External hubs: USB 2.0 chapter 11 and USB 3.2 revision 1.1 chapter 10. */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb.h (commit
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb_hub.c (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1998 The NetBSD Foundation, Inc. All rights reserved.
 * Copyright (c) 1998 Lennart Augustsson. All rights reserved.
 * Copyright (c) 2008-2022 Hans Petter Selasky
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb_request.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1998 The NetBSD Foundation, Inc. All rights reserved.
 * Copyright (c) 1998 Lennart Augustsson. All rights reserved.
 * Copyright (c) 2008-2020 Hans Petter Selasky. All rights reserved.
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
#include <tendril/spec.h>
#include <tendril/tendril.h>

#define HUB_CTRL_TIMEOUT_MS 5000
#define STATUS_RETRY_MS 100
#define STATUS_ERRORS_WARN 10
/* BEGIN FreeBSD sys/dev/usb/usb.h */
/*
 * FreeBSD's USB_EXTRA_POWER_UP_TIME, from its sys/dev/usb/usb.h
 * (BSD-2-Clause).
 */
#define EXTRA_POWER_UP_MS 20
/* END FreeBSD sys/dev/usb/usb.h */

#define HUB_GET_DESCRIPTOR 0
#define HUB_SET_DEPTH 1
#define HUB_UPDATE_HC 2
#define HUB_SET_TT 3
#define HUB_POWER 4
#define HUB_POWER_WAIT 5
#define HUB_RUNNING 6
#define HUB_FAILED 7

static void hub_init_step(struct tendril_hub *hub);

static void hub_name(struct tendril_hub *hub, char *buf, size_t size) {
    tendril_snprintf(buf, size, "hub %u", hub->dev->address);
}

/* Port operations over control transfers on the hub's endpoint 0. */

static void port_xfer_done(struct tendril_transfer *xfer) {
    struct tendril_port *port = xfer->context;
    int st = xfer->status;

    if (st == TENDRIL_OK
     && xfer->setup.request == TENDRIL_HUB_REQ_GET_STATUS) {
        if (xfer->actual < 4) {
            st = TENDRIL_ERR_PROTOCOL;
        } else {
            uint16_t status = (uint16_t)(port->buf[0] | port->buf[1] << 8);
            uint16_t change = (uint16_t)(port->buf[2] | port->buf[3] << 8);
            port->st.status = status;
            port->st.change = change;
            if (port->superspeed) {
                port->st.speed = TENDRIL_SPEED_SUPER;
                /*
                 * SuperSpeed hubs keep port power at bit 9, where USB 2.0 has
                 * low speed.
                 */
                port->st.status &= (uint16_t)~(TENDRIL_PORT_STAT_LOW_SPEED
                    | TENDRIL_PORT_STAT_HIGH_SPEED);
            } else if ((status & TENDRIL_PORT_STAT_LOW_SPEED) != 0) {
                port->st.speed = TENDRIL_SPEED_LOW;
            } else if ((status & TENDRIL_PORT_STAT_HIGH_SPEED) != 0) {
                port->st.speed = TENDRIL_SPEED_HIGH;
            } else {
                port->st.speed = TENDRIL_SPEED_FULL;
            }
        }
    }
    port->async.status = st;
    port->async.work.fn(&port->async.work);
}

static void port_request(
    struct tendril_port *port, uint8_t request_type, uint8_t request,
    int feature, size_t length
) {
    struct tendril_hub *hub = port->hub;
    struct tendril_transfer *xfer = &port->xfer;

    tendril_transfer_init(xfer, &hub->dev->ep0);
    xfer->setup.request_type = request_type;
    xfer->setup.request = request;
    xfer->setup.value = tendril_le16((uint16_t)PORT_FEATURE_SELECTOR(feature));
    /*
     * USB 3.2 revision 1.1 10.16.2.10: some features take a parameter in the
     * upper byte.
     */
    xfer->setup.index = tendril_le16(
        (uint16_t)(port->number | PORT_FEATURE_PARAM(feature) << 8)
    );
    xfer->setup.length = tendril_le16((uint16_t)length);
    xfer->buffer = length != 0 ? port->buf : NULL;
    xfer->length = length;
    xfer->timeout_ms = HUB_CTRL_TIMEOUT_MS;
    xfer->callback = port_xfer_done;
    xfer->context = port;
    int st = hub->stopping ? TENDRIL_ERR_NO_DEVICE
        : tendril_transfer_submit(xfer);
    if (st != TENDRIL_OK) {
        struct tendril_hc *hc = hub->hc;
        hc_lock(hc);
        tendril_async_complete(hc, &port->async, st);
        hc_unlock(hc);
    }
}

static void ext_get_status(struct tendril_port *port) {
    port_request(
        port, TENDRIL_DIR_IN | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_OTHER,
        TENDRIL_HUB_REQ_GET_STATUS, 0, 4
    );
}

static void ext_set_feature(struct tendril_port *port, int feature) {
    port_request(
        port, TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_OTHER,
        TENDRIL_HUB_REQ_SET_FEATURE, feature, 0
    );
}

static void ext_clear_feature(struct tendril_port *port, int feature) {
    port_request(
        port, TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_OTHER,
        TENDRIL_HUB_REQ_CLEAR_FEATURE, feature, 0
    );
}

static const struct tendril_hub_ops ext_hub_ops = {
    .get_status = ext_get_status,
    .set_feature = ext_set_feature,
    .clear_feature = ext_clear_feature
};

/* Hub-wide status changes */

static void hub_status_submit(struct tendril_hub *hub);

static void hub_clear_done(struct tendril_transfer *xfer) {
    struct tendril_hub *hub = xfer->context;
    hub->ctrl_busy = false;
    if (hub->hub_change_pending && !hub->stopping) {
        hub->hub_change_pending = false;
        tendril_hub_check_status(hub);
    }
}

static void hub_status_read(struct tendril_transfer *xfer) {
    struct tendril_hub *hub = xfer->context;
    char name[24];

    if (xfer->status != TENDRIL_OK || xfer->actual < 4 || hub->stopping) {
        hub->ctrl_busy = false;
        return;
    }
    uint16_t status = (uint16_t)(hub->ctrl_buf[0] | hub->ctrl_buf[1] << 8);
    uint16_t change = (uint16_t)(hub->ctrl_buf[2] | hub->ctrl_buf[3] << 8);
    int feature;
    if ((change & 0x0001) != 0) {
        feature = TENDRIL_HUB_C_LOCAL_POWER;
    } else if ((change & 0x0002) != 0) {
        hub_name(hub, name, sizeof(name));
        if ((status & 0x0002) != 0) {
            LOG_WARN("%s: over-current", name);
        }
        feature = TENDRIL_HUB_C_OVER_CURRENT;
    } else {
        hub->ctrl_busy = false;
        return;
    }
    uint32_t cleared = feature == TENDRIL_HUB_C_LOCAL_POWER ? (uint32_t)0x0001
        : (uint32_t)0x0002;
    if ((change & ~cleared & 0x0003) != 0) {
        hub->hub_change_pending = true;
    }
    tendril_transfer_init(&hub->ctrl, &hub->dev->ep0);
    hub->ctrl.setup.request_type = TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS
        | TENDRIL_RECIP_DEVICE;
    hub->ctrl.setup.request = TENDRIL_HUB_REQ_CLEAR_FEATURE;
    hub->ctrl.setup.value = tendril_le16((uint16_t)feature);
    hub->ctrl.timeout_ms = HUB_CTRL_TIMEOUT_MS;
    hub->ctrl.callback = hub_clear_done;
    hub->ctrl.context = hub;
    if (tendril_transfer_submit(&hub->ctrl) != TENDRIL_OK) {
        hub->ctrl_busy = false;
    }
}

void tendril_hub_check_status(struct tendril_hub *hub) {
    if (hub->ctrl_busy) {
        hub->hub_change_pending = true;
        return;
    }
    hub->ctrl_busy = true;
    tendril_transfer_init(&hub->ctrl, &hub->dev->ep0);
    hub->ctrl.setup.request_type = TENDRIL_DIR_IN | TENDRIL_TYPE_CLASS
        | TENDRIL_RECIP_DEVICE;
    hub->ctrl.setup.request = TENDRIL_HUB_REQ_GET_STATUS;
    hub->ctrl.setup.length = tendril_le16(4);
    hub->ctrl.buffer = hub->ctrl_buf;
    hub->ctrl.length = 4;
    hub->ctrl.timeout_ms = HUB_CTRL_TIMEOUT_MS;
    hub->ctrl.callback = hub_status_read;
    hub->ctrl.context = hub;
    if (tendril_transfer_submit(&hub->ctrl) != TENDRIL_OK) {
        hub->ctrl_busy = false;
    }
}

/* Status change endpoint */

/* BEGIN FreeBSD sys/dev/usb/usb_hub.c */
static void hub_status_halt_cleared(void *ctx, int status) {
    struct tendril_hub *hub = ctx;
    (void)status;
    if (!hub->stopping && !hub->suspended) {
        hc_lock(hub->hc);
        tendril_timer_arm(
            hub->hc, &hub->status_timer, STATUS_RETRY_MS * NS_PER_MS
        );
        hc_unlock(hub->hc);
    }
}
/* END FreeBSD sys/dev/usb/usb_hub.c */

static void hub_status_done(struct tendril_transfer *xfer) {
    struct tendril_hub *hub = xfer->context;
    struct tendril_hc *hc = hub->hc;
    char name[24];

    /* A suspended hub's changes are picked up by the port checks on resume. */
    if (hub->stopping || hub->suspended) {
        return;
    }
    switch (xfer->status) {
        case TENDRIL_OK: {
            hub->status_errors = 0;
            if (xfer->actual > 0 && (hub->status_buf[0] & 1) != 0) {
                tendril_hub_check_status(hub);
            }
            hc_lock(hc);
            for (size_t p = 1; p <= hub->num_ports && p / 8 < xfer->actual;
                 p++) {
                if ((hub->status_buf[p / 8] & ((uint32_t)1 << (p % 8))) != 0) {
                    tendril_port_kick(&hub->ports[p - 1]);
                }
            }
            hc_unlock(hc);
            hub_status_submit(hub);
            return;
        }
        case TENDRIL_ERR_NO_DEVICE:
        case TENDRIL_ERR_CANCELLED: {
            return;
        }
        default: {
            break;
        }
    }

    /* BEGIN FreeBSD sys/dev/usb/usb_hub.c */
    /*
     * As FreeBSD's sys/dev/usb/usb_hub.c (BSD-2-Clause, uhub_intr_callback)
     * does, clear the halt and try again for as long as the hub is there.
     */
    hub_name(hub, name, sizeof(name));
    if (++hub->status_errors == STATUS_ERRORS_WARN) {
        LOG_WARN(
            "%s: status endpoint keeps failing: %s", name,
            tendril_status_str(xfer->status)
        );
    } else {
        LOG_DEBUG(
            "%s: status endpoint error: %s", name,
            tendril_status_str(xfer->status)
        );
    }
    int st = tendril_endpoint_clear_halt(
        hub->status_xfer.endpoint, hub_status_halt_cleared, hub
    );
    if (st != TENDRIL_OK) {
        hub_status_halt_cleared(hub, TENDRIL_OK);
    }
    /* END FreeBSD sys/dev/usb/usb_hub.c */
}

static void hub_status_retry(struct tendril_work *work) {
    struct tendril_hub *hub = CONTAINER_OF(
        work, struct tendril_hub, status_timer.work
    );
    if (!hub->stopping && !hub->suspended) {
        hub_status_submit(hub);
    }
}

static void hub_status_submit(struct tendril_hub *hub) {
    struct tendril_interface *intf = &hub->dev->interfaces[0];
    struct tendril_endpoint *ep = tendril_interface_find_endpoint(
        intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_IN
    );
    if (ep == NULL) {
        return;
    }
    struct tendril_transfer *xfer = &hub->status_xfer;
    tendril_transfer_init(xfer, ep);
    xfer->buffer = hub->status_buf;
    xfer->length = MIN(
        (size_t)(hub->num_ports / 8 + 1),
        MIN((size_t)ep->max_packet, sizeof(hub->status_buf))
    );
    xfer->callback = hub_status_done;
    xfer->context = hub;
    int st = tendril_transfer_submit(xfer);
    if (st != TENDRIL_OK && st != TENDRIL_ERR_NO_DEVICE) {
        char name[24];
        hub_name(hub, name, sizeof(name));
        LOG_WARN(
            "%s: cannot poll the status endpoint: %s", name,
            tendril_status_str(st)
        );
    }
}

/* Initialization */

static void hub_init_fail(struct tendril_hub *hub, int st, const char *what) {
    char name[24];
    hub_name(hub, name, sizeof(name));
    if (st != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN("%s: %s failed: %s", name, what, tendril_status_str(st));
    }
    hub->init_step = HUB_FAILED;
    tendril_device_hub_ready(hub->dev, st);
}

static void hub_ctrl_done(struct tendril_transfer *xfer) {
    struct tendril_hub *hub = xfer->context;
    hub->init_async.status = xfer->status;
    hub_init_step(hub);
}

static void hub_ctrl(
    struct tendril_hub *hub, uint8_t request_type, uint8_t request,
    uint16_t value, uint16_t index, size_t length, const char *what
) {
    struct tendril_transfer *xfer = &hub->ctrl;
    tendril_transfer_init(xfer, &hub->dev->ep0);
    xfer->setup.request_type = request_type;
    xfer->setup.request = request;
    xfer->setup.value = tendril_le16(value);
    xfer->setup.index = tendril_le16(index);
    xfer->setup.length = tendril_le16((uint16_t)length);
    xfer->buffer = length != 0 ? hub->ctrl_buf : NULL;
    xfer->length = length;
    xfer->timeout_ms = HUB_CTRL_TIMEOUT_MS;
    xfer->callback = hub_ctrl_done;
    xfer->context = hub;
    int st = tendril_transfer_submit(xfer);
    if (st != TENDRIL_OK) {
        hub_init_fail(hub, st, what);
    }
}

static void hub_wait(struct tendril_hub *hub, uint64_t ms) {
    struct tendril_hc *hc = hub->hc;
    hub->init_async.status = TENDRIL_OK;
    hc_lock(hc);
    tendril_timer_arm(hc, &hub->init_timer, ms * NS_PER_MS);
    hc_unlock(hc);
}

static bool parse_hub_descriptor(struct tendril_hub *hub, size_t actual) {
    const uint8_t *d = hub->ctrl_buf;
    if (actual < 7 || d[0] < 7) {
        return false;
    }
    if (d[1] != (hub->superspeed ? TENDRIL_DT_SS_HUB : TENDRIL_DT_HUB)) {
        return false;
    }
    hub->num_ports = d[2];
    /* BEGIN FreeBSD sys/dev/usb/usb_hub.c */
    /*
     * FreeBSD's uhub_attach() refuses more ports than a bus has addresses for.
     */
    if (!hub->superspeed && hub->num_ports > 127) {
        return false;
    }
    /* END FreeBSD sys/dev/usb/usb_hub.c */
    hub->characteristics = (uint16_t)(d[3] | d[4] << 8);
    hub->power_on_delay_ms = (uint16_t)(d[5] * 2);
    if (hub->superspeed && hub->num_ports > 15) {
        hub->num_ports = 15;
    }
    if (!hub->superspeed && hub->dev->speed == TENDRIL_SPEED_HIGH) {
        uint16_t tttt = hub->characteristics & TENDRIL_HUB_CHAR_TTTT_MASK;
        hub->think_time = (uint8_t)(tttt >> TENDRIL_HUB_CHAR_TTTT_SHIFT);
    }
    return hub->num_ports != 0;
}

static void hub_mtt_done(void *ctx, int status) {
    struct tendril_hub *hub = ctx;
    hub->init_async.status = status;
    hub_init_step(hub);
}

static int multi_tt_alt(struct tendril_hub *hub) {
    struct tendril_interface *intf = &hub->dev->interfaces[0];
    if (hub->dev->speed != TENDRIL_SPEED_HIGH
     || hub->dev->desc.device_protocol != 2) {
        return -1;
    }
    for (size_t a = 0; a < intf->num_alts; a++) {
        if (intf->alts[a]->interface_protocol == 2) {
            return intf->alts[a]->alternate_setting;
        }
    }
    return -1;
}

static void hub_init_step(struct tendril_hub *hub) {
    struct tendril_hc *hc = hub->hc;
    struct tendril_device *dev = hub->dev;
    int st = hub->init_async.status;
    char name[24];

    if (hub->stopping || dev->gone) {
        hub_init_fail(hub, TENDRIL_ERR_NO_DEVICE, "initialization");
        return;
    }

    switch (hub->init_step) {
        case HUB_GET_DESCRIPTOR: {
            if ((st != TENDRIL_OK && st != TENDRIL_ERR_SHORT)
             || !parse_hub_descriptor(hub, hub->ctrl.actual)) {
                hub_init_fail(
                    hub, st != TENDRIL_OK ? st : TENDRIL_ERR_PROTOCOL,
                    "reading the hub descriptor"
                );
                return;
            }
            hub->ports = tendril_zalloc(
                hub->num_ports * sizeof(struct tendril_port)
            );
            if (hub->ports == NULL) {
                hub_init_fail(hub, TENDRIL_ERR_NO_MEMORY, "port allocation");
                return;
            }
            for (size_t i = 0; i < hub->num_ports; i++) {
                tendril_port_init(&hub->ports[i], hub, i + 1);
            }
            hub_name(hub, name, sizeof(name));
            LOG_INFO(
                "%s: %zu ports%s", name, hub->num_ports,
                hub->superspeed ? ", SuperSpeed"
                    : multi_tt_alt(hub) >= 0 ? ", multiple TTs"
                    : ""
            );
            if (hub->superspeed) {
                hub->init_step = HUB_SET_DEPTH;
                hub_ctrl(
                    hub,
                    TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS
                        | TENDRIL_RECIP_DEVICE,
                    TENDRIL_HUB_REQ_SET_HUB_DEPTH, (uint16_t)dev->depth, 0, 0,
                    "SET_HUB_DEPTH"
                );
                return;
            }
            hub->init_async.status = TENDRIL_OK;
            hub->init_step = HUB_SET_DEPTH;
            hub_init_step(hub);
            return;
        }
        case HUB_SET_DEPTH: {
            if (st != TENDRIL_OK) {
                hub_init_fail(hub, st, "SET_HUB_DEPTH");
                return;
            }
            int alt = hub->superspeed ? -1 : multi_tt_alt(hub);
            if (alt >= 0) {
                hub->multi_tt = true;
                hub->init_step = HUB_SET_TT;
                st = tendril_interface_set_alt(
                    &dev->interfaces[0], (uint8_t)alt, hub_mtt_done, hub
                );
                if (st != TENDRIL_OK) {
                    hub->multi_tt = false;
                    hub->init_async.status = TENDRIL_OK;
                    hub_init_step(hub);
                }
                return;
            }
            hub->init_async.status = TENDRIL_OK;
            hub->init_step = HUB_SET_TT;
            hub_init_step(hub);
            return;
        }
        case HUB_SET_TT: {
            if (st != TENDRIL_OK && hub->multi_tt) {
                hub_name(hub, name, sizeof(name));
                LOG_WARN(
                    "%s: cannot enable multiple TTs: %s", name,
                    tendril_status_str(st)
                );
                hub->multi_tt = false;
            }
            if (hc->ops->hub_update != NULL) {
                hub->init_step = HUB_UPDATE_HC;
                hc_lock(hc);
                st = hc->dead ? TENDRIL_ERR_NO_DEVICE
                    : hc->ops->hub_update(hc, dev, &hub->init_async);
                hc_unlock(hc);
                if (st != TENDRIL_OK) {
                    hub_init_fail(hub, st, "controller hub setup");
                }
                return;
            }
            hub->init_async.status = TENDRIL_OK;
            hub->init_step = HUB_UPDATE_HC;
            hub_init_step(hub);
            return;
        }
        case HUB_UPDATE_HC: {
            if (st != TENDRIL_OK) {
                hub_init_fail(hub, st, "controller hub setup");
                return;
            }
            hub->init_port = 0;
            hub->init_step = HUB_POWER;
            hub->init_async.status = TENDRIL_OK;
            hub_init_step(hub);
            return;
        }
        case HUB_POWER: {
            if (st != TENDRIL_OK) {
                hub_name(hub, name, sizeof(name));
                LOG_WARN(
                    "%s: powering port %zu failed: %s", name, hub->init_port,
                    tendril_status_str(st)
                );
            }
            if (hub->init_port < hub->num_ports) {
                hub->init_port++;
                hub_ctrl(
                    hub,
                    TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_OTHER,
                    TENDRIL_HUB_REQ_SET_FEATURE, TENDRIL_PORT_FEAT_POWER,
                    (uint16_t)hub->init_port, 0, "port power"
                );
                return;
            }
            hub->init_step = HUB_POWER_WAIT;
            /* BEGIN FreeBSD sys/dev/usb/usb_hub.c */
            /* USB 2.0 11.23.2.1: bPwrOn2PwrGood, plus FreeBSD's margin. */
            hub_wait(hub, hub->power_on_delay_ms + EXTRA_POWER_UP_MS);
            /* END FreeBSD sys/dev/usb/usb_hub.c */
            return;
        }
        case HUB_POWER_WAIT: {
            hub->init_step = HUB_RUNNING;
            hub_status_submit(hub);
            /*
             * The status change endpoint reports a connection when next
             * polled, every interval microframes.
             */
            struct tendril_endpoint *ep = hub->status_xfer.endpoint;
            uint64_t report_ms = ep != NULL ? (ep->interval + 7) / 8 : 0;
            hc_lock(hc);
            for (size_t i = 0; i < hub->num_ports; i++) {
                tendril_port_powered(&hub->ports[i], report_ms);
                tendril_port_kick(&hub->ports[i]);
            }
            hc_unlock(hc);
            tendril_device_hub_ready(dev, TENDRIL_OK);
            return;
        }
        default: {
            return;
        }
    }
}

static void hub_init_timer(struct tendril_work *work) {
    hub_init_step(CONTAINER_OF(work, struct tendril_hub, init_timer.work));
}

/* Controller hub setup completes through here. */
static void hub_init_async(struct tendril_work *work) {
    hub_init_step(CONTAINER_OF(work, struct tendril_hub, init_async.work));
}

void tendril_hub_start(struct tendril_device *dev) {
    struct tendril_hub *hub = tendril_zalloc(sizeof(struct tendril_hub));
    if (hub == NULL) {
        tendril_device_hub_ready(dev, TENDRIL_ERR_NO_MEMORY);
        return;
    }
    hub->hc = dev->hc;
    hub->dev = dev;
    hub->ops = &ext_hub_ops;
    hub->superspeed = tendril_device_is_ss(dev);
    /*
     * USB 2.0 7.2.1 and USB 3.2 revision 1.1 11.4: what a port gives a
     * high-power device, or one unit load.
     */
    hub->port_power = dev->self_powered ? 500 : 100;
    hub->port_power_ss = dev->self_powered ? 900 : 150;
    tendril_async_init(&hub->init_async, hub_init_async);
    tendril_timer_init(&hub->init_timer, hub_init_timer);
    tendril_timer_init(&hub->status_timer, hub_status_retry);
    dev->hub = hub;

    hub->init_step = HUB_GET_DESCRIPTOR;
    uint8_t type = hub->superspeed ? TENDRIL_DT_SS_HUB : TENDRIL_DT_HUB;
    size_t len = hub->superspeed ? 12 : 71;
    hub_ctrl(
        hub, TENDRIL_DIR_IN | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_DEVICE,
        TENDRIL_HUB_REQ_GET_DESCRIPTOR, (uint16_t)(type << 8), 0, len,
        "reading the hub descriptor"
    );
}

void tendril_hub_stop(struct tendril_hub *hub) {
    struct tendril_hc *hc = hub->hc;

    if (hub->stopping) {
        return;
    }
    hub->stopping = true;
    hc_lock(hc);
    bool init_waiting = tendril_timer_armed(&hub->init_timer);
    tendril_timer_cancel(hc, &hub->init_timer);
    tendril_timer_cancel(hc, &hub->status_timer);
    for (size_t i = 0; i < hub->num_ports; i++) {
        tendril_port_stop(&hub->ports[i]);
    }
    hc_unlock(hc);

    for (size_t i = 0; i < hub->num_ports; i++) {
        struct tendril_port *port = &hub->ports[i];
        struct tendril_device *child = port->device;
        if (child != NULL) {
            port->device = NULL;
            tendril_device_disconnect(child);
        }
    }
    if (init_waiting) {
        hub_init_fail(hub, TENDRIL_ERR_NO_DEVICE, "initialization");
    }
}

/* Transaction translators: USB 2.0 11.17.5, 11.24.2.3, and 11.24.2.9. */

/* BEGIN FreeBSD sys/dev/usb/usb_hub.c */
static void hub_tt_next(struct tendril_hub *hub);

static void hub_tt_done(struct tendril_transfer *xfer) {
    struct tendril_hub *hub = xfer->context;
    hub->tt_busy = false;
    if (xfer->status != TENDRIL_OK && xfer->status != TENDRIL_ERR_NO_DEVICE) {
        char name[24];
        hub_name(hub, name, sizeof(name));
        LOG_DEBUG(
            "%s: TT request failed: %s", name, tendril_status_str(xfer->status)
        );
    }
    hub_tt_next(hub);
}

static void hub_tt_next(struct tendril_hub *hub) {
    if (hub->tt_busy || hub->stopping) {
        return;
    }
    for (size_t i = 0; i < hub->num_ports; i++) {
        struct tendril_port *port = &hub->ports[i];
        if (port->tt_request == 0) {
            continue;
        }
        struct tendril_transfer *xfer = &hub->tt_xfer;
        tendril_transfer_init(xfer, &hub->dev->ep0);
        xfer->setup.request_type = TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS
            | TENDRIL_RECIP_OTHER;
        xfer->setup.request = port->tt_request;
        xfer->setup.value = tendril_le16(port->tt_value);
        /* A hub with a single TT wants 1 for it. */
        xfer->setup.index = tendril_le16(hub->multi_tt ? port->number : 1);
        xfer->timeout_ms = HUB_CTRL_TIMEOUT_MS;
        xfer->callback = hub_tt_done;
        xfer->context = hub;
        port->tt_request = 0;
        if (tendril_transfer_submit(xfer) == TENDRIL_OK) {
            hub->tt_busy = true;
            return;
        }
    }
}

/*
 * Queues a request for the TT serving `dev`. As in FreeBSD's
 * sys/dev/usb/usb_hub.c (BSD-2-Clause, uhub_tt_buffer_reset_async_locked), a
 * second request before the first went out resets the whole TT instead.
 */
static void queue_tt(
    struct tendril_device *dev, uint8_t request, uint16_t value
) {
    struct tendril_hub *hub = dev->tt_hub != NULL ? dev->tt_hub->hub : NULL;
    if (hub == NULL || hub->stopping || hub->ports == NULL || dev->tt_port == 0
     || dev->tt_port > hub->num_ports) {
        return;
    }
    struct tendril_port *port = &hub->ports[dev->tt_port - 1];
    if (port->tt_request != 0) {
        request = TENDRIL_HUB_REQ_RESET_TT;
        value = 0;
    }
    port->tt_request = request;
    port->tt_value = value;
    hub_tt_next(hub);
}
/* END FreeBSD sys/dev/usb/usb_hub.c */

void tendril_hub_clear_tt(
    struct tendril_device *dev, struct tendril_endpoint *ep
) {
    /* USB 2.0 Table 11-18. */
    uint32_t value = (uint32_t)(ep->address & TENDRIL_ENDPOINT_NUMBER_MASK)
        | (uint32_t)(dev->address & 0x7f) << 4 | (uint32_t)(ep->type & 3) << 11
        | ((ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0 ? (uint32_t)1 << 15
            : 0);
    queue_tt(dev, TENDRIL_HUB_REQ_CLEAR_TT_BUFFER, (uint16_t)value);
}

/* BEGIN FreeBSD sys/dev/usb/usb_request.c */
/*
 * As FreeBSD's sys/dev/usb/usb_request.c (BSD-2-Clause, usbd_req_re_enumerate)
 * does: the whole TT is reset when the device is the only LS/FS one on its HS
 * hub, else just the buffer of its default control endpoint is cleared.
 */
void tendril_hub_reset_tt(struct tendril_device *dev) {
    struct tendril_device *tt = dev->tt_hub;
    if (tt == NULL || tt->hub == NULL || tt->hub->ports == NULL) {
        return;
    }
    size_t slow = 0;
    for (size_t i = 0; dev->parent == tt && i < tt->hub->num_ports; i++) {
        struct tendril_device *child = tt->hub->ports[i].device;
        if (child != NULL && child->speed <= TENDRIL_SPEED_FULL) {
            slow++;
        }
    }
    if (dev->parent == tt && slow == 1) {
        queue_tt(dev, TENDRIL_HUB_REQ_RESET_TT, 0);
        return;
    }
    queue_tt(
        dev, TENDRIL_HUB_REQ_CLEAR_TT_BUFFER,
        (uint16_t)((dev->address & 0x7f) << 4)
    );
}
/* END FreeBSD sys/dev/usb/usb_request.c */

/* Power management */

bool tendril_hub_suspend(struct tendril_hub *hub) {
    struct tendril_hc *hc = hub->hc;

    if (hub->init_step != HUB_RUNNING || hub->stopping) {
        return false;
    }
    hc_lock(hc);
    bool ready = true;
    for (size_t i = 0; i < hub->num_ports && ready; i++) {
        struct tendril_port *port = &hub->ports[i];
        ready = tendril_port_idle(port)
            && (port->device == NULL || port->device->suspended);
    }
    if (ready) {
        hub->suspended = true;
        tendril_timer_cancel(hc, &hub->status_timer);
    }
    hc_unlock(hc);
    return ready;
}

void tendril_hub_resume(struct tendril_hub *hub) {
    struct tendril_hc *hc = hub->hc;

    hub->suspended = false;
    if (hub->stopping) {
        return;
    }
    hub->status_errors = 0;
    hub_status_submit(hub);
    /*
     * Changes during the suspend, a child's remote wakeup among them, went
     * unreported.
     */
    hc_lock(hc);
    for (size_t i = 0; i < hub->num_ports; i++) {
        tendril_port_kick(&hub->ports[i]);
    }
    hc_unlock(hc);
}

/* Called once the hub device itself is being freed. */
void tendril_hub_free(struct tendril_hub *hub) {
    struct tendril_hc *hc = hub->hc;
    hc_lock(hc);
    for (size_t i = 0; i < hub->num_ports; i++) {
        struct tendril_port *port = &hub->ports[i];
        tendril_timer_cancel(hc, &port->timer);
        if (list_linked(&port->async.work.node)) {
            list_del(&port->async.work.node);
        }
    }
    hc_unlock(hc);
    tendril_free(hub->ports, hub->num_ports * sizeof(struct tendril_port));
    tendril_free(hub, sizeof(struct tendril_hub));
}
