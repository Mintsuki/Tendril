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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/quirk/usb_quirk.h
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/input/ukbd.c,
 * sys/dev/hid/hkbd.c, and sys/dev/usb/usb_hid.c (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
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

/*
 * Parts of this file are derived from FreeBSD's sys/dev/hid/hms.c (commit
 * dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2020 Vladimir Kondratyev <wulf@FreeBSD.org>
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
#include "../../include/tendril/hid.h"
#include "../../include/tendril/spec.h"
#include "../../include/tendril/tendril.h"
#include "../internal.h"

/* HID 1.11 6.2.1: a HID descriptor with one class descriptor entry. */
#define HID_DESCRIPTOR_MIN_SIZE 9
#define HID_CTRL_TIMEOUT_MS 5000
#define HID_RETRY_MS 20
#define HID_MAX_ERRORS 50
#define KEY_ERROR_ROLLOVER 0x01
#define KEY_ERROR_UNDEFINED 0x03
#define KEY_GRAVE_ACCENT 0x35
#define KEY_NON_US_BACKSLASH 0x64
#define KEY_LEFT_CTRL 0xe0
/* HID Usage Tables 1.7, table 3.1: the first vendor-defined usage page. */
#define PAGE_VENDOR_FIRST 0xff00
#define BUTTON_SECONDARY 2
#define BUTTON_TERTIARY 3

/* BEGIN FreeBSD sys/dev/usb/quirk/usb_quirk.h, sys/dev/usb/input/ukbd.c */
/*
 * HID quirks: FreeBSD's UQ_HID_IGNORE, UQ_KBD_BOOTPROTO, and UQ_MS_VENDOR_BTN
 * (sys/dev/usb/quirk/usb_quirk.h, BSD-2-Clause), and the Apple ISO keyboards
 * whose two keys its sys/dev/usb/input/ukbd.c (BSD-2-Clause) swaps back.
 */
#define HQ_IGNORE ((uint32_t)1 << 0)
#define HQ_BOOT_PROTOCOL ((uint32_t)1 << 1)
#define HQ_VENDOR_BUTTONS ((uint32_t)1 << 2)
#define HQ_APPLE_SWAP ((uint32_t)1 << 3)
/* END FreeBSD sys/dev/usb/quirk/usb_quirk.h, sys/dev/usb/input/ukbd.c */

/* BEGIN FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/input/ukbd.c, sys/dev/usb/usbdevs */
/*
 * From FreeBSD's sys/dev/usb/quirk/usb_quirk.c and, for HQ_APPLE_SWAP, the
 * ukbd_apple_iso_models[] of its sys/dev/usb/input/ukbd.c (both BSD-2-Clause),
 * naming the IDs as its sys/dev/usb/usbdevs (BSD-2-Clause) does.
 */
static const struct tendril_id_quirk hid_quirks[] = {
    { 0x0b05, 0x1726, 0x0000, 0xffff, HQ_IGNORE }, /* ASUS LCM */
    { 0x046d, 0xc22d, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* LOGITECH G510S */
    { 0x051d, 0x0002, 0x0000, 0xffff, HQ_IGNORE }, /* APC UPS */
    { 0x051d, 0x0003, 0x0000, 0xffff, HQ_IGNORE }, /* APC UPS1000 */
    { 0x050d, 0x0375, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6H375USB */
    { 0x050d, 0x0551, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C550AVR */
    { 0x050d, 0x0750, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C1250TWRK */
    { 0x050d, 0x0751, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C1500TWRK */
    { 0x050d, 0x0900, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C900UNV */
    { 0x050d, 0x0910, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C100UNV */
    { 0x050d, 0x0912, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C120UNV */
    { 0x050d, 0x0980, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C800UNV */
    { 0x050d, 0x1100, 0x0000, 0xffff, HQ_IGNORE }, /* BELKIN F6C1100UNV */
    { 0x0764, 0x0005, 0x0000, 0xffff, HQ_IGNORE }, /* CYBERPOWER BC900D */
    { 0x0764, 0x0501, 0x0000, 0xffff, HQ_IGNORE }, /* CYBERPOWER 1500CAVRLCD */
    { 0x0764, 0x0601, 0x0000, 0xffff, HQ_IGNORE }, /* CYBERPOWER OR2200LCDRM2U */
    { 0x047c, 0xffff, 0x0000, 0xffff, HQ_IGNORE }, /* DELL2 VARIOUS_UPS */
    { 0x04b4, 0xfd13, 0x0000, 0xffff, HQ_IGNORE }, /* CYPRESS SILVERSHIELD */
    { 0x1163, 0x0100, 0x0000, 0xffff, HQ_IGNORE }, /* DELORME EARTHMATE */
    { 0x1d34, 0x0004, 0x0000, 0xffff, HQ_IGNORE }, /* DREAMLINK DL100B */
    { 0x04d8, 0x0002, 0x0000, 0xffff, HQ_IGNORE }, /* MICROCHIP PICOLCD20X2 */
    { 0x04d8, 0xc001, 0x0000, 0xffff, HQ_IGNORE }, /* MICROCHIP PICOLCD4X20 */
    { 0x06da, 0xffff, 0x0000, 0xffff, HQ_IGNORE }, /* LIEBERT POWERSURE_PXT */
    { 0x10af, 0x0004, 0x0000, 0xffff, HQ_IGNORE }, /* LIEBERT2 PSI1000 */
    { 0x10af, 0x0001, 0x0000, 0xffff, HQ_IGNORE }, /* LIEBERT2 POWERSURE_PSA */
    { 0x0463, 0x0001, 0x0000, 0xffff, HQ_IGNORE }, /* MGE UPS1 */
    { 0x0463, 0xffff, 0x0000, 0xffff, HQ_IGNORE }, /* MGE UPS2 */
    { 0x0d9f, 0x00a2, 0x0000, 0xffff, HQ_IGNORE }, /* POWERCOM IMPERIAL_SERIES */
    { 0x0d9f, 0x00a3, 0x0000, 0xffff, HQ_IGNORE }, /* POWERCOM SMART_KING_PRO */
    { 0x0d9f, 0x00a4, 0x0000, 0xffff, HQ_IGNORE }, /* POWERCOM WOW */
    { 0x0d9f, 0x00a5, 0x0000, 0xffff, HQ_IGNORE }, /* POWERCOM VANGUARD */
    { 0x0d9f, 0x00a6, 0x0000, 0xffff, HQ_IGNORE }, /* POWERCOM BLACK_KNIGHT_PRO */
    { 0x09ae, 0x1003, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 AVR550U */
    { 0x09ae, 0x1007, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 AVR750U */
    { 0x09ae, 0x1008, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 ECO550UPS */
    { 0x09ae, 0x1f06, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 T750_INTL */
    { 0x09ae, 0x1f0a, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 RT_2200_INTL */
    { 0x09ae, 0x2005, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 OMNI1000LCD */
    { 0x09ae, 0x2007, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 OMNI900LCD */
    { 0x09ae, 0x3012, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 SMART_2200RMXL2U */
    { 0x09ae, 0x3014, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 UPS_3014 */
    { 0x09ae, 0x4001, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 SU1500RTXL2UA */
    { 0x09ae, 0x4002, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 SU6000RT4U */
    { 0x09ae, 0x4003, 0x0000, 0xffff, HQ_IGNORE }, /* TRIPPLITE2 SU1500RTXL2UA_2 */
    { 0x05ac, 0x1290, 0x0000, 0xffff, HQ_IGNORE }, /* APPLE IPHONE */
    { 0x05ac, 0x1292, 0x0000, 0xffff, HQ_IGNORE }, /* APPLE IPHONE_3G */
    { 0x0665, 0x5161, 0x0000, 0xffff, HQ_IGNORE }, /* MEGATEC UPS */
    { 0x04b4, 0x0bad, 0x0000, 0xffff, HQ_IGNORE }, /* CYPRESS WISPY1A */
    { 0x1781, 0x083e, 0x0000, 0xffff, HQ_IGNORE }, /* METAGEEK WISPY1B */
    { 0x1781, 0x083f, 0x0000, 0xffff, HQ_IGNORE }, /* METAGEEK WISPY24X */
    { 0x1dd5, 0x5000, 0x0000, 0xffff, HQ_IGNORE }, /* METAGEEK2 WISPYDBX */
    { 0x045e, 0x00db, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* MICROSOFT NATURAL4000 */
    { 0x1b1c, 0x0a60, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* CORSAIR K60 */
    { 0x1b1c, 0x1b3f, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* CORSAIR K68 */
    { 0x1b1c, 0x1b09, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* CORSAIR K70 */
    { 0x1b1c, 0x1b13, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* CORSAIR K70_RGB */
    { 0x1b1c, 0x1b15, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* CORSAIR STRAFE */
    { 0x1b1c, 0x1b44, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* CORSAIR STRAFE2 */
    { 0x04d9, 0xa030, 0x0000, 0xffff, HQ_BOOT_PROTOCOL }, /* HOLTEK F85 */
    { 0x047d, 0x2041, 0x0000, 0xffff, HQ_VENDOR_BUTTONS }, /* KENSINGTON SLIMBLADE */
    { 0x05ac, 0x020f, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE FOUNTAIN_ISO */
    { 0x05ac, 0x0215, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE GEYSER_ISO */
    { 0x05ac, 0x0218, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE GEYSER3_ISO */
    { 0x05ac, 0x021b, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE GEYSER4_ISO */
    { 0x05ac, 0x022a, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE GEYSER4_HF_ISO */
    { 0x05ac, 0x021e, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE ALU_MINI_ISO */
    { 0x05ac, 0x0221, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE ALU_ISO */
    { 0x05ac, 0x0250, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE ALU_REVB_ISO */
    { 0x05ac, 0x0224, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING_ISO */
    { 0x05ac, 0x0231, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING2_ISO */
    { 0x05ac, 0x0237, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING3_ISO */
    { 0x05ac, 0x0240, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING4_ISO */
    { 0x05ac, 0x0243, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING4A_ISO */
    { 0x05ac, 0x0246, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING5_ISO */
    { 0x05ac, 0x0253, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING5A_ISO */
    { 0x05ac, 0x024d, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING6_ISO */
    { 0x05ac, 0x024a, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING6A_ISO */
    { 0x05ac, 0x0263, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING7_ISO */
    { 0x05ac, 0x025a, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING7A_ISO */
    { 0x05ac, 0x0291, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING8_ISO */
    { 0x05ac, 0x0273, 0x0000, 0xffff, HQ_APPLE_SWAP }, /* APPLE WELLSPRING9_ISO */
};
/* END FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/input/ukbd.c, sys/dev/usb/usbdevs */

/* HID 1.11 appendix B: the boot protocol report layouts. */
static const uint8_t boot_keyboard_desc[] = {
    0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x75, 0x01, 0x95, 0x08, 0x05, 0x07, 0x19, 0xe0, 0x29, 0xe7,
    0x15, 0x00, 0x25, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01,
    0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06,
    0x75, 0x08, 0x15, 0x00, 0x26, 0xff, 0x00, 0x05, 0x07, 0x19, 0x00, 0x29, 0xff, 0x81, 0x00, 0xc0
};

static const uint8_t boot_mouse_desc[] = {
    0x05, 0x01, 0x09, 0x02, 0xa1, 0x01, 0x09, 0x01, 0xa1, 0x00, 0x95, 0x03, 0x75, 0x01, 0x05, 0x09,
    0x19, 0x01, 0x29, 0x03, 0x15, 0x00, 0x25, 0x01, 0x81, 0x02, 0x95, 0x01, 0x75, 0x05, 0x81, 0x01,
    0x75, 0x08, 0x95, 0x02, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x15, 0x81, 0x25, 0x7f, 0x81, 0x06,
    0xc0, 0xc0
};

#define STEP_SET_BOOT 0
#define STEP_GET_REPORT_DESC 1
#define STEP_SET_REPORT_PROTOCOL 2
#define STEP_SET_IDLE 3
#define STEP_RUNNING 4

struct tendril_hid {
    struct tendril_interface *intf;
    struct tendril_device *dev;
    struct tendril_endpoint *in_ep;
    struct tendril_endpoint *out_ep;
    struct tendril_hid_report_desc *rd;
    int subclass;
    int protocol;
    uint32_t quirks;
    bool boot;
    bool stopped;
    bool suspended;
    uint32_t kinds;
    void *user;
    int step;
    struct tendril_async async;
    struct tendril_transfer ctrl;
    uint8_t *desc_buf;
    size_t desc_len;
    struct tendril_transfer in_xfer;
    uint8_t *in_buf;
    size_t in_size;
    size_t errors;
    struct tendril_timer retry;
    uint8_t keys[32];
    uint8_t modifiers;
    struct tendril_transfer led_xfer;
    uint8_t led_buf[16];
    bool led_busy;
    bool led_dirty;
    uint8_t leds;
};

static const struct tendril_hid_callbacks *callbacks;
static uint32_t hid_flags;

static bool bit_get(const uint8_t *map, size_t k) {
    return ((map[k / 8] >> (k % 8)) & 1) != 0;
}

static void bit_set(uint8_t *map, size_t k) {
    map[k / 8] |= (uint8_t)((uint32_t)1 << (k % 8));
}

/* Input processing */

/* BEGIN FreeBSD sys/dev/usb/input/ukbd.c */
static size_t apple_swap(const struct tendril_hid *hid, size_t key) {
    if ((hid->quirks & HQ_APPLE_SWAP) == 0) {
        return key;
    }
    if (key == KEY_GRAVE_ACCENT) {
        return KEY_NON_US_BACKSLASH;
    }
    if (key == KEY_NON_US_BACKSLASH) {
        return KEY_GRAVE_ACCENT;
    }
    return key;
}
/* END FreeBSD sys/dev/usb/input/ukbd.c */

/* The number, from 1, of the button a usage reports, or 0. */
static size_t button_number(const struct tendril_hid *hid, uint32_t usage) {
    size_t n = usage & 0xffff;
    if ((usage >> 16) == TENDRIL_HID_PAGE_BUTTON) {
        return n <= 32 ? n : 0;
    }
    /* BEGIN FreeBSD sys/dev/hid/hms.c */
    /*
     * The buttons FreeBSD's sys/dev/hid/hms.c (BSD-2-Clause) maps these usages
     * to.
     */
    if ((hid->quirks & HQ_VENDOR_BUTTONS) != 0 && (usage >> 16) == PAGE_VENDOR_FIRST) {
        if (n == 1) {
            return BUTTON_SECONDARY;
        }
        if (n == 2) {
            return BUTTON_TERTIARY;
        }
    }
    /* END FreeBSD sys/dev/hid/hms.c */
    return 0;
}

static void process_keys(struct tendril_hid *hid, uint8_t id, const uint8_t *data, size_t size) {
    const struct tendril_hid_report_desc *rd = hid->rd;
    uint8_t now[32] = { 0 };
    uint8_t covered[32] = { 0 };
    bool any = false;

    for (size_t i = 0; i < rd->num_fields; i++) {
        const struct tendril_hid_field *f = &rd->fields[i];
        if (f->type != TENDRIL_HID_INPUT || f->report_id != id || (f->flags & TENDRIL_HID_CONSTANT) != 0) {
            continue;
        }
        if ((f->flags & TENDRIL_HID_VARIABLE) != 0) {
            for (size_t e = 0; e < f->count; e++) {
                uint32_t usage = tendril_hid_field_usage(f, e);
                if ((usage >> 16) != TENDRIL_HID_PAGE_KEYBOARD || (usage & 0xffff) > 0xff) {
                    continue;
                }
                any = true;
                size_t key = apple_swap(hid, usage & 0xff);
                bit_set(covered, key);
                if (tendril_hid_field_get(f, data, size, e) != 0) {
                    bit_set(now, key);
                }
            }
            continue;
        }
        if ((f->usage_min >> 16) != TENDRIL_HID_PAGE_KEYBOARD) {
            continue;
        }
        any = true;
        for (size_t u = f->usage_min & 0xffff; u <= (f->usage_max & 0xffff) && u <= 0xff; u++) {
            bit_set(covered, apple_swap(hid, u));
        }
        for (size_t e = 0; e < f->count; e++) {
            uint32_t usage = tendril_hid_array_usage(f, tendril_hid_field_get(f, data, size, e));
            size_t key = usage & 0xffff;
            if ((usage >> 16) != TENDRIL_HID_PAGE_KEYBOARD || key > 0xff) {
                continue;
            }
            /*
             * HID Usage Tables 1.7, 10: ErrorRollOver to ErrorUndefined mean
             * the report is unusable.
             */
            if (key >= KEY_ERROR_ROLLOVER && key <= KEY_ERROR_UNDEFINED) {
                return;
            }
            if (key != 0) {
                bit_set(now, apple_swap(hid, key));
            }
        }
    }
    if (!any) {
        return;
    }

    uint8_t mods = 0;
    for (size_t k = 0; k < 8; k++) {
        bool down = bit_get(covered, KEY_LEFT_CTRL + k) ? bit_get(now, KEY_LEFT_CTRL + k)
            : ((hid->modifiers >> k) & 1) != 0;
        if (down) {
            mods |= (uint8_t)((uint32_t)1 << k);
        }
    }

    /* Releases first, so a quick re-press of a key is never lost. */
    for (size_t k = 0; k < 256; k++) {
        if (bit_get(covered, k) && bit_get(hid->keys, k) && !bit_get(now, k)) {
            hid->keys[k / 8] &= (uint8_t)~((uint32_t)1 << (k % 8));
            if (callbacks->key != NULL) {
                callbacks->key(hid, (uint16_t)k, false, mods);
            }
        }
    }
    for (size_t k = 0; k < 256; k++) {
        if (bit_get(covered, k) && !bit_get(hid->keys, k) && bit_get(now, k)) {
            bit_set(hid->keys, k);
            if (callbacks->key != NULL) {
                callbacks->key(hid, (uint16_t)k, true, mods);
            }
        }
    }
    hid->modifiers = mods;
}

static void process_pointer(struct tendril_hid *hid, uint8_t id, const uint8_t *data, size_t size) {
    const struct tendril_hid_report_desc *rd = hid->rd;
    struct tendril_hid_pointer_event ev;
    bool have = false;

    memset(&ev, 0, sizeof(struct tendril_hid_pointer_event));
    for (size_t i = 0; i < rd->num_fields; i++) {
        const struct tendril_hid_field *f = &rd->fields[i];
        if (f->type != TENDRIL_HID_INPUT || f->report_id != id || (f->flags & TENDRIL_HID_CONSTANT) != 0) {
            continue;
        }
        if ((f->flags & TENDRIL_HID_VARIABLE) == 0) {
            for (size_t e = 0; e < f->count; e++) {
                uint32_t usage = tendril_hid_array_usage(f, tendril_hid_field_get(f, data, size, e));
                size_t n = button_number(hid, usage);
                if (n != 0) {
                    ev.buttons |= (uint32_t)1 << (n - 1);
                    have = true;
                }
            }
            continue;
        }
        bool relative = (f->flags & TENDRIL_HID_RELATIVE) != 0;
        for (size_t e = 0; e < f->count; e++) {
            uint32_t usage = tendril_hid_field_usage(f, e);
            int32_t v = tendril_hid_field_get(f, data, size, e);
            switch (usage) {
                case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x30): {
                    if (relative) {
                        ev.dx += v;
                    } else {
                        ev.absolute = true;
                        ev.x = v;
                        ev.x_min = f->logical_min;
                        ev.x_max = f->logical_max;
                    }
                    have = true;
                    break;
                }
                case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x31): {
                    if (relative) {
                        ev.dy += v;
                    } else {
                        ev.absolute = true;
                        ev.y = v;
                        ev.y_min = f->logical_min;
                        ev.y_max = f->logical_max;
                    }
                    have = true;
                    break;
                }
                case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x38): {
                    ev.wheel += v;
                    have = true;
                    break;
                }
                case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_CONSUMER, 0x238): {
                    ev.pan += v;
                    have = true;
                    break;
                }
                default: {
                    size_t n = button_number(hid, usage);
                    if (n != 0) {
                        if (v != 0) {
                            ev.buttons |= (uint32_t)1 << (n - 1);
                        }
                        have = true;
                    }
                    break;
                }
            }
        }
    }
    if (have && callbacks->pointer != NULL) {
        callbacks->pointer(hid, &ev);
    }
}

static void in_submit(struct tendril_hid *hid);

static void in_done(struct tendril_transfer *xfer) {
    struct tendril_hid *hid = xfer->context;
    struct tendril_hc *hc = hid->dev->hc;

    if (hid->stopped) {
        return;
    }
    switch (xfer->status) {
        case TENDRIL_OK: {
            hid->errors = 0;
            const uint8_t *data = hid->in_buf;
            size_t size = xfer->actual;
            uint8_t id = 0;
            if (hid->rd->numbered && !hid->boot) {
                if (size == 0) {
                    break;
                }
                id = data[0];
                data++;
                size--;
            }
            if (callbacks->report != NULL) {
                callbacks->report(hid, id, data, size);
            }
            if (hid->stopped) {
                return;
            }
            if ((hid->kinds & TENDRIL_HID_KEYBOARD) != 0) {
                process_keys(hid, id, data, size);
            }
            if ((hid->kinds & (TENDRIL_HID_MOUSE | TENDRIL_HID_ABSOLUTE)) != 0) {
                process_pointer(hid, id, data, size);
            }
            break;
        }
        case TENDRIL_ERR_NO_DEVICE:
        case TENDRIL_ERR_CANCELLED:
        case TENDRIL_ERR_SUSPENDED: {
            return;
        }
        default: {
            if (++hid->errors > HID_MAX_ERRORS) {
                LOG_WARN("HID: input keeps failing (%s), giving up", tendril_status_str(xfer->status));
                return;
            }
            if (xfer->status == TENDRIL_ERR_STALL) {
                tendril_endpoint_clear_halt(hid->in_ep, NULL, NULL);
            }
            hc_lock(hc);
            tendril_timer_arm(hc, &hid->retry, HID_RETRY_MS * NS_PER_MS);
            hc_unlock(hc);
            return;
        }
    }
    if (!hid->stopped && !hid->suspended) {
        in_submit(hid);
    }
}

static void retry_work(struct tendril_work *work) {
    struct tendril_hid *hid = CONTAINER_OF(work, struct tendril_hid, retry.work);
    if (!hid->stopped && !hid->suspended) {
        in_submit(hid);
    }
}

static void in_submit(struct tendril_hid *hid) {
    struct tendril_transfer *xfer = &hid->in_xfer;
    tendril_transfer_init(xfer, hid->in_ep);
    xfer->buffer = hid->in_buf;
    xfer->length = hid->in_size;
    xfer->callback = in_done;
    xfer->context = hid;
    int st = tendril_transfer_submit(xfer);
    if (st != TENDRIL_OK && st != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN("HID: cannot poll input: %s", tendril_status_str(st));
    }
}

/* Setup */

static uint32_t classify(const struct tendril_hid_report_desc *rd) {
    uint32_t kinds = 0;
    for (size_t i = 0; i < rd->num_fields; i++) {
        const struct tendril_hid_field *f = &rd->fields[i];
        if (f->type != TENDRIL_HID_INPUT) {
            continue;
        }
        switch (f->application) {
            case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x06):
            case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x07): {
                kinds |= TENDRIL_HID_KEYBOARD;
                break;
            }
            case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x01):
            case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x02): {
                for (size_t e = 0; e < f->count && (f->flags & TENDRIL_HID_VARIABLE) != 0; e++) {
                    if (tendril_hid_field_usage(f, e) == TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x30)) {
                        kinds |= (f->flags & TENDRIL_HID_RELATIVE) != 0 ? TENDRIL_HID_MOUSE : TENDRIL_HID_ABSOLUTE;
                    }
                }
                break;
            }
            case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x04):
            case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_GENERIC_DESKTOP, 0x05): {
                kinds |= TENDRIL_HID_GAMEPAD;
                break;
            }
            case TENDRIL_HID_USAGE(TENDRIL_HID_PAGE_CONSUMER, 0x01): {
                kinds |= TENDRIL_HID_CONSUMER;
                break;
            }
            default: {
                if ((f->application >> 16) == TENDRIL_HID_PAGE_DIGITIZER) {
                    kinds |= TENDRIL_HID_ABSOLUTE;
                }
                break;
            }
        }
    }
    return kinds;
}

static size_t largest_input(const struct tendril_hid_report_desc *rd) {
    size_t max = 0;
    for (size_t i = 0; i < rd->num_reports; i++) {
        if (rd->reports[i].type == TENDRIL_HID_INPUT) {
            size_t n = (rd->reports[i].bits + 7) / 8 + (rd->numbered ? 1 : 0);
            max = MAX(max, n);
        }
    }
    return max;
}

static void hid_ready(struct tendril_hid *hid) {
    hid->kinds = classify(hid->rd);
    size_t report = largest_input(hid->rd);
    size_t mps = hid->in_ep->max_packet;
    /*
     * A full-size report ends the transfer only if the transfer is exactly
     * that long.
     */
    hid->in_size = report >= mps ? report : mps;
    hid->in_buf = tendril_zalloc(hid->in_size);
    if (hid->in_buf == NULL) {
        hid->stopped = true;
        return;
    }
    hid->step = STEP_RUNNING;
    LOG_INFO(
        "HID: interface %u ready:%s%s%s%s%s%s", hid->intf->number,
        (hid->kinds & TENDRIL_HID_KEYBOARD) != 0 ? " keyboard" : "",
        (hid->kinds & TENDRIL_HID_MOUSE) != 0 ? " mouse" : "",
        (hid->kinds & TENDRIL_HID_ABSOLUTE) != 0 ? " absolute pointer" : "",
        (hid->kinds & TENDRIL_HID_GAMEPAD) != 0 ? " gamepad" : "",
        (hid->kinds & TENDRIL_HID_CONSUMER) != 0 ? " consumer" : "", hid->boot ? " (boot protocol)" : ""
    );
    if (callbacks->attach != NULL) {
        callbacks->attach(hid);
    }
    if (!hid->stopped) {
        in_submit(hid);
    }
}

static void hid_fail(struct tendril_hid *hid, int st, const char *what) {
    if (st != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN("HID: %s failed: %s", what, tendril_status_str(st));
    }
    hid->stopped = true;
}

static void ctrl(
    struct tendril_hid *hid, int step, uint8_t request_type, uint8_t request, uint16_t value, void *data, size_t length
) {
    hid->step = step;
    int st = tendril_control_async(
        hid->dev, &hid->ctrl, request_type, request, value, hid->intf->number, data, length, &hid->async
    );
    if (st != TENDRIL_OK) {
        hid->async.status = st;
        hid->async.work.fn(&hid->async.work);
    }
}

static void set_idle(struct tendril_hid *hid) {
    /* HID 1.11 7.2.4: duration 0, report only on change. */
    ctrl(
        hid, STEP_SET_IDLE, TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE, TENDRIL_HID_REQ_SET_IDLE,
        0, NULL, 0
    );
}

/*
 * HID 1.11 4.2 and 4.3: a keyboard or a mouse of the boot interface subclass.
 */
static bool boot_capable(int subclass, int protocol) {
    return subclass == 1 && (protocol == 1 || protocol == 2);
}

static void start_boot(struct tendril_hid *hid) {
    hid->boot = true;
    ctrl(
        hid, STEP_SET_BOOT, TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE,
        TENDRIL_HID_REQ_SET_PROTOCOL, 0, NULL, 0
    );
}

/* BEGIN FreeBSD sys/dev/hid/hkbd.c, hms.c */
/*
 * A boot keyboard or mouse whose report descriptor cannot be read or parsed,
 * and a boot keyboard whose report descriptor has no keys, use the boot
 * protocol instead, as FreeBSD's sys/dev/hid/hkbd.c and hms.c (BSD-2-Clause)
 * do.
 */
static void report_fallback(struct tendril_hid *hid, int st, const char *what) {
    if (!boot_capable(hid->subclass, hid->protocol)) {
        hid_fail(hid, st, what);
        return;
    }
    LOG_WARN("HID: %s failed: %s; using the boot protocol", what, tendril_status_str(st));
    tendril_hid_free(hid->rd);
    hid->rd = NULL;
    start_boot(hid);
}

static bool has_keys(const struct tendril_hid_report_desc *rd) {
    for (size_t i = 0; i < rd->num_fields; i++) {
        const struct tendril_hid_field *f = &rd->fields[i];
        if (f->type == TENDRIL_HID_INPUT && (f->flags & TENDRIL_HID_CONSTANT) == 0
         && (f->usage_min >> 16) == TENDRIL_HID_PAGE_KEYBOARD) {
            return true;
        }
    }
    return false;
}
/* END FreeBSD sys/dev/hid/hkbd.c, hms.c */

static void init_step(struct tendril_work *work) {
    struct tendril_hid *hid = CONTAINER_OF(work, struct tendril_hid, async.work);
    int st = hid->async.status;

    if (hid->stopped || hid->dev->gone || st == TENDRIL_ERR_NO_DEVICE) {
        hid->stopped = true;
        return;
    }
    switch (hid->step) {
        case STEP_SET_BOOT: {
            /* A failure is ignored, as FreeBSD's hkbd.c does. */
            const uint8_t *desc = hid->protocol == 1 ? boot_keyboard_desc : boot_mouse_desc;
            size_t len = hid->protocol == 1 ? sizeof(boot_keyboard_desc) : sizeof(boot_mouse_desc);
            st = tendril_hid_parse(desc, len, &hid->rd);
            if (st != TENDRIL_OK) {
                hid_fail(hid, st, "boot descriptor parsing");
                return;
            }
            set_idle(hid);
            return;
        }
        case STEP_GET_REPORT_DESC: {
            /* BEGIN FreeBSD sys/dev/hid/hkbd.c, hms.c */
            if (st != TENDRIL_OK && st != TENDRIL_ERR_SHORT) {
                report_fallback(hid, st, "reading the report descriptor");
                return;
            }
            st = tendril_hid_parse(hid->desc_buf, hid->ctrl.actual, &hid->rd);
            if (st != TENDRIL_OK) {
                report_fallback(hid, st, "report descriptor parsing");
                return;
            }
            if (hid->subclass == 1 && hid->protocol == 1 && !has_keys(hid->rd)) {
                report_fallback(hid, TENDRIL_ERR_UNSUPPORTED, "finding keys in the report descriptor");
                return;
            }
            /* END FreeBSD sys/dev/hid/hkbd.c, hms.c */
            if (hid->subclass == 1) {
                ctrl(
                    hid, STEP_SET_REPORT_PROTOCOL, TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE,
                    TENDRIL_HID_REQ_SET_PROTOCOL, 1, NULL, 0
                );
                return;
            }
            set_idle(hid);
            return;
        }
        case STEP_SET_REPORT_PROTOCOL: {
            set_idle(hid);
            return;
        }
        case STEP_SET_IDLE: {
            /*
             * Without SET_IDLE the device reports at its default rate, which
             * works too.
             */
            hid_ready(hid);
            return;
        }
        default: {
            return;
        }
    }
}

/* BEGIN FreeBSD sys/dev/usb/usb_hid.c */
/*
 * The first HID descriptor among the interface's descriptors, before or after
 * its endpoint descriptors, with room for a class descriptor entry, as
 * hid_get_descriptor_from_usb() in FreeBSD's sys/dev/usb/usb_hid.c
 * (BSD-2-Clause) finds it.
 */
static const uint8_t *find_hid_descriptor(struct tendril_interface *intf) {
    size_t size;
    const uint8_t *p = tendril_interface_descriptors(intf, &size);
    const uint8_t *end = p + size;
    for (const uint8_t *q = tendril_desc_next(p, end); q != NULL; q = tendril_desc_next(q, end)) {
        if (q[1] == TENDRIL_DT_HID && q[0] >= HID_DESCRIPTOR_MIN_SIZE) {
            return q;
        }
    }
    return NULL;
}
/* END FreeBSD sys/dev/usb/usb_hid.c */

static bool hid_probe(struct tendril_interface *intf) {
    const struct tendril_interface_descriptor *id = tendril_interface_descriptor(intf);
    if (id->interface_class != TENDRIL_CLASS_HID) {
        return false;
    }
    uint32_t quirks = tendril_id_quirks(hid_quirks, ARRAY_SIZE(hid_quirks), &intf->dev->desc);
    if ((quirks & HQ_IGNORE) != 0) {
        return false;
    }
    struct tendril_endpoint *in_ep = tendril_interface_find_endpoint(intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_IN);
    if (in_ep == NULL) {
        return false;
    }

    size_t desc_len = 0;
    const uint8_t *hd = find_hid_descriptor(intf);
    if (hd != NULL) {
        /*
         * HID 1.11 6.2.1: (bDescriptorType, wDescriptorLength) entries from
         * offset 6.
         */
        size_t n = hd[5];
        for (size_t i = 0; i < n && 6 + i * 3 + 2 < hd[0]; i++) {
            if (hd[6 + i * 3] == TENDRIL_DT_HID_REPORT) {
                desc_len = (size_t)hd[7 + i * 3] | (size_t)hd[8 + i * 3] << 8;
                break;
            }
        }
    }
    bool boot = boot_capable(id->interface_subclass, id->interface_protocol);
    if (desc_len == 0 && !boot) {
        LOG_WARN("HID: interface %u has no report descriptor", intf->number);
        return false;
    }

    struct tendril_hid *hid = tendril_zalloc(sizeof(struct tendril_hid));
    if (hid == NULL) {
        return false;
    }
    if (desc_len != 0) {
        hid->desc_buf = tendril_zalloc(desc_len);
        if (hid->desc_buf == NULL) {
            tendril_free(hid, sizeof(struct tendril_hid));
            return false;
        }
        hid->desc_len = desc_len;
    }
    hid->intf = intf;
    hid->dev = intf->dev;
    hid->in_ep = in_ep;
    hid->out_ep = tendril_interface_find_endpoint(intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_OUT);
    hid->subclass = id->interface_subclass;
    hid->protocol = id->interface_protocol;
    hid->quirks = quirks;
    tendril_async_init(&hid->async, init_step);
    tendril_timer_init(&hid->retry, retry_work);
    tendril_interface_set_data(intf, hid);

    /* BEGIN FreeBSD sys/dev/hid/hkbd.c */
    /*
     * As FreeBSD's sys/dev/hid/hkbd.c (BSD-2-Clause) does for HQ_KBD_BOOTPROTO
     * keyboards.
     */
    if (desc_len == 0
     || (boot && ((hid_flags & TENDRIL_HID_BOOT_PROTOCOL) != 0 || (quirks & HQ_BOOT_PROTOCOL) != 0))) {
        start_boot(hid);
        return true;
    }
    /* END FreeBSD sys/dev/hid/hkbd.c */
    ctrl(
        hid, STEP_GET_REPORT_DESC, TENDRIL_DIR_IN | TENDRIL_TYPE_STANDARD | TENDRIL_RECIP_INTERFACE,
        TENDRIL_REQ_GET_DESCRIPTOR, TENDRIL_DT_HID_REPORT << 8, hid->desc_buf, (uint16_t)hid->desc_len
    );
    return true;
}

static void hid_disconnect(struct tendril_interface *intf) {
    struct tendril_hid *hid = tendril_interface_data(intf);
    struct tendril_hc *hc = intf->dev->hc;
    if (hid == NULL) {
        return;
    }
    bool attached = hid->step == STEP_RUNNING;
    hid->stopped = true;
    hc_lock(hc);
    tendril_timer_cancel(hc, &hid->retry);
    hc_unlock(hc);
    if (attached && callbacks->detach != NULL) {
        callbacks->detach(hid);
    }
    tendril_hid_free(hid->rd);
    tendril_free(hid->desc_buf, hid->desc_len);
    tendril_free(hid->in_buf, hid->in_size);
    tendril_free(hid, sizeof(struct tendril_hid));
}

static bool hid_suspend(struct tendril_interface *intf) {
    struct tendril_hid *hid = tendril_interface_data(intf);
    struct tendril_hc *hc = intf->dev->hc;
    if (hid == NULL || hid->stopped) {
        return true;
    }
    if (hid->step != STEP_RUNNING) {
        /* Still setting the device up. */
        return false;
    }
    hid->suspended = true;
    hc_lock(hc);
    tendril_timer_cancel(hc, &hid->retry);
    hc_unlock(hc);
    return true;
}

static void hid_resume(struct tendril_interface *intf) {
    struct tendril_hid *hid = tendril_interface_data(intf);
    if (hid == NULL || !hid->suspended) {
        return;
    }
    hid->suspended = false;
    hid->errors = 0;
    if (hid->stopped) {
        return;
    }
    in_submit(hid);
    if (hid->led_dirty && !hid->led_busy) {
        hid->led_dirty = false;
        tendril_hid_set_leds(hid, hid->leds);
    }
}

static struct tendril_driver hid_driver = {
    .name = "hid",
    .probe = hid_probe,
    .disconnect = hid_disconnect,
    .suspend = hid_suspend,
    .resume = hid_resume
};

void tendril_hid_register(const struct tendril_hid_callbacks *cb, uint32_t flags) {
    callbacks = cb;
    hid_flags = flags;
    tendril_driver_register(&hid_driver);
}

struct tendril_interface *tendril_hid_interface(struct tendril_hid *hid) {
    return hid->intf;
}

uint32_t tendril_hid_kinds(struct tendril_hid *hid) {
    return hid->kinds;
}

bool tendril_hid_boot_protocol(struct tendril_hid *hid) {
    return hid->boot;
}

const struct tendril_hid_report_desc *tendril_hid_descriptor(struct tendril_hid *hid) {
    return hid->rd;
}

void tendril_hid_set_user(struct tendril_hid *hid, void *user) {
    hid->user = user;
}

void *tendril_hid_user(struct tendril_hid *hid) {
    return hid->user;
}

/* Reports over the control pipe or the interrupt OUT endpoint */

/*
 * HID 1.11 5.6: a numbered report starts with its report ID byte, which the
 * driver adds or removes; the caller's buffer holds the rest.
 */
struct report_op {
    struct tendril_transfer xfer;
    void (*get_done)(void *ctx, int status, size_t actual);
    void (*set_done)(void *ctx, int status);
    void *ctx;
    uint8_t *buf;
    size_t size;
    size_t prefix;
    void *user_buf;
};

static void report_op_done(struct tendril_transfer *xfer) {
    struct report_op *op = xfer->context;
    int st = xfer->status;
    if (op->get_done != NULL) {
        size_t n = xfer->actual > op->prefix ? MIN(xfer->actual - op->prefix, op->size) : 0;
        memcpy(op->user_buf, op->buf + op->prefix, n);
        op->get_done(op->ctx, st, n);
    } else if (op->set_done != NULL) {
        op->set_done(op->ctx, st);
    }
    tendril_free(op->buf, op->size + 1);
    tendril_free(op, sizeof(struct report_op));
}

static struct report_op *report_op_new(uint8_t id, size_t size) {
    struct report_op *op = tendril_zalloc(sizeof(struct report_op));
    uint8_t *b = tendril_zalloc(size + 1);
    if (op == NULL || b == NULL) {
        tendril_free(op, sizeof(struct report_op));
        tendril_free(b, size + 1);
        return NULL;
    }
    op->buf = b;
    op->size = size;
    op->prefix = id != 0 ? 1 : 0;
    b[0] = id;
    return op;
}

static int report_op_submit(struct report_op *op) {
    int st = tendril_transfer_submit(&op->xfer);
    if (st != TENDRIL_OK) {
        tendril_free(op->buf, op->size + 1);
        tendril_free(op, sizeof(struct report_op));
    }
    return st;
}

int tendril_hid_get_report(
    struct tendril_hid *hid, int type, uint8_t id, void *buf, size_t size,
    void (*done)(void *ctx, int status, size_t actual), void *ctx
) {
    if (hid->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (size > 0xfffe) {
        return TENDRIL_ERR_INVALID;
    }
    struct report_op *op = report_op_new(id, size);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->get_done = done;
    op->ctx = ctx;
    op->user_buf = buf;
    struct tendril_transfer *xfer = &op->xfer;
    tendril_transfer_init(xfer, tendril_device_ep0(hid->dev));
    xfer->setup.request_type = TENDRIL_DIR_IN | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE;
    xfer->setup.request = TENDRIL_HID_REQ_GET_REPORT;
    xfer->setup.value = tendril_le16((uint16_t)(type << 8 | id));
    xfer->setup.index = tendril_le16(hid->intf->number);
    xfer->setup.length = tendril_le16((uint16_t)(op->prefix + size));
    xfer->buffer = op->buf;
    xfer->length = op->prefix + size;
    xfer->timeout_ms = HID_CTRL_TIMEOUT_MS;
    xfer->callback = report_op_done;
    xfer->context = op;
    return report_op_submit(op);
}

int tendril_hid_set_report(
    struct tendril_hid *hid, int type, uint8_t id, const void *buf, size_t size, void (*done)(void *ctx, int status),
    void *ctx
) {
    if (hid->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (size > 0xfffe) {
        return TENDRIL_ERR_INVALID;
    }
    struct report_op *op = report_op_new(id, size);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    memcpy(op->buf + op->prefix, buf, size);
    op->set_done = done;
    op->ctx = ctx;
    struct tendril_transfer *xfer = &op->xfer;
    /*
     * HID 1.11 4.4: output reports go to the interrupt OUT endpoint if there
     * is one.
     */
    if (type == TENDRIL_HID_OUTPUT && hid->out_ep != NULL) {
        tendril_transfer_init(xfer, hid->out_ep);
    } else {
        tendril_transfer_init(xfer, tendril_device_ep0(hid->dev));
        xfer->setup.request_type = TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE;
        xfer->setup.request = TENDRIL_HID_REQ_SET_REPORT;
        xfer->setup.value = tendril_le16((uint16_t)(type << 8 | id));
        xfer->setup.index = tendril_le16(hid->intf->number);
        xfer->setup.length = tendril_le16((uint16_t)(op->prefix + size));
        xfer->timeout_ms = HID_CTRL_TIMEOUT_MS;
    }
    xfer->buffer = op->buf;
    xfer->length = op->prefix + size;
    xfer->callback = report_op_done;
    xfer->context = op;
    return report_op_submit(op);
}

static void led_done(void *ctx, int status) {
    struct tendril_hid *hid = ctx;
    if (status == TENDRIL_ERR_CANCELLED || status == TENDRIL_ERR_SUSPENDED) {
        hid->led_dirty = true;
    }
    hid->led_busy = false;
    if (hid->led_dirty && !hid->stopped && !hid->suspended) {
        hid->led_dirty = false;
        tendril_hid_set_leds(hid, hid->leds);
    }
}

int tendril_hid_set_leds(struct tendril_hid *hid, uint8_t leds) {
    hid->leds = leds;
    if (hid->rd == NULL || hid->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (hid->led_busy || hid->suspended) {
        /* Sent once the current update finishes, or the device resumes. */
        hid->led_dirty = true;
        return TENDRIL_OK;
    }
    const struct tendril_hid_report_desc *rd = hid->rd;
    for (size_t i = 0; i < rd->num_fields; i++) {
        const struct tendril_hid_field *f = &rd->fields[i];
        if (f->type != TENDRIL_HID_OUTPUT || (f->usage_min >> 16) != TENDRIL_HID_PAGE_LED) {
            continue;
        }
        uint8_t id = f->report_id;
        size_t size = tendril_hid_report_size(rd, TENDRIL_HID_OUTPUT, id);
        if (size == 0 || size > sizeof(hid->led_buf)) {
            return TENDRIL_ERR_UNSUPPORTED;
        }
        memset(hid->led_buf, 0, sizeof(hid->led_buf));
        for (size_t j = 0; j < rd->num_fields; j++) {
            const struct tendril_hid_field *g = &rd->fields[j];
            if (g->type != TENDRIL_HID_OUTPUT || g->report_id != id || (g->flags & TENDRIL_HID_VARIABLE) == 0) {
                continue;
            }
            for (size_t e = 0; e < g->count; e++) {
                uint32_t usage = tendril_hid_field_usage(g, e);
                size_t n = usage & 0xffff;
                if ((usage >> 16) == TENDRIL_HID_PAGE_LED && n >= 1 && n <= 5) {
                    tendril_hid_field_set(g, hid->led_buf, size, e, (leds >> (n - 1)) & 1);
                }
            }
        }
        hid->led_busy = true;
        int st = tendril_hid_set_report(hid, TENDRIL_HID_OUTPUT, id, hid->led_buf, size, led_done, hid);
        if (st != TENDRIL_OK) {
            hid->led_busy = false;
        }
        return st;
    }
    return TENDRIL_ERR_UNSUPPORTED;
}
