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
 * Parts of this file are derived from FreeBSD's
 * sys/dev/usb/controller/xhci_pci.c (commit
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
 * sys/dev/usb/controller/ehci_pci.c (commit
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

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/quirk/usb_quirk.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1998 The NetBSD Foundation, Inc. All rights reserved.
 * Copyright (c) 1998 Lennart Augustsson. All rights reserved.
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/usbdevs (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * Copyright (c) 1998-2004 The NetBSD Foundation, Inc.
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

#include <stddef.h>
#include <stdint.h>
#include "../internal.h"
#include <tendril/spec.h>
#include <tendril/tendril.h>

#define ANY 0xffff

struct hc_quirk {
    uint16_t vendor;
    uint16_t device;
    int type;
    uint32_t quirks;
};

/* BEGIN FreeBSD sys/dev/usb/controller/xhci_pci.c, ehci_pci.c */
/*
 * From FreeBSD's sys/dev/usb/controller/xhci_pci.c and ehci_pci.c
 * (BSD-2-Clause), the latter's "dropped interrupts workaround".
 */
#define INTEL_LP \
    (TENDRIL_HC_QUIRK_INTEL_PORT_ROUTING | TENDRIL_HC_QUIRK_XHCI_CTRL_STEP \
        | TENDRIL_HC_QUIRK_XHCI_IMOD_LP)

static const struct hc_quirk hc_quirks[] = {
    { 0x1033, 0x0194, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_NO_64BIT },
    { 0x1912, 0x0014, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_NO_64BIT },
    { 0x1b73, 0x1000, TENDRIL_HC_XHCI, TENDRIL_HC_QUIRK_NO_MSI },
    { 0x8086, 0x0f35, TENDRIL_HC_XHCI, INTEL_LP },
    { 0x8086, 0x9c31, TENDRIL_HC_XHCI, INTEL_LP },
    { 0x8086, 0x1e31, TENDRIL_HC_XHCI, INTEL_LP },
    { 0x8086, 0x8c31, TENDRIL_HC_XHCI, INTEL_LP },
    { 0x8086, 0x8cb1, TENDRIL_HC_XHCI, INTEL_LP },
    { 0x8086, 0x9cb1, TENDRIL_HC_XHCI, INTEL_LP },
    { 0x1002, ANY, TENDRIL_HC_EHCI, TENDRIL_HC_QUIRK_EHCI_LOST_INTR },
    { 0x1106, ANY, TENDRIL_HC_EHCI, TENDRIL_HC_QUIRK_EHCI_LOST_INTR }
};
/* END FreeBSD sys/dev/usb/controller/xhci_pci.c, ehci_pci.c */

/* BEGIN FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/usbdevs */
/*
 * From FreeBSD's sys/dev/usb/quirk/usb_quirk.c (BSD-2-Clause), naming the IDs
 * as its sys/dev/usb/usbdevs (BSD-2-Clause) does.
 */
static const struct tendril_id_quirk device_quirks[] = {
    /* INSIDEOUT EDGEPORT4 */
    { 0x1608, 0x0001, 0x0094, 0x0094, TENDRIL_DEV_QUIRK_SWAP_UNICODE },
    /* LOGITECH UN53B */
    { 0x046d, 0xc032, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_NO_STRINGS },
    /* REALTEK RTL8196EU */
    { 0x0bda, 0x8196, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 },
    /* ELSA MODEM1 */
    { 0x05cc, 0x2265, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 },
    /* PLANEX2 MZKUE150N */
    { 0x2019, 0xab2f, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 },
    /* CISCOLINKSYS USB3GIGV1 */
    { 0x13b1, 0x0041, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 },
    /* QUALCOMMINC ZTE_MF730M */
    { 0x19d2, 0x1420, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_0 },
    /* PLOYTEC SPL_CRIMSON_1 */
    { 0x0a4a, 0xc150, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 },
    /* LENOVO TBT3LAN */
    { 0x17ef, 0x3069, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 },
    /* REALTEK RTL8153 */
    { 0x0bda, 0x8153, 0x31fd, 0x31fd, TENDRIL_DEV_QUIRK_CONFIG_INDEX_1 },
    /* MCT HUB0100 */
    { 0x0711, 0x0100, 0x0102, 0x0102, TENDRIL_DEV_QUIRK_BUS_POWERED },
    /* MCT USB232 */
    { 0x0711, 0x0210, 0x0102, 0x0102, TENDRIL_DEV_QUIRK_BUS_POWERED },
    /* DYMO LABELMANAGERPNP */
    { 0x0922, 0x1001, 0x0000, 0xffff, TENDRIL_DEV_QUIRK_DYMO_EJECT },
};
/* END FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/usbdevs */

uint32_t tendril_pci_hc_quirks(int type, uint16_t vendor, uint16_t device) {
    uint32_t quirks = 0;
    for (size_t i = 0; i < ARRAY_SIZE(hc_quirks); i++) {
        const struct hc_quirk *q = &hc_quirks[i];
        if (q->vendor == 0 || q->type != type || q->vendor != vendor) {
            continue;
        }
        if (q->device == ANY || q->device == device) {
            quirks |= q->quirks;
        }
    }
    return quirks;
}

uint32_t tendril_hc_quirk_lookup(struct tendril_hc *hc) {
    if (!hc->config.pci) {
        return 0;
    }
    return tendril_pci_hc_quirks(
        hc->config.type, hc->pci_vendor, hc->pci_device
    );
}

uint32_t tendril_id_quirks(
    const struct tendril_id_quirk *table, size_t count,
    const struct tendril_device_descriptor *desc
) {
    uint16_t vendor = tendril_le16(desc->vendor_id);
    uint16_t product = tendril_le16(desc->product_id);
    uint16_t bcd = tendril_le16(desc->bcd_device);
    uint32_t quirks = 0;
    for (size_t i = 0; i < count; i++) {
        const struct tendril_id_quirk *q = &table[i];
        if (q->vendor == vendor && (q->product == ANY || q->product == product)
         && bcd >= q->bcd_min && bcd <= q->bcd_max) {
            quirks |= q->quirks;
        }
    }
    return quirks;
}

uint32_t tendril_device_quirk_lookup(
    const struct tendril_device_descriptor *desc
) {
    return tendril_id_quirks(device_quirks, ARRAY_SIZE(device_quirks), desc);
}
