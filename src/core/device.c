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
 * Device enumeration and teardown. Like the port state machine, these steps
 * run from work items or transfer callbacks, one at a time per controller.
 */

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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb_freebsd.h
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008 Hans Petter Selasky. All rights reserved.
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb_transfer.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008-2021 Hans Petter Selasky. All rights reserved.
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb_device.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008-2023 Hans Petter Selasky
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb_msctest.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2008-2022 Hans Petter Selasky.
 * Copyright (c) 2021-2022 Idwer Vollering.
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

#include "../internal.h"

#define CTRL_TIMEOUT_MS 5000
/* BEGIN FreeBSD sys/dev/usb/usb.h */
/*
 * FreeBSD's USB_SET_ADDRESS_SETTLE, from its sys/dev/usb/usb.h
 * (BSD-2-Clause); USB 2.0 9.2.6.3 only asks for 2 ms.
 */
#define SET_ADDRESS_SETTLE_MS 10
/* END FreeBSD sys/dev/usb/usb.h */
#define MAX_CONFIGS 8
/* BEGIN FreeBSD sys/dev/usb/usb_freebsd.h */
/*
 * USB 2.0 4.1.1: at most five hubs on the way to a function. With depth
 * counting the hubs above a device, that is FreeBSD's USB_HUB_MAX_DEPTH, from
 * its sys/dev/usb/usb_freebsd.h, which counts from the root hub.
 */
#define MAX_HUB_DEPTH 4
/* END FreeBSD sys/dev/usb/usb_freebsd.h */

#define STEP_NONE 0
#define STEP_INIT 1
#define STEP_HC_ADDRESS 2
#define STEP_HC_ADDRESS_WAIT 3
#define STEP_DESC8 4
#define STEP_EP0_MPS 5
#define STEP_SET_ADDRESS 6
#define STEP_ADDRESS_WAIT 7
#define STEP_EP0_ADDRESS 8
#define STEP_DESC18 9
#define STEP_CONFIG_HEAD 10
#define STEP_CONFIG_FULL 11
#define STEP_BOS_HEAD 12
#define STEP_BOS_FULL 13
#define STEP_LANGID 14
#define STEP_STRING 15
#define STEP_SELF_POWER 16
#define STEP_CONFIGURE 17
#define STEP_SET_CONFIG 18
#define STEP_EJECT 19
#define STEP_UNCONFIGURE 20
#define STEP_UNCONFIGURE_DROP 21
#define STEP_HUB 22
#define STEP_RUNNING 23
#define STEP_TEARDOWN_WAIT 24
#define STEP_TEARDOWN_FREE 25
#define STEP_FREED 26

static void dev_async_work(struct tendril_work *work);
static void dev_timer_work(struct tendril_work *work);
static void start_teardown(struct tendril_device *dev);

bool tendril_device_is_ss(struct tendril_device *dev) {
    return dev->speed >= TENDRIL_SPEED_SUPER;
}

static uint16_t default_ep0_mps(int speed) {
    switch (speed) {
        case TENDRIL_SPEED_LOW:
        case TENDRIL_SPEED_FULL: {
            return 8;
        }
        case TENDRIL_SPEED_HIGH: {
            return 64;
        }
        default: {
            return 512;
        }
    }
}

/* BEGIN FreeBSD sys/dev/usb/usb_transfer.c */
/*
 * The packet size FreeBSD's sys/dev/usb/usb_transfer.c (BSD-2-Clause,
 * usbd_transfer_setup_sub() and usbd_get_std_packet_size()) settles on for an
 * endpoint: a bulk or control size rounds down to one the speed allows, an
 * interrupt or isochronous one is capped at the speed's largest, and 0 reads as
 * the largest. Isochronous endpoints keep a 0, unlike there, as it marks the
 * zero-bandwidth settings of USB 2.0 5.6.3, which take no transfers here.
 */
static uint16_t std_packet_size(uint8_t type, int speed, uint16_t mps) {
    bool ss = speed >= TENDRIL_SPEED_SUPER;
    switch (type) {
        case TENDRIL_ENDPOINT_XFER_INT: {
            uint16_t max = speed == TENDRIL_SPEED_LOW ? 8 : speed == TENDRIL_SPEED_FULL ? 64 : 1024;
            return mps == 0 || mps > max ? max : mps;
        }
        case TENDRIL_ENDPOINT_XFER_ISOC: {
            uint16_t max = speed == TENDRIL_SPEED_LOW ? 0 : speed == TENDRIL_SPEED_FULL ? 1023 : 1024;
            return MIN(mps, max);
        }
        default: {
            if (speed == TENDRIL_SPEED_FULL) {
                if (mps == 0 || mps >= 64) {
                    return 64;
                }
                return mps >= 32 ? 32 : mps >= 16 ? 16 : 8;
            }
            if (type == TENDRIL_ENDPOINT_XFER_BULK) {
                return speed == TENDRIL_SPEED_LOW ? 8 : speed == TENDRIL_SPEED_HIGH ? 512 : 1024;
            }
            return speed == TENDRIL_SPEED_LOW ? 8 : ss ? 512 : 64;
        }
    }
}
/* END FreeBSD sys/dev/usb/usb_transfer.c */

static void set_ep0_mps(struct tendril_device *dev, uint16_t mps) {
    dev->ep0_desc.max_packet_size = tendril_le16(mps);
    dev->ep0.max_packet = mps;
}

struct tendril_device *tendril_device_alloc(struct tendril_hc *hc, struct tendril_port *port, int speed) {
    struct tendril_device *dev = tendril_zalloc(sizeof(struct tendril_device));
    if (dev == NULL) {
        return NULL;
    }
    struct tendril_device *parent = port->hub->dev;

    dev->hc = hc;
    dev->parent = parent;
    dev->port = port;
    dev->refs = 1;
    dev->state = DEV_ENUMERATING;
    dev->speed = speed;
    dev->active_config = -1;
    list_init(&dev->node);
    tendril_async_init(&dev->async, dev_async_work);
    tendril_timer_init(&dev->timer, dev_timer_work);
    if (parent != NULL) {
        tendril_device_ref(parent);
    }

    if (parent == NULL) {
        dev->depth = 0;
        dev->root_port = port->number;
        dev->route = 0;
    } else {
        dev->depth = (uint8_t)(parent->depth + 1);
        dev->root_port = parent->root_port;
        dev->route = parent->route | ((uint32_t)MIN(port->number, 15) << (4 * parent->depth));
        if (speed <= TENDRIL_SPEED_FULL) {
            if (parent->speed == TENDRIL_SPEED_HIGH) {
                dev->tt_hub = parent;
                dev->tt_port = port->number;
            } else {
                dev->tt_hub = parent->tt_hub;
                dev->tt_port = parent->tt_port;
            }
        }
    }

    dev->ep0_desc.length = sizeof(struct tendril_endpoint_descriptor);
    dev->ep0_desc.type = TENDRIL_DT_ENDPOINT;
    dev->ep0_desc.endpoint_address = 0;
    dev->ep0_desc.attributes = TENDRIL_ENDPOINT_XFER_CONTROL;
    tendril_endpoint_setup(&dev->ep0, dev, NULL, &dev->ep0_desc, NULL);
    set_ep0_mps(dev, default_ep0_mps(speed));

    hc_lock(hc);
    list_add_tail(&hc->devices, &dev->node);
    hc_unlock(hc);
    return dev;
}

static void free_interfaces(struct tendril_device *dev) {
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        tendril_free(intf->alts, intf->num_alts * sizeof(struct tendril_interface_descriptor *));
        tendril_free(intf->alt_sizes, intf->num_alts * sizeof(size_t));
        tendril_free(intf->endpoints, intf->num_endpoints * sizeof(struct tendril_endpoint));
    }
    tendril_free(dev->interfaces, dev->num_interfaces * sizeof(struct tendril_interface));
    dev->interfaces = NULL;
    dev->num_interfaces = 0;
}

static void free_device(struct tendril_device *dev) {
    /* Children keep their ports, inside the hub, until they are freed too. */
    if (dev->hub != NULL) {
        tendril_hub_free(dev->hub);
    }
    free_interfaces(dev);
    for (size_t i = 0; i < dev->num_configs; i++) {
        tendril_free(dev->configs[i], dev->config_sizes[i]);
    }
    tendril_free(dev->configs, dev->num_configs * sizeof(uint8_t *));
    tendril_free(dev->config_sizes, dev->num_configs * sizeof(size_t));
    tendril_free(dev->bos, dev->bos_size);
    tendril_strfree(dev->manufacturer);
    tendril_strfree(dev->product);
    tendril_strfree(dev->serial);
    tendril_free(dev->buf, dev->buf_size);
    struct tendril_device *parent = dev->parent;
    tendril_free(dev, sizeof(struct tendril_device));
    if (parent != NULL) {
        tendril_device_unref(parent);
    }
}

void tendril_device_ref(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    hc_lock(hc);
    dev->refs++;
    hc_unlock(hc);
}

void tendril_device_unref(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    hc_lock(hc);
    bool last = --dev->refs == 0;
    hc_unlock(hc);
    if (last) {
        free_device(dev);
    }
}

void tendril_device_put(struct tendril_device *dev) {
    tendril_device_unref(dev);
}

/* Endpoints */

void tendril_endpoint_setup(
    struct tendril_endpoint *ep, struct tendril_device *dev, struct tendril_interface *intf,
    const struct tendril_endpoint_descriptor *desc, const uint8_t *end
) {
    memset(ep, 0, sizeof(struct tendril_endpoint));
    ep->dev = dev;
    ep->intf = intf;
    ep->desc = desc;
    ep->address = desc->endpoint_address;
    ep->type = desc->attributes & TENDRIL_ENDPOINT_XFER_MASK;
    list_init(&ep->transfers);

    uint16_t wmps = tendril_le16(desc->max_packet_size);
    ep->max_packet = std_packet_size(ep->type, dev->speed, wmps & TENDRIL_ENDPOINT_MAXP_MASK);
    ep->mult = 1;
    ep->max_burst = 0;

    if (end != NULL) {
        const uint8_t *p = tendril_desc_next((const uint8_t *)desc, end);
        if (p != NULL && p[1] == TENDRIL_DT_SS_ENDPOINT_COMPANION
         && p[0] >= sizeof(struct tendril_ss_ep_companion_descriptor)) {
            ep->companion = (const void *)p;
            p = tendril_desc_next(p, end);
            if (p != NULL && p[1] == TENDRIL_DT_SSP_ISO_ENDPOINT_COMPANION
             && p[0] >= sizeof(struct tendril_ssp_iso_ep_companion_descriptor)) {
                ep->ssp_companion = (const void *)p;
            }
        }
    }

    bool periodic = ep->type == TENDRIL_ENDPOINT_XFER_INT || ep->type == TENDRIL_ENDPOINT_XFER_ISOC;
    uint8_t binterval = desc->interval;

    if (tendril_device_is_ss(dev)) {
        if (ep->companion != NULL) {
            ep->max_burst = ep->companion->max_burst > 15 ? 15 : ep->companion->max_burst;
            if (ep->type == TENDRIL_ENDPOINT_XFER_BULK) {
                unsigned int n = ep->companion->attributes & 0x1f;
                ep->max_streams = n == 0 ? 0 : (uint32_t)1 << MIN(n, 16);
            } else if (ep->type == TENDRIL_ENDPOINT_XFER_ISOC && !(ep->companion->attributes & 0x80)) {
                ep->mult = (uint8_t)((ep->companion->attributes & 0x3) + 1);
            }
        }
        if (periodic) {
            if (ep->ssp_companion != NULL) {
                ep->max_esit_payload = tendril_le32(ep->ssp_companion->bytes_per_interval);
            } else if (ep->companion != NULL) {
                ep->max_esit_payload = tendril_le16(ep->companion->bytes_per_interval);
            } else {
                ep->max_esit_payload = (uint32_t)ep->max_packet * (ep->max_burst + (uint32_t)1) * ep->mult;
            }
        }
    } else if (dev->speed == TENDRIL_SPEED_HIGH && periodic) {
        ep->mult = (uint8_t)(((wmps & TENDRIL_ENDPOINT_MAXP_MULT_MASK) >> TENDRIL_ENDPOINT_MAXP_MULT_SHIFT) + 1);
        if (ep->mult > 3) {
            ep->mult = 3;
        }
        ep->max_esit_payload = (uint32_t)ep->max_packet * ep->mult;
    } else if (periodic) {
        ep->max_esit_payload = ep->max_packet;
    }

    if (!periodic) {
        /* HS bulk/control OUT NAK rate, used by xHCI only. */
        ep->interval_log2 = 0;
        ep->interval = 0;
        if (dev->speed == TENDRIL_SPEED_HIGH && binterval != 0 && ep->type != TENDRIL_ENDPOINT_XFER_ISOC) {
            ep->interval_log2 = (uint8_t)MIN(ilog2(binterval), 15);
        }
        return;
    }

    if (binterval == 0) {
        binterval = 1;
    }
    if (dev->speed <= TENDRIL_SPEED_FULL) {
        if (ep->type == TENDRIL_ENDPOINT_XFER_INT) {
            /* xHCI 1.2, 6.2.3.6: round bInterval * 8 down to a power of two. */
            ep->interval_log2 = (uint8_t)ilog2((uint32_t)binterval * 8);
        } else {
            ep->interval_log2 = (uint8_t)(MIN(binterval, 16) - 1 + 3);
        }
    } else {
        ep->interval_log2 = (uint8_t)(MIN(binterval, 16) - 1);
    }
    if (ep->interval_log2 > 15) {
        ep->interval_log2 = 15;
    }
    ep->interval = (uint32_t)1 << ep->interval_log2;
}

/* Control transfers for internal state machines */

static void control_async_done(struct tendril_transfer *xfer) {
    struct tendril_async *async = xfer->context;
    async->status = xfer->status;
    async->work.fn(&async->work);
}

static int control_async_ms(
    struct tendril_device *dev, struct tendril_transfer *xfer, uint8_t request_type, uint8_t request, uint16_t value,
    uint16_t index, void *data, uint16_t length, struct tendril_async *async, uint32_t timeout_ms
) {
    tendril_transfer_init(xfer, &dev->ep0);
    xfer->setup.request_type = request_type;
    xfer->setup.request = request;
    xfer->setup.value = tendril_le16(value);
    xfer->setup.index = tendril_le16(index);
    xfer->setup.length = tendril_le16(length);
    xfer->buffer = data;
    xfer->length = length;
    xfer->timeout_ms = timeout_ms;
    xfer->callback = control_async_done;
    xfer->context = async;
    return tendril_transfer_submit(xfer);
}

int tendril_control_async(
    struct tendril_device *dev, struct tendril_transfer *xfer, uint8_t request_type, uint8_t request, uint16_t value,
    uint16_t index, void *data, uint16_t length, struct tendril_async *async
) {
    return control_async_ms(dev, xfer, request_type, request, value, index, data, length, async, CTRL_TIMEOUT_MS);
}

/* Enumeration */

static bool ensure_buf(struct tendril_device *dev, size_t size) {
    if (dev->buf_size >= size) {
        return true;
    }
    uint8_t *buf = tendril_zalloc(size);
    if (buf == NULL) {
        return false;
    }
    tendril_free(dev->buf, dev->buf_size);
    dev->buf = buf;
    dev->buf_size = size;
    return true;
}

static void dev_name(struct tendril_device *dev, char *buf, size_t size) {
    tendril_snprintf(
        buf, size, "device %u (%04x:%04x)", dev->address,
        tendril_le16(dev->desc.vendor_id), tendril_le16(dev->desc.product_id)
    );
}

static void enum_fail(struct tendril_device *dev, int status, const char *what) {
    char name[48];
    dev_name(dev, name, sizeof(name));
    if (status != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN("%s: %s failed: %s", name, what, tendril_status_str(status));
    }
    dev->step = STEP_NONE;
    tendril_enum_done(dev->port, dev, status);
    if (!dev->gone) {
        tendril_device_disconnect(dev);
    }
}

static void get_descriptor(
    struct tendril_device *dev, int step, uint8_t type, uint8_t index, uint16_t langid, uint16_t length
) {
    dev->step = step;
    dev->op_busy = true;
    if (!ensure_buf(dev, length)) {
        enum_fail(dev, TENDRIL_ERR_NO_MEMORY, "buffer allocation");
        return;
    }
    int st = tendril_control_async(
        dev, &dev->xfer, TENDRIL_DIR_IN | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
        TENDRIL_REQ_GET_DESCRIPTOR, (uint16_t)(type << 8 | index), langid,
        dev->buf, length, &dev->async
    );
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, "GET_DESCRIPTOR");
    }
}

static void hc_op(
    struct tendril_device *dev, int step,
    int (*op)(struct tendril_hc *, struct tendril_device *, struct tendril_async *), const char *what
) {
    struct tendril_hc *hc = dev->hc;
    dev->step = step;
    dev->op_busy = true;
    hc_lock(hc);
    int st = hc->dead ? TENDRIL_ERR_NO_DEVICE : op(hc, dev, &dev->async);
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, what);
    }
}

static void dev_wait(struct tendril_device *dev, int step, uint64_t ms) {
    struct tendril_hc *hc = dev->hc;
    dev->step = step;
    dev->op_busy = true;
    dev->async.status = TENDRIL_OK;
    hc_lock(hc);
    tendril_timer_arm(hc, &dev->timer, ms * NS_PER_MS);
    hc_unlock(hc);
}

static void read_descriptor_8(struct tendril_device *dev) {
    get_descriptor(dev, STEP_DESC8, TENDRIL_DT_DEVICE, 0, 0, 8);
}

static void read_device_descriptor(struct tendril_device *dev) {
    get_descriptor(dev, STEP_DESC18, TENDRIL_DT_DEVICE, 0, 0, sizeof(struct tendril_device_descriptor));
}

static void next_config(struct tendril_device *dev);
static void after_configs(struct tendril_device *dev);
static void next_string(struct tendril_device *dev);
static void choose_and_configure(struct tendril_device *dev);

void tendril_device_enumerate(struct tendril_device *dev) {
    char name[48];
    dev_name(dev, name, sizeof(name));
    LOG_DEBUG(
        "new %s device on root port %u, depth %u", tendril_speed_str(dev->speed),
        dev->root_port, dev->depth
    );
    hc_op(dev, STEP_INIT, dev->hc->ops->device_init, "device setup");
}

static void addressed(struct tendril_device *dev) {
    dev->state = DEV_ADDRESSED;
    tendril_enum_release_dev(dev);
    read_device_descriptor(dev);
}

static bool parse_device_descriptor(struct tendril_device *dev, size_t actual) {
    const struct tendril_device_descriptor *d = (const void *)dev->buf;
    if (actual < sizeof(struct tendril_device_descriptor) || d->type != TENDRIL_DT_DEVICE) {
        return false;
    }
    memcpy(&dev->desc, d, sizeof(struct tendril_device_descriptor));
    dev->quirks |= tendril_device_quirk_lookup(&dev->desc);
    size_t n = dev->desc.num_configurations;
    if (n == 0) {
        return false;
    }
    if (n > MAX_CONFIGS) {
        n = MAX_CONFIGS;
    }
    dev->configs = tendril_zalloc(n * sizeof(uint8_t *));
    dev->config_sizes = tendril_zalloc(n * sizeof(size_t));
    if (dev->configs == NULL || dev->config_sizes == NULL) {
        tendril_free(dev->configs, n * sizeof(uint8_t *));
        tendril_free(dev->config_sizes, n * sizeof(size_t));
        dev->configs = NULL;
        dev->config_sizes = NULL;
        return false;
    }
    dev->num_configs = n;
    return true;
}

static void next_config(struct tendril_device *dev) {
    if (dev->index >= dev->num_configs) {
        after_configs(dev);
        return;
    }
    get_descriptor(
        dev, STEP_CONFIG_HEAD, TENDRIL_DT_CONFIGURATION, (uint8_t)dev->index, 0,
        sizeof(struct tendril_config_descriptor)
    );
}

static void after_configs(struct tendril_device *dev) {
    uint16_t bcd = tendril_le16(dev->desc.bcd_usb);
    if (bcd >= 0x0201) {
        get_descriptor(dev, STEP_BOS_HEAD, TENDRIL_DT_BOS, 0, 0, sizeof(struct tendril_bos_descriptor));
        return;
    }
    if (dev->quirks & TENDRIL_DEV_QUIRK_NO_STRINGS) {
        choose_and_configure(dev);
        return;
    }
    get_descriptor(dev, STEP_LANGID, TENDRIL_DT_STRING, 0, 0, 255);
}

static void after_bos(struct tendril_device *dev) {
    if (dev->quirks & TENDRIL_DEV_QUIRK_NO_STRINGS) {
        choose_and_configure(dev);
        return;
    }
    get_descriptor(dev, STEP_LANGID, TENDRIL_DT_STRING, 0, 0, 255);
}

static uint8_t string_index(struct tendril_device *dev, size_t which) {
    switch (which) {
        case 0: {
            return dev->desc.manufacturer_string;
        }
        case 1: {
            return dev->desc.product_string;
        }
        case 2: {
            return dev->desc.serial_string;
        }
        default: {
            return 0;
        }
    }
}

static void next_string(struct tendril_device *dev) {
    while (dev->index < 3 && string_index(dev, dev->index) == 0) {
        dev->index++;
    }
    if (dev->index >= 3) {
        choose_and_configure(dev);
        return;
    }
    get_descriptor(dev, STEP_STRING, TENDRIL_DT_STRING, string_index(dev, dev->index), dev->langid, 255);
}

/*
 * Decodes a UTF-16LE string descriptor (USB 2.0 9.6.7) into newly allocated
 * UTF-8, with control characters, which could drive a terminal the string is
 * logged to, replaced by '?'.
 */
static char *decode_string(const uint8_t *buf, size_t actual, bool swap) {
    if (actual < 2 || buf[0] < 2 || buf[1] != TENDRIL_DT_STRING) {
        return NULL;
    }
    size_t len = MIN((size_t)buf[0], actual);
    size_t units = (len - 2) / 2;
    /* At most 126 units of up to 3 bytes each, a surrogate pair taking two for 4. */
    char tmp[3 * 126 + 1];
    size_t o = 0;

    for (size_t i = 0; i < units; i++) {
        uint32_t c = (uint32_t)buf[2 + i * 2] | (uint32_t)buf[3 + i * 2] << 8;
        if (swap) {
            c = ((c & 0xff) << 8) | (c >> 8);
        }
        if (c >= 0xd800 && c < 0xdc00 && i + 1 < units) {
            uint32_t lo = (uint32_t)buf[4 + i * 2] | (uint32_t)buf[5 + i * 2] << 8;
            if (lo >= 0xdc00 && lo < 0xe000) {
                c = 0x10000 + ((c - 0xd800) << 10) + (lo - 0xdc00);
                i++;
            }
        }
        if (c >= 0xd800 && c < 0xe000) {
            c = 0xfffd;
        }
        if (c == 0) {
            break;
        }
        if (c < 0x20 || (c >= 0x7f && c < 0xa0)) {
            c = '?';
        }
        if (c < 0x80) {
            tmp[o++] = (char)c;
        } else if (c < 0x800) {
            tmp[o++] = (char)(0xc0 | c >> 6);
            tmp[o++] = (char)(0x80 | (c & 0x3f));
        } else if (c < 0x10000) {
            tmp[o++] = (char)(0xe0 | c >> 12);
            tmp[o++] = (char)(0x80 | ((c >> 6) & 0x3f));
            tmp[o++] = (char)(0x80 | (c & 0x3f));
        } else {
            tmp[o++] = (char)(0xf0 | c >> 18);
            tmp[o++] = (char)(0x80 | ((c >> 12) & 0x3f));
            tmp[o++] = (char)(0x80 | ((c >> 6) & 0x3f));
            tmp[o++] = (char)(0x80 | (c & 0x3f));
        }
    }
    /* Trailing spaces are common padding. */
    while (o > 0 && tmp[o - 1] == ' ') {
        o--;
    }
    tmp[o] = '\0';
    return tendril_strdup(tmp);
}

/* Configuration selection and setup */

static bool has_endpoints(struct tendril_device *dev, size_t index) {
    const uint8_t *end = dev->configs[index] + dev->config_sizes[index];
    return tendril_desc_find(dev->configs[index], end, TENDRIL_DT_ENDPOINT) != NULL;
}

/* BEGIN FreeBSD sys/dev/usb/usb_device.c */
/*
 * The first configuration, unless a quirk names another, as FreeBSD's
 * sys/dev/usb/usb_device.c (BSD-2-Clause, usb_alloc_device) chooses; like
 * there, one with fewer than two interfaces and no endpoints is passed over
 * for the next.
 */
static int choose_config(struct tendril_device *dev, bool *by_quirk) {
    static const uint32_t index_quirks[] = {
        TENDRIL_DEV_QUIRK_CONFIG_INDEX_0, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1, TENDRIL_DEV_QUIRK_CONFIG_INDEX_2,
        TENDRIL_DEV_QUIRK_CONFIG_INDEX_3, TENDRIL_DEV_QUIRK_CONFIG_INDEX_4
    };
    *by_quirk = false;
    for (size_t i = 0; i < ARRAY_SIZE(index_quirks); i++) {
        if ((dev->quirks & index_quirks[i]) && i < dev->num_configs && dev->configs[i] != NULL) {
            *by_quirk = true;
            return (int)i;
        }
    }
    for (size_t i = 0; i < dev->num_configs; i++) {
        if (dev->configs[i] == NULL) {
            continue;
        }
        const struct tendril_config_descriptor *cd = (const void *)dev->configs[i];
        if (i + 1 < dev->num_configs && cd->num_interfaces < 2 && !has_endpoints(dev, i)) {
            continue;
        }
        return (int)i;
    }
    for (size_t i = 0; i < dev->num_configs; i++) {
        if (dev->configs[i] != NULL) {
            return (int)i;
        }
    }
    return -1;
}

/*
 * Whether the port can power configuration `index`, which FreeBSD's
 * sys/dev/usb/usb_device.c (BSD-2-Clause, usbd_set_config_index) checks too.
 * bMaxPower counts 2 mA units, or 8 mA at SuperSpeed (USB 3.2 revision 1.1
 * 9.6.3); a port gives a high-power device five unit loads of 100 mA (USB 2.0
 * 7.2.1), or six of 150 mA at SuperSpeed (USB 3.2 revision 1.1 11.4), except
 * under a bus-powered hub, which gives one.
 */
static bool power_ok(struct tendril_device *dev, size_t index) {
    const struct tendril_config_descriptor *cd = (const void *)dev->configs[index];
    bool ss = tendril_device_is_ss(dev);
    unsigned int need = (unsigned int)cd->max_power * (ss ? 8 : 2);
    unsigned int have = ss ? 900 : 500;
    if (dev->parent != NULL) {
        have = ss ? dev->parent->hub->port_power_ss : dev->parent->hub->port_power;
    }
    return need <= have;
}
/* END FreeBSD sys/dev/usb/usb_device.c */

static struct tendril_interface *find_interface(struct tendril_device *dev, uint8_t number) {
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        if (dev->interfaces[i].number == number) {
            return &dev->interfaces[i];
        }
    }
    return NULL;
}

/* Builds interface objects for configuration `index`, with no endpoints yet. */
static int build_interfaces(struct tendril_device *dev, size_t index) {
    const uint8_t *config = dev->configs[index];
    const uint8_t *end = config + dev->config_sizes[index];

    /* Count distinct interface numbers rather than trusting bNumInterfaces. */
    size_t count = 0;
    uint8_t seen[32] = { 0 };
    for (const uint8_t *p = tendril_desc_find(config, end, TENDRIL_DT_INTERFACE); p != NULL;
        p = tendril_desc_find(tendril_desc_next(p, end), end, TENDRIL_DT_INTERFACE)) {
        if (p[0] < sizeof(struct tendril_interface_descriptor)) {
            continue;
        }
        uint8_t n = ((const struct tendril_interface_descriptor *)(const void *)p)->interface_number;
        if (!(seen[n / 8] & ((uint32_t)1 << (n % 8)))) {
            seen[n / 8] |= (uint8_t)((uint32_t)1 << (n % 8));
            count++;
        }
    }
    if (count == 0) {
        return TENDRIL_OK;
    }

    dev->interfaces = tendril_zalloc(count * sizeof(struct tendril_interface));
    if (dev->interfaces == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }

    const struct tendril_interface_assoc_descriptor *iad = NULL;
    for (const uint8_t *p = tendril_desc_next(config, end); p != NULL; p = tendril_desc_next(p, end)) {
        if (p[1] == TENDRIL_DT_INTERFACE_ASSOCIATION && p[0] >= sizeof(struct tendril_interface_assoc_descriptor)) {
            iad = (const void *)p;
            continue;
        }
        if (p[1] != TENDRIL_DT_INTERFACE || p[0] < sizeof(struct tendril_interface_descriptor)) {
            continue;
        }
        const struct tendril_interface_descriptor *id = (const void *)p;
        struct tendril_interface *intf = find_interface(dev, id->interface_number);
        if (intf == NULL) {
            intf = &dev->interfaces[dev->num_interfaces++];
            intf->dev = dev;
            intf->number = id->interface_number;
            if (iad != NULL && id->interface_number >= iad->first_interface
             && id->interface_number < iad->first_interface + iad->interface_count) {
                intf->iad = iad;
            }
        }

        /* The alternate setting runs up to the next interface descriptor. */
        const uint8_t *q = tendril_desc_next(p, end);
        while (q != NULL && q[1] != TENDRIL_DT_INTERFACE && q[1] != TENDRIL_DT_INTERFACE_ASSOCIATION) {
            q = tendril_desc_next(q, end);
        }
        size_t size = (size_t)((q != NULL ? q : end) - p);

        size_t num = intf->num_alts + 1;
        const struct tendril_interface_descriptor **alts = tendril_zalloc(
            num * sizeof(struct tendril_interface_descriptor *)
        );
        size_t *sizes = tendril_zalloc(num * sizeof(size_t));
        if (alts == NULL || sizes == NULL) {
            tendril_free(alts, num * sizeof(struct tendril_interface_descriptor *));
            tendril_free(sizes, num * sizeof(size_t));
            return TENDRIL_ERR_NO_MEMORY;
        }
        if (intf->num_alts != 0) {
            memcpy(alts, intf->alts, intf->num_alts * sizeof(struct tendril_interface_descriptor *));
            memcpy(sizes, intf->alt_sizes, intf->num_alts * sizeof(size_t));
            tendril_free(intf->alts, intf->num_alts * sizeof(struct tendril_interface_descriptor *));
            tendril_free(intf->alt_sizes, intf->num_alts * sizeof(size_t));
        }
        alts[intf->num_alts] = id;
        sizes[intf->num_alts] = size;
        intf->alts = alts;
        intf->alt_sizes = sizes;
        intf->num_alts++;
    }

    /* Start in the alternate setting numbered 0, or the first one listed. */
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        intf->cur_alt = 0;
        for (size_t a = 0; a < intf->num_alts; a++) {
            if (intf->alts[a]->alternate_setting == 0) {
                intf->cur_alt = a;
                break;
            }
        }
    }
    return TENDRIL_OK;
}

/*
 * The next endpoint descriptor of an alternate setting worth an endpoint
 * object: well formed, not for endpoint 0, and with an address not in `seen`.
 */
static const uint8_t *next_endpoint(const uint8_t *q, const uint8_t *end, uint32_t *seen) {
    for (q = tendril_desc_find(tendril_desc_next(q, end), end, TENDRIL_DT_ENDPOINT); q != NULL;
        q = tendril_desc_find(tendril_desc_next(q, end), end, TENDRIL_DT_ENDPOINT)) {
        if (q[0] < sizeof(struct tendril_endpoint_descriptor) || (q[2] & TENDRIL_ENDPOINT_NUMBER_MASK) == 0) {
            continue;
        }
        unsigned int index = (q[2] & TENDRIL_ENDPOINT_NUMBER_MASK) | (q[2] & TENDRIL_ENDPOINT_DIR_MASK ? 16 : 0);
        uint32_t bit = (uint32_t)1 << index;
        if (!(*seen & bit)) {
            *seen |= bit;
            return q;
        }
    }
    return NULL;
}

/* Creates endpoint objects for alternate setting `alt` of an interface. */
int tendril_interface_build_endpoints(
    struct tendril_interface *intf, size_t alt, struct tendril_endpoint **eps, size_t *count
) {
    const uint8_t *p = (const uint8_t *)intf->alts[alt];
    const uint8_t *end = p + intf->alt_sizes[alt];
    uint32_t seen = 0;
    size_t n = 0;

    for (const uint8_t *q = next_endpoint(p, end, &seen); q != NULL; q = next_endpoint(q, end, &seen)) {
        n++;
    }
    *count = n;
    *eps = NULL;
    if (n == 0) {
        return TENDRIL_OK;
    }
    struct tendril_endpoint *arr = tendril_zalloc(n * sizeof(struct tendril_endpoint));
    if (arr == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    size_t i = 0;
    seen = 0;
    for (const uint8_t *q = next_endpoint(p, end, &seen); q != NULL; q = next_endpoint(q, end, &seen)) {
        tendril_endpoint_setup(&arr[i++], intf->dev, intf, (const void *)q, end);
    }
    *eps = arr;
    return TENDRIL_OK;
}

#define MAX_DEVICE_ENDPOINTS 31

/*
 * Whether the controller should set an endpoint up next to those in `list`
 * and those open on interfaces other than `skip`: not when it takes no
 * transfers, having a packet size of 0, nor when another has its address.
 */
static bool endpoint_addable(
    struct tendril_device *dev, struct tendril_interface *skip, struct tendril_endpoint *ep,
    struct tendril_endpoint **list, size_t n
) {
    if (ep->max_packet == 0 || n >= MAX_DEVICE_ENDPOINTS) {
        return false;
    }
    for (size_t i = 0; i < n; i++) {
        if (list[i]->address == ep->address) {
            return false;
        }
    }
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (size_t e = 0; intf != skip && e < intf->num_endpoints; e++) {
            if (intf->endpoints[e].open && intf->endpoints[e].address == ep->address) {
                return false;
            }
        }
    }
    return true;
}

/* Collects the endpoints of the current alternate settings to set up into `list`. */
static size_t collect_endpoints(struct tendril_device *dev, struct tendril_endpoint **list) {
    size_t n = 0;
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (size_t e = 0; e < intf->num_endpoints; e++) {
            if (endpoint_addable(dev, NULL, &intf->endpoints[e], list, n)) {
                list[n++] = &intf->endpoints[e];
            }
        }
    }
    return n;
}

/* Collects the endpoints set up on the controller into `list`, from `intf` only unless NULL. */
static size_t collect_open_endpoints(
    struct tendril_device *dev, struct tendril_interface *only, struct tendril_endpoint **list
) {
    size_t n = 0;
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (size_t e = 0; (only == NULL || intf == only) && e < intf->num_endpoints; e++) {
            if (intf->endpoints[e].open && n < MAX_DEVICE_ENDPOINTS) {
                list[n++] = &intf->endpoints[e];
            }
        }
    }
    return n;
}

/* Enumeration ends with the device in use, configured or, failing that, not. */
static void enum_finish(struct tendril_device *dev) {
    dev->step = STEP_RUNNING;
    tendril_enum_done(dev->port, dev, TENDRIL_OK);
    tendril_bind_interfaces(dev);
    dev->announced = true;
    tendril_notify_added(dev);
}

static bool is_hub(struct tendril_device *dev) {
    if (dev->desc.device_class == TENDRIL_CLASS_HUB) {
        return true;
    }
    if (dev->num_interfaces == 0) {
        return false;
    }
    struct tendril_interface *intf = &dev->interfaces[0];
    return intf->alts[intf->cur_alt]->interface_class == TENDRIL_CLASS_HUB;
}

static void configure_endpoints(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    struct tendril_endpoint *add[MAX_DEVICE_ENDPOINTS];
    size_t num_add = collect_endpoints(dev, add);
    dev->step = STEP_CONFIGURE;
    dev->op_busy = true;
    hc_lock(hc);
    int st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->configure(hc, dev, NULL, 0, add, num_add, &dev->async);
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        free_interfaces(dev);
        dev->active_config = -1;
        enum_fail(dev, st, "endpoint setup");
    }
}

static void choose_and_configure(struct tendril_device *dev) {
    char name[48];

    dev_name(dev, name, sizeof(name));
    LOG_INFO(
        "%s: %s %s%s%s%s", name, tendril_speed_str(dev->speed),
        dev->manufacturer != NULL ? dev->manufacturer : "",
        dev->manufacturer != NULL ? " " : "",
        dev->product != NULL ? dev->product : "",
        dev->product == NULL && dev->manufacturer == NULL ? "(no strings)" : ""
    );

    bool by_quirk;
    int index = choose_config(dev, &by_quirk);
    /* BEGIN FreeBSD sys/dev/usb/usb_device.c */
    if (index < 0) {
        /* FreeBSD's usb_alloc_device keeps a device without configurations too, unconfigured. */
        LOG_WARN("%s: no usable configuration; left unconfigured", name);
        enum_finish(dev);
        return;
    }
    /* Where FreeBSD's sys/dev/usb/usb_device.c (BSD-2-Clause, usb_alloc_device) looks for one. */
    dev->cdrom_check = !by_quirk && index == 0 && dev->num_configs > 1;
    const struct tendril_config_descriptor *cd = (const void *)dev->configs[index];
    if (!power_ok(dev, (size_t)index)) {
        /* FreeBSD keeps such a device too, unconfigured. */
        LOG_WARN(
            "%s: configuration %u needs more power than the port has; left unconfigured", name,
            cd->configuration_value
        );
        enum_finish(dev);
        return;
    }
    /* END FreeBSD sys/dev/usb/usb_device.c */
    int st = build_interfaces(dev, (size_t)index);
    if (st == TENDRIL_OK) {
        for (size_t i = 0; i < dev->num_interfaces && st == TENDRIL_OK; i++) {
            struct tendril_interface *intf = &dev->interfaces[i];
            st = tendril_interface_build_endpoints(intf, intf->cur_alt, &intf->endpoints, &intf->num_endpoints);
        }
    }
    if (st != TENDRIL_OK) {
        free_interfaces(dev);
        enum_fail(dev, st, "configuration parsing");
        return;
    }
    dev->active_config = index;

    /* BEGIN FreeBSD sys/dev/usb/usb_device.c */
    /*
     * As in FreeBSD's sys/dev/usb/usb_device.c (BSD-2-Clause,
     * usbd_set_config_index), a device that may draw on both power sources
     * says which it uses in GET_STATUS (USB 2.0 9.4.5), which only matters for
     * a hub.
     */
    dev->self_powered = !(dev->quirks & TENDRIL_DEV_QUIRK_BUS_POWERED)
        && (cd->attributes & TENDRIL_CONFIG_ATTR_SELF_POWERED) != 0;
    if (dev->self_powered && (cd->attributes & TENDRIL_CONFIG_ATTR_BUS_POWERED) && is_hub(dev)) {
        dev->self_powered = false;
        dev->step = STEP_SELF_POWER;
        dev->op_busy = true;
        st = tendril_control_async(
            dev, &dev->xfer, TENDRIL_DIR_IN | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
            TENDRIL_REQ_GET_STATUS, 0, 0, dev->buf, 2, &dev->async
        );
        if (st != TENDRIL_OK) {
            dev->op_busy = false;
            free_interfaces(dev);
            dev->active_config = -1;
            enum_fail(dev, st, "GET_STATUS");
        }
        return;
    }
    /* END FreeBSD sys/dev/usb/usb_device.c */
    configure_endpoints(dev);
}

static void set_configuration(struct tendril_device *dev) {
    const struct tendril_config_descriptor *cd = (const void *)dev->configs[dev->active_config];
    dev->step = STEP_SET_CONFIG;
    dev->op_busy = true;
    int st = tendril_control_async(
        dev, &dev->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
        TENDRIL_REQ_SET_CONFIGURATION, cd->configuration_value, 0, NULL, 0,
        &dev->async
    );
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, "SET_CONFIGURATION");
    }
}

/* BEGIN FreeBSD sys/dev/usb/usb_device.c, usb_msctest.c */
/*
 * FreeBSD's UQ_MSC_DYMO_EJECT, per its sys/dev/usb/usb_device.c and
 * usb_msctest.c (BSD-2-Clause, usb_dymo_eject()): these bytes, written to the
 * first interface, a HID one, switch the label printer to its other mode, and
 * the device is left unconfigured to come back as that. False if not started.
 */
static bool dymo_eject(struct tendril_device *dev) {
    static const uint8_t command[3] = { 0x1b, 0x5a, 0x01 };

    if (dev->num_interfaces == 0) {
        return false;
    }
    struct tendril_interface *intf = &dev->interfaces[0];
    const struct tendril_interface_descriptor *id = intf->alts[intf->cur_alt];
    if (id->interface_class != TENDRIL_CLASS_HID || id->interface_subclass != 0) {
        return false;
    }
    struct tendril_endpoint *ep = NULL;
    for (size_t e = 0; e < intf->num_endpoints && ep == NULL; e++) {
        struct tendril_endpoint *cand = &intf->endpoints[e];
        if (cand->open && !(cand->address & TENDRIL_ENDPOINT_DIR_MASK)
         && (cand->type == TENDRIL_ENDPOINT_XFER_INT || cand->type == TENDRIL_ENDPOINT_XFER_BULK)) {
            ep = cand;
        }
    }
    if (ep == NULL || !ensure_buf(dev, sizeof(command))) {
        return false;
    }
    memcpy(dev->buf, command, sizeof(command));
    tendril_transfer_init(&dev->xfer, ep);
    dev->xfer.buffer = dev->buf;
    dev->xfer.length = sizeof(command);
    dev->xfer.timeout_ms = 1000;
    dev->xfer.callback = control_async_done;
    dev->xfer.context = &dev->async;
    dev->step = STEP_EJECT;
    dev->op_busy = true;
    if (tendril_transfer_submit(&dev->xfer) != TENDRIL_OK) {
        dev->op_busy = false;
        return false;
    }
    return true;
}
/* END FreeBSD sys/dev/usb/usb_device.c, usb_msctest.c */

static void configured_continue(struct tendril_device *dev) {
    char name[48];

    if (is_hub(dev) && dev->num_interfaces > 0) {
        /* BEGIN FreeBSD sys/dev/usb/usb_hub.c */
        /* FreeBSD's sys/dev/usb/usb_hub.c (BSD-2-Clause, uhub_attach) leaves these hubs unused too. */
        struct tendril_device *parent = dev->parent;
        const char *why = NULL;
        if (dev->depth > MAX_HUB_DEPTH) {
            why = "too deep";
        } else if (!dev->self_powered && parent != NULL && !parent->self_powered) {
            why = "bus-powered behind a bus-powered hub";
        }
        if (why != NULL) {
            dev_name(dev, name, sizeof(name));
            LOG_WARN("%s: hub ignored: %s", name, why);
            enum_finish(dev);
            return;
        }
        /* END FreeBSD sys/dev/usb/usb_hub.c */
        dev->step = STEP_HUB;
        dev->op_busy = true;
        tendril_hub_start(dev);
        return;
    }
    enum_finish(dev);
}

static void configured(struct tendril_device *dev) {
    dev->state = DEV_CONFIGURED;
    /* BEGIN FreeBSD sys/dev/usb/usb_device.c */
    if ((dev->quirks & TENDRIL_DEV_QUIRK_DYMO_EJECT) && dymo_eject(dev)) {
        return;
    }
    /* END FreeBSD sys/dev/usb/usb_device.c */
    configured_continue(dev);
}

/* Called by the hub driver when the hub finished initializing. */
void tendril_device_hub_ready(struct tendril_device *dev, int status) {
    dev->op_busy = false;
    if (dev->gone) {
        start_teardown(dev);
        return;
    }
    if (status != TENDRIL_OK) {
        enum_fail(dev, status, "hub setup");
        return;
    }
    enum_finish(dev);
}

static void control_step(
    struct tendril_device *dev, int step, uint8_t request_type, uint8_t request, uint16_t value,
    uint16_t index, uint32_t timeout_ms, const char *what
) {
    dev->step = step;
    dev->op_busy = true;
    int st = control_async_ms(
        dev, &dev->xfer, request_type, request, value, index, NULL, 0,
        &dev->async, timeout_ms
    );
    if (st != TENDRIL_OK) {
        dev->op_busy = false;
        enum_fail(dev, st, what);
    }
}

static bool descriptor_ok(int st) {
    return st == TENDRIL_OK || st == TENDRIL_ERR_SHORT;
}

static void dev_step(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    int st = dev->async.status;
    size_t actual = dev->xfer.actual;

    switch (dev->step) {
        case STEP_INIT: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "device setup");
                break;
            }
            if (hc->ops->device_address != NULL) {
                hc_op(dev, STEP_HC_ADDRESS, hc->ops->device_address, "address assignment");
                break;
            }
            read_descriptor_8(dev);
            break;
        }
        case STEP_HC_ADDRESS: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "address assignment");
                break;
            }
            /* BEGIN FreeBSD sys/dev/usb/usb_request.c */
            /* The settling time FreeBSD gives SET_ADDRESS when the controller sends it too. */
            dev_wait(dev, STEP_HC_ADDRESS_WAIT, SET_ADDRESS_SETTLE_MS);
            /* END FreeBSD sys/dev/usb/usb_request.c */
            break;
        }
        case STEP_HC_ADDRESS_WAIT: {
            /* xHCI 1.2, 4.3, step 7: only full-speed EP0 packet sizes vary. */
            if (dev->speed == TENDRIL_SPEED_FULL) {
                dev->state = DEV_ADDRESSED;
                tendril_enum_release_dev(dev);
                read_descriptor_8(dev);
                break;
            }
            addressed(dev);
            break;
        }
        case STEP_DESC8: {
            if (!descriptor_ok(st) || actual < 8 || dev->buf[1] != TENDRIL_DT_DEVICE) {
                enum_fail(dev, descriptor_ok(st) ? TENDRIL_ERR_PROTOCOL : st, "reading the device descriptor");
                break;
            }
            /* USB 2.0 5.5.3 and USB 3.2 revision 1.1 9.6.1 fix it for all speeds but full. */
            set_ep0_mps(dev, std_packet_size(TENDRIL_ENDPOINT_XFER_CONTROL, dev->speed, dev->buf[7]));
            if (hc->ops->device_address == NULL) {
                hc_lock(hc);
                dev->new_address = tendril_address_alloc(hc);
                hc_unlock(hc);
                if (dev->new_address == 0) {
                    enum_fail(dev, TENDRIL_ERR_NO_MEMORY, "address allocation");
                    break;
                }
            }
            hc_op(dev, STEP_EP0_MPS, hc->ops->ep0_update, "endpoint 0 update");
            break;
        }
        case STEP_EP0_MPS: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "endpoint 0 update");
                break;
            }
            if (hc->ops->device_address != NULL) {
                read_device_descriptor(dev);
                break;
            }
            /* BEGIN FreeBSD sys/dev/usb/usb_request.c */
            /* EP0 keeps targeting address 0 until STEP_EP0_ADDRESS; FreeBSD allows SET_ADDRESS a second. */
            control_step(
                dev, STEP_SET_ADDRESS, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                TENDRIL_REQ_SET_ADDRESS, dev->new_address, 0, 1000, "SET_ADDRESS"
            );
            /* END FreeBSD sys/dev/usb/usb_request.c */
            break;
        }
        case STEP_SET_ADDRESS: {
            /* BEGIN FreeBSD sys/dev/usb/usb_device.c, usb_request.c */
            /*
             * Like FreeBSD's sys/dev/usb/usb_device.c (BSD-2-Clause,
             * usb_alloc_device), carry on past a failure: some devices take the
             * address before the status stage completes, which reading the
             * descriptor at the new address then shows.
             */
            if (st == TENDRIL_ERR_NO_DEVICE) {
                enum_fail(dev, st, "SET_ADDRESS");
                break;
            }
            dev_wait(dev, STEP_ADDRESS_WAIT, SET_ADDRESS_SETTLE_MS);
            /* END FreeBSD sys/dev/usb/usb_device.c, usb_request.c */
            break;
        }
        case STEP_ADDRESS_WAIT: {
            dev->address = dev->new_address;
            hc_op(dev, STEP_EP0_ADDRESS, hc->ops->ep0_update, "endpoint 0 update");
            break;
        }
        case STEP_EP0_ADDRESS: {
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "endpoint 0 update");
                break;
            }
            addressed(dev);
            break;
        }
        case STEP_DESC18: {
            if (!descriptor_ok(st)) {
                enum_fail(dev, st, "reading the device descriptor");
                break;
            }
            if (!parse_device_descriptor(dev, actual)) {
                enum_fail(dev, TENDRIL_ERR_PROTOCOL, "parsing the device descriptor");
                break;
            }
            dev->index = 0;
            next_config(dev);
            break;
        }
        case STEP_CONFIG_HEAD: {
            const struct tendril_config_descriptor *cd = (const void *)dev->buf;
            if (!descriptor_ok(st) || actual < sizeof(struct tendril_config_descriptor)
             || cd->type != TENDRIL_DT_CONFIGURATION) {
                enum_fail(dev, descriptor_ok(st) ? TENDRIL_ERR_PROTOCOL : st, "reading a configuration descriptor");
                break;
            }
            uint16_t total = tendril_le16(cd->total_length);
            if (total < sizeof(struct tendril_config_descriptor)) {
                total = sizeof(struct tendril_config_descriptor);
            }
            get_descriptor(dev, STEP_CONFIG_FULL, TENDRIL_DT_CONFIGURATION, (uint8_t)dev->index, 0, total);
            break;
        }
        case STEP_CONFIG_FULL: {
            if (!descriptor_ok(st)) {
                enum_fail(dev, st, "reading a configuration descriptor");
                break;
            }
            if (!tendril_config_valid(dev->buf, actual)) {
                char name[48];
                dev_name(dev, name, sizeof(name));
                LOG_WARN("%s: configuration %zu is malformed", name, dev->index);
            } else {
                uint8_t *copy = tendril_glue_alloc(actual);
                if (copy == NULL) {
                    enum_fail(dev, TENDRIL_ERR_NO_MEMORY, "configuration copy");
                    break;
                }
                memcpy(copy, dev->buf, actual);
                /* Keep wTotalLength consistent with what was read. */
                ((struct tendril_config_descriptor *)(void *)copy)->total_length = tendril_le16((uint16_t)actual);
                dev->configs[dev->index] = copy;
                dev->config_sizes[dev->index] = actual;
            }
            dev->index++;
            next_config(dev);
            break;
        }
        case STEP_BOS_HEAD: {
            const struct tendril_bos_descriptor *bd = (const void *)dev->buf;
            if (!descriptor_ok(st) || actual < sizeof(struct tendril_bos_descriptor) || bd->type != TENDRIL_DT_BOS
             || tendril_le16(bd->total_length) < sizeof(struct tendril_bos_descriptor)) {
                after_bos(dev);
                break;
            }
            get_descriptor(dev, STEP_BOS_FULL, TENDRIL_DT_BOS, 0, 0, tendril_le16(bd->total_length));
            break;
        }
        case STEP_BOS_FULL: {
            if (descriptor_ok(st) && actual >= sizeof(struct tendril_bos_descriptor)
             && dev->buf[1] == TENDRIL_DT_BOS) {
                dev->bos = tendril_glue_alloc(actual);
                if (dev->bos != NULL) {
                    memcpy(dev->bos, dev->buf, actual);
                    dev->bos_size = actual;
                }
            }
            after_bos(dev);
            break;
        }
        case STEP_LANGID: {
            if (!descriptor_ok(st) || actual < 4 || dev->buf[1] != TENDRIL_DT_STRING) {
                choose_and_configure(dev);
                break;
            }
            dev->langid = (uint16_t)(dev->buf[2] | dev->buf[3] << 8);
            dev->index = 0;
            next_string(dev);
            break;
        }
        case STEP_STRING: {
            if (descriptor_ok(st)) {
                char *s = decode_string(dev->buf, actual, (dev->quirks & TENDRIL_DEV_QUIRK_SWAP_UNICODE) != 0);
                if (s != NULL) {
                    char **slot = dev->index == 0 ? &dev->manufacturer
                        : dev->index == 1 ? &dev->product
                        : &dev->serial;
                    tendril_strfree(*slot);
                    *slot = s;
                }
            }
            dev->index++;
            next_string(dev);
            break;
        }
        case STEP_SELF_POWER: {
            /* BEGIN FreeBSD sys/dev/usb/usb_device.c */
            /* USB 2.0 9.4.5: bit 0 of the device status; FreeBSD takes a failure as bus-powered. */
            dev->self_powered = st == TENDRIL_OK && actual >= 1 && (dev->buf[0] & 1) != 0;
            /* END FreeBSD sys/dev/usb/usb_device.c */
            configure_endpoints(dev);
            break;
        }
        case STEP_CONFIGURE: {
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                dev->active_config = -1;
                enum_fail(dev, st, "endpoint setup");
                break;
            }
            set_configuration(dev);
            break;
        }
        case STEP_SET_CONFIG: {
            /* BEGIN FreeBSD sys/dev/usb/usb_device.c */
            if (st != TENDRIL_OK && st != TENDRIL_ERR_NO_DEVICE && dev->port->config_failed) {
                /*
                 * Failing again after the device was enumerated anew leaves it
                 * unconfigured, as FreeBSD's sys/dev/usb/usb_device.c
                 * (BSD-2-Clause, usb_alloc_device) does.
                 */
                char name[48];
                dev_name(dev, name, sizeof(name));
                LOG_WARN("%s: SET_CONFIGURATION failed again (%s); left unconfigured", name, tendril_status_str(st));
                control_step(
                    dev, STEP_UNCONFIGURE, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                    TENDRIL_REQ_SET_CONFIGURATION, 0, 0, CTRL_TIMEOUT_MS, "SET_CONFIGURATION"
                );
                break;
            }
            if (st != TENDRIL_OK) {
                dev->port->config_failed = true;
                enum_fail(dev, st, "SET_CONFIGURATION");
                break;
            }
            /* END FreeBSD sys/dev/usb/usb_device.c */
            configured(dev);
            break;
        }
        /* BEGIN FreeBSD sys/dev/usb/usb_device.c */
        case STEP_EJECT: {
            if (st != TENDRIL_OK) {
                configured_continue(dev);
                break;
            }
            char name[48];
            dev_name(dev, name, sizeof(name));
            LOG_INFO("%s: switched to its other mode, left unconfigured", name);
            control_step(
                dev, STEP_UNCONFIGURE, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                TENDRIL_REQ_SET_CONFIGURATION, 0, 0, CTRL_TIMEOUT_MS, "SET_CONFIGURATION"
            );
            break;
        }
        /* END FreeBSD sys/dev/usb/usb_device.c */
        case STEP_UNCONFIGURE: {
            /* The device may be on its way out already; what it answered does not matter. */
            struct tendril_endpoint *drop[MAX_DEVICE_ENDPOINTS];
            size_t num_drop = collect_open_endpoints(dev, NULL, drop);
            dev->step = STEP_UNCONFIGURE_DROP;
            dev->op_busy = true;
            hc_lock(hc);
            st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->configure(hc, dev, drop, num_drop, NULL, 0, &dev->async);
            hc_unlock(hc);
            if (st != TENDRIL_OK) {
                dev->op_busy = false;
                enum_fail(dev, st, "endpoint setup");
            }
            break;
        }
        case STEP_UNCONFIGURE_DROP: {
            free_interfaces(dev);
            dev->active_config = -1;
            dev->state = DEV_ADDRESSED;
            if (st != TENDRIL_OK) {
                enum_fail(dev, st, "endpoint setup");
                break;
            }
            enum_finish(dev);
            break;
        }
        case STEP_TEARDOWN_WAIT: {
            hc_lock(hc);
            bool idle = dev->pending == 0;
            hc_unlock(hc);
            if (idle) {
                start_teardown(dev);
            }
            break;
        }
        case STEP_TEARDOWN_FREE: {
            hc_lock(hc);
            if (hc->ops->device_address == NULL) {
                tendril_address_free(hc, dev->new_address);
            }
            list_del(&dev->node);
            hc_unlock(hc);
            dev->step = STEP_FREED;
            dev->state = DEV_GONE;
            tendril_device_unref(dev);
            break;
        }
        default: {
            break;
        }
    }
}

static void dev_continue(struct tendril_device *dev) {
    dev->op_busy = false;
    if (dev->gone && dev->step < STEP_TEARDOWN_WAIT) {
        start_teardown(dev);
        return;
    }
    dev_step(dev);
}

static void dev_async_work(struct tendril_work *work) {
    dev_continue(CONTAINER_OF(work, struct tendril_device, async.work));
}

static void dev_timer_work(struct tendril_work *work) {
    dev_continue(CONTAINER_OF(work, struct tendril_device, timer.work));
}

/* Teardown */

static void cancel_endpoint(struct tendril_hc *hc, struct tendril_endpoint *ep, int status) {
    struct tendril_list *it;
    LIST_FOR_EACH(it, &ep->transfers) {
        struct tendril_transfer *xfer = CONTAINER_OF(it, struct tendril_transfer, internal.node);
        tendril_transfer_cancel_locked(hc, xfer, status);
    }
}

/* With the lock held. */
static void cancel_all(struct tendril_device *dev, int status, bool include_ep0) {
    struct tendril_hc *hc = dev->hc;
    if (include_ep0) {
        cancel_endpoint(hc, &dev->ep0, status);
    }
    for (size_t i = 0; i < dev->num_interfaces; i++) {
        struct tendril_interface *intf = &dev->interfaces[i];
        for (size_t e = 0; e < intf->num_endpoints; e++) {
            cancel_endpoint(hc, &intf->endpoints[e], status);
        }
    }
}

void tendril_device_disconnect(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;

    if (dev->gone) {
        return;
    }
    dev->gone = true;
    if (dev->hub != NULL) {
        tendril_hub_stop(dev->hub);
    }

    hc_lock(hc);
    cancel_all(dev, TENDRIL_ERR_NO_DEVICE, true);
    bool waiting_timer = list_linked(&dev->timer.node) || list_linked(&dev->timer.work.node);
    if (waiting_timer) {
        tendril_timer_cancel(hc, &dev->timer);
        dev->op_busy = false;
    }
    hc_unlock(hc);

    if (!dev->op_busy) {
        start_teardown(dev);
    }
}

static void start_teardown(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;

    if (dev->step >= STEP_TEARDOWN_FREE) {
        return;
    }
    if (dev->step != STEP_NONE && dev->step < STEP_RUNNING) {
        /* Enumeration was cut short; report it so the slot is released. */
        dev->step = STEP_NONE;
        tendril_enum_done(dev->port, dev, TENDRIL_ERR_NO_DEVICE);
    }
    dev->step = STEP_TEARDOWN_WAIT;

    hc_lock(hc);
    bool idle = dev->pending == 0;
    dev->teardown_wait = !idle;
    hc_unlock(hc);
    if (!idle) {
        return;
    }

    tendril_unbind_interfaces(dev);
    if (dev->announced) {
        dev->announced = false;
        tendril_notify_removed(dev);
    }

    dev->step = STEP_TEARDOWN_FREE;
    dev->op_busy = true;
    hc_lock(hc);
    hc->ops->device_free(hc, dev, &dev->async);
    hc_unlock(hc);
}

/* Called by transfer completion when the last transfer of a gone device finished. */
void tendril_device_transfers_idle(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    hc_lock(hc);
    if (dev->teardown_wait && dev->step == STEP_TEARDOWN_WAIT) {
        dev->teardown_wait = false;
        dev->async.status = TENDRIL_OK;
        tendril_queue_work(hc, &dev->async.work);
    }
    hc_unlock(hc);
}

/* Accessors */

struct tendril_hc *tendril_device_hc(struct tendril_device *dev) {
    return dev->hc;
}

struct tendril_device *tendril_device_parent(struct tendril_device *dev) {
    return dev->parent;
}

size_t tendril_device_port(struct tendril_device *dev) {
    return dev->port->number;
}

unsigned int tendril_device_depth(struct tendril_device *dev) {
    return dev->depth;
}

int tendril_device_speed(struct tendril_device *dev) {
    return dev->speed;
}

uint8_t tendril_device_address(struct tendril_device *dev) {
    return dev->address;
}

uint32_t tendril_device_quirks(struct tendril_device *dev) {
    return dev->quirks;
}

bool tendril_device_gone(struct tendril_device *dev) {
    return dev->gone;
}

const struct tendril_device_descriptor *tendril_device_descriptor(struct tendril_device *dev) {
    return &dev->desc;
}

const struct tendril_config_descriptor *tendril_device_config_descriptor(struct tendril_device *dev, size_t index) {
    if (index >= dev->num_configs) {
        return NULL;
    }
    return (const void *)dev->configs[index];
}

const struct tendril_config_descriptor *tendril_device_active_config(struct tendril_device *dev) {
    if (dev->active_config < 0) {
        return NULL;
    }
    return (const void *)dev->configs[dev->active_config];
}

const struct tendril_bos_descriptor *tendril_device_bos(struct tendril_device *dev) {
    return (const void *)dev->bos;
}

const char *tendril_device_manufacturer(struct tendril_device *dev) {
    return dev->manufacturer;
}

const char *tendril_device_product(struct tendril_device *dev) {
    return dev->product;
}

const char *tendril_device_serial(struct tendril_device *dev) {
    return dev->serial;
}

struct tendril_device *tendril_device_child(struct tendril_device *hub, size_t port) {
    if (hub->hub == NULL || port == 0 || port > hub->hub->num_ports) {
        return NULL;
    }
    return hub->hub->ports[port - 1].device;
}

size_t tendril_device_num_ports(struct tendril_device *hub) {
    return hub->hub != NULL ? hub->hub->num_ports : 0;
}

struct tendril_endpoint *tendril_device_ep0(struct tendril_device *dev) {
    return &dev->ep0;
}

size_t tendril_device_num_interfaces(struct tendril_device *dev) {
    return dev->num_interfaces;
}

struct tendril_interface *tendril_device_interface(struct tendril_device *dev, size_t index) {
    if (index >= dev->num_interfaces) {
        return NULL;
    }
    return &dev->interfaces[index];
}

struct tendril_interface *tendril_device_find_interface(struct tendril_device *dev, uint8_t number) {
    return find_interface(dev, number);
}

struct tendril_device *tendril_interface_device(struct tendril_interface *intf) {
    return intf->dev;
}

const struct tendril_interface_descriptor *tendril_interface_descriptor(struct tendril_interface *intf) {
    return intf->alts[intf->cur_alt];
}

const uint8_t *tendril_interface_descriptors(struct tendril_interface *intf, size_t *size) {
    *size = intf->alt_sizes[intf->cur_alt];
    return (const uint8_t *)intf->alts[intf->cur_alt];
}

const struct tendril_interface_assoc_descriptor *tendril_interface_association(struct tendril_interface *intf) {
    return intf->iad;
}

size_t tendril_interface_num_alts(struct tendril_interface *intf) {
    return intf->num_alts;
}

const struct tendril_interface_descriptor *tendril_interface_alt_descriptor(
    struct tendril_interface *intf, size_t index
) {
    if (index >= intf->num_alts) {
        return NULL;
    }
    return intf->alts[index];
}

size_t tendril_interface_num_endpoints(struct tendril_interface *intf) {
    return intf->num_endpoints;
}

struct tendril_endpoint *tendril_interface_endpoint(struct tendril_interface *intf, size_t index) {
    if (index >= intf->num_endpoints) {
        return NULL;
    }
    return &intf->endpoints[index];
}

struct tendril_endpoint *tendril_interface_find_endpoint(struct tendril_interface *intf, uint8_t type, uint8_t dir) {
    for (size_t i = 0; i < intf->num_endpoints; i++) {
        struct tendril_endpoint *ep = &intf->endpoints[i];
        if (ep->type == type && (ep->address & TENDRIL_ENDPOINT_DIR_MASK) == dir) {
            return ep;
        }
    }
    return NULL;
}

void tendril_interface_set_data(struct tendril_interface *intf, void *data) {
    intf->driver_data = data;
}

void *tendril_interface_data(struct tendril_interface *intf) {
    return intf->driver_data;
}

struct tendril_device *tendril_endpoint_device(struct tendril_endpoint *ep) {
    return ep->dev;
}

const struct tendril_endpoint_descriptor *tendril_endpoint_descriptor(struct tendril_endpoint *ep) {
    return ep->desc;
}

const struct tendril_ss_ep_companion_descriptor *tendril_endpoint_companion(struct tendril_endpoint *ep) {
    return ep->companion;
}

uint8_t tendril_endpoint_address(struct tendril_endpoint *ep) {
    return ep->address;
}

uint8_t tendril_endpoint_type(struct tendril_endpoint *ep) {
    return ep->type;
}

uint16_t tendril_endpoint_max_packet(struct tendril_endpoint *ep) {
    return ep->max_packet;
}

unsigned int tendril_endpoint_max_streams(struct tendril_endpoint *ep) {
    return ep->max_streams;
}

/* Embedder-requested operations */

struct dev_op {
    struct tendril_device *dev;
    struct tendril_async async;
    struct tendril_timer timer;
    struct tendril_transfer xfer;
    int step;
    void (*done)(void *ctx, int status);
    void *ctx;
    /* Endpoint and interface state being replaced. */
    struct tendril_interface *intf;
    size_t alt;
    struct tendril_endpoint *new_eps;
    size_t num_new_eps;
    struct tendril_endpoint *ep;
    uint8_t value;
    char *out;
    size_t out_size;
    /* Power management: a request is out, the give-up time, and the result once undone. */
    bool requested;
    uint64_t deadline;
    int result;
    uint8_t buf[256];
};

static void config_work(struct tendril_work *work);

static struct dev_op *op_alloc(
    struct tendril_device *dev, void (*fn)(struct tendril_work *), void (*done)(void *, int), void *ctx
) {
    struct dev_op *op = tendril_zalloc(sizeof(struct dev_op));
    if (op == NULL) {
        return NULL;
    }
    op->dev = dev;
    op->done = done;
    op->ctx = ctx;
    tendril_async_init(&op->async, fn);
    tendril_device_ref(dev);
    /* Counted like a transfer, so drivers are not disconnected before `done` runs. */
    hc_lock(dev->hc);
    dev->pending++;
    hc_unlock(dev->hc);
    return op;
}

static void op_finish(struct dev_op *op, int status) {
    if (op->async.work.fn == config_work) {
        op->dev->config_busy = false;
        op->dev->config_xfer = NULL;
    }
    void (*done)(void *, int) = op->done;
    void *ctx = op->ctx;
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    tendril_free(op, sizeof(struct dev_op));
    if (done != NULL) {
        done(ctx, status);
    }
    hc_lock(hc);
    dev->pending--;
    bool idle = dev->gone && dev->pending == 0;
    hc_unlock(hc);
    if (idle) {
        tendril_device_transfers_idle(dev);
    }
    tendril_device_unref(dev);
}

static unsigned int pending_on_interface(struct tendril_interface *intf) {
    unsigned int n = 0;
    for (size_t e = 0; e < intf->num_endpoints; e++) {
        struct tendril_list *it;
        LIST_FOR_EACH(it, &intf->endpoints[e].transfers) {
            n++;
        }
    }
    return n;
}

/* Alternate settings */

#define ALT_CONFIGURE 0
#define ALT_SET_INTERFACE 1

static void alt_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    struct tendril_interface *intf = op->intf;
    struct tendril_device *dev = op->dev;
    int st = op->async.status;

    if (dev->gone) {
        tendril_free(op->new_eps, op->num_new_eps * sizeof(struct tendril_endpoint));
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    switch (op->step) {
        case ALT_CONFIGURE: {
            if (st != TENDRIL_OK) {
                tendril_free(op->new_eps, op->num_new_eps * sizeof(struct tendril_endpoint));
                op_finish(op, st);
                return;
            }
            /* The old endpoints are closed; switch to the new ones before the request. */
            tendril_free(intf->endpoints, intf->num_endpoints * sizeof(struct tendril_endpoint));
            intf->endpoints = op->new_eps;
            intf->num_endpoints = op->num_new_eps;
            intf->cur_alt = op->alt;
            for (size_t e = 0; e < intf->num_endpoints; e++) {
                intf->endpoints[e].intf = intf;
            }
            op->new_eps = NULL;
            op->num_new_eps = 0;
            op->step = ALT_SET_INTERFACE;
            st = tendril_control_async(
                dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_INTERFACE,
                TENDRIL_REQ_SET_INTERFACE, intf->alts[op->alt]->alternate_setting, intf->number,
                NULL, 0, &op->async
            );
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case ALT_SET_INTERFACE: {
            /* USB 2.0 9.4.10: a device with one setting may stall the request. */
            if (st == TENDRIL_ERR_STALL && intf->num_alts == 1) {
                st = TENDRIL_OK;
            }
            op_finish(op, st);
            return;
        }
        default: {
            op_finish(op, TENDRIL_ERR_INVALID);
            return;
        }
    }
}

int tendril_interface_set_alt(
    struct tendril_interface *intf, uint8_t alt, void (*done)(void *ctx, int status), void *ctx
) {
    struct tendril_device *dev = intf->dev;
    struct tendril_hc *hc = dev->hc;
    size_t index = intf->num_alts;

    for (size_t a = 0; a < intf->num_alts; a++) {
        if (intf->alts[a]->alternate_setting == alt) {
            index = a;
            break;
        }
    }
    if (index == intf->num_alts) {
        return TENDRIL_ERR_INVALID;
    }
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->suspended) {
        return TENDRIL_ERR_SUSPENDED;
    }
    if (dev->config_busy || dev->pm_busy) {
        return TENDRIL_ERR_BUSY;
    }

    struct dev_op *op = op_alloc(dev, alt_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->intf = intf;
    op->alt = index;
    int st = tendril_interface_build_endpoints(intf, index, &op->new_eps, &op->num_new_eps);
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
        return st;
    }

    struct tendril_endpoint *drop[MAX_DEVICE_ENDPOINTS];
    struct tendril_endpoint *add[MAX_DEVICE_ENDPOINTS];
    size_t num_drop = collect_open_endpoints(dev, intf, drop);
    size_t num_add = 0;
    for (size_t e = 0; e < op->num_new_eps; e++) {
        if (endpoint_addable(dev, intf, &op->new_eps[e], add, num_add)) {
            add[num_add++] = &op->new_eps[e];
        }
    }

    op->step = ALT_CONFIGURE;
    hc_lock(hc);
    if (pending_on_interface(intf) != 0) {
        st = TENDRIL_ERR_BUSY;
    } else if (hc->dead) {
        st = TENDRIL_ERR_NO_DEVICE;
    } else {
        st = hc->ops->configure(hc, dev, drop, num_drop, add, num_add, &op->async);
    }
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        tendril_free(op->new_eps, op->num_new_eps * sizeof(struct tendril_endpoint));
        op->done = NULL;
        op_finish(op, st);
    }
    return st;
}

/* Halts */

#define HALT_CLEAR_FEATURE 0
#define HALT_RESET 1

static void halt_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    int st = op->async.status;

    if (dev->gone) {
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    if (op->step == HALT_CLEAR_FEATURE) {
        if (st != TENDRIL_OK) {
            op_finish(op, st);
            return;
        }
        op->step = HALT_RESET;
        hc_lock(hc);
        st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->endpoint_reset(hc, op->ep, &op->async);
        hc_unlock(hc);
        if (st != TENDRIL_OK) {
            op_finish(op, st);
        }
        return;
    }
    op_finish(op, st);
}

int tendril_endpoint_clear_halt(struct tendril_endpoint *ep, void (*done)(void *ctx, int status), void *ctx) {
    struct tendril_device *dev = ep->dev;

    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    struct dev_op *op = op_alloc(dev, halt_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->ep = ep;
    op->step = HALT_CLEAR_FEATURE;
    int st = tendril_control_async(
        dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_ENDPOINT,
        TENDRIL_REQ_CLEAR_FEATURE, TENDRIL_FEATURE_ENDPOINT_HALT, ep->address,
        NULL, 0, &op->async
    );
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
    }
    return st;
}

/* Streams */

static void streams_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    op_finish(op, op->dev->gone ? TENDRIL_ERR_NO_DEVICE : op->async.status);
}

int tendril_endpoint_alloc_streams(
    struct tendril_endpoint **eps, size_t count, unsigned int *streams, void (*done)(void *ctx, int status), void *ctx
) {
    if (count == 0 || *streams == 0) {
        return TENDRIL_ERR_INVALID;
    }
    struct tendril_device *dev = eps[0]->dev;
    struct tendril_hc *hc = dev->hc;

    if (hc->ops->alloc_streams == NULL || (hc->quirks & TENDRIL_HC_QUIRK_NO_STREAMS)) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    unsigned int n = *streams;
    for (size_t i = 0; i < count; i++) {
        if (eps[i]->dev != dev || eps[i]->type != TENDRIL_ENDPOINT_XFER_BULK || eps[i]->max_streams == 0) {
            return TENDRIL_ERR_UNSUPPORTED;
        }
        if (!eps[i]->open) {
            return TENDRIL_ERR_INVALID;
        }
        if (eps[i]->max_streams - 1 < n) {
            n = eps[i]->max_streams - 1;
        }
    }
    if (n == 0) {
        return TENDRIL_ERR_UNSUPPORTED;
    }
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->suspended) {
        return TENDRIL_ERR_SUSPENDED;
    }
    if (dev->config_busy || dev->pm_busy) {
        return TENDRIL_ERR_BUSY;
    }
    struct dev_op *op = op_alloc(dev, streams_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    hc_lock(hc);
    int st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->alloc_streams(hc, eps, count, n, &op->async);
    hc_unlock(hc);
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
        return st;
    }
    /* The controller may have granted fewer. */
    *streams = eps[0]->streams;
    return TENDRIL_OK;
}

/* Strings */

static void string_work(struct tendril_work *work) {
    struct dev_op *op = CONTAINER_OF(work, struct dev_op, async.work);
    struct tendril_device *dev = op->dev;
    int st = op->async.status;

    if (dev->gone) {
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    if (st != TENDRIL_OK && st != TENDRIL_ERR_SHORT) {
        op_finish(op, st);
        return;
    }
    char *s = decode_string(op->buf, op->xfer.actual, (dev->quirks & TENDRIL_DEV_QUIRK_SWAP_UNICODE) != 0);
    if (s == NULL) {
        op_finish(op, TENDRIL_ERR_PROTOCOL);
        return;
    }
    size_t n = tendril_strlen(s);
    if (op->out_size != 0) {
        size_t copy = MIN(n, op->out_size - 1);
        memcpy(op->out, s, copy);
        op->out[copy] = '\0';
    }
    tendril_strfree(s);
    op_finish(op, TENDRIL_OK);
}

int tendril_device_read_string(
    struct tendril_device *dev, uint8_t index, char *buf, size_t size, void (*done)(void *ctx, int status), void *ctx
) {
    if (index == 0) {
        return TENDRIL_ERR_INVALID;
    }
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    struct dev_op *op = op_alloc(dev, string_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->out = buf;
    op->out_size = size;
    uint16_t langid = dev->langid != 0 ? dev->langid : 0x0409;
    int st = tendril_control_async(
        dev, &op->xfer, TENDRIL_DIR_IN | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
        TENDRIL_REQ_GET_DESCRIPTOR, (uint16_t)(TENDRIL_DT_STRING << 8 | index),
        langid, op->buf, 255, &op->async
    );
    if (st != TENDRIL_OK) {
        op->done = NULL;
        op_finish(op, st);
    }
    return st;
}

/* Configuration changes */

#define CFG_WAIT 0
#define CFG_DECONFIGURE_REQUEST 1
#define CFG_DROP 2
#define CFG_ADD 3
#define CFG_SET_CONFIGURATION 4
#define CFG_UNDO 5

static void config_op_step(struct dev_op *op);

/* The index of the configuration with bConfigurationValue `value`, or -1. */
static int find_config(struct tendril_device *dev, uint8_t value) {
    for (size_t i = 0; i < dev->num_configs; i++) {
        const struct tendril_config_descriptor *cd = (const void *)dev->configs[i];
        if (cd != NULL && cd->configuration_value == value) {
            return (int)i;
        }
    }
    return -1;
}

static void config_work(struct tendril_work *work) {
    config_op_step(CONTAINER_OF(work, struct dev_op, async.work));
}

static void config_timer_work(struct tendril_work *work) {
    config_op_step(CONTAINER_OF(work, struct dev_op, timer.work));
}

static void config_op_step(struct dev_op *op) {
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    int st = op->async.status;

    if (dev->gone) {
        op_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    switch (op->step) {
        case CFG_WAIT: {
            /* Everything the drivers started must finish before they are unbound; this operation counts too. */
            hc_lock(hc);
            bool busy = dev->pending > 1;
            if (busy) {
                tendril_timer_arm(hc, &op->timer, NS_PER_MS);
            }
            hc_unlock(hc);
            if (busy) {
                return;
            }
            tendril_unbind_interfaces(dev);
            op->step = CFG_DECONFIGURE_REQUEST;
            st = tendril_control_async(
                dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                TENDRIL_REQ_SET_CONFIGURATION, 0, 0, NULL, 0, &op->async
            );
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case CFG_DECONFIGURE_REQUEST: {
            struct tendril_endpoint *drop[MAX_DEVICE_ENDPOINTS];
            size_t num_drop = collect_open_endpoints(dev, NULL, drop);
            op->step = CFG_DROP;
            hc_lock(hc);
            st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->configure(hc, dev, drop, num_drop, NULL, 0, &op->async);
            hc_unlock(hc);
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case CFG_DROP: {
            free_interfaces(dev);
            dev->active_config = -1;
            dev->state = DEV_ADDRESSED;
            if (st != TENDRIL_OK || op->value == 0) {
                op_finish(op, st);
                return;
            }
            int index = find_config(dev, op->value);
            if (index < 0) {
                op_finish(op, TENDRIL_ERR_INVALID);
                return;
            }
            st = build_interfaces(dev, (size_t)index);
            for (size_t i = 0; i < dev->num_interfaces && st == TENDRIL_OK; i++) {
                struct tendril_interface *intf = &dev->interfaces[i];
                st = tendril_interface_build_endpoints(intf, intf->cur_alt, &intf->endpoints, &intf->num_endpoints);
            }
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                op_finish(op, st);
                return;
            }
            dev->active_config = index;
            const struct tendril_config_descriptor *cd = (const void *)dev->configs[index];
            dev->self_powered = !(dev->quirks & TENDRIL_DEV_QUIRK_BUS_POWERED)
                && (cd->attributes & TENDRIL_CONFIG_ATTR_SELF_POWERED) != 0;
            struct tendril_endpoint *add[MAX_DEVICE_ENDPOINTS];
            size_t num_add = collect_endpoints(dev, add);
            op->step = CFG_ADD;
            hc_lock(hc);
            st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->configure(hc, dev, NULL, 0, add, num_add, &op->async);
            hc_unlock(hc);
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                dev->active_config = -1;
                op_finish(op, st);
            }
            return;
        }
        case CFG_ADD: {
            if (st != TENDRIL_OK) {
                free_interfaces(dev);
                dev->active_config = -1;
                op_finish(op, st);
                return;
            }
            op->step = CFG_SET_CONFIGURATION;
            st = tendril_control_async(
                dev, &op->xfer, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
                TENDRIL_REQ_SET_CONFIGURATION, op->value, 0, NULL, 0, &op->async
            );
            if (st != TENDRIL_OK) {
                op_finish(op, st);
            }
            return;
        }
        case CFG_SET_CONFIGURATION: {
            if (st != TENDRIL_OK) {
                /* The device stays unconfigured, so its endpoints go again. */
                struct tendril_endpoint *drop[MAX_DEVICE_ENDPOINTS];
                size_t num_drop = collect_open_endpoints(dev, NULL, drop);
                op->result = st;
                op->step = CFG_UNDO;
                hc_lock(hc);
                st = hc->dead ? TENDRIL_ERR_NO_DEVICE
                    : hc->ops->configure(hc, dev, drop, num_drop, NULL, 0, &op->async);
                hc_unlock(hc);
                if (st != TENDRIL_OK) {
                    free_interfaces(dev);
                    dev->active_config = -1;
                    op_finish(op, op->result);
                }
                return;
            }
            /* The new drivers' requests go through from their probe on. */
            dev->config_busy = false;
            dev->config_xfer = NULL;
            dev->state = DEV_CONFIGURED;
            tendril_bind_interfaces(dev);
            op_finish(op, st);
            return;
        }
        case CFG_UNDO: {
            free_interfaces(dev);
            dev->active_config = -1;
            op_finish(op, op->result);
            return;
        }
        default: {
            op_finish(op, TENDRIL_ERR_INVALID);
            return;
        }
    }
}

int tendril_device_set_configuration(
    struct tendril_device *dev, uint8_t value, void (*done)(void *ctx, int status), void *ctx
) {
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->suspended) {
        return TENDRIL_ERR_SUSPENDED;
    }
    if (dev->step != STEP_RUNNING || dev->hub != NULL || dev->config_busy || dev->pm_busy) {
        return TENDRIL_ERR_BUSY;
    }
    /* Checked now, as the change starts by tearing the current configuration down. */
    int index = value != 0 ? find_config(dev, value) : -1;
    if (value != 0 && index < 0) {
        return TENDRIL_ERR_INVALID;
    }
    if (index >= 0 && !power_ok(dev, (size_t)index)) {
        return TENDRIL_ERR_NO_POWER;
    }
    struct dev_op *op = op_alloc(dev, config_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    tendril_timer_init(&op->timer, config_timer_work);
    dev->config_busy = true;
    dev->config_xfer = &op->xfer;
    op->value = value;
    op->step = CFG_WAIT;
    /*
     * Drivers must stop their transfers before they are unbound: everything
     * outstanding is cancelled, control requests too, and new ones are refused.
     */
    hc_lock(dev->hc);
    cancel_all(dev, TENDRIL_ERR_CANCELLED, true);
    tendril_queue_work(dev->hc, &op->async.work);
    hc_unlock(dev->hc);
    return TENDRIL_OK;
}

/* Power management */

/* How long drivers get to finish their transfers before a suspend gives up. */
#define PM_WAIT_TIMEOUT_MS 5000

#define PM_SUSPEND_WAIT 0
#define PM_SUSPEND_WAKE 1
#define PM_SUSPEND_HCD 2
#define PM_SUSPEND_PORT 3
#define PM_SUSPEND_DONE 4
#define PM_RESUME_PARENT 5
#define PM_RESUME_PORT 6
#define PM_RESUME_CLEAR 7
#define PM_RESUME_DONE 8

static void pm_step(struct dev_op *op);

static void pm_work(struct tendril_work *work) {
    pm_step(CONTAINER_OF(work, struct dev_op, async.work));
}

static void pm_timer_work(struct tendril_work *work) {
    pm_step(CONTAINER_OF(work, struct dev_op, timer.work));
}

bool tendril_device_suspended(struct tendril_device *dev) {
    return dev->suspended;
}

bool tendril_device_can_wake(struct tendril_device *dev) {
    const struct tendril_config_descriptor *cd = tendril_device_active_config(dev);
    return cd != NULL && (cd->attributes & TENDRIL_CONFIG_ATTR_REMOTE_WAKEUP) != 0;
}

/* USB 3.2 revision 1.1 9.4.9: FUNCTION_SUSPEND goes to the first interface of each function. */
static bool function_head(struct tendril_interface *intf) {
    return intf->iad == NULL || intf->iad->first_interface == intf->number;
}

/* A suspended child counts on its hub to pass its wakeup on. */
static bool child_wake_armed(struct tendril_device *dev) {
    struct tendril_hub *hub = dev->hub;
    for (size_t i = 0; hub != NULL && i < hub->num_ports; i++) {
        struct tendril_device *child = hub->ports[i].device;
        if (child != NULL && (child->pm_wake || child->pm_functions)) {
            return true;
        }
    }
    return false;
}

static void pm_finish(struct dev_op *op, int status) {
    struct tendril_device *dev = op->dev;
    bool woke = dev->pm_woke && dev->suspended && status == TENDRIL_OK;
    dev->pm_busy = false;
    if (woke) {
        tendril_device_ref(dev);
    }
    op_finish(op, status);
    if (woke) {
        /* The device signalled a wakeup while its suspend was finishing. */
        tendril_device_woke(dev);
        tendril_device_unref(dev);
    }
}

static void pm_request(struct dev_op *op, uint8_t request_type, uint8_t request, uint16_t value, uint16_t index) {
    op->requested = true;
    int st = tendril_control_async(
        op->dev, &op->xfer, request_type, request, value, index, NULL, 0,
        &op->async
    );
    if (st != TENDRIL_OK) {
        hc_lock(op->dev->hc);
        tendril_async_complete(op->dev->hc, &op->async, st);
        hc_unlock(op->dev->hc);
    }
}

/*
 * Sends the next request arming (or, on resume, disarming) remote wakeup:
 * DEVICE_REMOTE_WAKEUP for USB 2.0 devices, FUNCTION_SUSPEND to each function
 * of a USB 3 device. False once none are left.
 */
static bool pm_next_wake_request(struct dev_op *op, bool arm) {
    struct tendril_device *dev = op->dev;

    if (!tendril_device_is_ss(dev)) {
        if (op->alt++ != 0 || !(arm ? op->value != 0 : dev->pm_wake)) {
            return false;
        }
        pm_request(
            op, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_DEVICE,
            arm ? TENDRIL_REQ_SET_FEATURE : TENDRIL_REQ_CLEAR_FEATURE, TENDRIL_FEATURE_DEVICE_REMOTE_WAKEUP, 0
        );
        return true;
    }
    if (!(arm ? op->value != 0 : dev->pm_functions)) {
        return false;
    }
    while (op->alt < dev->num_interfaces) {
        struct tendril_interface *intf = &dev->interfaces[op->alt++];
        if (!function_head(intf)) {
            continue;
        }
        unsigned int options = arm ? TENDRIL_FUNCTION_SUSPEND_LOW_POWER | TENDRIL_FUNCTION_SUSPEND_REMOTE_WAKE : 0;
        pm_request(
            op, TENDRIL_DIR_OUT | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_INTERFACE, TENDRIL_REQ_SET_FEATURE,
            TENDRIL_FEATURE_FUNCTION_SUSPEND, (uint16_t)(intf->number | options << 8)
        );
        return true;
    }
    return false;
}

/* Undoes a suspend that got partway: disarms wakeup, then lets the drivers carry on. */
static void pm_abort(struct dev_op *op, int status) {
    op->result = status;
    op->step = PM_RESUME_CLEAR;
    op->alt = 0;
    op->requested = false;
    op->async.status = TENDRIL_OK;
    pm_step(op);
}

static void pm_step(struct dev_op *op) {
    struct tendril_device *dev = op->dev;
    struct tendril_hc *hc = dev->hc;
    int st = op->async.status;
    char name[24];

    if (dev->gone) {
        pm_finish(op, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    switch (op->step) {
        case PM_SUSPEND_WAIT: {
            /* The drivers' transfers drain; this operation counts too. */
            hc_lock(hc);
            bool busy = dev->pending > 1;
            bool waiting = busy && tendril_now() < op->deadline;
            if (waiting) {
                tendril_timer_arm(hc, &op->timer, NS_PER_MS);
            }
            bool hub_ready = true;
            for (size_t i = 0; dev->hub != NULL && i < dev->hub->num_ports && !busy; i++) {
                struct tendril_port *port = &dev->hub->ports[i];
                if (!tendril_port_idle(port) || (port->device != NULL && !port->device->suspended)) {
                    hub_ready = false;
                }
            }
            hc_unlock(hc);
            if (waiting) {
                return;
            }
            if (busy || !hub_ready) {
                dev_name(dev, name, sizeof(name));
                LOG_WARN("%s: suspend abandoned: %s", name, busy ? "transfers did not finish" : "hub busy");
                pm_abort(op, TENDRIL_ERR_BUSY);
                return;
            }
            if (dev->hub != NULL && child_wake_armed(dev) && tendril_device_can_wake(dev)) {
                op->value = 1;
            }
            op->step = PM_SUSPEND_WAKE;
            op->alt = 0;
            op->requested = false;
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_SUSPEND_WAKE: {
            if (op->requested && st != TENDRIL_OK) {
                if (st == TENDRIL_ERR_NO_DEVICE) {
                    pm_finish(op, st);
                    return;
                }
                dev_name(dev, name, sizeof(name));
                LOG_DEBUG("%s: arming remote wakeup failed: %s", name, tendril_status_str(st));
            } else if (op->requested) {
                if (tendril_device_is_ss(dev)) {
                    dev->pm_functions = true;
                } else {
                    dev->pm_wake = true;
                }
            }
            op->requested = false;
            if (pm_next_wake_request(op, true)) {
                return;
            }
            op->step = PM_SUSPEND_HCD;
            if (hc->ops->device_suspend != NULL) {
                hc_lock(hc);
                st = hc->dead ? TENDRIL_ERR_NO_DEVICE : hc->ops->device_suspend(hc, dev, &op->async);
                hc_unlock(hc);
                if (st != TENDRIL_OK) {
                    pm_abort(op, st);
                }
                return;
            }
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_SUSPEND_HCD: {
            if (st != TENDRIL_OK) {
                pm_abort(op, st);
                return;
            }
            op->step = PM_SUSPEND_PORT;
            tendril_port_pm(dev->port, PORT_PM_SUSPEND, &op->async);
            return;
        }
        case PM_SUSPEND_PORT: {
            if (st != TENDRIL_OK) {
                pm_abort(op, st);
                return;
            }
            dev->suspended = true;
            dev_name(dev, name, sizeof(name));
            LOG_DEBUG("%s: suspended%s", name, dev->pm_wake || dev->pm_functions ? ", remote wakeup armed" : "");
            pm_finish(op, TENDRIL_OK);
            return;
        }
        case PM_RESUME_PARENT: {
            /* The hub above comes back first, which may take its own resume. */
            struct tendril_device *parent = dev->parent;
            if (parent != NULL && (parent->suspended || parent->pm_busy)) {
                if (tendril_now() >= op->deadline) {
                    pm_finish(op, TENDRIL_ERR_TIMEOUT);
                    return;
                }
                if (!parent->pm_busy) {
                    if (op->requested) {
                        pm_finish(op, TENDRIL_ERR_IO);
                        return;
                    }
                    st = tendril_device_resume(parent, NULL, NULL);
                    if (st != TENDRIL_OK) {
                        pm_finish(op, st);
                        return;
                    }
                    op->requested = true;
                }
                hc_lock(hc);
                tendril_timer_arm(hc, &op->timer, NS_PER_MS);
                hc_unlock(hc);
                return;
            }
            op->step = PM_RESUME_PORT;
            tendril_port_pm(dev->port, PORT_PM_RESUME, &op->async);
            return;
        }
        case PM_RESUME_PORT: {
            if (st != TENDRIL_OK) {
                pm_finish(op, st);
                return;
            }
            dev->suspended = false;
            op->step = PM_RESUME_CLEAR;
            op->alt = 0;
            op->requested = false;
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_RESUME_CLEAR: {
            if (op->requested && st != TENDRIL_OK) {
                dev_name(dev, name, sizeof(name));
                LOG_DEBUG("%s: disarming remote wakeup failed: %s", name, tendril_status_str(st));
            }
            op->requested = false;
            if (pm_next_wake_request(op, false)) {
                return;
            }
            dev->pm_wake = false;
            dev->pm_functions = false;
            op->step = PM_RESUME_DONE;
            op->async.status = TENDRIL_OK;
            pm_step(op);
            return;
        }
        case PM_RESUME_DONE: {
            bool woke = dev->pm_woke;
            dev->pm_woke = false;
            dev->pm_system = false;
            if (dev->hub != NULL) {
                tendril_hub_resume(dev->hub);
            }
            tendril_resume_interfaces(dev);
            if (op->result == TENDRIL_OK) {
                dev_name(dev, name, sizeof(name));
                LOG_DEBUG("%s: resumed%s", name, woke ? " by remote wakeup" : "");
            }
            if (woke && !dev->gone) {
                tendril_notify_woke(dev);
            }
            pm_finish(op, op->result);
            return;
        }
        default: {
            pm_finish(op, TENDRIL_ERR_INVALID);
            return;
        }
    }
}

int tendril_device_suspend(
    struct tendril_device *dev, bool remote_wake, void (*done)(void *ctx, int status), void *ctx
) {
    struct tendril_hc *hc = dev->hc;

    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    if (dev->step != STEP_RUNNING || dev->pm_busy || dev->config_busy) {
        return TENDRIL_ERR_BUSY;
    }
    if (dev->hub != NULL && !tendril_hub_suspend(dev->hub)) {
        return TENDRIL_ERR_BUSY;
    }
    int st;
    if (!tendril_suspend_interfaces(dev, &st)) {
        if (dev->hub != NULL) {
            tendril_hub_resume(dev->hub);
        }
        return st;
    }
    struct dev_op *op = op_alloc(dev, pm_work, done, ctx);
    if (op == NULL) {
        if (dev->hub != NULL) {
            tendril_hub_resume(dev->hub);
        }
        tendril_resume_interfaces(dev);
        return TENDRIL_ERR_NO_MEMORY;
    }
    tendril_timer_init(&op->timer, pm_timer_work);
    dev->pm_busy = true;
    dev->pm_woke = false;
    op->value = remote_wake && tendril_device_can_wake(dev);
    op->result = TENDRIL_OK;
    op->step = PM_SUSPEND_WAIT;
    op->deadline = tendril_now() + PM_WAIT_TIMEOUT_MS * NS_PER_MS;
    hc_lock(hc);
    cancel_all(dev, TENDRIL_ERR_CANCELLED, false);
    tendril_queue_work(hc, &op->async.work);
    hc_unlock(hc);
    return TENDRIL_OK;
}

int tendril_device_resume(struct tendril_device *dev, void (*done)(void *ctx, int status), void *ctx) {
    if (dev->gone) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (dev->pm_busy) {
        return TENDRIL_ERR_BUSY;
    }
    if (!dev->suspended) {
        return TENDRIL_ERR_INVALID;
    }
    struct dev_op *op = op_alloc(dev, pm_work, done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    tendril_timer_init(&op->timer, pm_timer_work);
    dev->pm_busy = true;
    op->result = TENDRIL_OK;
    op->step = PM_RESUME_PARENT;
    op->deadline = tendril_now() + PM_WAIT_TIMEOUT_MS * NS_PER_MS;
    hc_lock(dev->hc);
    tendril_queue_work(dev->hc, &op->async.work);
    hc_unlock(dev->hc);
    return TENDRIL_OK;
}

void tendril_device_woke(struct tendril_device *dev) {
    if (dev->gone) {
        return;
    }
    dev->pm_woke = true;
    if (dev->pm_busy || !dev->suspended) {
        /* The operation under way reports it once finished. */
        if (!dev->pm_busy) {
            dev->pm_woke = false;
        }
        return;
    }
    struct dev_op *op = op_alloc(dev, pm_work, NULL, NULL);
    if (op == NULL) {
        return;
    }
    tendril_timer_init(&op->timer, pm_timer_work);
    dev->pm_busy = true;
    dev->suspended = false;
    op->result = TENDRIL_OK;
    op->step = PM_RESUME_CLEAR;
    hc_lock(dev->hc);
    tendril_queue_work(dev->hc, &op->async.work);
    hc_unlock(dev->hc);
}
