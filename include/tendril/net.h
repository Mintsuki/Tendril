#ifndef TENDRIL_NET_H
#define TENDRIL_NET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <tendril/tendril.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Ethernet adapters: CDC ECM, CDC NCM, and RNDIS. Frames run from the
 * destination address to the end of the payload, without the frame check
 * sequence.
 */

struct tendril_net;

enum tendril_net_kind {
    TENDRIL_NET_ECM,
    TENDRIL_NET_NCM,
    TENDRIL_NET_RNDIS
};

struct tendril_net_callbacks {
    /*
     * An adapter is ready, passing on directed, broadcast, and all multicast
     * frames. `link` follows once the adapter reports its link.
     */
    void (*attach)(struct tendril_net *net);
    /* The adapter is gone; its sends and operations have all completed. */
    void (*detach)(struct tendril_net *net);
    /* The link came up, went down, or changed speed; rates are in bits per second, 0 if unknown. */
    void (*link)(struct tendril_net *net, bool up, uint32_t rx_bps, uint32_t tx_bps);
    /* A received frame, valid only during the call. */
    void (*receive)(struct tendril_net *net, const uint8_t *frame, size_t len);
};

/* Registers the network class driver; call before the first controller is added. */
void tendril_net_register(const struct tendril_net_callbacks *callbacks);

/*
 * Queues a frame, which must stay valid until `done`; frames go out in order.
 * While the device is suspended, frames wait for it to resume.
 */
enum tendril_status tendril_net_send(struct tendril_net *net, const void *frame, size_t len,
                                   void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Packet filter bits, as in CDC ECM. */
#define TENDRIL_NET_FILTER_PROMISCUOUS (1u << 0)
#define TENDRIL_NET_FILTER_ALL_MULTICAST (1u << 1)
#define TENDRIL_NET_FILTER_DIRECTED (1u << 2)
#define TENDRIL_NET_FILTER_BROADCAST (1u << 3)
#define TENDRIL_NET_FILTER_MULTICAST (1u << 4)

/*
 * Picks the frames the adapter passes on; adapters may pass on more than
 * asked for. TENDRIL_ERR_UNSUPPORTED if the adapter has no packet filter.
 */
enum tendril_status tendril_net_set_filter(struct tendril_net *net, uint16_t filter,
                                         void (*done)(void *ctx, enum tendril_status status), void *ctx);

/*
 * Sets the multicast addresses passed on under TENDRIL_NET_FILTER_MULTICAST:
 * `count` 6-byte addresses back to back, copied before returning.
 */
enum tendril_status tendril_net_set_multicast(struct tendril_net *net, const uint8_t *addrs, size_t count,
                                            void (*done)(void *ctx, enum tendril_status status), void *ctx);

/* Synchronous helper, running tendril_hc_poll() until done. */
enum tendril_status tendril_net_send_sync(struct tendril_net *net, const void *frame, size_t len);

struct tendril_device *tendril_net_device(struct tendril_net *net);
enum tendril_net_kind tendril_net_kind(struct tendril_net *net);
/* The number of the communications interface. */
uint8_t tendril_net_interface(struct tendril_net *net);
/* The adapter's 6-byte station address. */
const uint8_t *tendril_net_mac(struct tendril_net *net);
/* The longest frame the adapter takes. */
size_t tendril_net_max_frame(struct tendril_net *net);
/* How many addresses tendril_net_set_multicast() takes, 0 if it is unsupported. */
unsigned tendril_net_multicast_filters(struct tendril_net *net);
bool tendril_net_link_up(struct tendril_net *net);
void tendril_net_set_user(struct tendril_net *net, void *user);
void *tendril_net_user(struct tendril_net *net);

#ifdef __cplusplus
}
#endif

#endif
