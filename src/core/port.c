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
 * Port state machine, shared by root and external hubs. It only runs from
 * work items, which one tendril_hc_poll() dispatches at a time, so it needs the
 * lock only around the work and timer lists.
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

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usb_hub.h (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
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

#include "../internal.h"

/* BEGIN FreeBSD sys/dev/usb/usb.h, usb_request.c */
/*
 * USB 2.0 7.1.7.3 and 7.1.7.5 ask for at least 100 ms of debounce (TATTDB)
 * and 10 ms of reset recovery (TRSTRCY); these are FreeBSD's longer
 * USB_PORT_POWERUP_DELAY, USB_PORT_RESET_DELAY, and USB_PORT_RESET_RECOVERY,
 * from its sys/dev/usb/usb.h (BSD-2-Clause), and its usbd_req_reset_port()
 * timeout.
 */
#define DEBOUNCE_MS 300
#define RESET_POLL_MS 50
#define RESET_TIMEOUT_MS 1000
#define RESET_RECOVERY_MS 20
/* END FreeBSD sys/dev/usb/usb.h, usb_request.c */
#define DEBOUNCE_STEP_MS 25
#define DEBOUNCE_MAX_MS 1500
#define SS_ENABLE_WAIT_MS 100
/* BEGIN FreeBSD sys/dev/usb/usb_hub.h */
/* FreeBSD's USB_RESTART_MAX, from its sys/dev/usb/usb_hub.h (BSD-2-Clause). */
#define MAX_RESTARTS 5
/* END FreeBSD sys/dev/usb/usb_hub.h */
#define RETRY_DELAY_MS 100
#define MAX_ATTEMPTS 3
/*
 * USB 2.0 7.1.7.6: a device is suspended within 10 ms of its bus going idle,
 * and EHCI 1.0 4.3.1 wants those 10 ms to pass before a resume.
 */
#define SUSPEND_POLL_MS 2
#define SUSPEND_SETTLE_MS 10
#define SUSPEND_TIMEOUT_MS 100
/* BEGIN FreeBSD sys/dev/usb/usb_hub.c, usb.h */
/*
 * USB 2.0 7.1.7.7: resume signalling lasts 20 ms, then TRSMRCY gives the
 * device at least 10 ms; FreeBSD's sys/dev/usb/usb_hub.c (BSD-2-Clause,
 * usb_dev_resume_peer) waits 20 ms before a resume and gives the device
 * USB_PORT_RESUME_DELAY, from its sys/dev/usb/usb.h, after it.
 */
#define RESUME_DELAY_MS 20
#define RESUME_RECOVERY_MS 40
/* END FreeBSD sys/dev/usb/usb_hub.c, usb.h */
#define RESUME_POLL_MS 5
#define RESUME_TIMEOUT_MS 500

#define OP_NONE 0
#define OP_KICK 1
#define OP_STATUS 2
#define OP_CLEAR 3
#define OP_RESET 4
#define OP_RESET_STATUS 5
#define OP_RESET_CLEAR 6
#define OP_DISABLE 7
#define OP_ENUM_START 8
#define OP_PM_START 9
#define OP_SUSPEND 10
#define OP_SUSPEND_STATUS 11
#define OP_RESUME 12
#define OP_RESUME_STATUS 13
#define OP_RESUME_CLEAR 14

static struct tendril_hc *port_hc(struct tendril_port *port) {
    return port->hub->hc;
}

static const char *port_name(struct tendril_port *port, char *buf, size_t size) {
    struct tendril_device *hub = port->hub->dev;
    if (hub == NULL) {
        tendril_snprintf(buf, size, "root port %u", port->number);
    } else {
        tendril_snprintf(buf, size, "hub %u port %u", hub->address, port->number);
    }
    return buf;
}

static const struct {
    uint16_t bit;
    uint8_t feature;
} change_bits[] = {
    { TENDRIL_PORT_STAT_C_CONNECTION, TENDRIL_PORT_FEAT_C_CONNECTION },
    { TENDRIL_PORT_STAT_C_ENABLE, TENDRIL_PORT_FEAT_C_ENABLE },
    { TENDRIL_PORT_STAT_C_SUSPEND, TENDRIL_PORT_FEAT_C_SUSPEND },
    { TENDRIL_PORT_STAT_C_OVER_CURRENT, TENDRIL_PORT_FEAT_C_OVER_CURRENT },
    { TENDRIL_PORT_STAT_C_RESET, TENDRIL_PORT_FEAT_C_RESET },
    { TENDRIL_PORT_STAT_C_BH_RESET, TENDRIL_PORT_FEAT_C_BH_RESET },
    { TENDRIL_PORT_STAT_C_LINK_STATE, TENDRIL_PORT_FEAT_C_LINK_STATE },
    { TENDRIL_PORT_STAT_C_CONFIG_ERROR, TENDRIL_PORT_FEAT_C_CONFIG_ERROR }
};

static unsigned int link_state(struct tendril_port *port) {
    return (port->st.status & TENDRIL_SS_PORT_STAT_LINK_STATE_MASK) >> TENDRIL_SS_PORT_STAT_LINK_STATE_SHIFT;
}

static bool connected(struct tendril_port *port) {
    return (port->st.status & TENDRIL_PORT_STAT_CONNECTION) != 0;
}

static bool enabled(struct tendril_port *port) {
    return (port->st.status & TENDRIL_PORT_STAT_ENABLE) != 0;
}

static bool in_reset(struct tendril_port *port) {
    return (port->st.status & TENDRIL_PORT_STAT_RESET) != 0;
}

/* The link carries traffic again: out of U3, or out of USB 2.0 suspend. */
static bool link_active(struct tendril_port *port) {
    if (port->superspeed) {
        unsigned int ls = link_state(port);
        return ls == TENDRIL_SS_LINK_U0 || ls == TENDRIL_SS_LINK_U1 || ls == TENDRIL_SS_LINK_U2;
    }
    return enabled(port) && !(port->st.status & TENDRIL_PORT_STAT_SUSPEND);
}

static bool link_suspended(struct tendril_port *port) {
    if (port->superspeed) {
        return link_state(port) == TENDRIL_SS_LINK_U3;
    }
    return (port->st.status & TENDRIL_PORT_STAT_SUSPEND) != 0;
}

static uint64_t elapsed_ms(uint64_t since) {
    return (tendril_now() - since) / NS_PER_MS;
}

static void queue_op(struct tendril_port *port, int op) {
    struct tendril_hc *hc = port_hc(port);
    port->op = op;
    hc_lock(hc);
    tendril_queue_work(hc, &port->async.work);
    hc_unlock(hc);
}

static void handle_enum_failure(struct tendril_port *port);

/* Answers the device layer's suspend or resume request. */
static void pm_complete(struct tendril_port *port, int status) {
    struct tendril_hc *hc = port_hc(port);
    struct tendril_async *async = port->pm_async;
    if (async != NULL && port->pm_request == PORT_PM_NONE) {
        port->pm_async = NULL;
        hc_lock(hc);
        tendril_async_complete(hc, async, status);
        hc_unlock(hc);
    }
}

/*
 * Ends a run of the state machine. Deferred events are handled now, unless the
 * port waits on a timer or for the enumeration slot.
 */
static void port_idle(struct tendril_port *port) {
    struct tendril_hc *hc = port_hc(port);

    port->op = OP_NONE;
    if (hc->removing || port->hub->stopping) {
        port->busy = false;
        port->pm_request = PORT_PM_NONE;
        pm_complete(port, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    if (port->enum_failed) {
        port->enum_failed = false;
        handle_enum_failure(port);
        return;
    }
    if (port->kick) {
        queue_op(port, OP_KICK);
        return;
    }
    if (port->pm_request != PORT_PM_NONE) {
        queue_op(port, OP_PM_START);
        return;
    }
    port->busy = false;
}

static void port_wait(struct tendril_port *port, uint64_t ms) {
    struct tendril_hc *hc = port_hc(port);
    port->op = OP_NONE;
    hc_lock(hc);
    tendril_timer_arm(hc, &port->timer, ms * NS_PER_MS);
    hc_unlock(hc);
}

static void start_status(struct tendril_port *port, int op) {
    port->kick = false;
    port->op = op;
    port->hub->ops->get_status(port);
}

/* Clears the next pending change bit; false if none are left. */
static bool clear_next(struct tendril_port *port, int op) {
    for (size_t i = 0; i < ARRAY_SIZE(change_bits); i++) {
        if (port->st.change & change_bits[i].bit) {
            port->st.change &= ~change_bits[i].bit;
            port->op = op;
            port->hub->ops->clear_feature(port, change_bits[i].feature);
            return true;
        }
    }
    return false;
}

static void disconnect_device(struct tendril_port *port) {
    struct tendril_device *dev = port->device;
    if (dev != NULL) {
        port->device = NULL;
        tendril_device_disconnect(dev);
    }
}

static void give_up(struct tendril_port *port) {
    char name[32];
    LOG_WARN("%s: giving up on the device", port_name(port, name, sizeof(name)));
    port->state = PORT_DISABLED;
    if (!port->superspeed) {
        port->op = OP_DISABLE;
        port->hub->ops->clear_feature(port, TENDRIL_PORT_FEAT_ENABLE);
        return;
    }
    port_idle(port);
}

static void retry_later(struct tendril_port *port) {
    port->attempts++;
    if (port->attempts >= MAX_ATTEMPTS) {
        give_up(port);
        return;
    }
    port->state = PORT_WAIT_ENUM;
    port_wait(port, RETRY_DELAY_MS);
}

/* Enumeration slot: one device at a time may sit at the default address. */

static void start_reset(struct tendril_port *port);

static void enum_request(struct tendril_port *port) {
    struct tendril_hc *hc = port_hc(port);
    bool start = false;

    port->state = PORT_WAIT_ENUM;
    hc_lock(hc);
    if (hc->enum_port == NULL) {
        hc->enum_port = port;
        start = true;
    } else {
        list_add_tail(&hc->enum_queue, &port->enum_node);
    }
    hc_unlock(hc);
    if (start) {
        start_reset(port);
    }
}

/* With the lock held. */
static void enum_release_locked(struct tendril_hc *hc) {
    hc->enum_port = NULL;
    hc->enum_dev = NULL;
    if (!list_empty(&hc->enum_queue)) {
        struct tendril_port *next = CONTAINER_OF(hc->enum_queue.next, struct tendril_port, enum_node);
        list_del(&next->enum_node);
        hc->enum_port = next;
        next->op = OP_ENUM_START;
        tendril_queue_work(hc, &next->async.work);
    }
}

static void release_slot(struct tendril_port *port) {
    struct tendril_hc *hc = port_hc(port);
    hc_lock(hc);
    if (hc->enum_port == port && hc->enum_dev == NULL) {
        enum_release_locked(hc);
    }
    hc_unlock(hc);
}

void tendril_enum_release_dev(struct tendril_device *dev) {
    struct tendril_hc *hc = dev->hc;
    hc_lock(hc);
    if (hc->enum_dev == dev) {
        enum_release_locked(hc);
    }
    hc_unlock(hc);
}

/* Status checks in the stable states */

static void evaluate(struct tendril_port *port) {
    uint16_t changes = port->changes;
    char name[32];

    if ((changes & TENDRIL_PORT_STAT_C_OVER_CURRENT) && (port->st.status & TENDRIL_PORT_STAT_OVER_CURRENT)) {
        LOG_WARN("%s: over-current", port_name(port, name, sizeof(name)));
    }
    if ((changes & TENDRIL_PORT_STAT_C_CONFIG_ERROR) && port->superspeed) {
        LOG_WARN("%s: link configuration error", port_name(port, name, sizeof(name)));
    }

    bool conn = connected(port);
    if ((changes & TENDRIL_PORT_STAT_C_CONNECTION) || !conn) {
        if (port->device != NULL) {
            LOG_INFO("%s: device disconnected", port_name(port, name, sizeof(name)));
            disconnect_device(port);
        }
        if (port->state != PORT_DEBOUNCE) {
            port->state = PORT_IDLE;
        }
        port->attempts = 0;
        port->restarts = 0;
        port->config_failed = false;
    } else if (!port->superspeed && (changes & TENDRIL_PORT_STAT_C_ENABLE) && !enabled(port)
            && port->state == PORT_ATTACHED && port->device != NULL) {
        /* BEGIN FreeBSD sys/dev/usb/usb_hub.c (and the condition above) */
        /*
         * The hub disabled the port on its own, after a babble or a port error:
         * attach the device again, as FreeBSD's sys/dev/usb/usb_hub.c
         * (BSD-2-Clause, uhub_explore) does, a few times.
         */
        disconnect_device(port);
        if (++port->restarts > MAX_RESTARTS) {
            LOG_WARN("%s: port error, giving up", port_name(port, name, sizeof(name)));
            port->state = PORT_DISABLED;
        } else {
            LOG_INFO("%s: port error, attaching the device again", port_name(port, name, sizeof(name)));
            port->state = PORT_IDLE;
        }
        /* END FreeBSD sys/dev/usb/usb_hub.c */
    } else if (port->superspeed && port->device != NULL) {
        unsigned int ls = link_state(port);
        if (ls == TENDRIL_SS_LINK_INACTIVE || ls == TENDRIL_SS_LINK_COMPLIANCE) {
            LOG_INFO("%s: link failed, recovering", port_name(port, name, sizeof(name)));
            disconnect_device(port);
            port->state = PORT_IDLE;
        }
    } else if (port->state == PORT_SUSPENDED && port->device != NULL && !enabled(port) && !link_suspended(port)) {
        LOG_INFO("%s: suspended device lost, enumerating it again", port_name(port, name, sizeof(name)));
        disconnect_device(port);
        port->state = PORT_IDLE;
    }

    switch (port->state) {
        case PORT_IDLE: {
            if (conn) {
                port->state = PORT_DEBOUNCE;
                port->debounce_start = tendril_now();
                port->debounce_first = port->debounce_start;
                port_wait(port, DEBOUNCE_STEP_MS);
                return;
            }
            break;
        }
        case PORT_DEBOUNCE: {
            if (!conn) {
                port->state = PORT_IDLE;
                break;
            }
            if (changes & TENDRIL_PORT_STAT_C_CONNECTION) {
                if (elapsed_ms(port->debounce_first) > DEBOUNCE_MAX_MS) {
                    LOG_WARN("%s: connection never settled", port_name(port, name, sizeof(name)));
                    port->state = PORT_DISABLED;
                    break;
                }
                port->debounce_start = tendril_now();
            }
            if (elapsed_ms(port->debounce_start) >= DEBOUNCE_MS) {
                port->attempts = 0;
                port->config_failed = false;
                port->warm = false;
                if (port->superspeed) {
                    unsigned int ls = link_state(port);
                    port->warm = ls == TENDRIL_SS_LINK_INACTIVE || ls == TENDRIL_SS_LINK_COMPLIANCE;
                }
                enum_request(port);
                return;
            }
            port_wait(port, DEBOUNCE_STEP_MS);
            return;
        }
        case PORT_DISABLED: {
            if (!conn) {
                port->state = PORT_IDLE;
            }
            break;
        }
        case PORT_SUSPENDED: {
            if (port->device != NULL && link_active(port)) {
                LOG_DEBUG("%s: remote wakeup", port_name(port, name, sizeof(name)));
                port->woke = true;
                port->state = PORT_RESUME_RECOVERY;
                port_wait(port, RESUME_RECOVERY_MS);
                return;
            }
            break;
        }
        default: {
            break;
        }
    }
    port_idle(port);
}

static void status_done(struct tendril_port *port) {
    if (port->async.status != TENDRIL_OK) {
        if (port->async.status != TENDRIL_ERR_NO_DEVICE) {
            char name[32];
            LOG_WARN(
                "%s: status read failed: %s", port_name(port, name, sizeof(name)),
                tendril_status_str(port->async.status)
            );
        }
        port_idle(port);
        return;
    }
    port->changes = port->st.change;
    if (!clear_next(port, OP_CLEAR)) {
        evaluate(port);
    }
}

static void clear_done(struct tendril_port *port) {
    if (!clear_next(port, OP_CLEAR)) {
        evaluate(port);
    }
}

/* Reset */

static void reset_failed(struct tendril_port *port, const char *why) {
    char name[32];
    LOG_WARN("%s: reset failed: %s", port_name(port, name, sizeof(name)), why);
    release_slot(port);
    retry_later(port);
}

static void issue_reset(struct tendril_port *port) {
    port->op = OP_RESET;
    port->hub->ops->set_feature(port, port->warm ? TENDRIL_PORT_FEAT_BH_RESET : TENDRIL_PORT_FEAT_RESET);
}

static void start_reset(struct tendril_port *port) {
    struct tendril_hc *hc = port_hc(port);

    if (port->hub->stopping || hc->removing) {
        release_slot(port);
        port_idle(port);
        return;
    }
    port->state = PORT_RESET;
    port->reset_start = tendril_now();
    port->reset_issued = !port->superspeed || port->warm;
    if (!port->reset_issued) {
        /* USB3 ports enable themselves once the link trains. */
        start_status(port, OP_RESET_STATUS);
        return;
    }
    issue_reset(port);
}

static void reset_complete(struct tendril_port *port) {
    char name[32];

    if (!connected(port)) {
        LOG_DEBUG("%s: device left during reset", port_name(port, name, sizeof(name)));
        release_slot(port);
        port->state = PORT_IDLE;
        port_idle(port);
        return;
    }
    if (!enabled(port)) {
        reset_failed(port, "port not enabled");
        return;
    }
    port->warm = false;
    port->state = PORT_RECOVERY;
    port_wait(port, RESET_RECOVERY_MS);
}

static void reset_status_done(struct tendril_port *port) {
    if (port->async.status != TENDRIL_OK) {
        reset_failed(port, tendril_status_str(port->async.status));
        return;
    }

    bool done;
    if (!connected(port)) {
        done = true;
    } else if (!port->reset_issued) {
        done = enabled(port) && link_state(port) == TENDRIL_SS_LINK_U0;
        if (!done && elapsed_ms(port->reset_start) > SS_ENABLE_WAIT_MS) {
            unsigned int ls = link_state(port);
            port->warm = ls == TENDRIL_SS_LINK_INACTIVE || ls == TENDRIL_SS_LINK_COMPLIANCE;
            port->reset_issued = true;
            port->reset_start = tendril_now();
            issue_reset(port);
            return;
        }
    } else {
        done = !in_reset(port) && (port->st.change & (TENDRIL_PORT_STAT_C_RESET | TENDRIL_PORT_STAT_C_BH_RESET));
        /* PORT_RESET cleared with the port enabled ends the reset (USB 2.0 11.24.2.7.1.5), C_PORT_RESET or not. */
        if (!done && !in_reset(port) && enabled(port) && elapsed_ms(port->reset_start) > 50) {
            done = true;
        }
    }

    if (!done) {
        if (elapsed_ms(port->reset_start) > RESET_TIMEOUT_MS) {
            reset_failed(port, "timed out");
            return;
        }
        port_wait(port, RESET_POLL_MS);
        return;
    }
    if (!clear_next(port, OP_RESET_CLEAR)) {
        reset_complete(port);
    }
}

static void reset_clear_done(struct tendril_port *port) {
    if (!clear_next(port, OP_RESET_CLEAR)) {
        reset_complete(port);
    }
}

/* Enumeration results */

static void handle_enum_failure(struct tendril_port *port) {
    if (port->enum_status == TENDRIL_ERR_NO_DEVICE) {
        port->state = PORT_IDLE;
        port->kick = true;
        port_idle(port);
        return;
    }
    retry_later(port);
}

void tendril_enum_done(struct tendril_port *port, struct tendril_device *dev, int status) {
    char name[32];

    tendril_enum_release_dev(dev);
    if (port->device != dev) {
        /* Disconnected while enumerating; a status check already handled it. */
        return;
    }
    if (status == TENDRIL_OK) {
        port->state = PORT_ATTACHED;
        port->attempts = 0;
        return;
    }

    LOG_WARN("%s: enumeration failed: %s", port_name(port, name, sizeof(name)), tendril_status_str(status));
    if (status != TENDRIL_ERR_NO_DEVICE) {
        tendril_hub_reset_tt(dev);
    }
    disconnect_device(port);
    port->enum_status = status;
    if (port->busy) {
        port->enum_failed = true;
        return;
    }
    port->busy = true;
    handle_enum_failure(port);
}

/* Suspend and resume */

static void pm_start(struct tendril_port *port) {
    int request = port->pm_request;
    port->pm_request = PORT_PM_NONE;

    if (request == PORT_PM_SUSPEND) {
        if (port->state != PORT_ATTACHED || port->device == NULL) {
            pm_complete(port, port->device == NULL ? TENDRIL_ERR_NO_DEVICE : TENDRIL_ERR_BUSY);
            port_idle(port);
            return;
        }
        port->state = PORT_SUSPENDING;
        port->pm_start = tendril_now();
        port->op = OP_SUSPEND;
        if (port->superspeed) {
            port->hub->ops->set_feature(port, PORT_FEATURE(TENDRIL_PORT_FEAT_LINK_STATE, TENDRIL_SS_LINK_U3));
        } else {
            port->hub->ops->set_feature(port, TENDRIL_PORT_FEAT_SUSPEND);
        }
        return;
    }
    if (request == PORT_PM_RESUME) {
        if (port->state == PORT_ATTACHED) {
            /* Remote wakeup got there first. */
            pm_complete(port, TENDRIL_OK);
            port_idle(port);
            return;
        }
        if (port->state != PORT_SUSPENDED || port->device == NULL) {
            pm_complete(port, port->device == NULL ? TENDRIL_ERR_NO_DEVICE : TENDRIL_ERR_BUSY);
            port_idle(port);
            return;
        }
        port->state = PORT_RESUME_DELAY;
        port->woke = false;
        port_wait(port, RESUME_DELAY_MS);
        return;
    }
    port_idle(port);
}

static void start_resume(struct tendril_port *port) {
    port->state = PORT_RESUMING;
    port->pm_start = tendril_now();
    port->op = OP_RESUME;
    if (port->superspeed) {
        port->hub->ops->set_feature(port, PORT_FEATURE(TENDRIL_PORT_FEAT_LINK_STATE, TENDRIL_SS_LINK_U0));
    } else {
        port->hub->ops->clear_feature(port, TENDRIL_PORT_FEAT_SUSPEND);
    }
}

/* A status read during a suspend or resume found the device gone: the usual checks take over. */
static void pm_lost(struct tendril_port *port, int status) {
    pm_complete(port, status);
    port->state = PORT_ATTACHED;
    port->changes = port->st.change;
    if (!clear_next(port, OP_CLEAR)) {
        evaluate(port);
    }
}

static void suspend_status_done(struct tendril_port *port) {
    char name[32];

    if (port->async.status != TENDRIL_OK) {
        port->state = PORT_ATTACHED;
        pm_complete(port, port->async.status);
        port_idle(port);
        return;
    }
    if (!connected(port) || (port->st.change & TENDRIL_PORT_STAT_C_CONNECTION)) {
        pm_lost(port, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    if (link_suspended(port)) {
        port->state = PORT_SUSPENDED;
        port_wait(port, SUSPEND_SETTLE_MS);
        return;
    }
    if (elapsed_ms(port->pm_start) > SUSPEND_TIMEOUT_MS) {
        LOG_WARN("%s: the port did not suspend", port_name(port, name, sizeof(name)));
        pm_lost(port, TENDRIL_ERR_TIMEOUT);
        return;
    }
    port_wait(port, SUSPEND_POLL_MS);
}

static void resume_status_done(struct tendril_port *port) {
    char name[32];

    if (port->async.status != TENDRIL_OK) {
        port->state = PORT_SUSPENDED;
        pm_complete(port, port->async.status);
        port_idle(port);
        return;
    }
    if (!connected(port) || (port->st.change & TENDRIL_PORT_STAT_C_CONNECTION)) {
        pm_lost(port, TENDRIL_ERR_NO_DEVICE);
        return;
    }
    if (link_active(port)) {
        port->changes = port->st.change;
        port->state = PORT_RESUME_RECOVERY;
        if (!clear_next(port, OP_RESUME_CLEAR)) {
            port_wait(port, RESUME_RECOVERY_MS);
        }
        return;
    }
    /* OHCI 1.0a, HcRhPortStatus PES: a root port may read disabled until its resume completes. */
    bool lost = !port->superspeed && !enabled(port) && !link_suspended(port);
    if (lost || elapsed_ms(port->pm_start) > RESUME_TIMEOUT_MS) {
        /* Not coming back: enumerate it again. */
        LOG_WARN("%s: resume failed", port_name(port, name, sizeof(name)));
        pm_complete(port, lost ? TENDRIL_ERR_NO_DEVICE : TENDRIL_ERR_TIMEOUT);
        disconnect_device(port);
        port->state = PORT_IDLE;
        port->changes = port->st.change;
        if (!clear_next(port, OP_CLEAR)) {
            evaluate(port);
        }
        return;
    }
    port_wait(port, RESUME_POLL_MS);
}

static void resume_done(struct tendril_port *port) {
    struct tendril_device *dev = port->device;
    bool woke = port->woke;

    port->state = PORT_ATTACHED;
    port->woke = false;
    pm_complete(port, TENDRIL_OK);
    if (woke && dev != NULL) {
        tendril_device_woke(dev);
    }
    port_idle(port);
}

void tendril_port_pm(struct tendril_port *port, int request, struct tendril_async *async) {
    struct tendril_hc *hc = port_hc(port);
    hc_lock(hc);
    port->pm_request = request;
    port->pm_async = async;
    if (!port->busy) {
        port->busy = true;
        port->op = OP_PM_START;
        tendril_queue_work(hc, &port->async.work);
    }
    hc_unlock(hc);
}

/* With the lock held. */
bool tendril_port_idle(struct tendril_port *port) {
    return !port->busy && !tendril_timer_armed(&port->timer) && port->pm_request == PORT_PM_NONE;
}

/* Event handlers */

static void port_timer(struct tendril_work *work) {
    struct tendril_port *port = CONTAINER_OF(work, struct tendril_port, timer.work);
    struct tendril_hc *hc = port_hc(port);

    if (hc->removing || port->hub->stopping) {
        if (port->state == PORT_RESET || port->state == PORT_RECOVERY) {
            release_slot(port);
        }
        port_idle(port);
        return;
    }
    switch (port->state) {
        case PORT_DEBOUNCE: {
            start_status(port, OP_STATUS);
            break;
        }
        case PORT_WAIT_ENUM: {
            enum_request(port);
            break;
        }
        case PORT_RESET: {
            start_status(port, OP_RESET_STATUS);
            break;
        }
        case PORT_SUSPENDING: {
            start_status(port, OP_SUSPEND_STATUS);
            break;
        }
        case PORT_SUSPENDED: {
            /* The suspend has settled. */
            pm_complete(port, TENDRIL_OK);
            port_idle(port);
            break;
        }
        case PORT_RESUME_DELAY: {
            start_resume(port);
            break;
        }
        case PORT_RESUMING: {
            start_status(port, OP_RESUME_STATUS);
            break;
        }
        case PORT_RESUME_RECOVERY: {
            resume_done(port);
            break;
        }
        case PORT_RECOVERY: {
            struct tendril_device *dev = tendril_device_alloc(hc, port, port->st.speed);
            if (dev == NULL) {
                release_slot(port);
                give_up(port);
                break;
            }
            port->device = dev;
            port->state = PORT_ENUM;
            hc_lock(hc);
            hc->enum_dev = dev;
            hc_unlock(hc);
            /* Status checks may run during enumeration, to notice disconnects. */
            port_idle(port);
            tendril_device_enumerate(dev);
            break;
        }
        default: {
            port_idle(port);
            break;
        }
    }
}

static void port_work(struct tendril_work *work) {
    struct tendril_port *port = CONTAINER_OF(work, struct tendril_port, async.work);

    switch (port->op) {
        case OP_KICK: {
            start_status(port, OP_STATUS);
            break;
        }
        case OP_STATUS: {
            status_done(port);
            break;
        }
        case OP_CLEAR: {
            clear_done(port);
            break;
        }
        case OP_RESET: {
            if (port->async.status != TENDRIL_OK) {
                reset_failed(port, tendril_status_str(port->async.status));
                break;
            }
            port_wait(port, RESET_POLL_MS);
            break;
        }
        case OP_RESET_STATUS: {
            reset_status_done(port);
            break;
        }
        case OP_RESET_CLEAR: {
            reset_clear_done(port);
            break;
        }
        case OP_ENUM_START: {
            start_reset(port);
            break;
        }
        case OP_PM_START: {
            if (port_hc(port)->removing || port->hub->stopping) {
                port_idle(port);
                break;
            }
            pm_start(port);
            break;
        }
        case OP_SUSPEND: {
            if (port->async.status != TENDRIL_OK) {
                port->state = PORT_ATTACHED;
                pm_complete(port, port->async.status);
                port_idle(port);
                break;
            }
            port_wait(port, SUSPEND_POLL_MS);
            break;
        }
        case OP_SUSPEND_STATUS: {
            suspend_status_done(port);
            break;
        }
        case OP_RESUME: {
            if (port->async.status != TENDRIL_OK) {
                port->state = PORT_SUSPENDED;
                pm_complete(port, port->async.status);
                port_idle(port);
                break;
            }
            port_wait(port, RESUME_POLL_MS);
            break;
        }
        case OP_RESUME_STATUS: {
            resume_status_done(port);
            break;
        }
        case OP_RESUME_CLEAR: {
            if (!clear_next(port, OP_RESUME_CLEAR)) {
                port_wait(port, RESUME_RECOVERY_MS);
            }
            break;
        }
        default: {
            port_idle(port);
            break;
        }
    }
}

void tendril_port_init(struct tendril_port *port, struct tendril_hub *hub, size_t number) {
    memset(port, 0, sizeof(struct tendril_port));
    port->hub = hub;
    port->number = (uint8_t)number;
    port->superspeed = hub->superspeed;
    port->state = PORT_IDLE;
    tendril_async_init(&port->async, port_work);
    tendril_timer_init(&port->timer, port_timer);
    list_init(&port->enum_node);
}

/* With the lock held. */
void tendril_port_kick(struct tendril_port *port) {
    port->kick = true;
    if (!port->busy) {
        port->busy = true;
        port->op = OP_KICK;
        tendril_queue_work(port_hc(port), &port->async.work);
    }
}

/* Stops all activity on a port of a hub going away; the lock is held. */
void tendril_port_stop(struct tendril_port *port) {
    struct tendril_hc *hc = port_hc(port);
    tendril_timer_cancel(hc, &port->timer);
    if (list_linked(&port->enum_node)) {
        list_del(&port->enum_node);
    }
    if (hc->enum_port == port && hc->enum_dev == NULL) {
        enum_release_locked(hc);
    }
}
