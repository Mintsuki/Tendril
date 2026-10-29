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

#ifndef TENDRIL_TENDRIL_NET_H
#define TENDRIL_TENDRIL_NET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "tendril.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Ethernet adapters: CDC ECM, CDC NCM, and RNDIS. Frames run from the
 * destination address to the end of the payload, without the frame check
 * sequence.
 */

struct tendril_net;

#define TENDRIL_NET_ECM 0
#define TENDRIL_NET_NCM 1
#define TENDRIL_NET_RNDIS 2

struct tendril_net_callbacks {
    /*
     * An adapter is ready, passing on directed, broadcast, and all multicast
     * frames. `link` follows once the adapter reports its link.
     */
    void (*attach)(struct tendril_net *net);
    /* The adapter is gone; its sends and operations have all completed. */
    void (*detach)(struct tendril_net *net);
    /*
     * The link came up, went down, or changed speed; rates are in bits per
     * second, 0 if unknown.
     */
    void (*link)(struct tendril_net *net, bool up, uint32_t rx_bps, uint32_t tx_bps);
    /* A received frame, valid only during the call. */
    void (*receive)(struct tendril_net *net, const uint8_t *frame, size_t len);
};

/*
 * Registers the network class driver; call before the first controller is
 * added.
 */
void tendril_net_register(const struct tendril_net_callbacks *callbacks);

/*
 * Queues a frame, which must stay valid until `done`; frames go out in order.
 * While the device is suspended, frames wait for it to resume.
 */
int tendril_net_send(
    struct tendril_net *net, const void *frame, size_t len, void (*done)(void *ctx, int status), void *ctx
);

/* Packet filter bits, as in CDC ECM. */
#define TENDRIL_NET_FILTER_PROMISCUOUS ((uint32_t)1 << 0)
#define TENDRIL_NET_FILTER_ALL_MULTICAST ((uint32_t)1 << 1)
#define TENDRIL_NET_FILTER_DIRECTED ((uint32_t)1 << 2)
#define TENDRIL_NET_FILTER_BROADCAST ((uint32_t)1 << 3)
#define TENDRIL_NET_FILTER_MULTICAST ((uint32_t)1 << 4)

/*
 * Picks the frames the adapter passes on; adapters may pass on more than asked
 * for. TENDRIL_ERR_UNSUPPORTED if the adapter has no packet filter.
 */
int tendril_net_set_filter(struct tendril_net *net, uint16_t filter, void (*done)(void *ctx, int status), void *ctx);

/*
 * Sets the multicast addresses passed on under TENDRIL_NET_FILTER_MULTICAST:
 * `count` 6-byte addresses back to back, copied before returning.
 */
int tendril_net_set_multicast(
    struct tendril_net *net, const uint8_t *addrs, size_t count, void (*done)(void *ctx, int status), void *ctx
);

/* Synchronous helper, running tendril_hc_poll() until done. */
int tendril_net_send_sync(struct tendril_net *net, const void *frame, size_t len);

struct tendril_device *tendril_net_device(struct tendril_net *net);
int tendril_net_kind(struct tendril_net *net);
/* The number of the communications interface. */
uint8_t tendril_net_interface(struct tendril_net *net);
/* The adapter's 6-byte station address. */
const uint8_t *tendril_net_mac(struct tendril_net *net);
/* The longest frame the adapter takes. */
size_t tendril_net_max_frame(struct tendril_net *net);
/*
 * How many addresses tendril_net_set_multicast() takes, 0 if it is
 * unsupported.
 */
size_t tendril_net_multicast_filters(struct tendril_net *net);
bool tendril_net_link_up(struct tendril_net *net);
void tendril_net_set_user(struct tendril_net *net, void *user);
void *tendril_net_user(struct tendril_net *net);

#ifdef __cplusplus
}
#endif

#endif
