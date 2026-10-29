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
 * Serial ports. CDC ACM follows the CDC PSTN subclass specification revision
 * 1.2 (February 9, 2007). CP210x follows Silicon Labs application note AN571
 * revision 0.4 and FreeBSD's sys/dev/usb/serial/uslcom.c (ISC license). CH34x,
 * whose maker publishes no protocol, follows FreeBSD's
 * sys/dev/usb/serial/uchcom.c (BSD-2-Clause). FTDI and PL2303 parts are
 * handled in serial_ftdi.c and serial_pl2303.c.
 */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/serial/uslcom.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * Copyright (c) 2006 Jonathan Gray <jsg@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/serial/uftdi.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (lennart@augustsson.net).
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/serial/umodem.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * Copyright (c) 2003 M. Warner Losh <imp@FreeBSD.org>
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/serial/uchcom.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2007, Takanori Watanabe
 * All rights reserved.
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
 *
 * Copyright (c) 2007 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Takuya SHIOZAKI (tshiozak@netbsd.org).
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
 * Parts of this file are derived from FreeBSD's sys/dev/usb/serial/uplcom.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2001-2003, 2005 Shunsuke Akiyama <akiyama@jp.FreeBSD.org>.
 * All rights reserved.
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
 *
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Ichiro FUKUHARA (ichiro@ichiro.org).
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tendril/serial.h>
#include "../internal.h"
#include "serial_internal.h"
#include <tendril/spec.h>
#include <tendril/tendril.h>

#define CTRL_TIMEOUT_MS 5000
#define RETRY_MS 20
#define MAX_ERRORS 50
/* BEGIN FreeBSD sys/dev/usb/serial/uslcom.c */
/*
 * CP210x parts have no status endpoint; FreeBSD's uslcom.c (ISC) asks them
 * four times a second.
 */
#define CP210X_POLL_MS 250
/* END FreeBSD sys/dev/usb/serial/uslcom.c */
/* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c */
#define SKIP_BY_NAME 0xff
/* END FreeBSD sys/dev/usb/serial/uftdi.c */

#define LINE_STATES \
    (TENDRIL_SERIAL_DCD | TENDRIL_SERIAL_DSR | TENDRIL_SERIAL_RI \
        | TENDRIL_SERIAL_CTS)

/* CDC 1.2 and CDC PSTN 1.2 */
#define CDC_SUBCLASS_ACM 0x02
#define CDC_PROTOCOL_NONE 0x00
#define CDC_PROTOCOL_AT 0x01
#define CDC_CALL_MANAGEMENT 0x01
#define CDC_ACM 0x02
#define CDC_UNION 0x06
#define CM_HANDLES 0x01
#define CM_OVER_DATA 0x02
#define ACM_COMM_FEATURE 0x01
#define ACM_BREAK 0x04
#define ACM_SET_COMM_FEATURE 0x02
#define ACM_SET_LINE_CODING 0x20
#define ACM_SET_CONTROL_LINE_STATE 0x22
#define ACM_SEND_BREAK 0x23
#define ACM_SERIAL_STATE 0x20
#define ACM_ABSTRACT_STATE 0x01
#define ACM_DATA_MULTIPLEXED 0x0002
#define ANY 0xffff

/* BEGIN FreeBSD sys/dev/usb/quirk/usb_quirk.h, sys/dev/usb/serial/umodem.c */
/*
 * FreeBSD's UQ_ASSUME_CM_OVER_DATA and UQ_IGNORE_CDC_CM
 * (sys/dev/usb/quirk/usb_quirk.h, BSD-2-Clause), handled as its
 * sys/dev/usb/serial/umodem.c (BSD-2-Clause) does.
 */
#define SQ_ASSUME_CM_OVER_DATA ((uint32_t)1 << 0)
#define SQ_IGNORE_CDC_CM ((uint32_t)1 << 1)
/* END FreeBSD sys/dev/usb/quirk/usb_quirk.h, sys/dev/usb/serial/umodem.c */

/* CH34x */
/* BEGIN FreeBSD sys/dev/usb/serial/uchcom.c */
#define CH_GET_VERSION 0x5f
#define CH_READ_REG 0x95
#define CH_WRITE_REG 0x9a
#define CH_RESET 0xa1
#define CH_SET_DTRRTS 0xa4
#define CH343_WRITE_REG 0xa8
#define CH_REG_STAT1 0x06
#define CH_REG_BPS_PRE 0x12
#define CH_REG_BPS_DIV 0x13
#define CH_REG_BPS_PAD 0x0f
#define CH_REG_BREAK1 0x05
#define CH_REG_LCR1 0x18
#define CH_DTR 0x20
#define CH_RTS 0x40
#define CH_BREAK 0x01
#define CH_ABREAK 0x10
#define CH343_BREAK 0x80
#define CH_LCR1_RX 0x80
#define CH_LCR1_TX 0x40
#define CH_LCR1_PAREVEN 0x10
#define CH_LCR1_PARENB 0x08
#define CH_LCR1_STOPB 0x04
#define CH_T 0x08
#define CH_CL 0x04
#define CH_CT 0x90
#define CH343_CT 0x80
#define CH_VERSION_20 0x20
#define CH_INTR_STAT 2
#define CH_INTR_MIN 4
/* END FreeBSD sys/dev/usb/serial/uchcom.c */

#define CH_341 0
#define CH_343 1

/* CP210x */
#define CP_IFC_ENABLE 0x00
#define CP_SET_LINE_CTL 0x03
#define CP_SET_BREAK 0x05
#define CP_SET_MHS 0x07
#define CP_GET_MDMSTS 0x08
#define CP_SET_FLOW 0x13
#define CP_SET_BAUDRATE 0x1e
#define CP_DTR_ACTIVE 0x01
#define CP_CTS_HANDSHAKE 0x08
#define CP_RTS_ACTIVE 0x40
#define CP_RTS_HANDSHAKE 0x80

struct serial_id {
    uint16_t vendor;
    uint16_t product;
    int kind;
    /* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c */
    /*
     * FTDI interfaces left for JTAG use, one bit each, or SKIP_BY_NAME, as in
     * FreeBSD's uftdi.c.
     */
    uint8_t skip;
    /* END FreeBSD sys/dev/usb/serial/uftdi.c */
};

/* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c, uplcom.c, uchcom.c, uslcom.c, sys/dev/usb/usbdevs */
/*
 * From the ID tables of FreeBSD's sys/dev/usb/serial/uftdi.c, uplcom.c, and
 * uchcom.c (BSD-2-Clause) and uslcom.c (ISC license), naming the IDs as its
 * sys/dev/usb/usbdevs (BSD-2-Clause) does.
 */
static const struct serial_id serial_ids[] = {
    { 0x03eb, 0x2109, TENDRIL_SERIAL_FTDI, 0 }, /* ATMEL STK541 */
    { 0x03f0, 0x3524, TENDRIL_SERIAL_PL2303, 0 }, /* HP LD220 */
    { 0x0403, 0x5259, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCX8_USB_PHOENIX */
    { 0x0403, 0x6001, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SERIAL_8U232AM */
    { 0x0403, 0x6004, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SERIAL_8U232AM4 */
    { 0x0403, 0x6006, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI 8U232AM_ALT */
    /* FTDI SERIAL_2232C */
    { 0x0403, 0x6010, TENDRIL_SERIAL_FTDI, SKIP_BY_NAME },
    { 0x0403, 0x6011, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SERIAL_4232H */
    { 0x0403, 0x6014, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI 232H */
    { 0x0403, 0x6015, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI 232EX */
    { 0x0403, 0x8372, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SERIAL_8U100AX */
    { 0x0403, 0x8a98, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TIAO */
    { 0x0403, 0x9378, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GENERIC */
    { 0x0403, 0x9379, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SR_RADIO */
    { 0x0403, 0x937a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI XM_RADIO */
    { 0x0403, 0x937c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI HD_RADIO */
    { 0x0403, 0x9868, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI DOTEC */
    { 0x0403, 0x9e90, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SERIAL_2232D */
    { 0x0403, 0x9f80, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CANDAPTER */
    { 0x0403, 0xa6d0, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI XDS100V2 */
    { 0x0403, 0xa6d1, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI XDS100V3 */
    { 0x0403, 0xabb8, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI NXTCAM */
    { 0x0403, 0xb810, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI USINT_CAT */
    { 0x0403, 0xb811, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI USINT_WKEY */
    { 0x0403, 0xb812, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI USINT_RS232 */
    { 0x0403, 0xbaf8, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI OOCDLINK */
    { 0x0403, 0xbbe2, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI KTLINK */
    { 0x0403, 0xbca0, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI SIGNALYZER_ST */
    { 0x0403, 0xbca1, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI SIGNALYZER_SLITE */
    { 0x0403, 0xbca2, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI SIGNALYZER_SH2 */
    { 0x0403, 0xbca4, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI SIGNALYZER_SH4 */
    { 0x0403, 0xbcd8, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI LM3S_DEVEL_BOARD */
    { 0x0403, 0xbcd9, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI LM3S_EVAL_BOARD */
    { 0x0403, 0xbcda, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI LM3S_ICDI_B_BOARD */
    { 0x0403, 0xbdc8, TENDRIL_SERIAL_FTDI, 0x01 }, /* FTDI TURTELIZER2 */
    { 0x0403, 0xbfd8, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI OPENDCC */
    { 0x0403, 0xbfd9, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI OPENDCC_SNIFFER */
    { 0x0403, 0xbfda, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI OPENDCC_THROTTLE */
    { 0x0403, 0xbfdb, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI OPENDCC_GATEWAY */
    { 0x0403, 0xbfdc, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI OPENDCC_GBM */
    { 0x0403, 0xc7d0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI RRCIRKITS_LOCOBUFFER */
    { 0x0403, 0xc850, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI DMX4ALL */
    { 0x0403, 0xc991, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ASK_RDR400 */
    { 0x0403, 0xcaa0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MICRO_CHAMELEON */
    { 0x0403, 0xcc48, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TACTRIX_OPENPORT_13M */
    { 0x0403, 0xcc49, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TACTRIX_OPENPORT_13S */
    { 0x0403, 0xcc4a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TACTRIX_OPENPORT_13U */
    { 0x0403, 0xd010, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_0 */
    { 0x0403, 0xd011, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_1 */
    { 0x0403, 0xd012, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_2 */
    { 0x0403, 0xd013, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_3 */
    { 0x0403, 0xd014, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_4 */
    { 0x0403, 0xd015, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_5 */
    { 0x0403, 0xd016, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_6 */
    { 0x0403, 0xd017, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SCS_DEVICE_7 */
    { 0x0403, 0xd070, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IPLUS */
    { 0x0403, 0xd071, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IPLUS2 */
    { 0x0403, 0xd388, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_0 */
    { 0x0403, 0xd389, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_1 */
    { 0x0403, 0xd38a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_2 */
    { 0x0403, 0xd38b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_3 */
    { 0x0403, 0xd38c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_4 */
    { 0x0403, 0xd38d, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_5 */
    { 0x0403, 0xd38e, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_6 */
    { 0x0403, 0xd38f, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CONVERTER_7 */
    { 0x0403, 0xd578, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI COM4SM */
    { 0x0403, 0xd678, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GAMMASCOUT */
    { 0x0403, 0xd738, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI PROPOX_JTAGCABLEII */
    { 0x0403, 0xd780, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI LENZ_LIUSB */
    /* FTDI CHAMSYS_24_MASTER_WING */
    { 0x0403, 0xdaf8, TENDRIL_SERIAL_FTDI, 0 },
    { 0x0403, 0xdaf9, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CHAMSYS_PC_WING */
    { 0x0403, 0xdafa, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CHAMSYS_USB_DMX */
    /* FTDI CHAMSYS_MIDI_TIMECODE */
    { 0x0403, 0xdafb, TENDRIL_SERIAL_FTDI, 0 },
    { 0x0403, 0xdafc, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CHAMSYS_MINI_WING */
    { 0x0403, 0xdafd, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CHAMSYS_MAXI_WING */
    { 0x0403, 0xdafe, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CHAMSYS_MEDIA_WING */
    { 0x0403, 0xdaff, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CHAMSYS_WING */
    { 0x0403, 0xdc00, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI WESTREX_MODEL_777 */
    { 0x0403, 0xdc01, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI WESTREX_MODEL_8900F */
    { 0x0403, 0xdd20, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ACG_HFDUAL */
    { 0x0403, 0xdf28, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ARTEMIS */
    { 0x0403, 0xdf30, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ATIK_ATK16 */
    { 0x0403, 0xdf31, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ATIK_ATK16HR */
    { 0x0403, 0xdf32, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ATIK_ATK16C */
    { 0x0403, 0xdf33, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ATIK_ATK16HRC */
    { 0x0403, 0xdf35, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ATIK_ATK16IC */
    { 0x0403, 0xe000, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_USR */
    { 0x0403, 0xe001, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_MSM1 */
    { 0x0403, 0xe002, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_KL100 */
    { 0x0403, 0xe004, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_WS550 */
    { 0x0403, 0xe006, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_EC3000 */
    { 0x0403, 0xe008, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_WS888 */
    { 0x0403, 0xe009, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_TWS550 */
    { 0x0403, 0xe00a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_FEM */
    { 0x0403, 0xe050, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI YEI_SERVOCENTER31 */
    { 0x0403, 0xe0e4, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SITOP_UPS500S */
    { 0x0403, 0xe0e8, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_FHZ1300PC */
    { 0x0403, 0xe0e9, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_WS500 */
    { 0x0403, 0xe0ea, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_HS485 */
    { 0x0403, 0xe0eb, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UMS100 */
    { 0x0403, 0xe0ec, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_TFD128 */
    { 0x0403, 0xe0ed, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_FM3RX */
    { 0x0403, 0xe0ee, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_WS777 */
    { 0x0403, 0xe0ef, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_EM1010PC */
    { 0x0403, 0xe0f0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_CSI8 */
    { 0x0403, 0xe0f1, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_EM1000DL */
    { 0x0403, 0xe0f2, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_PCK100 */
    { 0x0403, 0xe0f3, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_RFP500 */
    { 0x0403, 0xe0f4, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_FS20SIG */
    { 0x0403, 0xe0f5, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UTP8 */
    { 0x0403, 0xe0f6, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_WS300PC */
    { 0x0403, 0xe0f7, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_WS444PC */
    { 0x0403, 0xe40b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI PHI_FISCO */
    { 0x0403, 0xe520, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ECO_PRO_CDS */
    { 0x0403, 0xe548, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ACTIVE_ROBOTS */
    { 0x0403, 0xe6c8, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI KBS */
    { 0x0403, 0xe700, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELSTER_UNICOM */
    { 0x0403, 0xe729, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI RMP200 */
    { 0x0403, 0xe808, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E808 */
    { 0x0403, 0xe809, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E809 */
    { 0x0403, 0xe80a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E80A */
    { 0x0403, 0xe80b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E80B */
    { 0x0403, 0xe80c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E80C */
    { 0x0403, 0xe80d, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E80D */
    { 0x0403, 0xe80e, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E80E */
    { 0x0403, 0xe80f, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E80F */
    { 0x0403, 0xe888, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI EISCOU */
    { 0x0403, 0xe889, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI UOPTBR */
    { 0x0403, 0xe88a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI EMCU2D */
    { 0x0403, 0xe88b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI PCMSFU */
    { 0x0403, 0xe88c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI EMCU2H */
    { 0x0403, 0xe88d, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E88D */
    { 0x0403, 0xe88e, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E88E */
    { 0x0403, 0xe88f, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI GUDEADS_E88F */
    { 0x0403, 0xea90, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ECLO_COM_1WIRE */
    { 0x0403, 0xebe0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TNC_X */
    { 0x0403, 0xec88, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TERATRONIK_VCP */
    { 0x0403, 0xec89, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TERATRONIK_D2XX */
    { 0x0403, 0xed22, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI REU_TINY */
    { 0x0403, 0xed71, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI HO870 */
    { 0x0403, 0xed72, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI HO720 */
    { 0x0403, 0xed73, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI HO730 */
    { 0x0403, 0xed74, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI HO820 */
    { 0x0403, 0xee18, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MAXSTREAM */
    { 0x0403, 0xeee8, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_KW */
    { 0x0403, 0xeee9, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_YS */
    { 0x0403, 0xeeea, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_Y6 */
    { 0x0403, 0xeeeb, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_Y8 */
    { 0x0403, 0xeeec, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_IC */
    { 0x0403, 0xeeed, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_DB9 */
    { 0x0403, 0xeeee, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_RS232 */
    { 0x0403, 0xeeef, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MHAM_Y9 */
    { 0x0403, 0xef50, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI DOMINTELL_DGQG */
    { 0x0403, 0xef51, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI DOMINTELL_DUSB */
    { 0x0403, 0xf068, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UAD8 */
    { 0x0403, 0xf069, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UDA7 */
    { 0x0403, 0xf06a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_USI2 */
    { 0x0403, 0xf06b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_T1100 */
    { 0x0403, 0xf06c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_PCD200 */
    { 0x0403, 0xf06d, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_ULA200 */
    { 0x0403, 0xf06e, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_ALC8500 */
    { 0x0403, 0xf06f, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_FHZ1000PC */
    { 0x0403, 0xf070, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI VARDAAN */
    { 0x0403, 0xf0c0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI PERLE_ULTRAPORT */
    { 0x0403, 0xf0c8, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SPROG_II */
    { 0x0403, 0xf208, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI PIEGROUP */
    { 0x0403, 0xf2d0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ACTZWAVE */
    { 0x0403, 0xf3c0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI 4N_GALAXY_DE_1 */
    { 0x0403, 0xf3c1, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI 4N_GALAXY_DE_2 */
    { 0x0403, 0xf3c2, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI 4N_GALAXY_DE_3 */
    { 0x0403, 0xf448, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SDMUSBQSS */
    { 0x0403, 0xf449, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MASTERDEVEL2 */
    { 0x0403, 0xf44a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI FUTURE_0 */
    { 0x0403, 0xf44b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI FUTURE_1 */
    { 0x0403, 0xf44c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI FUTURE_2 */
    { 0x0403, 0xf460, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI OCEANIC */
    { 0x0403, 0xf608, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CTI_USB_MINI_485 */
    { 0x0403, 0xf60b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CTI_USB_NANO_485 */
    { 0x0403, 0xf680, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SUUNTO_SPORTS */
    { 0x0403, 0xf850, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI USB_UIRT */
    { 0x0403, 0xf857, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI USBX_707 */
    { 0x0403, 0xf9d0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CCSICDU20_0 */
    { 0x0403, 0xf9d1, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CCSICDU40_1 */
    { 0x0403, 0xf9d2, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CCSMACHX_2 */
    { 0x0403, 0xf9d3, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CCSLOAD_N_GO_3 */
    { 0x0403, 0xf9d4, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CCSICDU64_4 */
    { 0x0403, 0xf9d5, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CCSPRIME8_5 */
    { 0x0403, 0xfa00, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI USBSERIAL */
    { 0x0403, 0xfa01, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MX2_3 */
    { 0x0403, 0xfa02, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MX4_5 */
    { 0x0403, 0xfa03, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI LK202 */
    { 0x0403, 0xfa04, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI LK204 */
    { 0x0403, 0xfa05, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MTXORB_5 */
    { 0x0403, 0xfa06, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI MTXORB_6 */
    { 0x0403, 0xfa10, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI RELAIS */
    { 0x0403, 0xfa33, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TAVIR_STK500 */
    { 0x0403, 0xfa88, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI PCDJ_DAC2 */
    { 0x0403, 0xfad0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ACCESSO */
    { 0x0403, 0xfaf0, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI THORLABS */
    { 0x0403, 0xfb58, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UR100 */
    { 0x0403, 0xfb59, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_CLI7000 */
    { 0x0403, 0xfb5a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UM100 */
    { 0x0403, 0xfb5b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UO100 */
    { 0x0403, 0xfb5c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_PPS7330 */
    { 0x0403, 0xfb5d, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_TFM100 */
    { 0x0403, 0xfb5e, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UDF77 */
    { 0x0403, 0xfb5f, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI ELV_UIO88 */
    { 0x0403, 0xfb80, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI R2000KU_TRUE_RNG */
    { 0x0403, 0xfb99, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI BCS_SE923 */
    { 0x0403, 0xfbfa, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI 232RL */
    { 0x0403, 0xfc08, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CFA_632 */
    { 0x0403, 0xfc09, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CFA_634 */
    { 0x0403, 0xfc0a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI XF_547 */
    { 0x0403, 0xfc0b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CFA_633 */
    { 0x0403, 0xfc0c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CFA_631 */
    { 0x0403, 0xfc0d, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CFA_635 */
    { 0x0403, 0xfc0e, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI XF_640 */
    { 0x0403, 0xfc0f, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI XF_642 */
    { 0x0403, 0xfc60, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IRTRANS */
    { 0x0403, 0xfc70, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SPECIAL_1 */
    { 0x0403, 0xfc71, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI R2X0 */
    { 0x0403, 0xfc72, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SPECIAL_3 */
    { 0x0403, 0xfc73, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SPECIAL_4 */
    { 0x0403, 0xfc82, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI SEMC_DSS20 */
    { 0x0403, 0xfd60, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI RM_CANVIEW */
    { 0x0403, 0xfe38, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI VNHCPCUSB_D */
    { 0x0403, 0xff00, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI AMC232 */
    /* FTDI SCIENCESCOPE_LOGBOOKML */
    { 0x0403, 0xff18, TENDRIL_SERIAL_FTDI, 0 },
    /* FTDI SCIENCESCOPE_LS_LOGBOOK */
    { 0x0403, 0xff1c, TENDRIL_SERIAL_FTDI, 0 },
    /* FTDI SCIENCESCOPE_HS_LOGBOOK */
    { 0x0403, 0xff1d, TENDRIL_SERIAL_FTDI, 0 },
    { 0x0403, 0xff20, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI TTUSB */
    { 0x0403, 0xff38, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_US485 */
    { 0x0403, 0xff39, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_PICPRO */
    { 0x0403, 0xff3a, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_PCMCIA */
    { 0x0403, 0xff3b, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_PK1 */
    { 0x0403, 0xff3c, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_RS232MON */
    { 0x0403, 0xff3d, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_APP70 */
    { 0x0403, 0xff3e, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_PEDO */
    { 0x0403, 0xff3f, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI IBS_PROD */
    { 0x0403, 0xffa8, TENDRIL_SERIAL_FTDI, 0 }, /* FTDI CANUSB */
    { 0x0411, 0x00b3, TENDRIL_SERIAL_FTDI, 0 }, /* MELCO PCOPRS1 */
    { 0x0413, 0x2101, TENDRIL_SERIAL_PL2303, 0 }, /* LEADTEK 9531 */
    { 0x0456, 0xf000, TENDRIL_SERIAL_FTDI, 0x01 }, /* ANALOGDEVICES GNICE */
    /* ANALOGDEVICES GNICEPLUS */
    { 0x0456, 0xf001, TENDRIL_SERIAL_FTDI, 0x01 },
    { 0x045b, 0x0053, TENDRIL_SERIAL_CP210X, 0 }, /* RENESAS RX610 */
    { 0x045e, 0x0079, TENDRIL_SERIAL_PL2303, 0 }, /* MICROSOFT 700WX */
    { 0x0471, 0x066a, TENDRIL_SERIAL_CP210X, 0 }, /* PHILIPS ACE1001 */
    { 0x0489, 0xe000, TENDRIL_SERIAL_CP210X, 0 }, /* FOXCONN TCOM_TC_300 */
    { 0x0489, 0xe003, TENDRIL_SERIAL_CP210X, 0 }, /* FOXCONN PIRELLI_DP_L10 */
    { 0x04a5, 0x4027, TENDRIL_SERIAL_PL2303, 0 }, /* ACERP S81 */
    { 0x04b8, 0x0521, TENDRIL_SERIAL_PL2303, 0 }, /* EPSON CRESSI_EDY */
    { 0x04b8, 0x0522, TENDRIL_SERIAL_PL2303, 0 }, /* EPSON N2ITION3 */
    { 0x04bb, 0x0a03, TENDRIL_SERIAL_PL2303, 0 }, /* IODATA USBRSAQ */
    { 0x04bb, 0x0a0e, TENDRIL_SERIAL_PL2303, 0 }, /* IODATA USBRSAQ5 */
    { 0x04bf, 0x0115, TENDRIL_SERIAL_PL2303, 0 }, /* TDK UPA9664 */
    { 0x04bf, 0x0117, TENDRIL_SERIAL_PL2303, 0 }, /* TDK UHA6400 */
    { 0x04da, 0x3900, TENDRIL_SERIAL_PL2303, 0 }, /* PANASONIC TYTP50P6S */
    { 0x04e8, 0x8001, TENDRIL_SERIAL_PL2303, 0 }, /* SAMSUNG I330 */
    { 0x050d, 0x0257, TENDRIL_SERIAL_PL2303, 0 }, /* BELKIN F5U257 */
    { 0x0547, 0x2008, TENDRIL_SERIAL_PL2303, 0 }, /* ANCHOR SERIAL */
    { 0x054c, 0x0437, TENDRIL_SERIAL_PL2303, 0 }, /* SONY QN3 */
    { 0x0557, 0x2008, TENDRIL_SERIAL_PL2303, 0 }, /* ATEN UC232A */
    { 0x0557, 0x2022, TENDRIL_SERIAL_PL2303, 0 }, /* ATEN UC232B */
    { 0x056e, 0x5003, TENDRIL_SERIAL_PL2303, 0 }, /* ELECOM UCSGT */
    { 0x056e, 0x5004, TENDRIL_SERIAL_PL2303, 0 }, /* ELECOM UCSGT0 */
    { 0x0584, 0xb000, TENDRIL_SERIAL_PL2303, 0 }, /* RATOC REXUSB60 */
    { 0x0584, 0xb020, TENDRIL_SERIAL_FTDI, 0 }, /* RATOC REXUSB60F */
    { 0x058f, 0x9720, TENDRIL_SERIAL_PL2303, 0 }, /* ALCOR AU9720 */
    { 0x05ad, 0x0fba, TENDRIL_SERIAL_PL2303, 0 }, /* YCCABLE PL2303 */
    { 0x05d1, 0x1011, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US101 */
    { 0x05d1, 0x1013, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US324 */
    { 0x05d1, 0x1017, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US235 */
    { 0x05d1, 0x1019, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US320 */
    { 0x05d1, 0x1021, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US159 */
    { 0x05d1, 0x2011, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US701_12 */
    { 0x05d1, 0x2012, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US701_34 */
    { 0x05d1, 0x2021, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US279_12 */
    { 0x05d1, 0x2022, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US279_34 */
    { 0x05d1, 0x2023, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US279_56 */
    { 0x05d1, 0x2024, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US279_78 */
    { 0x05d1, 0x3011, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US346_12 */
    { 0x05d1, 0x3012, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US346_34 */
    { 0x05d1, 0x5001, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US257 */
    { 0x05d1, 0x5002, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US25701 */
    { 0x05d1, 0x6001, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US313 */
    { 0x05d1, 0x8001, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US842_12 */
    { 0x05d1, 0x8002, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US842_34 */
    { 0x05d1, 0x8003, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US842_56 */
    { 0x05d1, 0x8004, TENDRIL_SERIAL_FTDI, 0 }, /* BRAINBOXES US842_78 */
    { 0x0647, 0x0100, TENDRIL_SERIAL_FTDI, 0 }, /* ACTON SPECTRAPRO */
    { 0x065a, 0xc000, TENDRIL_SERIAL_FTDI, 0 }, /* OPTO CRD7734 */
    { 0x065a, 0xc001, TENDRIL_SERIAL_FTDI, 0 }, /* OPTO CRD7734_1 */
    { 0x067b, 0x0307, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC MOTOROLA */
    { 0x067b, 0x04bb, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC RSAQ2 */
    { 0x067b, 0x0609, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC ALLTRONIX_GPRS */
    { 0x067b, 0x0611, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC ALDIGA_AL11U */
    { 0x067b, 0x0612, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC MICROMAX_610U */
    { 0x067b, 0x1234, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC DCU11 */
    { 0x067b, 0x206a, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC UIC_MSR206 */
    { 0x067b, 0x2303, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PL2303 */
    { 0x067b, 0x23a3, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PL2303GC */
    { 0x067b, 0x23b3, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PL2303GB */
    { 0x067b, 0x23c3, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PL2303GT */
    { 0x067b, 0x23d3, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PL2303GL */
    { 0x067b, 0x23e3, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PL2303GE */
    { 0x067b, 0x23f3, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PL2303GS */
    { 0x067b, 0x331a, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC HCR331 */
    { 0x067b, 0xaaa0, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC PHAROS */
    { 0x067b, 0xaaa2, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC RSAQ3 */
    { 0x06ce, 0x8311, TENDRIL_SERIAL_FTDI, 0 }, /* CONTEC COM1USBH */
    { 0x0731, 0x0528, TENDRIL_SERIAL_PL2303, 0 }, /* SONYERICSSON DCU10 */
    { 0x0731, 0x2003, TENDRIL_SERIAL_PL2303, 0 }, /* SONYERICSSON DATAPILOT */
    { 0x0745, 0x0001, TENDRIL_SERIAL_PL2303, 0 }, /* SYNTECH CPT8001C */
    { 0x0745, 0x1000, TENDRIL_SERIAL_CP210X, 0 }, /* SYNTECH CYPHERLAB100 */
    { 0x078b, 0x1234, TENDRIL_SERIAL_PL2303, 0 }, /* NOKIA2 CA42 */
    { 0x079b, 0x0027, TENDRIL_SERIAL_PL2303, 0 }, /* SAGEM USBSERIAL */
    { 0x07aa, 0x002a, TENDRIL_SERIAL_PL2303, 0 }, /* COREGA CGUSBRS232R */
    { 0x0833, 0x012e, TENDRIL_SERIAL_PL2303, 0 }, /* SOURCENEXT KEIKAI8_CHG */
    { 0x0833, 0x039f, TENDRIL_SERIAL_PL2303, 0 }, /* SOURCENEXT KEIKAI8 */
    { 0x0846, 0x1100, TENDRIL_SERIAL_CP210X, 0 }, /* NETGEAR M4100 */
    { 0x0856, 0xac01, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USOTL4 */
    { 0x0856, 0xac02, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USTL4 */
    { 0x0856, 0xac03, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USO9ML2 */
    { 0x0856, 0xac11, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USOPTL4 */
    { 0x0856, 0xac12, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USPTL4 */
    { 0x0856, 0xac16, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USO9ML2DR_2 */
    { 0x0856, 0xac17, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USO9ML2DR */
    { 0x0856, 0xac18, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USOPTL4DR2 */
    { 0x0856, 0xac19, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS USOPTL4DR */
    { 0x0856, 0xac25, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS 485USB9F_2W */
    { 0x0856, 0xac26, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS 485USB9F_4W */
    { 0x0856, 0xac27, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS 232USB9M */
    { 0x0856, 0xac33, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS 485USBTB_2W */
    { 0x0856, 0xac34, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS 485USBTB_4W */
    { 0x0856, 0xac49, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS TTL5USB9M */
    { 0x0856, 0xac50, TENDRIL_SERIAL_FTDI, 0 }, /* BBELECTRONICS TTL3USB9M */
    /* BBELECTRONICS ZZ_PROG1_USB */
    { 0x0856, 0xba02, TENDRIL_SERIAL_FTDI, 0 },
    { 0x08e6, 0x5501, TENDRIL_SERIAL_CP210X, 0 }, /* GEMALTO PROXPU */
    { 0x08fd, 0x000a, TENDRIL_SERIAL_CP210X, 0 }, /* DIGIANSWER ZIGBEE802154 */
    { 0x093c, 0x0601, TENDRIL_SERIAL_FTDI, 0 }, /* INTREPIDCS VALUECAN */
    { 0x093c, 0x0701, TENDRIL_SERIAL_FTDI, 0 }, /* INTREPIDCS NEOVI */
    { 0x0acd, 0x0300, TENDRIL_SERIAL_FTDI, 0 }, /* IDTECH IDT1221U */
    { 0x0b39, 0x0103, TENDRIL_SERIAL_FTDI, 0 }, /* SIIG2 DK201 */
    { 0x0b39, 0x0421, TENDRIL_SERIAL_FTDI, 0 }, /* SIIG2 US2308 */
    { 0x0b41, 0x0011, TENDRIL_SERIAL_PL2303, 0 }, /* HAL IMR001 */
    { 0x0b63, 0x6530, TENDRIL_SERIAL_PL2303, 0 }, /* ADLINK ND6530 */
    { 0x0b8c, 0x2303, TENDRIL_SERIAL_PL2303, 0 }, /* SMART PL2303 */
    { 0x0bed, 0x1100, TENDRIL_SERIAL_CP210X, 0 }, /* MEI CASHFLOW_SC */
    { 0x0bed, 0x1101, TENDRIL_SERIAL_CP210X, 0 }, /* MEI S2000 */
    { 0x0c26, 0x0004, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM SP1 */
    { 0x0c26, 0x0009, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP2C1 */
    { 0x0c26, 0x000a, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP2C2 */
    { 0x0c26, 0x000b, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP2D */
    { 0x0c26, 0x000c, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP2VT */
    { 0x0c26, 0x000d, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP2VR */
    { 0x0c26, 0x0010, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP4KVT */
    { 0x0c26, 0x0011, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP4KVR */
    { 0x0c26, 0x0012, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP2KVT */
    { 0x0c26, 0x0013, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM RP2KVR */
    { 0x0c26, 0x0018, TENDRIL_SERIAL_FTDI, 0 }, /* ICOM OPC_U_UC */
    { 0x0c33, 0x0010, TENDRIL_SERIAL_FTDI, 0 }, /* GNOTOMETRICS USB */
    { 0x0c52, 0x2101, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2101 */
    { 0x0c52, 0x2102, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2102 */
    { 0x0c52, 0x2103, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2103 */
    { 0x0c52, 0x2104, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2104 */
    { 0x0c52, 0x2211, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2201_1 */
    { 0x0c52, 0x2212, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2202_1 */
    { 0x0c52, 0x2213, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2203_1 */
    { 0x0c52, 0x2221, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2201_2 */
    { 0x0c52, 0x2222, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2202_2 */
    { 0x0c52, 0x2223, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2203_2 */
    { 0x0c52, 0x2411, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2401_1 */
    { 0x0c52, 0x2412, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2402_1 */
    { 0x0c52, 0x2413, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2403_1 */
    { 0x0c52, 0x2421, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2401_2 */
    { 0x0c52, 0x2422, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2402_2 */
    { 0x0c52, 0x2423, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2403_2 */
    { 0x0c52, 0x2431, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2401_3 */
    { 0x0c52, 0x2432, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2402_3 */
    { 0x0c52, 0x2433, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2403_3 */
    { 0x0c52, 0x2441, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2401_4 */
    { 0x0c52, 0x2442, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2402_4 */
    { 0x0c52, 0x2443, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2403_4 */
    { 0x0c52, 0x2811, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_1 */
    { 0x0c52, 0x2812, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_1 */
    { 0x0c52, 0x2813, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_1 */
    { 0x0c52, 0x2821, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_2 */
    { 0x0c52, 0x2822, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_2 */
    { 0x0c52, 0x2823, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_2 */
    { 0x0c52, 0x2831, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_3 */
    { 0x0c52, 0x2832, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_3 */
    { 0x0c52, 0x2833, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_3 */
    { 0x0c52, 0x2841, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_4 */
    { 0x0c52, 0x2842, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_4 */
    { 0x0c52, 0x2843, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_4 */
    { 0x0c52, 0x2851, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_5 */
    { 0x0c52, 0x2852, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_5 */
    { 0x0c52, 0x2853, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_5 */
    { 0x0c52, 0x2861, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_6 */
    { 0x0c52, 0x2862, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_6 */
    { 0x0c52, 0x2863, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_6 */
    { 0x0c52, 0x2871, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_7 */
    { 0x0c52, 0x2872, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_7 */
    { 0x0c52, 0x2873, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_7 */
    { 0x0c52, 0x2881, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2801_8 */
    { 0x0c52, 0x2882, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2802_8 */
    { 0x0c52, 0x2883, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2803_8 */
    { 0x0c52, 0x9020, TENDRIL_SERIAL_FTDI, 0 }, /* SEALEVEL 2106 */
    { 0x0c6c, 0x04b2, TENDRIL_SERIAL_FTDI, 0 }, /* JETI SPC1201 */
    { 0x0c7d, 0x0005, TENDRIL_SERIAL_FTDI, 0 }, /* ELEKTOR FT323R */
    { 0x0d3a, 0x0300, TENDRIL_SERIAL_FTDI, 0 }, /* POSIFLEX PP7000 */
    { 0x0d46, 0x2020, TENDRIL_SERIAL_FTDI, 0 }, /* KOBIL CONV_B1 */
    { 0x0d46, 0x2021, TENDRIL_SERIAL_FTDI, 0 }, /* KOBIL CONV_KAAN */
    { 0x0dcd, 0x0001, TENDRIL_SERIAL_FTDI, 0 }, /* UNKNOWN4 NF_RIC */
    { 0x0df7, 0x0620, TENDRIL_SERIAL_PL2303, 0 }, /* MOBILEACTION MA620 */
    { 0x0e55, 0x110b, TENDRIL_SERIAL_PL2303, 0 }, /* SPEEDDRAGON MS3303H */
    { 0x0ea0, 0x6858, TENDRIL_SERIAL_PL2303, 0 }, /* OTI DKU5 */
    { 0x0eba, 0x1080, TENDRIL_SERIAL_PL2303, 0 }, /* ITEGNO WM1080A */
    { 0x0eba, 0x2080, TENDRIL_SERIAL_PL2303, 0 }, /* ITEGNO WM2080A */
    { 0x0f94, 0x0001, TENDRIL_SERIAL_FTDI, 0 }, /* FALCOM TWIST */
    { 0x0f94, 0x0005, TENDRIL_SERIAL_FTDI, 0 }, /* FALCOM SAMBA */
    { 0x0fcf, 0x1003, TENDRIL_SERIAL_CP210X, 0 }, /* DYNASTREAM ANTDEVBOARD */
    { 0x0fcf, 0x1004, TENDRIL_SERIAL_CP210X, 0 }, /* DYNASTREAM ANT2USB */
    { 0x0fcf, 0x1006, TENDRIL_SERIAL_CP210X, 0 }, /* DYNASTREAM ANTDEVBOARD2 */
    { 0x0fd8, 0x0001, TENDRIL_SERIAL_FTDI, 0 }, /* LARSENBRUSGAARD ALTITRACK */
    { 0x0fde, 0xca05, TENDRIL_SERIAL_CP210X, 0 }, /* OWL CM_160 */
    { 0x103e, 0x03e8, TENDRIL_SERIAL_FTDI, 0 }, /* TTI QL355P */
    { 0x10a6, 0xaa26, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS2 DCU11CLONE */
    { 0x10ab, 0x10c5, TENDRIL_SERIAL_CP210X, 0 }, /* USI MC60 */
    { 0x10b5, 0xac70, TENDRIL_SERIAL_PL2303, 0 }, /* PLX CA42 */
    { 0x10c4, 0x0f91, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS VSTABI */
    { 0x10c4, 0x1101, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS ARKHAM_DS101_M */
    { 0x10c4, 0x1601, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS ARKHAM_DS101_A */
    { 0x10c4, 0x800a, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS BSM7DUSB */
    { 0x10c4, 0x803b, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS POLOLU */
    { 0x10c4, 0x8043, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS SB_PARAMOUNT_ME */
    { 0x10c4, 0x8044, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CYGNAL_DEBUG */
    { 0x10c4, 0x8066, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS ARGUSISP */
    { 0x10c4, 0x806f, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS IMS_USB_RS422 */
    { 0x10c4, 0x807a, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CRUMB128 */
    { 0x10c4, 0x80c4, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS OPTRIS_MSPRO */
    { 0x10c4, 0x80ca, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS DEGREE */
    { 0x10c4, 0x80dd, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS TRACIENT */
    { 0x10c4, 0x80ed, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS TRAQMATE */
    { 0x10c4, 0x80f6, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS SUUNTO */
    { 0x10c4, 0x8115, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS ARYGON_MIFARE */
    { 0x10c4, 0x813d, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS BURNSIDE */
    { 0x10c4, 0x813f, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS TAMSMASTER */
    { 0x10c4, 0x814a, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS WMRBATT */
    { 0x10c4, 0x814b, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS WMRRIGTALK */
    { 0x10c4, 0x8156, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS B_G_H3000 */
    { 0x10c4, 0x815e, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS HELICOM */
    { 0x10c4, 0x815f, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS HAMLINKUSB */
    { 0x10c4, 0x818b, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS AVIT_USB_TTL */
    { 0x10c4, 0x819f, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS MJS_TOSLINK */
    { 0x10c4, 0x81a6, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS WAVIT */
    { 0x10c4, 0x81a9, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS MULTIPLEX_RC */
    { 0x10c4, 0x81ac, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS MSD_DASHHAWK */
    { 0x10c4, 0x81ad, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS INSYS_MODEM */
    { 0x10c4, 0x81c8, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS LIPOWSKY_JTAG */
    { 0x10c4, 0x81e2, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS LIPOWSKY_LIN */
    { 0x10c4, 0x81e7, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS AEROCOMM */
    { 0x10c4, 0x81e8, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS ZEPHYR_BIO */
    { 0x10c4, 0x81f2, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS EMS_C1007 */
    { 0x10c4, 0x8218, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS LIPOWSKY_HARP */
    { 0x10c4, 0x822b, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS C2_EDGE_MODEM */
    { 0x10c4, 0x826b, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CYGNAL_GPS */
    { 0x10c4, 0x8293, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS TELEGESIS_ETRX2 */
    { 0x10c4, 0x82f9, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS PROCYON_AVS */
    { 0x10c4, 0x8341, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS MC35PU */
    { 0x10c4, 0x8382, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CYGNAL */
    { 0x10c4, 0x83a8, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS AMBER_AMB2560 */
    { 0x10c4, 0x83d8, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS DEKTEK_DTAPLUS */
    { 0x10c4, 0x8411, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS KYOCERA_GPS */
    { 0x10c4, 0x8418, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS IRZ_SG10 */
    { 0x10c4, 0x846e, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS BEI_VCP */
    { 0x10c4, 0x8477, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS BALLUFF_RFID */
    { 0x10c4, 0x85ea, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS AC_SERV_IBUS */
    { 0x10c4, 0x85eb, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS AC_SERV_CIS */
    { 0x10c4, 0x85f8, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS V_PREON32 */
    { 0x10c4, 0x8664, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS AC_SERV_CAN */
    { 0x10c4, 0x8665, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS AC_SERV_OBD */
    { 0x10c4, 0x88a4, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS MMB_ZIGBEE */
    { 0x10c4, 0x88a5, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS INGENI_ZIGBEE */
    { 0x10c4, 0x8a2a, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS HUBZ */
    { 0x10c4, 0x8b34, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS BV_AV2010_10 */
    { 0x10c4, 0xea60, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CP2102 */
    { 0x10c4, 0xea61, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CP210X_2 */
    { 0x10c4, 0xea70, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CP210X_3 */
    { 0x10c4, 0xea71, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS INFINITY_MIC */
    { 0x10c4, 0xea80, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS CP210X_4 */
    { 0x10c4, 0xf001, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS USBSCOPE50 */
    { 0x10c4, 0xf002, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS USBWAVE12 */
    { 0x10c4, 0xf003, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS USBPULSE100 */
    { 0x10c4, 0xf004, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS USBCOUNT50 */
    { 0x10c5, 0xea61, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS3 GPRS_MODEM */
    { 0x10ce, 0xea61, TENDRIL_SERIAL_CP210X, 0 }, /* SILABS4 100EU_MODEM */
    { 0x11ad, 0x0701, TENDRIL_SERIAL_PL2303, 0 }, /* SANWA KB_USB2 */
    { 0x11f5, 0x0001, TENDRIL_SERIAL_PL2303, 0 }, /* SIEMENS3 SX1 */
    { 0x11f5, 0x0003, TENDRIL_SERIAL_PL2303, 0 }, /* SIEMENS3 X65 */
    { 0x11f5, 0x0004, TENDRIL_SERIAL_PL2303, 0 }, /* SIEMENS3 X75 */
    { 0x11f5, 0x0005, TENDRIL_SERIAL_PL2303, 0 }, /* SIEMENS3 EF81 */
    { 0x11f6, 0x2001, TENDRIL_SERIAL_PL2303, 0 }, /* NETINDEX WS002IN */
    { 0x11f7, 0x02df, TENDRIL_SERIAL_PL2303, 0 }, /* ALCATEL OT535 */
    { 0x1209, 0x1002, TENDRIL_SERIAL_FTDI, 0 }, /* INTERBIOMETRICS IOBOARD */
    /* INTERBIOMETRICS MINI_IOBOARD */
    { 0x1209, 0x1006, TENDRIL_SERIAL_FTDI, 0 },
    { 0x128d, 0x0001, TENDRIL_SERIAL_FTDI, 0 }, /* TESTO USB_INTERFACE */
    { 0x1342, 0x0202, TENDRIL_SERIAL_FTDI, 0 }, /* MOBILITY USB_SERIAL */
    { 0x13ad, 0x9999, TENDRIL_SERIAL_CP210X, 0 }, /* BALTECH CARDREADER */
    { 0x1453, 0x4026, TENDRIL_SERIAL_PL2303, 0 }, /* RADIOSHACK USBCABLE */
    { 0x1457, 0x5118, TENDRIL_SERIAL_FTDI, 0x01 }, /* FIC NEO1973_DEBUG */
    { 0x1546, 0x0501, TENDRIL_SERIAL_FTDI, 0 }, /* UBLOX XPLR_M9 */
    { 0x1555, 0x0004, TENDRIL_SERIAL_CP210X, 0 }, /* OWEN AC4 */
    { 0x15ba, 0x0003, TENDRIL_SERIAL_FTDI, 0x01 }, /* OLIMEX ARM_USB_OCD */
    { 0x15ba, 0x002b, TENDRIL_SERIAL_FTDI, 0x01 }, /* OLIMEX ARM_USB_OCD_H */
    { 0x166a, 0x0101, TENDRIL_SERIAL_CP210X, 0 }, /* CLIPSAL 560884 */
    { 0x166a, 0x0201, TENDRIL_SERIAL_CP210X, 0 }, /* CLIPSAL 5500PACA */
    { 0x166a, 0x0301, TENDRIL_SERIAL_CP210X, 0 }, /* CLIPSAL 5800PC */
    { 0x166a, 0x0303, TENDRIL_SERIAL_CP210X, 0 }, /* CLIPSAL 5500PCU */
    { 0x166a, 0x0304, TENDRIL_SERIAL_CP210X, 0 }, /* CLIPSAL 5000CT2 */
    { 0x166a, 0x0305, TENDRIL_SERIAL_CP210X, 0 }, /* CLIPSAL C5000CT2 */
    { 0x166a, 0x0401, TENDRIL_SERIAL_CP210X, 0 }, /* CLIPSAL L51xx */
    { 0x16d6, 0x0001, TENDRIL_SERIAL_CP210X, 0 }, /* JABLOTRON PC60B */
    { 0x16dc, 0x0010, TENDRIL_SERIAL_CP210X, 0 }, /* WIENERPLEINBAUS PL512 */
    { 0x16dc, 0x0011, TENDRIL_SERIAL_CP210X, 0 }, /* WIENERPLEINBAUS RCM */
    { 0x16dc, 0x0012, TENDRIL_SERIAL_CP210X, 0 }, /* WIENERPLEINBAUS MPOD */
    { 0x16dc, 0x0015, TENDRIL_SERIAL_CP210X, 0 }, /* WIENERPLEINBAUS CML */
    { 0x1781, 0x0c30, TENDRIL_SERIAL_FTDI, 0 }, /* METAGEEK TELLSTICK */
    { 0x17a8, 0x0001, TENDRIL_SERIAL_CP210X, 0 }, /* KAMSTRUP OPTICALEYE */
    { 0x17a8, 0x0005, TENDRIL_SERIAL_CP210X, 0 }, /* KAMSTRUP MBUS_250D */
    { 0x17f4, 0xaaaa, TENDRIL_SERIAL_CP210X, 0 }, /* WAVESENSE JAZZ */
    { 0x1843, 0x0200, TENDRIL_SERIAL_CP210X, 0 }, /* VAISALA CABLE */
    { 0x18ef, 0xe00f, TENDRIL_SERIAL_CP210X, 0 }, /* ELV USBI2C */
    { 0x1a79, 0x6001, TENDRIL_SERIAL_FTDI, 0 }, /* BAYER CONTOUR_CABLE */
    { 0x1a86, 0x5523, TENDRIL_SERIAL_CH34X, 0 }, /* WCH2 CH341SER_2 */
    { 0x1a86, 0x55d3, TENDRIL_SERIAL_CH34X, 0 }, /* WCH2 CH343SER */
    { 0x1a86, 0x55d4, TENDRIL_SERIAL_CH34X, 0 }, /* WCH2 CH9102SER */
    { 0x1a86, 0x7522, TENDRIL_SERIAL_CH34X, 0 }, /* WCH2 CH341SER_3 */
    { 0x1a86, 0x7523, TENDRIL_SERIAL_CH34X, 0 }, /* WCH2 CH341SER */
    { 0x1adb, 0x0001, TENDRIL_SERIAL_CP210X, 0 }, /* SEL C662 */
    /* MATRIXORBITAL FTDI_RANGE_0100 */
    { 0x1b3d, 0x0100, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0101 */
    { 0x1b3d, 0x0101, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0102 */
    { 0x1b3d, 0x0102, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0103 */
    { 0x1b3d, 0x0103, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0104 */
    { 0x1b3d, 0x0104, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0105 */
    { 0x1b3d, 0x0105, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0106 */
    { 0x1b3d, 0x0106, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0107 */
    { 0x1b3d, 0x0107, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0108 */
    { 0x1b3d, 0x0108, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0109 */
    { 0x1b3d, 0x0109, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_010A */
    { 0x1b3d, 0x010a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_010B */
    { 0x1b3d, 0x010b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_010C */
    { 0x1b3d, 0x010c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_010D */
    { 0x1b3d, 0x010d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_010E */
    { 0x1b3d, 0x010e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_010F */
    { 0x1b3d, 0x010f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0110 */
    { 0x1b3d, 0x0110, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0111 */
    { 0x1b3d, 0x0111, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0112 */
    { 0x1b3d, 0x0112, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0113 */
    { 0x1b3d, 0x0113, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0114 */
    { 0x1b3d, 0x0114, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0115 */
    { 0x1b3d, 0x0115, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0116 */
    { 0x1b3d, 0x0116, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0117 */
    { 0x1b3d, 0x0117, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0118 */
    { 0x1b3d, 0x0118, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0119 */
    { 0x1b3d, 0x0119, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_011A */
    { 0x1b3d, 0x011a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_011B */
    { 0x1b3d, 0x011b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_011C */
    { 0x1b3d, 0x011c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_011D */
    { 0x1b3d, 0x011d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_011E */
    { 0x1b3d, 0x011e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_011F */
    { 0x1b3d, 0x011f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0120 */
    { 0x1b3d, 0x0120, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0121 */
    { 0x1b3d, 0x0121, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0122 */
    { 0x1b3d, 0x0122, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0123 */
    { 0x1b3d, 0x0123, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0124 */
    { 0x1b3d, 0x0124, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0125 */
    { 0x1b3d, 0x0125, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0126 */
    { 0x1b3d, 0x0126, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0128 */
    { 0x1b3d, 0x0128, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0129 */
    { 0x1b3d, 0x0129, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_012A */
    { 0x1b3d, 0x012a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_012B */
    { 0x1b3d, 0x012b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_012D */
    { 0x1b3d, 0x012d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_012E */
    { 0x1b3d, 0x012e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_012F */
    { 0x1b3d, 0x012f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0130 */
    { 0x1b3d, 0x0130, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0131 */
    { 0x1b3d, 0x0131, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0132 */
    { 0x1b3d, 0x0132, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0133 */
    { 0x1b3d, 0x0133, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0134 */
    { 0x1b3d, 0x0134, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0135 */
    { 0x1b3d, 0x0135, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0136 */
    { 0x1b3d, 0x0136, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0137 */
    { 0x1b3d, 0x0137, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0138 */
    { 0x1b3d, 0x0138, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0139 */
    { 0x1b3d, 0x0139, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_013A */
    { 0x1b3d, 0x013a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_013B */
    { 0x1b3d, 0x013b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_013C */
    { 0x1b3d, 0x013c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_013D */
    { 0x1b3d, 0x013d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_013E */
    { 0x1b3d, 0x013e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_013F */
    { 0x1b3d, 0x013f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0140 */
    { 0x1b3d, 0x0140, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0141 */
    { 0x1b3d, 0x0141, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0142 */
    { 0x1b3d, 0x0142, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0143 */
    { 0x1b3d, 0x0143, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0144 */
    { 0x1b3d, 0x0144, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0145 */
    { 0x1b3d, 0x0145, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0146 */
    { 0x1b3d, 0x0146, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0147 */
    { 0x1b3d, 0x0147, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0148 */
    { 0x1b3d, 0x0148, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0149 */
    { 0x1b3d, 0x0149, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_014A */
    { 0x1b3d, 0x014a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_014B */
    { 0x1b3d, 0x014b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_014C */
    { 0x1b3d, 0x014c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_014D */
    { 0x1b3d, 0x014d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_014E */
    { 0x1b3d, 0x014e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_014F */
    { 0x1b3d, 0x014f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0150 */
    { 0x1b3d, 0x0150, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0151 */
    { 0x1b3d, 0x0151, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0152 */
    { 0x1b3d, 0x0152, TENDRIL_SERIAL_FTDI, 0 },
    { 0x1b3d, 0x0153, TENDRIL_SERIAL_FTDI, 0 }, /* MATRIXORBITAL MOUA */
    /* MATRIXORBITAL FTDI_RANGE_0159 */
    { 0x1b3d, 0x0159, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_015A */
    { 0x1b3d, 0x015a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_015B */
    { 0x1b3d, 0x015b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_015C */
    { 0x1b3d, 0x015c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_015D */
    { 0x1b3d, 0x015d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_015E */
    { 0x1b3d, 0x015e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_015F */
    { 0x1b3d, 0x015f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0160 */
    { 0x1b3d, 0x0160, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0161 */
    { 0x1b3d, 0x0161, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0162 */
    { 0x1b3d, 0x0162, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0163 */
    { 0x1b3d, 0x0163, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0164 */
    { 0x1b3d, 0x0164, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0165 */
    { 0x1b3d, 0x0165, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0166 */
    { 0x1b3d, 0x0166, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0167 */
    { 0x1b3d, 0x0167, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0168 */
    { 0x1b3d, 0x0168, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0169 */
    { 0x1b3d, 0x0169, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_016A */
    { 0x1b3d, 0x016a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_016B */
    { 0x1b3d, 0x016b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_016C */
    { 0x1b3d, 0x016c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_016D */
    { 0x1b3d, 0x016d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_016E */
    { 0x1b3d, 0x016e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_016F */
    { 0x1b3d, 0x016f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0170 */
    { 0x1b3d, 0x0170, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0171 */
    { 0x1b3d, 0x0171, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0172 */
    { 0x1b3d, 0x0172, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0173 */
    { 0x1b3d, 0x0173, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0174 */
    { 0x1b3d, 0x0174, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0175 */
    { 0x1b3d, 0x0175, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0176 */
    { 0x1b3d, 0x0176, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0177 */
    { 0x1b3d, 0x0177, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0178 */
    { 0x1b3d, 0x0178, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0179 */
    { 0x1b3d, 0x0179, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_017A */
    { 0x1b3d, 0x017a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_017B */
    { 0x1b3d, 0x017b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_017C */
    { 0x1b3d, 0x017c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_017D */
    { 0x1b3d, 0x017d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_017E */
    { 0x1b3d, 0x017e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_017F */
    { 0x1b3d, 0x017f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0180 */
    { 0x1b3d, 0x0180, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0181 */
    { 0x1b3d, 0x0181, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0182 */
    { 0x1b3d, 0x0182, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0183 */
    { 0x1b3d, 0x0183, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0184 */
    { 0x1b3d, 0x0184, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0185 */
    { 0x1b3d, 0x0185, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0186 */
    { 0x1b3d, 0x0186, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0187 */
    { 0x1b3d, 0x0187, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0188 */
    { 0x1b3d, 0x0188, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0189 */
    { 0x1b3d, 0x0189, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_018A */
    { 0x1b3d, 0x018a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_018B */
    { 0x1b3d, 0x018b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_018C */
    { 0x1b3d, 0x018c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_018D */
    { 0x1b3d, 0x018d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_018E */
    { 0x1b3d, 0x018e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_018F */
    { 0x1b3d, 0x018f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0190 */
    { 0x1b3d, 0x0190, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0191 */
    { 0x1b3d, 0x0191, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0192 */
    { 0x1b3d, 0x0192, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0193 */
    { 0x1b3d, 0x0193, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0194 */
    { 0x1b3d, 0x0194, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0195 */
    { 0x1b3d, 0x0195, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0196 */
    { 0x1b3d, 0x0196, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0197 */
    { 0x1b3d, 0x0197, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0198 */
    { 0x1b3d, 0x0198, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_0199 */
    { 0x1b3d, 0x0199, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_019A */
    { 0x1b3d, 0x019a, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_019B */
    { 0x1b3d, 0x019b, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_019C */
    { 0x1b3d, 0x019c, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_019D */
    { 0x1b3d, 0x019d, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_019E */
    { 0x1b3d, 0x019e, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_019F */
    { 0x1b3d, 0x019f, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A0 */
    { 0x1b3d, 0x01a0, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A1 */
    { 0x1b3d, 0x01a1, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A2 */
    { 0x1b3d, 0x01a2, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A3 */
    { 0x1b3d, 0x01a3, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A4 */
    { 0x1b3d, 0x01a4, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A5 */
    { 0x1b3d, 0x01a5, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A6 */
    { 0x1b3d, 0x01a6, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A7 */
    { 0x1b3d, 0x01a7, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A8 */
    { 0x1b3d, 0x01a8, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01A9 */
    { 0x1b3d, 0x01a9, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01AA */
    { 0x1b3d, 0x01aa, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01AB */
    { 0x1b3d, 0x01ab, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01AC */
    { 0x1b3d, 0x01ac, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01AD */
    { 0x1b3d, 0x01ad, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01AE */
    { 0x1b3d, 0x01ae, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01AF */
    { 0x1b3d, 0x01af, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B0 */
    { 0x1b3d, 0x01b0, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B1 */
    { 0x1b3d, 0x01b1, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B2 */
    { 0x1b3d, 0x01b2, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B3 */
    { 0x1b3d, 0x01b3, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B4 */
    { 0x1b3d, 0x01b4, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B5 */
    { 0x1b3d, 0x01b5, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B6 */
    { 0x1b3d, 0x01b6, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B7 */
    { 0x1b3d, 0x01b7, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B8 */
    { 0x1b3d, 0x01b8, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01B9 */
    { 0x1b3d, 0x01b9, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01BA */
    { 0x1b3d, 0x01ba, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01BB */
    { 0x1b3d, 0x01bb, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01BC */
    { 0x1b3d, 0x01bc, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01BD */
    { 0x1b3d, 0x01bd, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01BE */
    { 0x1b3d, 0x01be, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01BF */
    { 0x1b3d, 0x01bf, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C0 */
    { 0x1b3d, 0x01c0, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C1 */
    { 0x1b3d, 0x01c1, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C2 */
    { 0x1b3d, 0x01c2, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C3 */
    { 0x1b3d, 0x01c3, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C4 */
    { 0x1b3d, 0x01c4, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C5 */
    { 0x1b3d, 0x01c5, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C6 */
    { 0x1b3d, 0x01c6, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C7 */
    { 0x1b3d, 0x01c7, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C8 */
    { 0x1b3d, 0x01c8, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01C9 */
    { 0x1b3d, 0x01c9, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01CA */
    { 0x1b3d, 0x01ca, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01CB */
    { 0x1b3d, 0x01cb, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01CC */
    { 0x1b3d, 0x01cc, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01CD */
    { 0x1b3d, 0x01cd, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01CE */
    { 0x1b3d, 0x01ce, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01CF */
    { 0x1b3d, 0x01cf, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D0 */
    { 0x1b3d, 0x01d0, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D1 */
    { 0x1b3d, 0x01d1, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D2 */
    { 0x1b3d, 0x01d2, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D3 */
    { 0x1b3d, 0x01d3, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D4 */
    { 0x1b3d, 0x01d4, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D5 */
    { 0x1b3d, 0x01d5, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D6 */
    { 0x1b3d, 0x01d6, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D7 */
    { 0x1b3d, 0x01d7, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D8 */
    { 0x1b3d, 0x01d8, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01D9 */
    { 0x1b3d, 0x01d9, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01DA */
    { 0x1b3d, 0x01da, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01DB */
    { 0x1b3d, 0x01db, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01DC */
    { 0x1b3d, 0x01dc, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01DD */
    { 0x1b3d, 0x01dd, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01DE */
    { 0x1b3d, 0x01de, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01DF */
    { 0x1b3d, 0x01df, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E0 */
    { 0x1b3d, 0x01e0, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E1 */
    { 0x1b3d, 0x01e1, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E2 */
    { 0x1b3d, 0x01e2, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E3 */
    { 0x1b3d, 0x01e3, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E4 */
    { 0x1b3d, 0x01e4, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E5 */
    { 0x1b3d, 0x01e5, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E6 */
    { 0x1b3d, 0x01e6, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E7 */
    { 0x1b3d, 0x01e7, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E8 */
    { 0x1b3d, 0x01e8, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01E9 */
    { 0x1b3d, 0x01e9, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01EA */
    { 0x1b3d, 0x01ea, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01EB */
    { 0x1b3d, 0x01eb, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01EC */
    { 0x1b3d, 0x01ec, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01ED */
    { 0x1b3d, 0x01ed, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01EE */
    { 0x1b3d, 0x01ee, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01EF */
    { 0x1b3d, 0x01ef, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F0 */
    { 0x1b3d, 0x01f0, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F1 */
    { 0x1b3d, 0x01f1, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F2 */
    { 0x1b3d, 0x01f2, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F3 */
    { 0x1b3d, 0x01f3, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F4 */
    { 0x1b3d, 0x01f4, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F5 */
    { 0x1b3d, 0x01f5, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F6 */
    { 0x1b3d, 0x01f6, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F7 */
    { 0x1b3d, 0x01f7, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F8 */
    { 0x1b3d, 0x01f8, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01F9 */
    { 0x1b3d, 0x01f9, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01FA */
    { 0x1b3d, 0x01fa, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01FB */
    { 0x1b3d, 0x01fb, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01FC */
    { 0x1b3d, 0x01fc, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01FD */
    { 0x1b3d, 0x01fd, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01FE */
    { 0x1b3d, 0x01fe, TENDRIL_SERIAL_FTDI, 0 },
    /* MATRIXORBITAL FTDI_RANGE_01FF */
    { 0x1b3d, 0x01ff, TENDRIL_SERIAL_FTDI, 0 },
    { 0x1b91, 0x0064, TENDRIL_SERIAL_FTDI, 0 }, /* TML USB_SERIAL */
    { 0x1bc9, 0x6001, TENDRIL_SERIAL_FTDI, 0 }, /* ALTI2 N3 */
    { 0x1be3, 0x07a6, TENDRIL_SERIAL_CP210X, 0 }, /* WAGO SERVICECABLE */
    { 0x1c0c, 0x0102, TENDRIL_SERIAL_FTDI, 0x01 }, /* IONICS PLUGCOMPUTER */
    /* DRESDENELEKTRONIK SENSORTERMINALBOARD */
    { 0x1cf1, 0x0001, TENDRIL_SERIAL_FTDI, 0 },
    /* DRESDENELEKTRONIK WIRELESSHANDHELDTERMINAL */
    { 0x1cf1, 0x0004, TENDRIL_SERIAL_FTDI, 0 },
    /* DRESDENELEKTRONIK DE_RFNODE */
    { 0x1cf1, 0x001c, TENDRIL_SERIAL_FTDI, 0 },
    /* DRESDENELEKTRONIK LEVELSHIFTERSTICKLOWCOST */
    { 0x1cf1, 0x0022, TENDRIL_SERIAL_FTDI, 0 },
    { 0x1e29, 0x0102, TENDRIL_SERIAL_CP210X, 0 }, /* FESTO CPX_USB */
    { 0x1e29, 0x0501, TENDRIL_SERIAL_CP210X, 0 }, /* FESTO CMSP */
    { 0x1fb9, 0x0100, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 121 */
    { 0x1fb9, 0x0200, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 218A */
    { 0x1fb9, 0x0201, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 219 */
    { 0x1fb9, 0x0202, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 233 */
    { 0x1fb9, 0x0203, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 235 */
    { 0x1fb9, 0x0300, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 335 */
    { 0x1fb9, 0x0301, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 336 */
    { 0x1fb9, 0x0302, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 350 */
    { 0x1fb9, 0x0303, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 371 */
    { 0x1fb9, 0x0400, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 411 */
    { 0x1fb9, 0x0401, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 425 */
    { 0x1fb9, 0x0402, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 455A */
    { 0x1fb9, 0x0403, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 475A */
    { 0x1fb9, 0x0404, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 465 */
    { 0x1fb9, 0x0600, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 625A */
    { 0x1fb9, 0x0601, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 642A */
    { 0x1fb9, 0x0602, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 648 */
    { 0x1fb9, 0x0700, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 737 */
    { 0x1fb9, 0x0701, TENDRIL_SERIAL_CP210X, 0 }, /* LAKESHORE 776 */
    { 0x20b7, 0x0713, TENDRIL_SERIAL_FTDI, 0x01 }, /* QIHARDWARE JTAGSERIAL */
    { 0x2100, 0x9e52, TENDRIL_SERIAL_FTDI, 0 }, /* RTSYSTEMS SERIAL_VX7 */
    { 0x2100, 0x9e54, TENDRIL_SERIAL_FTDI, 0 }, /* RTSYSTEMS CT29B */
    /* WESTMOUNTAIN RIGBLASTER_ADVANTAGE */
    { 0x2405, 0x0003, TENDRIL_SERIAL_CP210X, 0 },
    { 0x2478, 0x2008, TENDRIL_SERIAL_PL2303, 0 }, /* TRIPPLITE U209 */
    { 0x3195, 0xf190, TENDRIL_SERIAL_CP210X, 0 }, /* LINKINSTRUMENTS MSO19 */
    { 0x3195, 0xf280, TENDRIL_SERIAL_CP210X, 0 }, /* LINKINSTRUMENTS MSO28 */
    { 0x3195, 0xf281, TENDRIL_SERIAL_CP210X, 0 }, /* LINKINSTRUMENTS MSO28_2 */
    { 0x413c, 0x9500, TENDRIL_SERIAL_CP210X, 0 }, /* DELL DW700 */
    { 0x4348, 0x5523, TENDRIL_SERIAL_CH34X, 0 }, /* WCH CH341SER */
    { 0x5050, 0x0100, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SB485 */
    { 0x5050, 0x0101, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH AP485 */
    { 0x5050, 0x0102, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SB422 */
    { 0x5050, 0x0103, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SB485_2 */
    { 0x5050, 0x0104, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH AP485_2 */
    { 0x5050, 0x0105, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SB422_2 */
    { 0x5050, 0x0106, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SB485S */
    { 0x5050, 0x0107, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SB485C */
    { 0x5050, 0x0300, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH LEC */
    { 0x5050, 0x0301, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SB232 */
    { 0x5050, 0x0400, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH TMU */
    { 0x5050, 0x0500, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH IRAMP */
    { 0x5050, 0x0700, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH DRAK5 */
    { 0x5050, 0x0800, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO8X8 */
    { 0x5050, 0x0900, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO4X4 */
    { 0x5050, 0x0a00, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO2X2 */
    { 0x5050, 0x0b00, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO10X1 */
    { 0x5050, 0x0c00, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO30X3 */
    { 0x5050, 0x0d00, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO60X3 */
    { 0x5050, 0x0e00, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO2X16 */
    { 0x5050, 0x0f00, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH QUIDO3X32 */
    { 0x5050, 0x1000, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH DRAK6 */
    { 0x5050, 0x8000, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH UPSUSB */
    { 0x5050, 0x8001, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH MU */
    { 0x5050, 0x8002, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH SIMUKEY */
    { 0x5050, 0x8003, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH AD4USB */
    { 0x5050, 0x8004, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH GMUX */
    { 0x5050, 0x8005, TENDRIL_SERIAL_FTDI, 0 }, /* PAPOUCH GMSR */
    { 0x5372, 0x2303, TENDRIL_SERIAL_PL2303, 0 }, /* PROLIFIC2 PL2303 */
    { 0x6189, 0x2068, TENDRIL_SERIAL_PL2303, 0 }, /* SITECOM SERIAL */
    { 0x9e88, 0x9e8f, TENDRIL_SERIAL_FTDI, 0x01 }, /* MARVELL SHEEVAPLUG */
    { 0xdaae, 0xead6, TENDRIL_SERIAL_CP210X, 0 }, /* DATAAPEX MULTICOM */
    { 0xdeee, 0x0300, TENDRIL_SERIAL_FTDI, 0 }, /* EVOLUTION ER1 */
    { 0xdeee, 0x0302, TENDRIL_SERIAL_FTDI, 0 }, /* EVOLUTION HYBRID */
    { 0xdeee, 0x0303, TENDRIL_SERIAL_FTDI, 0 }, /* EVOLUTION RCM4 */
};
/* END FreeBSD sys/dev/usb/serial/uftdi.c, uplcom.c, uchcom.c, uslcom.c, sys/dev/usb/usbdevs */

/* BEGIN FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/usbdevs */
/*
 * From FreeBSD's sys/dev/usb/quirk/usb_quirk.c (BSD-2-Clause), naming the IDs
 * as its sys/dev/usb/usbdevs (BSD-2-Clause) does.
 */
static const struct tendril_id_quirk serial_quirks[] = {
    /* METRICOM RICOCHET_GS */
    { 0x0870, 0x0001, 0x0100, 0x0100, SQ_ASSUME_CM_OVER_DATA },
    /* SANYO SCP4900 */
    { 0x0474, 0x0701, 0x0000, 0x0000, SQ_ASSUME_CM_OVER_DATA },
    /* MOTOROLA2 T720C */
    { 0x22b8, 0x2822, 0x0001, 0x0001, SQ_ASSUME_CM_OVER_DATA },
    /* EICON DIVA852 */
    { 0x0734, 0x4905, 0x0100, 0x0100, SQ_ASSUME_CM_OVER_DATA },
    /* SIEMENS2 ES75 */
    { 0x0681, 0x0034, 0x0000, 0x0000, SQ_ASSUME_CM_OVER_DATA },
    /* QUALCOMM CDMA_MSM */
    { 0x1004, 0x6000, 0x0000, 0xffff, SQ_ASSUME_CM_OVER_DATA },
    /* QUALCOMM2 CDMA_MSM */
    { 0x05c6, 0x3196, 0x0000, 0xffff, SQ_ASSUME_CM_OVER_DATA },
    /* CURITEL UM150 */
    { 0x106c, 0x3711, 0x0000, 0xffff, SQ_ASSUME_CM_OVER_DATA },
    /* CURITEL UM175 */
    { 0x106c, 0x3714, 0x0000, 0xffff, SQ_ASSUME_CM_OVER_DATA },
    /* VERTEX VW110L */
    { 0x1fe7, 0x0100, 0x0000, 0xffff, SQ_ASSUME_CM_OVER_DATA },
    /* BALTECH SMARTCARDREADER */
    { 0x13ad, 0xf019, 0x0000, 0xffff, SQ_IGNORE_CDC_CM },
};
/* END FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/usbdevs */

struct acm_id {
    uint16_t vendor;
    uint16_t product;
    uint16_t class;
    uint16_t subclass;
    uint16_t protocol;
};

/* BEGIN FreeBSD sys/dev/usb/serial/umodem.c, sys/dev/usb/usbdevs */
/*
 * The interfaces FreeBSD's umodem.c drives (umodem_dual_devs[] and
 * umodem_host_devs[]), ANY matching every value, naming the IDs as its usbdevs
 * does. Left out is its Huawei match for the vendor-specific CDC ACM protocol,
 * which its sys/dev/usb/net/if_urndis.c (ISC license) wins.
 */
static const struct acm_id acm_ids[] = {
    { ANY, ANY, TENDRIL_CLASS_CDC, CDC_SUBCLASS_ACM, CDC_PROTOCOL_AT },
    { ANY, ANY, TENDRIL_CLASS_CDC, CDC_SUBCLASS_ACM, CDC_PROTOCOL_NONE },
    { 0x12d1, ANY, TENDRIL_CLASS_VENDOR, 0x02, 0x01 }, /* HUAWEI */
    { 0x12d1, ANY, TENDRIL_CLASS_VENDOR, 0x02, 0x02 }, /* HUAWEI */
    { 0x12d1, ANY, TENDRIL_CLASS_VENDOR, 0x02, 0x10 }, /* HUAWEI */
    { 0x12d1, ANY, TENDRIL_CLASS_VENDOR, 0x02, 0x12 }, /* HUAWEI */
    { 0x12d1, ANY, TENDRIL_CLASS_VENDOR, 0x02, 0x61 }, /* HUAWEI */
    { 0x12d1, ANY, TENDRIL_CLASS_VENDOR, 0x02, 0x62 }, /* HUAWEI */
    { 0x12d1, ANY, TENDRIL_CLASS_VENDOR, 0x0f, 0xff }, /* HUAWEI */
    { 0x0482, 0x0203, ANY, ANY, ANY }, /* KYOCERA AHK3001V */
    { 0x1199, 0x0218, ANY, ANY, ANY }, /* SIERRA MC5720 */
    { 0x106c, 0x3701, ANY, ANY, ANY }, /* CURITEL PC5740 */
    { 0x0416, 0x5011, ANY, ANY, ANY }, /* WINBOND CDC */
};
/* END FreeBSD sys/dev/usb/serial/umodem.c, sys/dev/usb/usbdevs */

static const struct tendril_serial_callbacks *callbacks;

static void put_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

void serial_status_update(struct tendril_serial *port, uint32_t status) {
    uint32_t events = status & ~LINE_STATES;
    if ((status & LINE_STATES) == port->status && events == 0) {
        return;
    }
    port->status = status & LINE_STATES;
    if (port->attached && callbacks->status != NULL) {
        callbacks->status(port, status);
    }
}

/* Control requests, run one at a time in the order queued */

void serial_deliver(
    struct tendril_serial *port, const uint8_t *data, size_t len
) {
    if (callbacks->receive != NULL) {
        callbacks->receive(port, data, len);
    }
}

static struct serial_op *op_new(void (*done)(void *, int), void *ctx) {
    struct serial_op *op = tendril_zalloc(sizeof(struct serial_op));
    if (op != NULL) {
        op->done = done;
        op->ctx = ctx;
    }
    return op;
}

void serial_op_add(
    struct serial_op *op, uint8_t type, uint8_t request, uint16_t value,
    uint16_t index, const void *data, size_t length
) {
    if (op->count >= MAX_REQS || length > sizeof(op->reqs[0].data)) {
        op->status = TENDRIL_ERR_INVALID;
        return;
    }
    struct serial_req *r = &op->reqs[op->count++];
    r->type = type;
    r->request = request;
    r->value = value;
    r->index = index;
    r->length = length;
    if (data != NULL) {
        memcpy(r->data, data, length);
    }
}

static void op_kick(struct tendril_serial *port);

static void op_request_done(struct tendril_serial *port, int st) {
    struct serial_op *op = port->op;
    if (st == TENDRIL_ERR_SHORT) {
        st = TENDRIL_OK;
    }
    if (st == TENDRIL_ERR_NO_DEVICE) {
        port->stopped = true;
    }
    if (st != TENDRIL_OK && (!op->tolerant || st == TENDRIL_ERR_NO_DEVICE)
     && op->status == TENDRIL_OK) {
        op->status = st;
    }
    if (op->after != NULL) {
        op->after(port, op, op->next - 1, st);
    }
}

static void ctrl_done(struct tendril_transfer *xfer) {
    struct tendril_serial *port = xfer->context;
    op_request_done(port, xfer->status);
    op_kick(port);
}

static void op_kick(struct tendril_serial *port) {
    if (port->in_kick || (port->suspended && !port->stopped)) {
        return;
    }
    port->in_kick = true;
    for (;;) {
        struct serial_op *op = port->op;
        if (op == NULL) {
            if (list_empty(&port->ops)) {
                break;
            }
            op = CONTAINER_OF(port->ops.next, struct serial_op, node);
            list_del(&op->node);
            port->op = op;
        }
        if (op->next < op->count && op->status == TENDRIL_OK
         && !port->stopped) {
            struct serial_req *r = &op->reqs[op->next++];
            struct tendril_transfer *xfer = &port->ctrl;
            tendril_transfer_init(xfer, tendril_device_ep0(port->dev));
            xfer->setup.request_type = r->type;
            xfer->setup.request = r->request;
            xfer->setup.value = tendril_le16(r->value);
            xfer->setup.index = tendril_le16(r->index);
            xfer->setup.length = tendril_le16((uint16_t)r->length);
            xfer->buffer = r->length != 0 ? r->data : NULL;
            xfer->length = r->length;
            xfer->timeout_ms = CTRL_TIMEOUT_MS;
            xfer->callback = ctrl_done;
            xfer->context = port;
            int st = tendril_transfer_submit(xfer);
            if (st == TENDRIL_OK) {
                break;
            }
            op_request_done(port, st);
            continue;
        }
        port->op = NULL;
        int st = port->stopped ? TENDRIL_ERR_NO_DEVICE : op->status;
        void (*done)(void *, int) = op->done;
        void *ctx = op->ctx;
        tendril_free(op, sizeof(struct serial_op));
        if (done != NULL) {
            done(ctx, st);
        }
    }
    port->in_kick = false;
}

static int op_queue(struct tendril_serial *port, struct serial_op *op) {
    int st = op->status;
    if (st != TENDRIL_OK) {
        tendril_free(op, sizeof(struct serial_op));
        return st;
    }
    list_add_tail(&port->ops, &op->node);
    op_kick(port);
    return TENDRIL_OK;
}

/* CDC ACM */

static void acm_line(struct tendril_serial *port, struct serial_op *op) {
    const struct tendril_serial_config *c = &port->config;
    uint8_t coding[7];
    put_le32(coding, c->baud);
    coding[4] = c->stop_bits;
    coding[5] = c->parity;
    coding[6] = c->data_bits;
    serial_op_add(
        op, REQ_CLASS_OUT, ACM_SET_LINE_CODING, 0, port->ctrl_intf->number,
        coding, sizeof(coding)
    );
}

static void acm_lines(struct tendril_serial *port, struct serial_op *op) {
    serial_op_add(
        op, REQ_CLASS_OUT, ACM_SET_CONTROL_LINE_STATE,
        (uint16_t)((port->dtr ? 1 : 0) | (port->rts ? 2 : 0)),
        port->ctrl_intf->number, NULL, 0
    );
}

static void acm_break(struct tendril_serial *port, struct serial_op *op) {
    /* BEGIN FreeBSD sys/dev/usb/serial/umodem.c */
    /*
     * FreeBSD's umodem.c sends no break to a device without SendBreak in its
     * capabilities.
     */
    if ((port->acm_caps & ACM_BREAK) == 0) {
        op->status = TENDRIL_ERR_UNSUPPORTED;
        return;
    }
    /* END FreeBSD sys/dev/usb/serial/umodem.c */
    serial_op_add(
        op, REQ_CLASS_OUT, ACM_SEND_BREAK, port->in_break ? 0xffff : 0,
        port->ctrl_intf->number, NULL, 0
    );
}

/* CH34x */

/* BEGIN FreeBSD sys/dev/usb/serial/uchcom.c */
static void ch_write_reg(
    struct tendril_serial *port, struct serial_op *op, uint8_t reg1,
    uint8_t val1, uint8_t reg2, uint8_t val2
) {
    serial_op_add(
        op, REQ_VENDOR_OUT,
        port->chip == CH_343 ? CH343_WRITE_REG : CH_WRITE_REG,
        (uint16_t)(reg1 | reg2 << 8), (uint16_t)(val1 | val2 << 8), NULL, 0
    );
}

static void ch_line(struct tendril_serial *port, struct serial_op *op) {
    const struct tendril_serial_config *c = &port->config;
    uint32_t clk = 12000000;
    uint8_t divisor;
    uint32_t max = port->chip == CH_343 ? 6000000 : 2000000;

    if (c->baud == 0 || c->baud > max
     || c->parity > TENDRIL_SERIAL_PARITY_EVEN) {
        op->status = c->baud == 0 || c->baud > max ? TENDRIL_ERR_INVALID
            : TENDRIL_ERR_UNSUPPORTED;
        return;
    }
    /*
     * The prescaler choice of FreeBSD's uchcom.c; the factor counts up to 256
     * at the scaled clock.
     */
    if (c->baud >= 256000 && port->chip == CH_343) {
        divisor = 7;
    } else if (c->baud > 23529) {
        clk /= 2;
        divisor = 3;
    } else if (c->baud > 2941) {
        clk /= 16;
        divisor = 2;
    } else if (c->baud > 367) {
        clk /= 128;
        divisor = 1;
    } else {
        clk = 11719;
        divisor = 0;
    }
    uint8_t factor = (uint8_t)(256 - clk / c->baud);
    if (c->baud == 921600 && port->chip != CH_343) {
        divisor = 7;
        factor = 243;
    }
    uint8_t lcr = CH_LCR1_RX | CH_LCR1_TX | (uint8_t)(c->data_bits - 5);
    if (c->stop_bits != TENDRIL_SERIAL_STOP_1) {
        lcr |= CH_LCR1_STOPB;
    }
    if (c->parity != TENDRIL_SERIAL_PARITY_NONE) {
        lcr |= CH_LCR1_PARENB
            | (c->parity == TENDRIL_SERIAL_PARITY_EVEN ? CH_LCR1_PAREVEN : 0);
    }
    uint16_t value = (uint16_t)(CH_T | CH_CL
        | (port->chip == CH_343 ? CH343_CT : CH_CT) | lcr << 8);
    uint16_t index = (uint16_t)(factor << 8 | divisor
        | (port->chip == CH_343 ? 0 : 0x80));
    serial_op_add(op, REQ_VENDOR_OUT, CH_RESET, value, index, NULL, 0);
}

static void ch_lines(struct tendril_serial *port, struct serial_op *op) {
    /* The lines are active low. */
    uint8_t bits = (uint8_t)~((port->dtr ? CH_DTR : 0)
        | (port->rts ? CH_RTS : 0));
    if (port->ch_version < CH_VERSION_20 && port->chip != CH_343) {
        ch_write_reg(port, op, CH_REG_STAT1, bits, CH_REG_STAT1, bits);
    } else {
        serial_op_add(op, REQ_VENDOR_OUT, CH_SET_DTRRTS, bits, 0, NULL, 0);
    }
}

static void ch_break_after(
    struct tendril_serial *port, struct serial_op *op, size_t index, int st
) {
    if (index != 0 || st != TENDRIL_OK) {
        return;
    }
    uint8_t brk = op->reqs[0].data[0];
    uint8_t lcr = op->reqs[0].data[1];
    if (port->in_break) {
        brk &= (uint8_t)~CH_BREAK;
        lcr &= (uint8_t)~CH_LCR1_TX;
    } else {
        brk |= CH_BREAK;
        lcr |= CH_LCR1_TX;
    }
    ch_write_reg(port, op, CH_REG_BREAK1, brk, CH_REG_LCR1, lcr);
}

static void ch_break(struct tendril_serial *port, struct serial_op *op) {
    if (port->chip == CH_343) {
        ch_write_reg(
            port, op,
            (uint8_t)(CH343_BREAK | (port->in_break ? 0 : CH_ABREAK)), 0, 0, 0
        );
        return;
    }
    /*
     * Read the break and line control registers, then flip their break bits.
     */
    serial_op_add(
        op, REQ_VENDOR_IN, CH_READ_REG, CH_REG_BREAK1 | CH_REG_LCR1 << 8, 0,
        NULL, 8
    );
    op->after = ch_break_after;
}
/* END FreeBSD sys/dev/usb/serial/uchcom.c */

/* CP210x */

/* BEGIN FreeBSD sys/dev/usb/serial/uslcom.c */
static void cp_line(struct tendril_serial *port, struct serial_op *op) {
    const struct tendril_serial_config *c = &port->config;
    uint8_t rate[4];
    uint8_t flow[16] = { 0 };
    uint16_t iface = port->intf->number;

    put_le32(rate, c->baud);
    serial_op_add(
        op, REQ_VENDOR_IFACE_OUT, CP_SET_BAUDRATE, 0, iface, rate, sizeof(rate)
    );
    serial_op_add(
        op, REQ_VENDOR_IFACE_OUT, CP_SET_LINE_CTL,
        (uint16_t)(c->stop_bits | c->parity << 4 | c->data_bits << 8), iface,
        NULL, 0
    );
    /*
     * AN571 revision 0.4, table 9: the handshake word and the flow replacement
     * word, then thresholds left at zero.
     */
    put_le32(
        flow,
        (port->dtr ? CP_DTR_ACTIVE : 0) | (c->rtscts ? CP_CTS_HANDSHAKE : 0)
    );
    put_le32(
        flow + 4,
        c->rtscts ? CP_RTS_HANDSHAKE : (port->rts ? CP_RTS_ACTIVE : 0)
    );
    serial_op_add(
        op, REQ_VENDOR_IFACE_OUT, CP_SET_FLOW, 0, iface, flow, sizeof(flow)
    );
}

static void cp_lines(struct tendril_serial *port, struct serial_op *op) {
    /*
     * The high byte says which lines the low byte sets; RTS stays with the
     * handshake when that is on.
     */
    uint16_t value = (uint16_t)(0x0100 | (port->dtr ? 1 : 0));
    if (!port->config.rtscts) {
        value |= (uint16_t)(0x0200 | (port->rts ? 2 : 0));
    }
    serial_op_add(
        op, REQ_VENDOR_IFACE_OUT, CP_SET_MHS, value, port->intf->number, NULL,
        0
    );
}

static void cp_break(struct tendril_serial *port, struct serial_op *op) {
    serial_op_add(
        op, REQ_VENDOR_IFACE_OUT, CP_SET_BREAK, port->in_break ? 1 : 0,
        port->intf->number, NULL, 0
    );
}

static void cp_status_after(
    struct tendril_serial *port, struct serial_op *op, size_t index, int st
) {
    (void)index;
    port->poll_queued = false;
    if (st == TENDRIL_OK) {
        uint8_t m = op->reqs[0].data[0];
        uint32_t status = ((m & 0x10) != 0 ? TENDRIL_SERIAL_CTS : 0)
            | ((m & 0x20) != 0 ? TENDRIL_SERIAL_DSR : 0)
            | ((m & 0x40) != 0 ? TENDRIL_SERIAL_RI : 0)
            | ((m & 0x80) != 0 ? TENDRIL_SERIAL_DCD : 0);
        serial_status_update(port, status);
    }
}
/* END FreeBSD sys/dev/usb/serial/uslcom.c */

/* Line settings by kind */

static void build_line(struct tendril_serial *port, struct serial_op *op) {
    switch (port->kind) {
        case TENDRIL_SERIAL_ACM: {
            acm_line(port, op);
            break;
        }
        case TENDRIL_SERIAL_FTDI: {
            serial_ftdi_line(port, op);
            break;
        }
        case TENDRIL_SERIAL_PL2303: {
            serial_pl2303_line(port, op);
            break;
        }
        case TENDRIL_SERIAL_CH34X: {
            ch_line(port, op);
            break;
        }
        default: {
            cp_line(port, op);
            break;
        }
    }
}

static void build_lines(struct tendril_serial *port, struct serial_op *op) {
    switch (port->kind) {
        case TENDRIL_SERIAL_ACM: {
            acm_lines(port, op);
            break;
        }
        case TENDRIL_SERIAL_FTDI: {
            serial_ftdi_lines(port, op);
            break;
        }
        case TENDRIL_SERIAL_PL2303: {
            serial_pl2303_lines(port, op);
            break;
        }
        case TENDRIL_SERIAL_CH34X: {
            ch_lines(port, op);
            break;
        }
        default: {
            cp_lines(port, op);
            break;
        }
    }
}

static void build_break(struct tendril_serial *port, struct serial_op *op) {
    switch (port->kind) {
        case TENDRIL_SERIAL_ACM: {
            acm_break(port, op);
            break;
        }
        case TENDRIL_SERIAL_FTDI: {
            serial_ftdi_break(port, op);
            break;
        }
        case TENDRIL_SERIAL_PL2303: {
            serial_pl2303_break(port, op);
            break;
        }
        case TENDRIL_SERIAL_CH34X: {
            ch_break(port, op);
            break;
        }
        default: {
            cp_break(port, op);
            break;
        }
    }
}

/* Receiving */

static void rx_submit(struct tendril_serial *port, size_t i);

static void schedule_retry(struct tendril_serial *port) {
    hc_lock(port->hc);
    tendril_timer_arm(port->hc, &port->timer, RETRY_MS * NS_PER_MS);
    hc_unlock(port->hc);
}

static void rx_done(struct tendril_transfer *xfer) {
    struct tendril_serial *port = xfer->context;
    size_t i = (size_t)(xfer - port->rx);
    int st = xfer->status;

    if (port->stopped || st == TENDRIL_ERR_NO_DEVICE
     || st == TENDRIL_ERR_CANCELLED) {
        return;
    }
    if (st == TENDRIL_OK || st == TENDRIL_ERR_SHORT) {
        port->rx_errors = 0;
        if (xfer->actual != 0) {
            if (port->kind == TENDRIL_SERIAL_FTDI) {
                serial_ftdi_receive(port, port->rx_buf[i], xfer->actual);
            } else {
                serial_deliver(port, port->rx_buf[i], xfer->actual);
            }
        }
        if (!port->stopped && !port->suspended) {
            rx_submit(port, i);
        }
        return;
    }
    if (++port->rx_errors > MAX_ERRORS) {
        LOG_WARN(
            "serial: receiving keeps failing (%s), giving up",
            tendril_status_str(st)
        );
        return;
    }
    if (st == TENDRIL_ERR_STALL) {
        tendril_endpoint_clear_halt(port->in_ep, NULL, NULL);
    }
    port->rx_idle |= (uint32_t)1 << i;
    schedule_retry(port);
}

static void rx_submit(struct tendril_serial *port, size_t i) {
    struct tendril_transfer *xfer = &port->rx[i];
    tendril_transfer_init(xfer, port->in_ep);
    xfer->buffer = port->rx_buf[i];
    xfer->length = port->rx_size;
    xfer->callback = rx_done;
    xfer->context = port;
    int st = tendril_transfer_submit(xfer);
    if (st == TENDRIL_ERR_NO_DEVICE) {
        port->stopped = true;
    } else if (st != TENDRIL_OK) {
        port->rx_idle |= (uint32_t)1 << i;
        schedule_retry(port);
    }
}

/* Status from the interrupt endpoint */

static void intr_submit(struct tendril_serial *port);

static void intr_done(struct tendril_transfer *xfer) {
    struct tendril_serial *port = xfer->context;
    int st = xfer->status;
    const uint8_t *b = port->intr_buf;
    size_t n = xfer->actual;

    if (port->stopped || st == TENDRIL_ERR_NO_DEVICE
     || st == TENDRIL_ERR_CANCELLED) {
        return;
    }
    if (st != TENDRIL_OK && st != TENDRIL_ERR_SHORT) {
        if (++port->intr_errors <= MAX_ERRORS) {
            if (st == TENDRIL_ERR_STALL) {
                tendril_endpoint_clear_halt(port->intr_ep, NULL, NULL);
            }
            port->intr_idle = true;
            schedule_retry(port);
        }
        return;
    }
    port->intr_errors = 0;
    switch (port->kind) {
        case TENDRIL_SERIAL_ACM: {
            /*
             * A notification header, then for SerialState two bytes laid out
             * as the status bits here.
             */
            if (n >= 10 && b[1] == ACM_SERIAL_STATE) {
                serial_status_update(port, b[8] & (uint32_t)0x7f);
            }
            break;
        }
        case TENDRIL_SERIAL_PL2303: {
            serial_pl2303_status(port, b, n);
            break;
        }
        case TENDRIL_SERIAL_CH34X: {
            /* BEGIN FreeBSD sys/dev/usb/serial/uchcom.c */
            if (n >= CH_INTR_MIN) {
                /* Active-low CTS, DSR, RI, and DCD in the low nibble. */
                uint8_t s
                    = (uint8_t)~b[port->chip == CH_343 ? n - 1 : CH_INTR_STAT];
                uint32_t status = ((s & 1) != 0 ? TENDRIL_SERIAL_CTS : 0)
                    | ((s & 2) != 0 ? TENDRIL_SERIAL_DSR : 0)
                    | ((s & 4) != 0 ? TENDRIL_SERIAL_RI : 0)
                    | ((s & 8) != 0 ? TENDRIL_SERIAL_DCD : 0);
                serial_status_update(port, status);
            }
            /* END FreeBSD sys/dev/usb/serial/uchcom.c */
            break;
        }
        default: {
            break;
        }
    }
    if (!port->stopped && !port->suspended) {
        intr_submit(port);
    }
}

static void intr_submit(struct tendril_serial *port) {
    struct tendril_transfer *xfer = &port->intr;
    tendril_transfer_init(xfer, port->intr_ep);
    xfer->buffer = port->intr_buf;
    xfer->length = MIN(
        (size_t)port->intr_ep->max_packet, sizeof(port->intr_buf)
    );
    xfer->callback = intr_done;
    xfer->context = port;
    int st = tendril_transfer_submit(xfer);
    if (st == TENDRIL_ERR_NO_DEVICE) {
        port->stopped = true;
    } else if (st != TENDRIL_OK) {
        port->intr_idle = true;
        schedule_retry(port);
    }
}

static void timer_work(struct tendril_work *work) {
    struct tendril_serial *port = CONTAINER_OF(
        work, struct tendril_serial, timer.work
    );
    if (port->stopped || !port->attached || port->suspended) {
        return;
    }
    for (size_t i = 0; i < RX_BUFS; i++) {
        if ((port->rx_idle & ((uint32_t)1 << i)) != 0) {
            port->rx_idle &= ~((uint32_t)1 << i);
            rx_submit(port, i);
        }
    }
    if (port->intr_idle) {
        port->intr_idle = false;
        intr_submit(port);
    }
    /* BEGIN FreeBSD sys/dev/usb/serial/uslcom.c */
    if (port->kind == TENDRIL_SERIAL_CP210X && !port->poll_queued
     && !port->stopped) {
        struct serial_op *op = op_new(NULL, NULL);
        if (op != NULL) {
            serial_op_add(
                op, REQ_VENDOR_IFACE_IN, CP_GET_MDMSTS, 0, port->intf->number,
                NULL, 1
            );
            op->after = cp_status_after;
            port->poll_queued = true;
            op_queue(port, op);
        }
        hc_lock(port->hc);
        if (!tendril_timer_armed(&port->timer)) {
            tendril_timer_arm(
                port->hc, &port->timer, CP210X_POLL_MS * NS_PER_MS
            );
        }
        hc_unlock(port->hc);
    }
    /* END FreeBSD sys/dev/usb/serial/uslcom.c */
}

/* Writing */

static void tx_kick(struct tendril_serial *port);

static void tx_done(struct tendril_transfer *xfer) {
    struct tendril_serial *port = xfer->context;
    struct serial_write *w = port->writing;
    int st = xfer->status;

    port->writing = NULL;
    if (st == TENDRIL_ERR_STALL) {
        tendril_endpoint_clear_halt(port->out_ep, NULL, NULL);
    }
    if (w->done != NULL) {
        w->done(w->ctx, st);
    }
    tendril_free(w, sizeof(struct serial_write));
    tx_kick(port);
}

static void tx_kick(struct tendril_serial *port) {
    while (port->writing == NULL && !list_empty(&port->writes)
        && (!port->suspended || port->stopped)) {
        struct serial_write *w = CONTAINER_OF(
            port->writes.next, struct serial_write, node
        );
        list_del(&w->node);
        int st = port->stopped ? TENDRIL_ERR_NO_DEVICE : TENDRIL_OK;
        if (st == TENDRIL_OK) {
            struct tendril_transfer *xfer = &port->tx;
            port->writing = w;
            tendril_transfer_init(xfer, port->out_ep);
            xfer->buffer = (void *)(uintptr_t)w->data;
            xfer->length = w->len;
            xfer->callback = tx_done;
            xfer->context = port;
            st = tendril_transfer_submit(xfer);
            if (st == TENDRIL_OK) {
                return;
            }
            port->writing = NULL;
        }
        if (w->done != NULL) {
            w->done(w->ctx, st);
        }
        tendril_free(w, sizeof(struct serial_write));
    }
}

/* Setting up a port */

static struct tendril_driver serial_driver;

static void port_ready(void *ctx, int st) {
    struct tendril_serial *port = ctx;
    if (port->stopped || st == TENDRIL_ERR_NO_DEVICE) {
        return;
    }
    if (st != TENDRIL_OK) {
        LOG_WARN(
            "serial: setting up interface %u failed: %s", port->intf->number,
            tendril_status_str(st)
        );
    }
    port->attached = true;
    const char *kind = port->kind == TENDRIL_SERIAL_ACM ? "CDC ACM"
        : port->kind == TENDRIL_SERIAL_FTDI ? "FTDI"
        : port->kind == TENDRIL_SERIAL_PL2303 ? "PL2303"
        : port->kind == TENDRIL_SERIAL_CH34X ? "CH34x"
        : "CP210x";
    LOG_INFO("serial: interface %u ready (%s)", port->intf->number, kind);
    if (callbacks->attach != NULL) {
        callbacks->attach(port);
    }
    for (size_t i = 0; i < RX_BUFS && !port->stopped; i++) {
        rx_submit(port, i);
    }
    if (port->intr_ep != NULL && !port->stopped) {
        intr_submit(port);
    }
    if (port->kind == TENDRIL_SERIAL_CP210X && !port->stopped) {
        hc_lock(port->hc);
        tendril_timer_arm(port->hc, &port->timer, 0);
        hc_unlock(port->hc);
    }
    tx_kick(port);
}

/* BEGIN FreeBSD sys/dev/usb/serial/uchcom.c */
static void ch_version_after(
    struct tendril_serial *port, struct serial_op *op, size_t index, int st
) {
    if (index != 0) {
        return;
    }
    port->ch_version = st == TENDRIL_OK ? op->reqs[0].data[0] : 0;
    if (port->chip != CH_343) {
        /*
         * What FreeBSD's uchcom.c sets before the line settings: reset, then
         * the prescaler and padding registers.
         */
        serial_op_add(op, REQ_VENDOR_OUT, CH_RESET, 0, 0, NULL, 0);
        ch_write_reg(port, op, CH_REG_BPS_PRE, 0x82, CH_REG_BPS_DIV, 0xd9);
        ch_write_reg(port, op, 0x2c, 0x07, CH_REG_BPS_PAD, 0);
    }
    ch_line(port, op);
    ch_lines(port, op);
}
/* END FreeBSD sys/dev/usb/serial/uchcom.c */

static int port_setup(struct tendril_serial *port) {
    struct serial_op *op = op_new(port_ready, port);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    op->tolerant = true;
    switch (port->kind) {
        case TENDRIL_SERIAL_ACM: {
            /* BEGIN FreeBSD sys/dev/usb/serial/umodem.c */
            /*
             * Call management multiplexed over the data interface (CDC PSTN
             * 1.2, 6.3.1), as umodem.c asks for.
             */
            if ((port->quirks & SQ_ASSUME_CM_OVER_DATA) == 0
             && (port->cm_caps & CM_OVER_DATA) != 0
             && (port->acm_caps & ACM_COMM_FEATURE) != 0) {
                static const uint8_t state[2] = {
                    ACM_DATA_MULTIPLEXED & 0xff, ACM_DATA_MULTIPLEXED >> 8
                };
                serial_op_add(
                    op, REQ_CLASS_OUT, ACM_SET_COMM_FEATURE,
                    ACM_ABSTRACT_STATE, port->ctrl_intf->number, state,
                    sizeof(state)
                );
            }
            /* END FreeBSD sys/dev/usb/serial/umodem.c */
            acm_line(port, op);
            acm_lines(port, op);
            break;
        }
        case TENDRIL_SERIAL_FTDI: {
            serial_ftdi_setup(port, op);
            break;
        }
        case TENDRIL_SERIAL_PL2303: {
            serial_pl2303_setup(port, op);
            break;
        }
        case TENDRIL_SERIAL_CH34X: {
            /* BEGIN FreeBSD sys/dev/usb/serial/uchcom.c */
            serial_op_add(op, REQ_VENDOR_IN, CH_GET_VERSION, 0, 0, NULL, 8);
            op->after = ch_version_after;
            /* END FreeBSD sys/dev/usb/serial/uchcom.c */
            break;
        }
        default: {
            serial_op_add(
                op, REQ_VENDOR_IFACE_OUT, CP_IFC_ENABLE, 1, port->intf->number,
                NULL, 0
            );
            cp_line(port, op);
            cp_lines(port, op);
            break;
        }
    }
    return op_queue(port, op);
}

static const struct serial_id *lookup_id(struct tendril_device *dev) {
    uint16_t vendor = tendril_le16(dev->desc.vendor_id);
    uint16_t product = tendril_le16(dev->desc.product_id);
    for (size_t i = 0; i < ARRAY_SIZE(serial_ids); i++) {
        if (serial_ids[i].vendor == vendor
         && serial_ids[i].product == product) {
            return &serial_ids[i];
        }
    }
    return NULL;
}

static bool acm_match(
    struct tendril_device *dev, const struct tendril_interface_descriptor *id
) {
    uint16_t vendor = tendril_le16(dev->desc.vendor_id);
    uint16_t product = tendril_le16(dev->desc.product_id);
    for (size_t i = 0; i < ARRAY_SIZE(acm_ids); i++) {
        const struct acm_id *a = &acm_ids[i];
        if ((a->vendor == ANY || a->vendor == vendor)
         && (a->product == ANY || a->product == product)
         && (a->class == ANY || a->class == id->interface_class)
         && (a->subclass == ANY || a->subclass == id->interface_subclass)
         && (a->protocol == ANY || a->protocol == id->interface_protocol)) {
            return true;
        }
    }
    return false;
}

/*
 * The interface's first CDC functional descriptor of a subtype, if at least
 * `length` bytes long.
 */
static const uint8_t *cdc_descriptor(
    struct tendril_interface *intf, int subtype, size_t length
) {
    size_t size;
    const uint8_t *p = tendril_interface_descriptors(intf, &size);
    const uint8_t *end = p + size;
    for (const uint8_t *q = tendril_desc_next(p, end); q != NULL;
         q = tendril_desc_next(q, end)) {
        if (q[1] == TENDRIL_DT_CS_INTERFACE && q[0] >= 3 && q[2] == subtype) {
            return q[0] >= length ? q : NULL;
        }
    }
    return NULL;
}

/* BEGIN FreeBSD sys/dev/usb/serial/umodem.c */
/*
 * The data interface, as FreeBSD's umodem.c finds it: the one the Call
 * Management functional descriptor names, else the first subordinate of the
 * Union one (CDC PSTN 1.2, 5.3.1; CDC 1.2, 5.2.3.2), else a neighbour, the one
 * after winning unless only the one before is a CDC data interface.
 */
static struct tendril_interface *acm_data_interface(
    struct tendril_interface *intf, uint32_t quirks
) {
    struct tendril_device *dev = intf->dev;
    const uint8_t *cm = (quirks & SQ_IGNORE_CDC_CM) != 0 ? NULL
        : cdc_descriptor(intf, CDC_CALL_MANAGEMENT, 5);
    if (cm != NULL) {
        return tendril_device_find_interface(dev, cm[4]);
    }
    const uint8_t *un = cdc_descriptor(intf, CDC_UNION, 5);
    if (un != NULL) {
        return tendril_device_find_interface(dev, un[4]);
    }
    size_t index = (size_t)(intf - dev->interfaces);
    struct tendril_interface *found = NULL;
    bool found_data = false;
    for (size_t k = 0; k < 2; k++) {
        if ((k == 0 && index == 0)
         || (k == 1 && index + 1 >= dev->num_interfaces)) {
            continue;
        }
        struct tendril_interface *n
            = &dev->interfaces[k == 0 ? index - 1 : index + 1];
        const struct tendril_interface_descriptor *nd
            = tendril_interface_descriptor(n);
        bool data = nd->interface_class == TENDRIL_CLASS_CDC_DATA
            && nd->interface_subclass == 0;
        if (data || !found_data) {
            found = n;
            found_data = found_data || data;
        }
    }
    return found;
}
/* END FreeBSD sys/dev/usb/serial/umodem.c */

static bool data_endpoints(struct tendril_serial *port) {
    port->in_ep = tendril_interface_find_endpoint(
        port->intf, TENDRIL_ENDPOINT_XFER_BULK, TENDRIL_DIR_IN
    );
    port->out_ep = tendril_interface_find_endpoint(
        port->intf, TENDRIL_ENDPOINT_XFER_BULK, TENDRIL_DIR_OUT
    );
    return port->in_ep != NULL && port->out_ep != NULL;
}

static void port_free(struct tendril_serial *port) {
    for (size_t i = 0; i < RX_BUFS; i++) {
        tendril_free(port->rx_buf[i], port->rx_size);
    }
    tendril_free(port, sizeof(struct tendril_serial));
}

static void alt_done(void *ctx, int st) {
    struct tendril_serial *port = ctx;
    if (port->stopped || st != TENDRIL_OK || !data_endpoints(port)) {
        if (st != TENDRIL_ERR_NO_DEVICE && !port->stopped) {
            LOG_WARN(
                "serial: no data endpoints on interface %u", port->intf->number
            );
        }
        port->stopped = true;
        return;
    }
    port_setup(port);
}

static bool serial_probe(struct tendril_interface *intf) {
    const struct tendril_interface_descriptor *id
        = tendril_interface_descriptor(intf);
    struct tendril_device *dev = intf->dev;
    const struct serial_id *e = lookup_id(dev);
    struct tendril_interface *data = intf;
    uint32_t quirks = 0;
    int kind;

    if (e != NULL) {
        kind = e->kind;
        /* BEGIN FreeBSD sys/dev/usb/serial/uftdi.c */
        if (kind == TENDRIL_SERIAL_FTDI && intf->number < 8) {
            bool skip = e->skip != SKIP_BY_NAME
                ? (e->skip & ((uint32_t)1 << intf->number)) != 0
                : intf->number == 0 && dev->product != NULL
                    && tendril_strlen(dev->product) >= 7
                    && memcmp(dev->product, "XDS100V", 7) == 0;
            if (skip) {
                /* A JTAG interface, left for whoever drives JTAG. */
                return false;
            }
        }
        /* END FreeBSD sys/dev/usb/serial/uftdi.c */
    } else if (acm_match(dev, id)) {
        kind = TENDRIL_SERIAL_ACM;
        quirks = tendril_id_quirks(
            serial_quirks, ARRAY_SIZE(serial_quirks), &dev->desc
        );
        data = acm_data_interface(intf, quirks);
        if (data == NULL
         || (data != intf && !tendril_interface_claim(data, &serial_driver))) {
            return false;
        }
    } else {
        return false;
    }

    struct tendril_serial *port = tendril_zalloc(
        sizeof(struct tendril_serial)
    );
    if (port == NULL) {
        return false;
    }
    port->intf = data;
    port->ctrl_intf = intf;
    port->dev = dev;
    port->hc = dev->hc;
    port->kind = kind;
    port->quirks = quirks;
    port->config.baud = 115200;
    port->config.data_bits = 8;
    port->config.parity = TENDRIL_SERIAL_PARITY_NONE;
    port->config.stop_bits = TENDRIL_SERIAL_STOP_1;
    port->dtr = true;
    port->rts = true;
    list_init(&port->ops);
    list_init(&port->writes);
    tendril_timer_init(&port->timer, timer_work);

    switch (kind) {
        case TENDRIL_SERIAL_FTDI: {
            if (!serial_ftdi_init(port)) {
                tendril_free(port, sizeof(struct tendril_serial));
                return false;
            }
            break;
        }
        case TENDRIL_SERIAL_PL2303: {
            serial_pl2303_init(port);
            port->intr_ep = tendril_interface_find_endpoint(
                intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_IN
            );
            /* BEGIN FreeBSD sys/dev/usb/serial/uplcom.c */
            if (!data_endpoints(port) && dev->num_interfaces > 1) {
                /*
                 * Some carry the data on a second interface, as FreeBSD's
                 * uplcom.c (BSD-2-Clause) handles.
                 */
                struct tendril_interface *second = &dev->interfaces[1];
                if (second != intf
                 && tendril_interface_claim(second, &serial_driver)) {
                    port->intf = second;
                }
            }
            /* END FreeBSD sys/dev/usb/serial/uplcom.c */
            break;
        }
        case TENDRIL_SERIAL_CH34X: {
            /* BEGIN FreeBSD sys/dev/usb/serial/uchcom.c */
            /*
             * The CH343 and CH9102 put the data on a second interface;
             * FreeBSD's uchcom.c tells them apart by that.
             */
            port->chip = CH_341;
            port->intr_ep = tendril_interface_find_endpoint(
                intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_IN
            );
            if (dev->num_interfaces > 1 && intf == &dev->interfaces[0]) {
                struct tendril_interface *second = &dev->interfaces[1];
                if (tendril_interface_claim(second, &serial_driver)) {
                    port->intf = second;
                    port->chip = CH_343;
                }
            }
            /* END FreeBSD sys/dev/usb/serial/uchcom.c */
            break;
        }
        case TENDRIL_SERIAL_ACM: {
            /* BEGIN FreeBSD sys/dev/usb/serial/umodem.c */
            /*
             * With no Call Management descriptor, umodem.c takes call
             * management over the data interface as given.
             */
            const uint8_t *cm = cdc_descriptor(intf, CDC_CALL_MANAGEMENT, 5);
            const uint8_t *acm = cdc_descriptor(intf, CDC_ACM, 4);
            port->cm_caps = cm != NULL ? cm[3] : CM_HANDLES | CM_OVER_DATA;
            port->acm_caps = acm != NULL ? acm[3] : 0;
            /* END FreeBSD sys/dev/usb/serial/umodem.c */
            port->intr_ep = tendril_interface_find_endpoint(
                intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_IN
            );
            break;
        }
        default: {
            break;
        }
    }
    tendril_interface_set_data(intf, port);

    bool have = data_endpoints(port);
    size_t mps = have ? port->in_ep->max_packet : 64;
    port->rx_size = MAX(mps, (size_t)64) * 4;
    for (size_t i = 0; i < RX_BUFS; i++) {
        port->rx_buf[i] = tendril_zalloc(port->rx_size);
        if (port->rx_buf[i] == NULL) {
            tendril_interface_set_data(intf, NULL);
            port_free(port);
            return false;
        }
    }
    if (!have && kind == TENDRIL_SERIAL_ACM) {
        /*
         * CDC data interfaces may keep their endpoints in alternate setting 1.
         */
        struct tendril_interface *d = port->intf;
        for (size_t a = 0; a < d->num_alts; a++) {
            if (d->alts[a]->num_endpoints >= 2) {
                int st = tendril_interface_set_alt(
                    d, d->alts[a]->alternate_setting, alt_done, port
                );
                if (st != TENDRIL_OK) {
                    port->stopped = true;
                }
                return true;
            }
        }
    }
    if (!have) {
        LOG_WARN(
            "serial: no data endpoints on interface %u", port->intf->number
        );
        port->stopped = true;
        return true;
    }
    if (port_setup(port) != TENDRIL_OK) {
        port->stopped = true;
    }
    return true;
}

static bool serial_suspend(struct tendril_interface *intf) {
    struct tendril_serial *port = tendril_interface_data(intf);
    if (port == NULL || port->stopped) {
        return true;
    }
    /*
     * Still being set up, writing, or changing settings; the modem status poll
     * may finish on its own.
     */
    if (!port->attached || port->writing != NULL || !list_empty(&port->writes)
     || !list_empty(&port->ops) || (port->op != NULL && !port->poll_queued)) {
        return false;
    }
    port->suspended = true;
    hc_lock(port->hc);
    tendril_timer_cancel(port->hc, &port->timer);
    hc_unlock(port->hc);
    return true;
}

static void serial_resume(struct tendril_interface *intf) {
    struct tendril_serial *port = tendril_interface_data(intf);
    if (port == NULL || !port->suspended) {
        return;
    }
    port->suspended = false;
    if (port->stopped) {
        return;
    }
    port->rx_idle = 0;
    port->intr_idle = false;
    for (size_t i = 0; i < RX_BUFS && !port->stopped; i++) {
        rx_submit(port, i);
    }
    if (port->intr_ep != NULL && !port->stopped) {
        intr_submit(port);
    }
    if (port->kind == TENDRIL_SERIAL_CP210X && !port->stopped) {
        hc_lock(port->hc);
        tendril_timer_arm(port->hc, &port->timer, 0);
        hc_unlock(port->hc);
    }
    op_kick(port);
    tx_kick(port);
}

static void serial_disconnect(struct tendril_interface *intf) {
    struct tendril_serial *port = tendril_interface_data(intf);
    if (port == NULL) {
        return;
    }
    port->stopped = true;
    hc_lock(port->hc);
    tendril_timer_cancel(port->hc, &port->timer);
    hc_unlock(port->hc);
    while (!list_empty(&port->writes)) {
        struct serial_write *w = CONTAINER_OF(
            port->writes.next, struct serial_write, node
        );
        list_del(&w->node);
        if (w->done != NULL) {
            w->done(w->ctx, TENDRIL_ERR_NO_DEVICE);
        }
        tendril_free(w, sizeof(struct serial_write));
    }
    op_kick(port);
    if (port->attached && callbacks->detach != NULL) {
        callbacks->detach(port);
    }
    tendril_interface_set_data(intf, NULL);
    port_free(port);
}

static struct tendril_driver serial_driver = {
    .name = "serial",
    .probe = serial_probe,
    .disconnect = serial_disconnect,
    .suspend = serial_suspend,
    .resume = serial_resume
};

void tendril_serial_register(const struct tendril_serial_callbacks *cb) {
    callbacks = cb;
    tendril_driver_register(&serial_driver);
}

/* Operations */

static bool config_valid(const struct tendril_serial_config *c) {
    return c->baud != 0 && c->data_bits >= 5 && c->data_bits <= 8
        && c->parity <= TENDRIL_SERIAL_PARITY_SPACE
        && c->stop_bits <= TENDRIL_SERIAL_STOP_2;
}

int tendril_serial_configure(
    struct tendril_serial *port, const struct tendril_serial_config *config,
    void (*done)(void *ctx, int status), void *ctx
) {
    if (port->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (!config_valid(config)) {
        return TENDRIL_ERR_INVALID;
    }
    struct serial_op *op = op_new(done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    struct tendril_serial_config old = port->config;
    port->config = *config;
    build_line(port, op);
    if (port->kind == TENDRIL_SERIAL_CP210X && !config->rtscts) {
        /* Leaving handshake mode, RTS goes back to what was asked for. */
        cp_lines(port, op);
    }
    int st = op_queue(port, op);
    if (st != TENDRIL_OK) {
        port->config = old;
    }
    return st;
}

int tendril_serial_set_lines(
    struct tendril_serial *port, bool dtr, bool rts,
    void (*done)(void *ctx, int status), void *ctx
) {
    if (port->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    struct serial_op *op = op_new(done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    port->dtr = dtr;
    port->rts = rts;
    build_lines(port, op);
    return op_queue(port, op);
}

int tendril_serial_set_break(
    struct tendril_serial *port, bool on, void (*done)(void *ctx, int status),
    void *ctx
) {
    if (port->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    struct serial_op *op = op_new(done, ctx);
    if (op == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    port->in_break = on;
    build_break(port, op);
    return op_queue(port, op);
}

int tendril_serial_write(
    struct tendril_serial *port, const void *data, size_t len,
    void (*done)(void *ctx, int status), void *ctx
) {
    if (port->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (len == 0 || data == NULL) {
        return TENDRIL_ERR_INVALID;
    }
    struct serial_write *w = tendril_zalloc(sizeof(struct serial_write));
    if (w == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    w->data = data;
    w->len = len;
    w->done = done;
    w->ctx = ctx;
    list_add_tail(&port->writes, &w->node);
    if (port->attached) {
        tx_kick(port);
    }
    return TENDRIL_OK;
}

struct sync_wait {
    volatile bool done;
    int status;
};

static void sync_done(void *ctx, int st) {
    struct sync_wait *w = ctx;
    w->status = st;
    w->done = true;
}

int tendril_serial_configure_sync(
    struct tendril_serial *port, const struct tendril_serial_config *config
) {
    struct tendril_hc *hc = port->hc;
    struct sync_wait w = { false, TENDRIL_OK };
    int st = tendril_serial_configure(port, config, sync_done, &w);
    if (st != TENDRIL_OK) {
        return st;
    }
    tendril_hc_wait(hc, &w.done);
    return w.status;
}

int tendril_serial_write_sync(
    struct tendril_serial *port, const void *data, size_t len
) {
    struct tendril_hc *hc = port->hc;
    struct sync_wait w = { false, TENDRIL_OK };
    int st = tendril_serial_write(port, data, len, sync_done, &w);
    if (st != TENDRIL_OK) {
        return st;
    }
    tendril_hc_wait(hc, &w.done);
    return w.status;
}

/* Accessors */

struct tendril_device *tendril_serial_device(struct tendril_serial *port) {
    return port->dev;
}

int tendril_serial_kind(struct tendril_serial *port) {
    return port->kind;
}

uint8_t tendril_serial_interface(struct tendril_serial *port) {
    return port->intf->number;
}

uint32_t tendril_serial_status(struct tendril_serial *port) {
    return port->status;
}

void tendril_serial_set_user(struct tendril_serial *port, void *user) {
    port->user = user;
}

void *tendril_serial_user(struct tendril_serial *port) {
    return port->user;
}
