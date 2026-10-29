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
 * USB mass storage: Bulk-Only Transport 1.0, CBI Transport 1.1, and UAS as
 * UASP 1.0 lays it out, carrying SCSI-family commands. The quirks and their
 * handling, the guess at CBI status, and the transfer size limits follow
 * FreeBSD's sys/dev/usb/storage/umass.c (BSD-2-Clause). The UAS information
 * unit layouts and pipe usage IDs, which T10 does not publish, follow
 * SerenityOS's Kernel/Devices/Storage/USB/UAS/Structures.h (BSD-2-Clause).
 */

/*
 * Parts of this file are derived from FreeBSD's sys/dev/usb/storage/umass.c
 * (commit dcc5adb27e3b00f003c2256a9e9aa2fcc20c4d32):
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 1999 MAEKAWA Masahide <bishop@rr.iij4u.or.jp>,
 *               Nick Hibma <n_hibma@FreeBSD.org>
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
 */

/*
 * Parts of this file are derived from SerenityOS's
 * Kernel/Devices/Storage/USB/UAS/Structures.h (commit
 * 7af126f893a9241482a6f22405055343c2800338):
 *
 * Copyright (c) 2024, Leon Albrecht <leon.a@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
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
 * Parts of this file are derived from OpenBSD's sys/dev/usb/umass.c (commit
 * fba0a8da167d48e1d94a300389b91b0a48708155):
 *
 * Copyright (c) 2003 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Charles M. Hannum.
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
 *
 * Copyright (c) 1999 MAEKAWA Masahide <bishop@rr.iij4u.or.jp>,
 *               Nick Hibma <n_hibma@freebsd.org>
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
 *     $FreeBSD: src/sys/dev/usb/umass.c,v 1.13 2000/03/26 01:39:12 n_hibma Exp $
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tendril/msc.h>
#include "../internal.h"
#include <tendril/spec.h>
#include <tendril/tendril.h>

#define SUBCLASS_NOT_REPORTED 0x00
#define SUBCLASS_RBC 0x01
#define SUBCLASS_MMC 0x02
#define SUBCLASS_QIC157 0x03
#define SUBCLASS_UFI 0x04
#define SUBCLASS_SFF8070I 0x05
#define SUBCLASS_SCSI 0x06

#define PROTO_CBI 0x00
#define PROTO_CB 0x01
#define PROTO_BBB_OLD 0x02
#define PROTO_BBB 0x50
#define PROTO_UAS 0x62

#define REQ_ADSC 0x00
#define REQ_GET_MAX_LUN 0xfe
#define REQ_BBB_RESET 0xff

#define CBW_SIGNATURE 0x43425355
#define CSW_SIGNATURE 0x53425355
/* BEGIN FreeBSD sys/dev/usb/storage/umass.c */
/*
 * Signatures some devices put in their CSWs, accepted with
 * MQ_WRONG_CSW_SIGNATURE, as FreeBSD's umass.c does.
 */
#define CSW_SIGNATURE_OLYMPUS 0x55425355
#define CSW_SIGNATURE_AS_CBW 0x43425355
/* END FreeBSD sys/dev/usb/storage/umass.c */
#define CBW_SIZE 31
#define CSW_SIZE 13

#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_REQUEST_SENSE 0x03
#define SCSI_INQUIRY 0x12
#define SCSI_MODE_SENSE6 0x1a
#define SCSI_START_STOP_UNIT 0x1b
#define SCSI_READ_CAPACITY10 0x25
#define SCSI_READ10 0x28
#define SCSI_WRITE10 0x2a
#define SCSI_SYNC_CACHE10 0x35
#define SCSI_MODE_SENSE10 0x5a
#define SCSI_READ16 0x88
#define SCSI_WRITE16 0x8a
#define SCSI_SERVICE_ACTION_IN16 0x9e
#define SA_READ_CAPACITY16 0x10

#define STATUS_GOOD 0x00
#define STATUS_CHECK 0x02
#define STATUS_BUSY 0x08
#define STATUS_TASK_SET_FULL 0x28

#define SENSE_NO_SENSE 0x0
#define SENSE_RECOVERED 0x1
#define SENSE_NOT_READY 0x2
#define SENSE_ILLEGAL_REQUEST 0x5
#define SENSE_UNIT_ATTENTION 0x6
#define SENSE_DATA_PROTECT 0x7
#define SENSE_ABORTED 0xb

#define ASC_NOT_READY 0x04
#define ASCQ_START_REQUIRED 0x02
#define ASC_MEDIUM_CHANGED 0x28
#define ASC_NO_MEDIUM 0x3a

#define TYPE_DIRECT 0x00
#define TYPE_CDROM 0x05
#define TYPE_OPTICAL 0x07
#define TYPE_RBC 0x0e
#define TYPE_NONE 0x1f

/* BEGIN SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
/*
 * UAS information units and pipe usage IDs, as SerenityOS's Structures.h has
 * them.
 */
#define IU_COMMAND 0x01
#define IU_SENSE 0x03
#define IU_RESPONSE 0x04
#define IU_TASK_MGMT 0x05
#define IU_READ_READY 0x06
#define IU_WRITE_READY 0x07
#define TM_ABORT_TASK 0x01
#define RC_TM_COMPLETE 0x00
#define RC_TM_SUCCEEDED 0x08
#define DT_PIPE_USAGE 0x24
#define PIPE_COMMAND 1
#define PIPE_STATUS 2
#define PIPE_DATA_IN 3
#define PIPE_DATA_OUT 4
/* END SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
#define PIPE_BIT(id) ((uint32_t)1 << (id))
#define UAS_PIPES \
    (PIPE_BIT(PIPE_COMMAND) | PIPE_BIT(PIPE_STATUS) | PIPE_BIT(PIPE_DATA_IN) \
        | PIPE_BIT(PIPE_DATA_OUT))
/*
 * A status IU always travels as one packet, so the largest packet holds any of
 * them.
 */
#define IU_BUF_SIZE 512

#define CMD_TIMEOUT_MS 20000
#define LONG_TIMEOUT_MS 60000
#define CTRL_TIMEOUT_MS 5000
#define READY_TIMEOUT_MS 15000
#define READY_POLL_MS 100
#define MAX_RETRIES 4
#define PROBE_TRIES 8
#define MAX_LUNS 16
#define MAX_TAGS 32

/* BEGIN FreeBSD sys/dev/usb/quirk/usb_quirk.h, sys/dev/usb/storage/umass.c, sys/dev/usb/usb_msctest.c */
/*
 * FreeBSD's UQ_MSC_* quirks (sys/dev/usb/quirk/usb_quirk.h, BSD-2-Clause),
 * handled as its umass.c, and for MQ_SCSI_EJECT its usb_msctest.c, do.
 */
#define MQ_NO_GETMAXLUN ((uint32_t)1 << 0)
#define MQ_NO_TEST_UNIT_READY ((uint32_t)1 << 1)
#define MQ_NO_SYNC_CACHE ((uint32_t)1 << 2)
#define MQ_NO_INQUIRY ((uint32_t)1 << 3)
#define MQ_NO_START_STOP ((uint32_t)1 << 4)
#define MQ_IGNORE_RESIDUE ((uint32_t)1 << 5)
#define MQ_WRONG_CSW_SIGNATURE ((uint32_t)1 << 6)
#define MQ_READ_CAP_OFF_BY_ONE ((uint32_t)1 << 7)
#define MQ_PAD_TO_12 ((uint32_t)1 << 8)
#define MQ_SHUTTLE_INIT ((uint32_t)1 << 9)
#define MQ_ALT_IFACE_1 ((uint32_t)1 << 10)
#define MQ_WIRE_BBB ((uint32_t)1 << 11)
#define MQ_WIRE_CB ((uint32_t)1 << 12)
#define MQ_WIRE_CBI ((uint32_t)1 << 13)
#define MQ_CMD_SCSI ((uint32_t)1 << 14)
#define MQ_CMD_ATAPI ((uint32_t)1 << 15)
#define MQ_CMD_UFI ((uint32_t)1 << 16)
#define MQ_CMD_RBC ((uint32_t)1 << 17)
#define MQ_IGNORE ((uint32_t)1 << 18)
#define MQ_NO_RS_CLEAR_UA ((uint32_t)1 << 19)
#define MQ_SCSI_EJECT ((uint32_t)1 << 20)
/* END FreeBSD sys/dev/usb/quirk/usb_quirk.h, sys/dev/usb/storage/umass.c, sys/dev/usb/usb_msctest.c */

/* BEGIN FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/usbdevs */
/*
 * From FreeBSD's sys/dev/usb/quirk/usb_quirk.c (BSD-2-Clause), naming the IDs
 * as its sys/dev/usb/usbdevs (BSD-2-Clause) does.
 */
static const struct tendril_id_quirk msc_quirks[] = {
    /* ADDON ATTACHE */
    {
        0x0d7d, 0x1300, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* ADDON A256MB */
    {
        0x0d7d, 0x1400, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* ADDON DISKPRO512 */
    {
        0x0d7d, 0x1420, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* ADDONICS2 CABLE_205 */
    { 0x0bf6, 0xa001, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* AIPTEK POCKETCAM3M */
    { 0x08ca, 0x2011, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* ALCOR UMCR_9361 */
    {
        0x058f, 0x9361, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_GETMAXLUN
    },
    /* APACER HT202 */
    {
        0x1005, 0xb113, 0x0000, 0xffff,
        MQ_NO_TEST_UNIT_READY | MQ_NO_SYNC_CACHE
    },
    /* ASAHIOPTICAL OPTIO230 */
    {
        0x0a17, 0x0004, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* ASAHIOPTICAL OPTIO330 */
    {
        0x0a17, 0x0006, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    { 0x0a17, 0xffff, 0x0000, 0xffff, MQ_NO_RS_CLEAR_UA }, /* ASAHIOPTICAL */
    { 0x14af, 0xaf01, 0x0000, 0xffff, MQ_NO_SYNC_CACHE }, /* ATP EUSB */
    /* BELKIN USB2SCSI */
    { 0x050d, 0x0115, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* CASIO QV_DIGICAM */
    {
        0x07cf, 0x1001, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* CCYU ED1064 */
    { 0x1065, 0x2136, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* CENTURY EX35QUAT */
    {
        0x07f7, 0x011e, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
    },
    /* CREATIVE NOMAD */
    {
        0x041e, 0x4106, 0x0001, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_READ_CAP_OFF_BY_ONE
    },
    /* CREATIVE STAGE_SE_MINI */
    { 0x041e, 0x3295, 0x0000, 0xffff, MQ_NO_INQUIRY },
    /* CYPRESS XX6830XX */
    { 0x04b4, 0x6830, 0x0000, 0xffff, MQ_NO_GETMAXLUN | MQ_NO_SYNC_CACHE },
    /* EMTEC DANEELEC4GB */
    { 0x13fe, 0x1e20, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* DESKNOTE UCR_61S2B */
    { 0x1019, 0x0c55, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* DMI CFSM_RW */
    { 0x0c0b, 0xa109, 0x0000, 0xffff, MQ_CMD_SCSI | MQ_NO_GETMAXLUN },
    { 0x13fe, 0x2240, 0x0000, 0xffff, MQ_NO_SYNC_CACHE }, /* EMTEC RUF2PS */
    /* EPSON STYLUS_875DC */
    {
        0x04b8, 0x0601, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* EPSON STYLUS_895 */
    {
        0x04b8, 0x0602, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_GETMAXLUN
    },
    /* FEIYA 5IN1 */
    { 0x090c, 0x1132, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    { 0x090c, 0x6200, 0x0000, 0xffff, MQ_NO_SYNC_CACHE }, /* FEIYA ELANGO */
    { 0x07ab, 0xfc01, 0x0000, 0xffff, MQ_CMD_SCSI }, /* FREECOM DVD */
    /* FUJIPHOTO MASS0100 */
    {
        0x04cb, 0x0100, 0x0000, 0xffff,
        MQ_WIRE_CBI | MQ_CMD_ATAPI | MQ_NO_RS_CLEAR_UA | MQ_NO_SYNC_CACHE
    },
    { 0x091e, 0x23c0, 0x0000, 0xffff, MQ_NO_INQUIRY }, /* GARMIN DAKOTA20 */
    /* GARMIN FORERUNNER230 */
    { 0x091e, 0x086d, 0x0000, 0xffff, MQ_NO_INQUIRY },
    { 0x091e, 0x2459, 0x0000, 0xffff, MQ_NO_INQUIRY }, /* GARMIN GPSMAP62S */
    /* GARMIN EDGETOURINGPLUS */
    { 0x091e, 0x26c8, 0x0000, 0xffff, MQ_NO_INQUIRY },
    /* GARMIN INSTINCTSOLAR */
    { 0x091e, 0x0d8a, 0x0000, 0xffff, MQ_NO_INQUIRY },
    /* GENESYS GL641USB2IDE */
    {
        0x05e3, 0x0702, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
            | MQ_NO_SYNC_CACHE
    },
    /* GENESYS GL641USB2IDE_2 */
    {
        0x05e3, 0x0701, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_ATAPI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
    },
    /* GENESYS GL641USB */
    {
        0x05e3, 0x0700, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
    },
    /* GENESYS GL641USB_2 */
    {
        0x05e3, 0x0760, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_WRONG_CSW_SIGNATURE
    },
    /* GENESYS GL3220 */
    { 0x05e3, 0x0732, 0x0000, 0xffff, MQ_NO_INQUIRY | MQ_NO_RS_CLEAR_UA },
    /* HAGIWARA FG */
    { 0x0693, 0x0005, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* HAGIWARA FGSM */
    { 0x0693, 0x0002, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* HITACHI DVDCAM_DZ_MV100A */
    { 0x04a4, 0x0004, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_SCSI },
    /* HITACHI DVDCAM_USB */
    {
        0x04a4, 0x001e, 0x0000, 0xffff,
        MQ_WIRE_CBI | MQ_CMD_ATAPI | MQ_NO_INQUIRY
    },
    { 0x03f0, 0x0307, 0x0000, 0xffff, MQ_CMD_ATAPI }, /* HP CDW4E */
    /* HP CDW8200 */
    {
        0x03f0, 0x0207, 0x0000, 0xffff,
        MQ_WIRE_CBI | MQ_CMD_ATAPI | MQ_NO_TEST_UNIT_READY | MQ_NO_START_STOP
    },
    /* HUAWEI E3372_INIT */
    { 0x12d1, 0x1f01, 0x0000, 0xffff, MQ_NO_INQUIRY | MQ_NO_GETMAXLUN },
    /* IMAGINATION DBX1 */
    {
        0x149a, 0x2107, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_WRONG_CSW_SIGNATURE
    },
    /* INSYSTEM USBCABLE */
    {
        0x05ab, 0x081a, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_ATAPI | MQ_NO_TEST_UNIT_READY | MQ_NO_START_STOP
            | MQ_ALT_IFACE_1
    },
    /* INSYSTEM ATAPI */
    { 0x05ab, 0x0031, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_RBC },
    /* INSYSTEM STORAGE_V2 */
    { 0x05ab, 0x5701, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_RBC },
    /* IODATA IU_CD2 */
    { 0x04bb, 0x0204, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* IODATA DVR_UEH8 */
    { 0x04bb, 0x0206, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* IOMEGA ZIP100 */
    {
        0x059b, 0x0001, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_TEST_UNIT_READY
    },
    { 0x152d, 0x3569, 0x0000, 0xffff, MQ_NO_GETMAXLUN }, /* JMICRON JMS566 */
    { 0x152d, 0x0567, 0x0000, 0xffff, MQ_NO_GETMAXLUN }, /* JMICRON JMS567 */
    /* JMICRON JM20337 */
    {
        0x152d, 0x2338, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_SYNC_CACHE
    },
    { 0x0951, 0x162b, 0x0000, 0xffff, MQ_NO_INQUIRY }, /* KINGSTON HYPERX3_0 */
    /* KINGSTON DATATRAVELER3_0 */
    { 0x0951, 0x1666, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* KYOCERA FINECAM_L3 */
    {
        0x0482, 0x0105, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* KYOCERA FINECAM_S3X */
    {
        0x0482, 0x0100, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_ATAPI | MQ_NO_INQUIRY
    },
    /* KYOCERA FINECAM_S4 */
    {
        0x0482, 0x0101, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_ATAPI | MQ_NO_INQUIRY
    },
    /* KYOCERA FINECAM_S5 */
    {
        0x0482, 0x0103, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    { 0x059f, 0xa601, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_RBC }, /* LACIE HD */
    /* LEXAR CF_READER */
    {
        0x05dc, 0xb002, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    { 0x05dc, 0x0001, 0x0000, 0xffff, MQ_CMD_SCSI }, /* LEXAR JUMPSHOT */
    { 0x05dc, 0xa833, 0x0000, 0xffff, MQ_NO_INQUIRY }, /* LEXAR JUMPDRIVE */
    { 0x0789, 0x0033, 0x0000, 0xffff, MQ_CMD_SCSI }, /* LOGITEC LDR_H443SU2 */
    /* LOGITEC LDR_H443U2 */
    { 0x0789, 0x00b3, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* MEIZU M6_SL */
    {
        0x0492, 0x0140, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY | MQ_NO_SYNC_CACHE
    },
    /* MELCO DUBPXXG */
    {
        0x0411, 0x001c, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
    },
    /* MICROTECH DPCM */
    {
        0x07af, 0x0006, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_SCSI | MQ_NO_TEST_UNIT_READY | MQ_NO_START_STOP
    },
    { 0x0634, 0x0655, 0x0000, 0xffff, MQ_NO_SYNC_CACHE }, /* MICRON REALSSD */
    /* MICROTECH SCSIDB25 */
    { 0x07af, 0x0004, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* MICROTECH SCSIHD50 */
    { 0x07af, 0x0005, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    { 0x0686, 0x4017, 0x0000, 0xffff, MQ_CMD_SCSI }, /* MINOLTA E223 */
    /* MINOLTA F300 */
    { 0x0686, 0x4011, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* MITSUMI CDRRW */
    { 0x03ee, 0x0000, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_ATAPI },
    /* MOTOROLA2 E398 */
    {
        0x22b8, 0x4810, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_GETMAXLUN
    },
    { 0x1cae, 0xffff, 0x0000, 0xffff, MQ_NO_SYNC_CACHE }, /* MPMAN */
    /* MSYSTEMS DISKONKEY */
    {
        0x08ec, 0x0010, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE | MQ_NO_GETMAXLUN
            | MQ_NO_RS_CLEAR_UA
    },
    /* MSYSTEMS DISKONKEY2 */
    { 0x08ec, 0x0011, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_ATAPI },
    /* MYSON HEDEN */
    { 0x04cf, 0x8818, 0x0000, 0xffff, MQ_IGNORE_RESIDUE | MQ_NO_SYNC_CACHE },
    /* NEODIO ND3260 */
    { 0x0aec, 0x3260, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* NETAC CF_CARD */
    {
        0x0dd8, 0x1060, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* NETAC ONLYDISK */
    {
        0x0dd8, 0x0003, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* NETCHIP CLIK_40 */
    { 0x0525, 0xa140, 0x0000, 0xffff, MQ_CMD_ATAPI | MQ_NO_INQUIRY },
    /* NETCHIP POCKETBOOK */
    { 0x0525, 0xa4a5, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* NIKON D300 */
    { 0x04b0, 0x041a, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* NORELSYS NS1081 */
    { 0x2537, 0x1081, 0x0000, 0xffff, MQ_NO_RS_CLEAR_UA | MQ_NO_INQUIRY },
    /* OLYMPUS C1 */
    {
        0x07b4, 0x0102, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_WRONG_CSW_SIGNATURE
    },
    { 0x07b4, 0x0105, 0x0000, 0xffff, MQ_NO_GETMAXLUN }, /* OLYMPUS C700 */
    /* OLYMPUS E_1 */
    {
        0x07b4, 0x0102, 0x0000, 0xffff,
        MQ_NO_GETMAXLUN | MQ_NO_TEST_UNIT_READY | MQ_NO_SYNC_CACHE
    },
    /* OLYMPUS E_330 */
    {
        0x07b4, 0x0118, 0x0000, 0xffff,
        MQ_NO_GETMAXLUN | MQ_NO_TEST_UNIT_READY | MQ_NO_SYNC_CACHE
            | MQ_NO_START_STOP
    },
    /* OLYMPUS E_PM1 */
    {
        0x07b4, 0x012c, 0x0000, 0xffff,
        MQ_NO_GETMAXLUN | MQ_NO_TEST_UNIT_READY | MQ_NO_SYNC_CACHE
            | MQ_NO_START_STOP
    },
    /* OLYMPUS E_M1 */
    {
        0x07b4, 0x012e, 0x0000, 0xffff,
        MQ_NO_GETMAXLUN | MQ_NO_TEST_UNIT_READY | MQ_NO_SYNC_CACHE
            | MQ_NO_START_STOP
    },
    /* ONSPEC SDS_HOTFIND_D */
    {
        0x07c4, 0x0400, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_GETMAXLUN | MQ_NO_SYNC_CACHE
    },
    { 0x07c4, 0xa001, 0x0000, 0xffff, MQ_CMD_SCSI }, /* ONSPEC CFMS_RW */
    { 0x07c4, 0xa109, 0x0000, 0xffff, MQ_CMD_SCSI }, /* ONSPEC CFSM_COMBO */
    { 0x07c4, 0xa005, 0x0000, 0xffff, MQ_CMD_SCSI }, /* ONSPEC CFSM_READER */
    { 0x07c4, 0xa006, 0x0000, 0xffff, MQ_CMD_SCSI }, /* ONSPEC CFSM_READER2 */
    /* ONSPEC MDCFE_B_CF_READER */
    { 0x07c4, 0xa000, 0x0000, 0xffff, MQ_CMD_SCSI },
    /* ONSPEC MDSM_B_READER */
    { 0x07c4, 0xa103, 0x0000, 0xffff, MQ_CMD_SCSI | MQ_NO_INQUIRY },
    { 0x07c4, 0xa003, 0x0000, 0xffff, MQ_CMD_SCSI }, /* ONSPEC READER */
    /* ONSPEC UCF100 */
    {
        0x07c4, 0xa400, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_ATAPI | MQ_NO_INQUIRY | MQ_NO_GETMAXLUN
    },
    /* ONSPEC2 IMAGEMATE_SDDR55 */
    { 0x55aa, 0xa103, 0x0000, 0xffff, MQ_CMD_SCSI | MQ_NO_GETMAXLUN },
    /* PANASONIC KXL840AN */
    {
        0x04da, 0x0d01, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_ATAPI | MQ_NO_GETMAXLUN
    },
    /* PANASONIC KXLCB20AN */
    { 0x04da, 0x0d0a, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* PANASONIC KXLCB35AN */
    { 0x04da, 0x0d0e, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    { 0x04da, 0x0901, 0x0000, 0xffff, MQ_CMD_UFI }, /* PANASONIC LS120CAM */
    /* PLEXTOR 40_12_40U */
    {
        0x093b, 0x0011, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_TEST_UNIT_READY
    },
    /* PNY ATTACHE2 */
    {
        0x154b, 0x0010, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE | MQ_NO_START_STOP
    },
    { 0x067b, 0x2506, 0x0000, 0xffff, MQ_NO_SYNC_CACHE }, /* PROLIFIC PL2506 */
    /* QUALCOMMINC ZTE_MF730M */
    { 0x19d2, 0x1420, 0x0000, 0xffff, MQ_NO_GETMAXLUN | MQ_NO_INQUIRY },
    { 0x148f, 0x2878, 0x0001, 0x0001, MQ_IGNORE }, /* RALINK RT_STOR */
    /* REALTEK RTW8821CU_CD */
    { 0x0bda, 0x1a2b, 0x0000, 0xffff, MQ_SCSI_EJECT },
    /* SAMSUNG_TECHWIN DIGIMAX_410 */
    {
        0x0839, 0x000a, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* SANDISK SDDR05A */
    {
        0x0781, 0x0001, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_SCSI | MQ_READ_CAP_OFF_BY_ONE
    },
    /* SANDISK SDDR09 */
    {
        0x0781, 0x0200, 0x0000, 0xffff,
        MQ_CMD_SCSI | MQ_READ_CAP_OFF_BY_ONE | MQ_NO_GETMAXLUN
    },
    /* SANDISK SDDR12 */
    {
        0x0781, 0x0100, 0x0000, 0xffff,
        MQ_WIRE_CB | MQ_CMD_SCSI | MQ_READ_CAP_OFF_BY_ONE
    },
    /* SANDISK SDCZ2_128 */
    {
        0x0781, 0x7100, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE | MQ_NO_SYNC_CACHE
    },
    /* SANDISK SDCZ2_256 */
    {
        0x0781, 0x7104, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* SANDISK SDCZ4_128 */
    {
        0x0781, 0x7112, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* SANDISK SDCZ4_256 */
    {
        0x0781, 0x7113, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* SANDISK SDCZ48_32 */
    {
        0x0781, 0x5581, 0x0000, 0xffff,
        MQ_NO_SYNC_CACHE | MQ_NO_TEST_UNIT_READY
    },
    /* SANDISK SDDR31 */
    {
        0x0781, 0x0002, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_READ_CAP_OFF_BY_ONE
    },
    /* SANDISK IMAGEMATE_SDDR289 */
    { 0x0781, 0xb6ba, 0x0000, 0xffff, MQ_NO_SYNC_CACHE | MQ_NO_GETMAXLUN },
    /* SCANLOGIC SL11R */
    {
        0x04ce, 0x0002, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_ATAPI | MQ_NO_INQUIRY
    },
    /* SHUTTLE EUSB */
    {
        0x04e6, 0x0001, 0x0000, 0xffff,
        MQ_WIRE_CBI | MQ_CMD_ATAPI | MQ_NO_TEST_UNIT_READY | MQ_NO_START_STOP
            | MQ_SHUTTLE_INIT
    },
    /* SHUTTLE CDRW */
    { 0x04e6, 0x0101, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_ATAPI },
    /* SHUTTLE CF */
    { 0x04e6, 0x000a, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_ATAPI },
    /* SHUTTLE EUSBATAPI */
    { 0x04e6, 0x0009, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_ATAPI },
    { 0x04e6, 0x0005, 0x0000, 0xffff, MQ_CMD_SCSI }, /* SHUTTLE EUSBCFSM */
    /* SHUTTLE EUSCSI */
    { 0x04e6, 0x0002, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* SHUTTLE HIFD */
    { 0x04e6, 0x0007, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_SCSI },
    /* SHUTTLE SDDR09 */
    { 0x04e6, 0x0003, 0x0000, 0xffff, MQ_CMD_SCSI | MQ_NO_GETMAXLUN },
    /* SHUTTLE ZIOMMC */
    { 0x04e6, 0x0006, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_SCSI },
    /* SIGMATEL I_BEAD100 */
    {
        0x066f, 0x8008, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_SHUTTLE_INIT
    },
    /* SIIG WINTERREADER */
    {
        0x07cc, 0x0330, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_IGNORE_RESIDUE
    },
    /* SKANHEX MD_7425 */
    {
        0x0d96, 0x410a, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* SKANHEX SX_520Z */
    {
        0x0d96, 0x5200, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    { 0x0e39, 0x1700, 0x0000, 0xffff, MQ_NO_INQUIRY }, /* SMART2 G2MEMKEY */
    /* SONY HANDYCAM */
    { 0x054c, 0x002e, 0x0500, 0x0500, MQ_WIRE_CB | MQ_CMD_RBC | MQ_PAD_TO_12 },
    /* SONY CLIE_40_MS */
    {
        0x054c, 0x006d, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* SONY DSC */
    { 0x054c, 0x0010, 0x0500, 0x0500, MQ_WIRE_CB | MQ_CMD_RBC | MQ_PAD_TO_12 },
    /* SONY DSC */
    { 0x054c, 0x0010, 0x0600, 0x0600, MQ_WIRE_CB | MQ_CMD_RBC | MQ_PAD_TO_12 },
    { 0x054c, 0x0010, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_RBC }, /* SONY DSC */
    /* SONY HANDYCAM */
    { 0x054c, 0x002e, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_RBC },
    { 0x054c, 0x0032, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_RBC }, /* SONY MSC */
    /* SONY MS_MSC_U03 */
    { 0x054c, 0x0069, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_UFI },
    /* SONY MS_NW_MS7 */
    {
        0x054c, 0x0025, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_GETMAXLUN
    },
    /* SONY MS_PEG_N760C */
    {
        0x054c, 0x0058, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* SONY MSACUS1 */
    {
        0x054c, 0x002d, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_GETMAXLUN
    },
    /* SONY PORTABLE_HDD_V2 */
    { 0x054c, 0x002b, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* SUPERTOP IDE */
    { 0x14cd, 0x6600, 0x0000, 0xffff, MQ_IGNORE_RESIDUE | MQ_NO_SYNC_CACHE },
    /* SUPERTOP FLASHDRIVE */
    {
        0x14cd, 0x121c, 0x0000, 0xffff,
        MQ_NO_TEST_UNIT_READY | MQ_NO_SYNC_CACHE
    },
    { 0x0436, 0x0005, 0x0000, 0xffff, MQ_CMD_SCSI }, /* TAUGA CAMERAMATE */
    /* TEAC FD05PUB */
    { 0x0644, 0x0000, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_UFI },
    /* TECLAST TLC300 */
    {
        0x071b, 0x3203, 0x0000, 0xffff,
        MQ_NO_TEST_UNIT_READY | MQ_NO_SYNC_CACHE
    },
    /* TOSHIBA TRANSMEMORY */
    { 0x0930, 0x6545, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* TREK MEMKEY */
    {
        0x0a16, 0x8888, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* TREK THUMBDRIVE_8MB */
    {
        0x0a16, 0x9988, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_ATAPI | MQ_IGNORE_RESIDUE
    },
    /* TRUMPION C3310 */
    { 0x090a, 0x1100, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_UFI },
    { 0x090a, 0x1200, 0x0000, 0xffff, MQ_CMD_RBC }, /* TRUMPION MP3 */
    { 0x090a, 0x1001, 0x0000, 0xffff, MQ_CMD_SCSI }, /* TRUMPION T33520 */
    /* TWINMOS MDIV */
    { 0x126f, 0x1325, 0x0000, 0xffff, MQ_WIRE_BBB | MQ_CMD_SCSI },
    /* VERBATIM STORENGO */
    { 0x18a5, 0x0243, 0x0000, 0xffff, MQ_IGNORE_RESIDUE },
    /* VIA USB2IDEBRIDGE */
    {
        0x040d, 0x6204, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_SYNC_CACHE
    },
    /* VIALABS USB30SATABRIDGE */
    { 0x2109, 0x0700, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    { 0x2109, 0x0701, 0x0000, 0xffff, MQ_NO_INQUIRY }, /* VIALABS VL701 */
    /* VIVITAR 35XX */
    {
        0x0636, 0x0003, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_INQUIRY
    },
    /* WESTERN COMBO */
    {
        0x1058, 0x0200, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
    },
    /* WESTERN EXTHDD */
    {
        0x1058, 0x0400, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
    },
    /* WESTERN MYBOOK */
    {
        0x1058, 0x0901, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_SYNC_CACHE
    },
    /* WESTERN MYPASSPORT_01 */
    { 0x1058, 0x0746, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_02 */
    { 0x1058, 0x0748, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_03 */
    { 0x1058, 0x074a, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_04 */
    { 0x1058, 0x074c, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_05 */
    { 0x1058, 0x074e, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_06 */
    { 0x1058, 0x07a6, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_07 */
    { 0x1058, 0x07a8, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_08 */
    { 0x1058, 0x07aa, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_09 */
    { 0x1058, 0x07ac, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_10 */
    { 0x1058, 0x07ae, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORT_11 */
    { 0x1058, 0x0741, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_00 */
    { 0x1058, 0x070a, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_01 */
    { 0x1058, 0x071a, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_02 */
    { 0x1058, 0x0730, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_03 */
    { 0x1058, 0x0732, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_04 */
    { 0x1058, 0x0740, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_05 */
    { 0x1058, 0x0742, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_06 */
    { 0x1058, 0x0750, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_07 */
    { 0x1058, 0x0752, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_08 */
    { 0x1058, 0x07a0, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTES_09 */
    { 0x1058, 0x07a2, 0x0000, 0xffff, MQ_NO_SYNC_CACHE },
    /* WESTERN MYPASSPORTUL_00 */
    { 0x1058, 0x0743, 0x0000, 0xffff, MQ_NO_TEST_UNIT_READY },
    /* WINMAXGROUP FLASH64MC */
    {
        0x0ed1, 0x6660, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_NO_INQUIRY | MQ_CMD_SCSI
    },
    /* YANO FW800HD */
    {
        0x094f, 0x05fc, 0x0000, 0xffff,
        MQ_WIRE_BBB | MQ_CMD_SCSI | MQ_NO_START_STOP | MQ_IGNORE_RESIDUE
    },
    /* YANO U640MO */
    { 0x094f, 0x0101, 0x0000, 0xffff, MQ_WIRE_CBI | MQ_CMD_ATAPI },
    /* YEDATA FLASHBUSTERU */
    {
        0x057b, 0x0000, 0x0000, 0x007f,
        MQ_WIRE_CB | MQ_CMD_UFI | MQ_NO_RS_CLEAR_UA | MQ_NO_TEST_UNIT_READY
    },
    /* YEDATA FLASHBUSTERU */
    {
        0x057b, 0x0000, 0x0080, 0x0080,
        MQ_WIRE_CBI | MQ_CMD_UFI | MQ_NO_RS_CLEAR_UA | MQ_NO_TEST_UNIT_READY
    },
    /* YEDATA FLASHBUSTERU */
    {
        0x057b, 0x0000, 0x0081, 0xffff,
        MQ_WIRE_CBI | MQ_CMD_UFI | MQ_NO_RS_CLEAR_UA
    },
    /* ZORAN EX20DSC */
    { 0x0595, 0x4343, 0x0000, 0xffff, MQ_WIRE_CB | MQ_CMD_ATAPI },
};
/* END FreeBSD sys/dev/usb/quirk/usb_quirk.c, sys/dev/usb/usbdevs */

#define TRANSPORT_BBB 0
#define TRANSPORT_CBI 1
#define TRANSPORT_CB 2
#define TRANSPORT_UAS 3

#define CMDSET_SCSI 0
#define CMDSET_RBC 1
#define CMDSET_ATAPI 2
#define CMDSET_UFI 3

struct tendril_msc;

struct msc_cmd {
    struct tendril_list node;
    struct tendril_msc *msc;
    uint8_t cdb[16];
    size_t cdb_len;
    uint8_t lun;
    bool in;
    void *data;
    size_t len;
    uint32_t timeout_ms;
    void (*done)(struct msc_cmd *cmd);
    void *ctx;
    /*
     * The transport status; the SCSI status and sense data count only when it
     * is TENDRIL_OK.
     */
    int status;
    int scsi_status;
    size_t actual;
    uint8_t sense[32];
    size_t sense_len;
    /* UAS */
    uint16_t tag;
    size_t pending;
    bool have_status;
    bool data_posted;
    int failed;
    struct tendril_transfer iu_xfer;
    struct tendril_transfer status_xfer;
    struct tendril_transfer data_xfer;
    uint8_t iu[32];
    uint8_t status_buf[IU_BUF_SIZE];
};

#define LUN_INQUIRY 0
#define LUN_TEST 1
#define LUN_START 2
#define LUN_CAPACITY10 3
#define LUN_CAPACITY16 4
#define LUN_MODE_SENSE 5
#define LUN_IO 6
#define LUN_IO_START 7
#define LUN_FLUSH 8

struct tendril_msc_lun {
    struct tendril_msc *msc;
    uint8_t number;
    int type;
    bool removable;
    bool present;
    bool write_protected;
    bool attached;
    bool no_sync_cache;
    bool no_mode_sense;
    uint64_t blocks;
    size_t block_size;
    char vendor[9];
    char product[17];
    char revision[5];
    void *user;
    /*
     * One command at a time: a probe of the medium, or a chunk of the current
     * request.
     */
    struct msc_cmd cmd;
    uint8_t buf[192];
    int step;
    size_t tries;
    bool busy;
    bool started;
    bool probe_pending;
    bool changed;
    bool old_present;
    bool old_write_protected;
    uint64_t old_blocks;
    size_t old_block_size;
    uint64_t ready_deadline;
    struct tendril_timer timer;
    struct tendril_list requests;
    struct tendril_msc_request *current;
    void (*check_done)(void *ctx, int status);
    void *check_ctx;
};

#define SETUP_ALT 0
#define SETUP_STREAMS 1
#define SETUP_FALLBACK 2
#define SETUP_SHUTTLE 3
#define SETUP_MAX_LUN 4

/* BEGIN FreeBSD sys/dev/usb/usb_msctest.c */
/*
 * The checks FreeBSD's sys/dev/usb/usb_msctest.c (BSD-2-Clause) makes of a
 * device's first interface before any driver uses it: for an auto-install
 * CD-ROM (usb_iface_is_cdrom), and the MQ_SCSI_EJECT one (usb_msc_eject with
 * MSC_EJECT_STOPUNIT). Their data and status stages get the two seconds that
 * its bbb_command_start() gives the data stage of these commands.
 */
#define PRE_NONE 0
#define PRE_CDROM 1
#define PRE_EJECT_TEST 2
#define PRE_EJECT_STOP 3
#define PRE_TIMEOUT_MS 2000
#define PRE_TRIES 3
#define PRE_RETRY_MS 1000
/* END FreeBSD sys/dev/usb/usb_msctest.c */

#define T_IDLE 0
#define T_CBW 1
#define T_DATA 2
#define T_DATA_CLEAR 3
#define T_CSW 4
#define T_CSW_CLEAR 5
#define T_ADSC 6
#define T_INTR 7
#define T_INTR_CLEAR 8
#define T_FAIL_CLEAR 9
#define T_RESET 10
#define T_RESET_CLEAR_IN 11
#define T_RESET_CLEAR_OUT 12

struct tendril_msc {
    struct tendril_interface *intf;
    struct tendril_device *dev;
    struct tendril_hc *hc;
    int transport;
    int cmdset;
    uint32_t quirks;
    bool pad12;
    bool stopped;
    bool broken;
    bool suspended;
    size_t max_bytes;
    size_t alt;
    /*
     * The Bulk-Only or CBI setting to fall back to when UAS cannot be set up,
     * or -1.
     */
    int fallback_alt;
    int fallback_transport;
    int fallback_cmdset;
    int setup;
    struct tendril_async async;
    uint8_t ctrl_buf[16];
    struct tendril_transfer ctrl;
    size_t num_luns;
    struct tendril_msc_lun *luns[MAX_LUNS];
    struct tendril_list queue;
    /*
     * Bulk-Only and CBI: one command at a time, with automatic REQUEST SENSE.
     */
    struct tendril_endpoint *bulk_in;
    struct tendril_endpoint *bulk_out;
    struct tendril_endpoint *intr_in;
    struct msc_cmd *active;
    struct msc_cmd *sensing;
    int xstep;
    uint32_t tag;
    bool csw_retried;
    int recover_status;
    uint8_t cbw[CBW_SIZE];
    uint8_t csw[CSW_SIZE];
    uint8_t intr_buf[2];
    struct tendril_transfer xfer;
    struct msc_cmd sense_cmd;
    /* UAS */
    struct tendril_endpoint *cmd_ep;
    struct tendril_endpoint *status_ep;
    struct tendril_endpoint *data_in_ep;
    struct tendril_endpoint *data_out_ep;
    bool streams;
    size_t num_streams;
    size_t num_tags;
    size_t next_tag;
    /*
     * Tags in use, including those of failed commands that wait for an ABORT
     * TASK.
     */
    uint64_t tag_busy;
    uint64_t abort_mask;
    uint8_t abort_lun[MAX_TAGS + 1];
    size_t in_flight;
    uint16_t tm_tag;
    uint16_t tm_target;
    bool tm_busy;
    bool tm_ok;
    size_t tm_pending;
    struct tendril_transfer tm_xfer;
    struct tendril_transfer tm_status;
    uint8_t tm_iu[16];
    uint8_t tm_buf[IU_BUF_SIZE];
    /* The usb_msctest.c checks: the one running, and those still to come. */
    int pre;
    bool pre_cdrom;
    bool pre_eject;
    size_t pre_tries;
    struct msc_cmd pre_cmd;
    uint8_t pre_buf[36];
    struct tendril_timer pre_timer;
};

static const struct tendril_msc_callbacks *callbacks;

static void put_be16(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}

static void put_be32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)v;
}

static void put_be64(uint8_t *p, uint64_t v) {
    put_be32(p, (uint32_t)(v >> 32));
    put_be32(p + 4, (uint32_t)v);
}

static uint16_t get_be16(const uint8_t *p) {
    return (uint16_t)(p[0] << 8 | p[1]);
}

static uint32_t get_be32(const uint8_t *p) {
    return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8
        | p[3];
}

static uint64_t get_be64(const uint8_t *p) {
    return (uint64_t)get_be32(p) << 32 | get_be32(p + 4);
}

static void put_le32(uint8_t *p, uint32_t v) {
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t get_le32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16
        | (uint32_t)p[3] << 24;
}

/*
 * Sense key, ASC, and ASCQ from fixed or descriptor format sense data; false
 * without sense data.
 */
static bool sense_codes(
    const struct msc_cmd *cmd, uint8_t *key, uint8_t *asc, uint8_t *ascq
) {
    const uint8_t *s = cmd->sense;
    uint8_t code = s[0] & 0x7f;

    *asc = 0;
    *ascq = 0;
    if ((code == 0x72 || code == 0x73) && cmd->sense_len >= 2) {
        *key = s[1] & 0xf;
        if (cmd->sense_len >= 4) {
            *asc = s[2];
            *ascq = s[3];
        }
        return true;
    }
    if ((code == 0x70 || code == 0x71) && cmd->sense_len >= 3) {
        *key = s[2] & 0xf;
        if (cmd->sense_len >= 14) {
            *asc = s[12];
            *ascq = s[13];
        }
        return true;
    }
    return false;
}

static void transport_kick(struct tendril_msc *msc);

static void cmd_queue(struct tendril_msc *msc, struct msc_cmd *cmd) {
    cmd->msc = msc;
    cmd->status = TENDRIL_OK;
    cmd->scsi_status = STATUS_GOOD;
    cmd->actual = 0;
    cmd->sense_len = 0;
    if (msc->stopped || msc->broken) {
        cmd->status = msc->stopped ? TENDRIL_ERR_NO_DEVICE : TENDRIL_ERR_IO;
        cmd->done(cmd);
        return;
    }
    if (msc->pad12 && cmd->cdb_len < 12) {
        memset(cmd->cdb + cmd->cdb_len, 0, (uint32_t)12 - cmd->cdb_len);
        cmd->cdb_len = 12;
    }
    list_add_tail(&msc->queue, &cmd->node);
    transport_kick(msc);
}

/* Bulk-Only and CBI */

static void transport_step(struct tendril_msc *msc, int st);

static void xfer_done(struct tendril_transfer *xfer) {
    transport_step(xfer->context, xfer->status);
}

static void op_done(void *ctx, int st) {
    transport_step(ctx, st);
}

static void submit(
    struct tendril_msc *msc, int step, struct tendril_endpoint *ep, void *buf,
    size_t len, uint32_t timeout_ms
) {
    struct tendril_transfer *xfer = &msc->xfer;
    msc->xstep = step;
    tendril_transfer_init(xfer, ep);
    xfer->buffer = buf;
    xfer->length = len;
    xfer->timeout_ms = timeout_ms;
    xfer->callback = xfer_done;
    xfer->context = msc;
    int st = tendril_transfer_submit(xfer);
    if (st != TENDRIL_OK) {
        transport_step(msc, st);
    }
}

static void clear_halt(
    struct tendril_msc *msc, int step, struct tendril_endpoint *ep
) {
    msc->xstep = step;
    int st = tendril_endpoint_clear_halt(ep, op_done, msc);
    if (st != TENDRIL_OK) {
        transport_step(msc, st);
    }
}

static void class_request(
    struct tendril_msc *msc, int step, uint8_t request, void *data, size_t len,
    uint32_t timeout_ms
) {
    struct tendril_transfer *xfer = &msc->ctrl;
    msc->xstep = step;
    tendril_transfer_init(xfer, tendril_device_ep0(msc->dev));
    xfer->setup.request_type = TENDRIL_DIR_OUT | TENDRIL_TYPE_CLASS
        | TENDRIL_RECIP_INTERFACE;
    xfer->setup.request = request;
    xfer->setup.index = tendril_le16(msc->intf->number);
    xfer->setup.length = tendril_le16((uint16_t)len);
    xfer->buffer = data;
    xfer->length = len;
    xfer->timeout_ms = timeout_ms;
    xfer->callback = xfer_done;
    xfer->context = msc;
    int st = tendril_transfer_submit(xfer);
    if (st != TENDRIL_OK) {
        transport_step(msc, st);
    }
}

static void bbb_start(struct tendril_msc *msc) {
    struct msc_cmd *cmd = msc->active;
    uint8_t *cbw = msc->cbw;

    memset(cbw, 0, CBW_SIZE);
    put_le32(cbw, CBW_SIGNATURE);
    put_le32(cbw + 4, ++msc->tag);
    put_le32(cbw + 8, (uint32_t)cmd->len);
    cbw[12] = cmd->in ? 0x80 : 0x00;
    cbw[13] = cmd->lun;
    cbw[14] = (uint8_t)cmd->cdb_len;
    memcpy(cbw + 15, cmd->cdb, cmd->cdb_len);
    msc->csw_retried = false;
    submit(msc, T_CBW, msc->bulk_out, cbw, CBW_SIZE, CTRL_TIMEOUT_MS);
}

static void cbi_start(struct tendril_msc *msc) {
    struct msc_cmd *cmd = msc->active;
    /*
     * CBI 1.1, 2.3.1: the device may NAK the command transport for as long as
     * the command runs.
     */
    class_request(
        msc, T_ADSC, REQ_ADSC, cmd->cdb, (uint16_t)cmd->cdb_len,
        cmd->timeout_ms
    );
}

static void transport_start(struct tendril_msc *msc) {
    if (msc->transport == TRANSPORT_BBB) {
        bbb_start(msc);
    } else {
        cbi_start(msc);
    }
}

/*
 * Completes the active command; a finished REQUEST SENSE completes the command
 * it was for.
 */
static void transport_finish(struct tendril_msc *msc, int st) {
    struct msc_cmd *cmd = msc->active;

    msc->active = NULL;
    msc->xstep = T_IDLE;
    cmd->status = st;
    if (cmd == &msc->sense_cmd) {
        struct msc_cmd *orig = msc->sensing;
        uint8_t key, asc, ascq;
        msc->sensing = NULL;
        orig->sense_len = 0;
        if (st == TENDRIL_OK && cmd->scsi_status == STATUS_GOOD) {
            orig->sense_len = (uint8_t)MIN(cmd->actual, sizeof(orig->sense));
        }
        orig->scsi_status = STATUS_CHECK;
        /* BEGIN FreeBSD sys/dev/usb/storage/umass.c */
        /*
         * As FreeBSD's umass.c (umass_cam_sense_cb) takes them, NO SENSE means
         * the command passed, which is all a CB device without status says,
         * and so does UNIT ATTENTION on an INQUIRY of an MQ_NO_RS_CLEAR_UA
         * device, which REQUEST SENSE leaves in that condition.
         */
        if (sense_codes(orig, &key, &asc, &ascq)
         && (key == SENSE_NO_SENSE
          || (key == SENSE_UNIT_ATTENTION
           && (msc->quirks & MQ_NO_RS_CLEAR_UA) != 0
           && orig->cdb[0] == SCSI_INQUIRY))) {
            orig->scsi_status = STATUS_GOOD;
        }
        /* END FreeBSD sys/dev/usb/storage/umass.c */
        orig->status = st == TENDRIL_ERR_NO_DEVICE ? st : TENDRIL_OK;
        cmd = orig;
    }
    cmd->done(cmd);
    transport_kick(msc);
}

/*
 * Fetches sense data for the active command, which failed or whose status is
 * unknown.
 */
static void auto_sense(struct tendril_msc *msc) {
    struct msc_cmd *cmd = msc->active;
    struct msc_cmd *s = &msc->sense_cmd;

    if (cmd == s) {
        s->scsi_status = STATUS_CHECK;
        transport_finish(msc, TENDRIL_OK);
        return;
    }
    msc->sensing = cmd;
    memset(s->cdb, 0, sizeof(s->cdb));
    s->cdb[0] = SCSI_REQUEST_SENSE;
    s->cdb[1] = msc->cmdset == CMDSET_UFI ? (uint8_t)(cmd->lun << 5) : 0;
    s->cdb[4] = 18;
    s->cdb_len = msc->pad12 ? 12 : 6;
    s->lun = cmd->lun;
    s->in = true;
    s->data = cmd->sense;
    s->len = 18;
    s->timeout_ms = CTRL_TIMEOUT_MS;
    s->scsi_status = STATUS_GOOD;
    s->actual = 0;
    s->sense_len = 0;
    s->msc = msc;
    msc->active = s;
    transport_start(msc);
}

/*
 * Bulk-Only 1.0, 5.3.4 reset recovery, or the CBI 1.1, 2.2 command block
 * reset, then both bulk halts cleared.
 */
static void start_recovery(struct tendril_msc *msc, int st) {
    msc->recover_status = st;
    LOG_DEBUG("MSC: reset recovery after %s", tendril_status_str(st));
    if (msc->transport == TRANSPORT_BBB) {
        class_request(msc, T_RESET, REQ_BBB_RESET, NULL, 0, CTRL_TIMEOUT_MS);
        return;
    }
    memset(msc->ctrl_buf, 0xff, 12);
    msc->ctrl_buf[0] = 0x1d;
    msc->ctrl_buf[1] = 0x04;
    class_request(msc, T_RESET, REQ_ADSC, msc->ctrl_buf, 12, CTRL_TIMEOUT_MS);
}

static void read_csw(struct tendril_msc *msc) {
    submit(
        msc, T_CSW, msc->bulk_in, msc->csw, CSW_SIZE, msc->active->timeout_ms
    );
}

static void bbb_check_csw(struct tendril_msc *msc) {
    struct msc_cmd *cmd = msc->active;
    const uint8_t *csw = msc->csw;
    uint32_t sig = get_le32(csw);

    if ((msc->quirks & MQ_WRONG_CSW_SIGNATURE) != 0
     && (sig == CSW_SIGNATURE_OLYMPUS || sig == CSW_SIGNATURE_AS_CBW)) {
        sig = CSW_SIGNATURE;
    }
    /*
     * Bulk-Only 1.0, 6.3 and 6.5: an invalid CSW, or a phase error, calls for
     * reset recovery.
     */
    if (msc->xfer.actual != CSW_SIZE || sig != CSW_SIGNATURE
     || get_le32(csw + 4) != msc->tag || csw[12] > 1) {
        start_recovery(msc, TENDRIL_ERR_PROTOCOL);
        return;
    }
    uint32_t residue = get_le32(csw + 8);
    if ((msc->quirks & MQ_IGNORE_RESIDUE) == 0 && residue != 0) {
        size_t relevant = cmd->len - MIN((size_t)residue, cmd->len);
        cmd->actual = MIN(cmd->actual, relevant);
    }
    if (csw[12] == 1) {
        auto_sense(msc);
        return;
    }
    cmd->scsi_status = STATUS_GOOD;
    transport_finish(msc, TENDRIL_OK);
}

/* BEGIN FreeBSD sys/dev/usb/storage/umass.c */
static void cbi_status(struct tendril_msc *msc) {
    struct msc_cmd *cmd = msc->active;
    if (msc->transport == TRANSPORT_CBI) {
        submit(msc, T_INTR, msc->intr_in, msc->intr_buf, 2, cmd->timeout_ms);
        return;
    }
    if (cmd == &msc->sense_cmd) {
        cmd->scsi_status = STATUS_GOOD;
        transport_finish(msc, TENDRIL_OK);
        return;
    }
    auto_sense(msc);
}
/* END FreeBSD sys/dev/usb/storage/umass.c */

static void cbi_check_status(struct tendril_msc *msc) {
    struct msc_cmd *cmd = msc->active;
    const uint8_t *b = msc->intr_buf;
    bool failed;

    if (msc->xfer.actual < 2) {
        start_recovery(msc, TENDRIL_ERR_PROTOCOL);
        return;
    }
    if (msc->cmdset == CMDSET_UFI) {
        /*
         * CBI 1.1, 3.4.3.1.3: UFI reports ASC and ASCQ, which REQUEST SENSE
         * and INQUIRY leave alone.
         */
        failed = (b[0] != 0 || b[1] != 0) && cmd->cdb[0] != SCSI_REQUEST_SENSE
            && cmd->cdb[0] != SCSI_INQUIRY;
    } else if (b[0] != 0) {
        /*
         * Not a command completion interrupt (CBI 1.1, 3.4.3.1.1), so the
         * status is unknown.
         */
        if (cmd == &msc->sense_cmd) {
            cmd->scsi_status = STATUS_GOOD;
            transport_finish(msc, TENDRIL_OK);
        } else {
            auto_sense(msc);
        }
        return;
    } else if ((b[1] & 3) == 2) {
        start_recovery(msc, TENDRIL_ERR_PROTOCOL);
        return;
    } else {
        failed = (b[1] & 3) != 0;
    }
    if (!failed) {
        cmd->scsi_status = STATUS_GOOD;
        transport_finish(msc, TENDRIL_OK);
        return;
    }
    if (cmd->len != 0) {
        /*
         * CBI 1.1, 2.4.3.1.3: the data pipe may have stalled after the last
         * data of a failed command.
         */
        clear_halt(msc, T_FAIL_CLEAR, cmd->in ? msc->bulk_in : msc->bulk_out);
        return;
    }
    auto_sense(msc);
}

static void transport_step(struct tendril_msc *msc, int st) {
    struct msc_cmd *cmd = msc->active;

    if (cmd == NULL) {
        return;
    }
    if (st == TENDRIL_ERR_NO_DEVICE) {
        msc->stopped = true;
        transport_finish(msc, st);
        return;
    }
    switch (msc->xstep) {
        case T_CBW: {
            /*
             * Bulk-Only 1.0, 5.3.1: a stalled CBW also calls for reset
             * recovery.
             */
            if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else if (cmd->len != 0) {
                submit(
                    msc, T_DATA, cmd->in ? msc->bulk_in : msc->bulk_out,
                    cmd->data, cmd->len, cmd->timeout_ms
                );
            } else {
                read_csw(msc);
            }
            break;
        }
        case T_ADSC: {
            if (st == TENDRIL_ERR_STALL) {
                /*
                 * CBI 1.1, 2.3.2.1: a stalled ADSC failed the command, and no
                 * interrupt follows.
                 */
                auto_sense(msc);
            } else if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else if (cmd->len != 0) {
                submit(
                    msc, T_DATA, cmd->in ? msc->bulk_in : msc->bulk_out,
                    cmd->data, cmd->len, cmd->timeout_ms
                );
            } else {
                cbi_status(msc);
            }
            break;
        }
        case T_DATA: {
            cmd->actual = msc->xfer.actual;
            if (st == TENDRIL_ERR_STALL) {
                /*
                 * The device ended the data stage early; status follows the
                 * cleared halt.
                 */
                clear_halt(
                    msc, T_DATA_CLEAR, cmd->in ? msc->bulk_in : msc->bulk_out
                );
            } else if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else if (msc->transport == TRANSPORT_BBB) {
                read_csw(msc);
            } else {
                cbi_status(msc);
            }
            break;
        }
        case T_DATA_CLEAR: {
            if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else if (msc->transport == TRANSPORT_BBB) {
                read_csw(msc);
            } else {
                cbi_status(msc);
            }
            break;
        }
        case T_CSW: {
            /*
             * Bulk-Only 1.0, figure 2: one more try at the CSW after clearing
             * a stall.
             */
            if (st == TENDRIL_ERR_STALL && !msc->csw_retried) {
                clear_halt(msc, T_CSW_CLEAR, msc->bulk_in);
            } else if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else {
                bbb_check_csw(msc);
            }
            break;
        }
        case T_CSW_CLEAR: {
            if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else {
                msc->csw_retried = true;
                read_csw(msc);
            }
            break;
        }
        case T_INTR: {
            if (st == TENDRIL_ERR_STALL) {
                clear_halt(msc, T_INTR_CLEAR, msc->intr_in);
            } else if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else {
                cbi_check_status(msc);
            }
            break;
        }
        case T_INTR_CLEAR: {
            if (st != TENDRIL_OK) {
                start_recovery(msc, st);
            } else {
                auto_sense(msc);
            }
            break;
        }
        case T_FAIL_CLEAR: {
            auto_sense(msc);
            break;
        }
        case T_RESET: {
            /* The halts are cleared whatever became of the reset. */
            clear_halt(msc, T_RESET_CLEAR_IN, msc->bulk_in);
            break;
        }
        case T_RESET_CLEAR_IN: {
            clear_halt(msc, T_RESET_CLEAR_OUT, msc->bulk_out);
            break;
        }
        case T_RESET_CLEAR_OUT: {
            transport_finish(msc, msc->recover_status);
            break;
        }
        default: {
            break;
        }
    }
}

/* UAS */

static void uas_xfer_done(struct tendril_transfer *xfer);
static void uas_tm_kick(struct tendril_msc *msc);

static int uas_post(
    struct tendril_msc *msc, struct msc_cmd *cmd,
    struct tendril_transfer *xfer, struct tendril_endpoint *ep, void *buf,
    size_t len, bool stream
) {
    tendril_transfer_init(xfer, ep);
    xfer->buffer = buf;
    xfer->length = len;
    xfer->stream = msc->streams && stream ? cmd->tag : 0;
    xfer->timeout_ms = cmd->timeout_ms;
    xfer->callback = uas_xfer_done;
    xfer->context = cmd;
    int st = tendril_transfer_submit(xfer);
    if (st == TENDRIL_OK) {
        cmd->pending++;
    }
    return st;
}

/*
 * Records a failure and cancels the command's other transfers; completion
 * waits for them.
 */
static void uas_fail(struct msc_cmd *cmd, int st) {
    if (cmd->failed == TENDRIL_OK) {
        cmd->failed = st;
    }
    tendril_transfer_cancel(&cmd->status_xfer);
    tendril_transfer_cancel(&cmd->data_xfer);
    tendril_transfer_cancel(&cmd->iu_xfer);
}

static void uas_complete(struct tendril_msc *msc, struct msc_cmd *cmd) {
    size_t tag = cmd->tag;
    int st = cmd->failed;

    if (st == TENDRIL_OK && !cmd->have_status) {
        st = TENDRIL_ERR_PROTOCOL;
    }
    msc->in_flight--;
    if (st == TENDRIL_OK) {
        msc->tag_busy &= ~((uint64_t)1 << tag);
    } else if (st == TENDRIL_ERR_NO_DEVICE) {
        msc->stopped = true;
        msc->tag_busy &= ~((uint64_t)1 << tag);
    } else {
        /*
         * The device may still hold the command; the tag waits for an ABORT
         * TASK.
         */
        msc->abort_mask |= (uint64_t)1 << tag;
        msc->abort_lun[tag] = cmd->lun;
    }
    cmd->status = st;
    cmd->done(cmd);
    transport_kick(msc);
}

/* BEGIN SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
static void uas_status_iu(struct tendril_msc *msc, struct msc_cmd *cmd) {
    const uint8_t *iu = cmd->status_buf;
    size_t n = cmd->status_xfer.actual;
    int st;

    if (n < 4 || get_be16(iu + 2) != cmd->tag) {
        /*
         * A late IU of an aborted command, which can only come without
         * streams.
         */
        st = uas_post(
            msc, cmd, &cmd->status_xfer, msc->status_ep, cmd->status_buf,
            IU_BUF_SIZE, true
        );
        if (st != TENDRIL_OK) {
            uas_fail(cmd, st);
        }
        return;
    }
    switch (iu[0]) {
        case IU_READ_READY:
        case IU_WRITE_READY: {
            bool in = iu[0] == IU_READ_READY;
            if (msc->streams) {
                st = TENDRIL_OK;
            } else if (cmd->len == 0 || in != cmd->in || cmd->data_posted) {
                st = TENDRIL_ERR_PROTOCOL;
            } else {
                cmd->data_posted = true;
                st = uas_post(
                    msc, cmd, &cmd->data_xfer,
                    in ? msc->data_in_ep : msc->data_out_ep, cmd->data,
                    cmd->len, false
                );
            }
            if (st == TENDRIL_OK) {
                st = uas_post(
                    msc, cmd, &cmd->status_xfer, msc->status_ep,
                    cmd->status_buf, IU_BUF_SIZE, true
                );
            }
            if (st != TENDRIL_OK) {
                uas_fail(cmd, st);
            }
            break;
        }
        case IU_SENSE: {
            if (n < 16) {
                uas_fail(cmd, TENDRIL_ERR_PROTOCOL);
                break;
            }
            cmd->have_status = true;
            cmd->scsi_status = iu[6];
            size_t len = MIN((size_t)get_be16(iu + 14), n - 16);
            cmd->sense_len = (uint8_t)MIN(len, sizeof(cmd->sense));
            memcpy(cmd->sense, iu + 16, cmd->sense_len);
            /*
             * After a failure the device may never move the rest of the data.
             */
            if (cmd->scsi_status != STATUS_GOOD) {
                tendril_transfer_cancel(&cmd->data_xfer);
            }
            break;
        }
        default: {
            /* A Response IU for a command means the device did not take it. */
            uas_fail(cmd, TENDRIL_ERR_PROTOCOL);
            break;
        }
    }
}
/* END SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */

static void uas_halt_done(void *ctx, int st) {
    struct msc_cmd *cmd = ctx;
    (void)st;
    if (--cmd->pending == 0) {
        uas_complete(cmd->msc, cmd);
    }
}

static void uas_xfer_done(struct tendril_transfer *xfer) {
    struct msc_cmd *cmd = xfer->context;
    struct tendril_msc *msc = cmd->msc;
    int st = xfer->status;

    cmd->pending--;
    if (st == TENDRIL_ERR_CANCELLED
     && (cmd->failed != TENDRIL_OK || cmd->have_status)) {
        if (xfer == &cmd->data_xfer) {
            cmd->actual = xfer->actual;
        }
    } else if (xfer == &cmd->status_xfer) {
        if (st == TENDRIL_OK) {
            uas_status_iu(msc, cmd);
        } else {
            uas_fail(cmd, st);
        }
    } else if (xfer == &cmd->data_xfer) {
        cmd->actual = xfer->actual;
        if (st == TENDRIL_ERR_STALL) {
            /* The status still comes once the halt is cleared. */
            if (tendril_endpoint_clear_halt(xfer->endpoint, uas_halt_done, cmd)
             == TENDRIL_OK) {
                cmd->pending++;
            } else {
                uas_fail(cmd, st);
            }
        } else if (st != TENDRIL_OK) {
            uas_fail(cmd, st);
        }
    } else if (st != TENDRIL_OK) {
        uas_fail(cmd, st);
    }
    if (cmd->pending == 0) {
        uas_complete(msc, cmd);
    }
}

static void uas_submit(
    struct tendril_msc *msc, struct msc_cmd *cmd, size_t tag
) {
    uint8_t *iu = cmd->iu;
    int st;

    cmd->tag = (uint16_t)tag;
    cmd->pending = 0;
    cmd->have_status = false;
    cmd->data_posted = false;
    cmd->failed = TENDRIL_OK;
    msc->in_flight++;

    memset(iu, 0, sizeof(cmd->iu));
    /* BEGIN SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
    iu[0] = IU_COMMAND;
    put_be16(iu + 2, (uint16_t)tag);
    /*
     * Task attribute SIMPLE, and the LUN in single level format, as
     * SerenityOS's Structures.h lays the IU out.
     */
    iu[9] = cmd->lun;
    memcpy(iu + 16, cmd->cdb, cmd->cdb_len);
    /* END SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */

    /*
     * The status pipe, and with streams the data pipe, listen before the
     * command goes out.
     */
    st = uas_post(
        msc, cmd, &cmd->status_xfer, msc->status_ep, cmd->status_buf,
        IU_BUF_SIZE, true
    );
    if (st == TENDRIL_OK && msc->streams && cmd->len != 0) {
        cmd->data_posted = true;
        st = uas_post(
            msc, cmd, &cmd->data_xfer,
            cmd->in ? msc->data_in_ep : msc->data_out_ep, cmd->data, cmd->len,
            true
        );
    }
    if (st == TENDRIL_OK) {
        st = uas_post(
            msc, cmd, &cmd->iu_xfer, msc->cmd_ep, iu, sizeof(cmd->iu), false
        );
    }
    if (st != TENDRIL_OK) {
        uas_fail(cmd, st);
        if (cmd->pending == 0) {
            uas_complete(msc, cmd);
        }
    }
}

static size_t uas_alloc_tag(struct tendril_msc *msc) {
    /*
     * Tags rotate, so that a late IU of an old command cannot pass for a new
     * one's.
     */
    for (size_t i = 0; i < msc->num_tags; i++) {
        size_t tag = msc->next_tag;
        msc->next_tag = msc->next_tag % msc->num_tags + 1;
        if ((msc->tag_busy & ((uint64_t)1 << tag)) == 0) {
            msc->tag_busy |= (uint64_t)1 << tag;
            return tag;
        }
    }
    return 0;
}

static void uas_kick(struct tendril_msc *msc) {
    uas_tm_kick(msc);
    while (!list_empty(&msc->queue) && !msc->stopped) {
        /*
         * Without streams the status pipe is shared, so commands go one at a
         * time.
         */
        if (!msc->streams && (msc->in_flight != 0 || msc->tm_busy)) {
            break;
        }
        size_t tag = uas_alloc_tag(msc);
        if (tag == 0) {
            break;
        }
        struct msc_cmd *cmd = CONTAINER_OF(
            msc->queue.next, struct msc_cmd, node
        );
        list_del(&cmd->node);
        uas_submit(msc, cmd, tag);
    }
}

static void uas_fail_queue(struct tendril_msc *msc, int st) {
    while (!list_empty(&msc->queue)) {
        struct msc_cmd *cmd = CONTAINER_OF(
            msc->queue.next, struct msc_cmd, node
        );
        list_del(&cmd->node);
        cmd->status = st;
        cmd->done(cmd);
    }
}

static void uas_tm_finish(struct tendril_msc *msc) {
    uint64_t bit = (uint64_t)1 << msc->tm_target;

    msc->tm_busy = false;
    if (msc->tm_ok) {
        msc->abort_mask &= ~bit;
        msc->tag_busy &= ~bit;
    } else if (!msc->stopped) {
        LOG_ERROR("MSC: UAS ABORT TASK failed, giving up on the device");
        msc->broken = true;
        msc->abort_mask = 0;
        uas_fail_queue(msc, TENDRIL_ERR_IO);
    }
    transport_kick(msc);
}

static void uas_tm_done(struct tendril_transfer *xfer) {
    struct tendril_msc *msc = xfer->context;
    int st = xfer->status;

    msc->tm_pending--;
    if (st == TENDRIL_ERR_NO_DEVICE) {
        msc->stopped = true;
    }
    if (xfer == &msc->tm_status) {
        const uint8_t *iu = msc->tm_buf;
        if (st != TENDRIL_OK) {
            tendril_transfer_cancel(&msc->tm_xfer);
        } else if (xfer->actual >= 8 && iu[0] == IU_RESPONSE
                && get_be16(iu + 2) == msc->tm_tag) {
            /* BEGIN SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h (and the condition above) */
            msc->tm_ok = iu[7] == RC_TM_COMPLETE || iu[7] == RC_TM_SUCCEEDED;
            /* END SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
        } else if (tendril_transfer_submit(xfer) == TENDRIL_OK) {
            /* Some other command's late IU. */
            msc->tm_pending++;
        }
    } else if (st != TENDRIL_OK) {
        tendril_transfer_cancel(&msc->tm_status);
    }
    if (msc->tm_pending == 0) {
        uas_tm_finish(msc);
    }
}

static void uas_tm_kick(struct tendril_msc *msc) {
    if (msc->tm_busy || msc->abort_mask == 0 || msc->stopped || msc->broken) {
        return;
    }
    if (!msc->streams && msc->in_flight != 0) {
        return;
    }
    size_t target = 0;
    while ((msc->abort_mask & ((uint64_t)1 << target)) == 0) {
        target++;
    }
    uint8_t *iu = msc->tm_iu;

    msc->tm_target = (uint16_t)target;
    msc->tm_busy = true;
    msc->tm_ok = false;
    msc->tm_pending = 0;
    memset(iu, 0, sizeof(msc->tm_iu));
    /* BEGIN SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
    iu[0] = IU_TASK_MGMT;
    put_be16(iu + 2, msc->tm_tag);
    iu[4] = TM_ABORT_TASK;
    put_be16(iu + 6, (uint16_t)target);
    iu[9] = msc->abort_lun[target];
    /* END SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
    LOG_DEBUG("MSC: aborting UAS tag %zu", target);

    struct tendril_transfer *xfer = &msc->tm_status;
    tendril_transfer_init(xfer, msc->status_ep);
    xfer->buffer = msc->tm_buf;
    xfer->length = IU_BUF_SIZE;
    xfer->stream = msc->streams ? msc->tm_tag : 0;
    xfer->timeout_ms = CTRL_TIMEOUT_MS;
    xfer->callback = uas_tm_done;
    xfer->context = msc;
    int st = tendril_transfer_submit(xfer);
    if (st == TENDRIL_OK) {
        msc->tm_pending++;
        xfer = &msc->tm_xfer;
        tendril_transfer_init(xfer, msc->cmd_ep);
        xfer->buffer = iu;
        xfer->length = sizeof(msc->tm_iu);
        xfer->timeout_ms = CTRL_TIMEOUT_MS;
        xfer->callback = uas_tm_done;
        xfer->context = msc;
        st = tendril_transfer_submit(xfer);
        if (st == TENDRIL_OK) {
            msc->tm_pending++;
        } else {
            tendril_transfer_cancel(&msc->tm_status);
        }
    }
    if (st == TENDRIL_ERR_NO_DEVICE) {
        msc->stopped = true;
    }
    if (msc->tm_pending == 0) {
        uas_tm_finish(msc);
    }
}

static void transport_kick(struct tendril_msc *msc) {
    if (msc->transport == TRANSPORT_UAS) {
        uas_kick(msc);
        return;
    }
    if (msc->active != NULL || list_empty(&msc->queue)) {
        return;
    }
    msc->active = CONTAINER_OF(msc->queue.next, struct msc_cmd, node);
    list_del(&msc->active->node);
    transport_start(msc);
}

/* Logical units */

#define V_OK 0
/* Unit attention, aborted command, or a transport error: try again. */
#define V_RETRY 1
/* Becoming ready or busy: try again later. */
#define V_WAIT 2
#define V_START 3
#define V_NO_MEDIUM 4
#define V_CHANGED 5
#define V_FAIL 6

static int judge(const struct msc_cmd *cmd, int *st) {
    uint8_t key, asc, ascq;

    *st = cmd->status;
    if (cmd->status != TENDRIL_OK) {
        return cmd->status == TENDRIL_ERR_NO_DEVICE ? V_FAIL : V_RETRY;
    }
    switch (cmd->scsi_status) {
        case STATUS_GOOD: {
            return V_OK;
        }
        case STATUS_CHECK: {
            break;
        }
        case STATUS_BUSY:
        case STATUS_TASK_SET_FULL: {
            *st = TENDRIL_ERR_BUSY;
            return V_WAIT;
        }
        default: {
            *st = TENDRIL_ERR_IO;
            return V_RETRY;
        }
    }
    if (!sense_codes(cmd, &key, &asc, &ascq)) {
        *st = TENDRIL_ERR_IO;
        return V_RETRY;
    }
    switch (key) {
        case SENSE_NO_SENSE:
        case SENSE_RECOVERED: {
            *st = TENDRIL_OK;
            return V_OK;
        }
        case SENSE_NOT_READY: {
            if (asc == ASC_NO_MEDIUM) {
                *st = TENDRIL_ERR_NO_MEDIUM;
                return V_NO_MEDIUM;
            }
            *st = TENDRIL_ERR_BUSY;
            return asc == ASC_NOT_READY && ascq == ASCQ_START_REQUIRED
                ? V_START
                : V_WAIT;
        }
        case SENSE_UNIT_ATTENTION: {
            if (asc == ASC_MEDIUM_CHANGED) {
                *st = TENDRIL_ERR_MEDIUM_CHANGED;
                return V_CHANGED;
            }
            *st = TENDRIL_ERR_IO;
            return V_RETRY;
        }
        case SENSE_DATA_PROTECT: {
            *st = TENDRIL_ERR_WRITE_PROTECTED;
            return V_FAIL;
        }
        case SENSE_ILLEGAL_REQUEST: {
            *st = TENDRIL_ERR_INVALID;
            return V_FAIL;
        }
        default: {
            *st = TENDRIL_ERR_IO;
            return V_RETRY;
        }
    }
}

static void lun_cmd_done(struct msc_cmd *cmd);
static void lun_next(struct tendril_msc_lun *lun);

static void lun_issue(
    struct tendril_msc_lun *lun, int step, const uint8_t *cdb, size_t cdb_len,
    void *data, size_t len, bool in, uint32_t timeout_ms
) {
    struct tendril_msc *msc = lun->msc;
    struct msc_cmd *cmd = &lun->cmd;

    lun->step = step;
    memset(cmd->cdb, 0, sizeof(cmd->cdb));
    memcpy(cmd->cdb, cdb, cdb_len);
    if (msc->cmdset == CMDSET_UFI) {
        cmd->cdb[1] = (uint8_t)(cmd->cdb[1] | lun->number << 5);
    }
    cmd->cdb_len = (uint8_t)cdb_len;
    cmd->lun = lun->number;
    cmd->in = in;
    cmd->data = data;
    cmd->len = len;
    cmd->timeout_ms = timeout_ms;
    cmd->done = lun_cmd_done;
    cmd->ctx = lun;
    cmd_queue(msc, cmd);
}

static bool block_device(const struct tendril_msc_lun *lun) {
    return lun->type == TYPE_DIRECT || lun->type == TYPE_CDROM
        || lun->type == TYPE_OPTICAL || lun->type == TYPE_RBC;
}

/*
 * Waits for a unit becoming ready, polling until READY_TIMEOUT_MS has passed.
 */
static bool wait_ready(struct tendril_msc_lun *lun) {
    struct tendril_hc *hc = lun->msc->hc;
    uint64_t now = tendril_now();

    if (lun->ready_deadline == 0) {
        lun->ready_deadline = now + READY_TIMEOUT_MS * NS_PER_MS;
    } else if (now >= lun->ready_deadline) {
        return false;
    }
    hc_lock(hc);
    tendril_timer_arm(hc, &lun->timer, READY_POLL_MS * NS_PER_MS);
    hc_unlock(hc);
    return true;
}

static void probe_capacity(struct tendril_msc_lun *lun);
static void probe_done(struct tendril_msc_lun *lun, bool present);

static void issue_start(struct tendril_msc_lun *lun, int step) {
    static const uint8_t cdb[6] = { SCSI_START_STOP_UNIT, 0, 0, 0, 0x01, 0 };
    lun_issue(lun, step, cdb, sizeof(cdb), NULL, 0, false, LONG_TIMEOUT_MS);
}

static void probe_test(struct tendril_msc_lun *lun) {
    static const uint8_t cdb[6] = { SCSI_TEST_UNIT_READY, 0, 0, 0, 0, 0 };
    uint32_t quirks = lun->msc->quirks;
    /* BEGIN FreeBSD sys/dev/usb/storage/umass.c */
    /*
     * FreeBSD's umass.c sends START STOP UNIT in place of TEST UNIT READY with
     * MQ_NO_TEST_UNIT_READY, and takes that as done with MQ_NO_START_STOP.
     */
    if ((quirks & MQ_NO_TEST_UNIT_READY) != 0) {
        if ((quirks & MQ_NO_START_STOP) != 0) {
            probe_capacity(lun);
        } else {
            issue_start(lun, LUN_TEST);
        }
        return;
    }
    /* END FreeBSD sys/dev/usb/storage/umass.c */
    lun_issue(lun, LUN_TEST, cdb, sizeof(cdb), NULL, 0, false, CMD_TIMEOUT_MS);
}

static void probe_capacity(struct tendril_msc_lun *lun) {
    static const uint8_t cdb[10] = { SCSI_READ_CAPACITY10 };
    lun_issue(
        lun, LUN_CAPACITY10, cdb, sizeof(cdb), lun->buf, 8, true,
        CMD_TIMEOUT_MS
    );
}

static void probe_capacity16(struct tendril_msc_lun *lun) {
    static const uint8_t cdb[16] = {
        SCSI_SERVICE_ACTION_IN16, SA_READ_CAPACITY16, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 32
    };
    lun_issue(
        lun, LUN_CAPACITY16, cdb, sizeof(cdb), lun->buf, 32, true,
        CMD_TIMEOUT_MS
    );
}

static void probe_mode_sense(struct tendril_msc_lun *lun) {
    uint8_t cdb[10] = { 0 };
    /*
     * All pages, current values; only the header's write protect bit matters.
     */
    if (lun->msc->cmdset == CMDSET_SCSI) {
        cdb[0] = SCSI_MODE_SENSE6;
        cdb[2] = 0x3f;
        cdb[4] = sizeof(lun->buf);
        lun_issue(
            lun, LUN_MODE_SENSE, cdb, 6, lun->buf, sizeof(lun->buf), true,
            CMD_TIMEOUT_MS
        );
    } else {
        cdb[0] = SCSI_MODE_SENSE10;
        cdb[2] = 0x3f;
        put_be16(cdb + 7, sizeof(lun->buf));
        lun_issue(
            lun, LUN_MODE_SENSE, cdb, 10, lun->buf, sizeof(lun->buf), true,
            CMD_TIMEOUT_MS
        );
    }
}

static void probe_inquiry(struct tendril_msc_lun *lun) {
    static const uint8_t cdb[6] = { SCSI_INQUIRY, 0, 0, 0, 36, 0 };
    lun_issue(
        lun, LUN_INQUIRY, cdb, sizeof(cdb), lun->buf, 36, true, CMD_TIMEOUT_MS
    );
}

static void copy_string(char *dst, const uint8_t *src, size_t n) {
    size_t len = 0;
    for (size_t i = 0; i < n; i++) {
        dst[i] = src[i] >= 0x20 && src[i] < 0x7f ? (char)src[i] : ' ';
        if (dst[i] != ' ') {
            len = i + 1;
        }
    }
    dst[len] = '\0';
}

static void probe_start(struct tendril_msc_lun *lun, bool initial) {
    lun->busy = true;
    lun->tries = 0;
    lun->started = false;
    lun->ready_deadline = 0;
    lun->old_present = lun->present;
    lun->old_write_protected = lun->write_protected;
    lun->old_blocks = lun->blocks;
    lun->old_block_size = lun->block_size;
    if (!initial) {
        if (block_device(lun)) {
            probe_test(lun);
        } else {
            probe_done(lun, false);
        }
        return;
    }
    if ((lun->msc->quirks & MQ_NO_INQUIRY) == 0) {
        probe_inquiry(lun);
        return;
    }
    lun->type = TYPE_DIRECT;
    lun->removable = true;
    probe_test(lun);
}

static void lun_free(struct tendril_msc_lun *lun) {
    struct tendril_msc *msc = lun->msc;
    hc_lock(msc->hc);
    tendril_timer_cancel(msc->hc, &lun->timer);
    hc_unlock(msc->hc);
    msc->luns[lun->number] = NULL;
    tendril_free(lun, sizeof(struct tendril_msc_lun));
}

static void probe_done(struct tendril_msc_lun *lun, bool present) {
    struct tendril_msc *msc = lun->msc;

    if (!present) {
        lun->blocks = 0;
        lun->block_size = 0;
        lun->write_protected = false;
    }
    lun->present = present;
    bool changed = lun->changed || present != lun->old_present
        || lun->blocks != lun->old_blocks
        || lun->block_size != lun->old_block_size
        || lun->write_protected != lun->old_write_protected;
    lun->changed = false;
    lun->busy = false;
    lun->ready_deadline = 0;

    if (present && (changed || !lun->attached)) {
        LOG_INFO(
            "MSC: LUN %u: %llu blocks of %zu bytes%s", lun->number,
            (unsigned long long)lun->blocks, lun->block_size,
            lun->write_protected ? ", write protected" : ""
        );
    } else if (!present && (changed || !lun->attached) && block_device(lun)) {
        LOG_INFO("MSC: LUN %u: no medium", lun->number);
    }
    if (!lun->attached) {
        lun->attached = true;
        if (callbacks->attach != NULL) {
            callbacks->attach(lun);
        }
    } else if (changed && callbacks->medium_changed != NULL) {
        callbacks->medium_changed(lun);
    }
    if (lun->check_done != NULL) {
        void (*done)(void *, int) = lun->check_done;
        lun->check_done = NULL;
        done(lun->check_ctx, TENDRIL_OK);
    }
    if (!msc->stopped) {
        lun_next(lun);
    }
}

static void probe_result(struct tendril_msc_lun *lun, int v, int st) {
    struct msc_cmd *cmd = &lun->cmd;
    struct tendril_msc *msc = lun->msc;

    switch (lun->step) {
        case LUN_INQUIRY: {
            if (v == V_OK && cmd->actual >= 5) {
                const uint8_t *b = lun->buf;
                /* Peripheral qualifier 3 or type 1Fh: no unit at this LUN. */
                if ((b[0] >> 5) == 3 || (b[0] & 0x1f) == TYPE_NONE) {
                    lun_free(lun);
                    return;
                }
                lun->type = b[0] & 0x1f;
                lun->removable = (b[1] & 0x80) != 0;
                if (cmd->actual >= 36) {
                    copy_string(lun->vendor, b + 8, 8);
                    copy_string(lun->product, b + 16, 16);
                    copy_string(lun->revision, b + 32, 4);
                }
                LOG_INFO(
                    "MSC: LUN %u: \"%s\" \"%s\" \"%s\", type %02x%s",
                    lun->number, lun->vendor, lun->product, lun->revision,
                    lun->type, lun->removable ? ", removable" : ""
                );
                lun->tries = 0;
                if (block_device(lun)) {
                    probe_test(lun);
                } else {
                    probe_done(lun, false);
                }
                return;
            }
            if (v != V_FAIL && ++lun->tries < PROBE_TRIES) {
                probe_inquiry(lun);
                return;
            }
            LOG_WARN(
                "MSC: LUN %u: INQUIRY failed: %s", lun->number,
                tendril_status_str(st)
            );
            lun_free(lun);
            return;
        }
        case LUN_TEST: {
            switch (v) {
                case V_OK: {
                    lun->tries = 0;
                    probe_capacity(lun);
                    return;
                }
                case V_CHANGED: {
                    lun->changed = true;
                    if (++lun->tries < PROBE_TRIES) {
                        probe_test(lun);
                        return;
                    }
                    break;
                }
                case V_RETRY: {
                    if (++lun->tries < PROBE_TRIES) {
                        probe_test(lun);
                        return;
                    }
                    break;
                }
                case V_START: {
                    if ((msc->quirks & MQ_NO_START_STOP) == 0
                     && !lun->started) {
                        lun->started = true;
                        issue_start(lun, LUN_START);
                        return;
                    }
                    if (wait_ready(lun)) {
                        return;
                    }
                    break;
                }
                case V_WAIT: {
                    if (wait_ready(lun)) {
                        return;
                    }
                    break;
                }
                case V_FAIL: {
                    /*
                     * A unit rejecting TEST UNIT READY may still answer READ
                     * CAPACITY.
                     */
                    if (st == TENDRIL_ERR_INVALID) {
                        probe_capacity(lun);
                        return;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
            probe_done(lun, false);
            return;
        }
        case LUN_START: {
            probe_test(lun);
            return;
        }
        case LUN_CAPACITY10:
        case LUN_CAPACITY16: {
            if (v == V_OK
             && cmd->actual
             >= (size_t)(lun->step == LUN_CAPACITY10 ? 8 : 12)) {
                const uint8_t *b = lun->buf;
                uint64_t last;
                if (lun->step == LUN_CAPACITY10) {
                    last = get_be32(b);
                    lun->block_size = get_be32(b + 4);
                    if ((msc->quirks & MQ_READ_CAP_OFF_BY_ONE) != 0) {
                        /* BEGIN FreeBSD sys/dev/usb/storage/umass.c (and the condition above) */
                        /*
                         * The block count in place of the last block, which
                         * FreeBSD's umass.c corrects here only.
                         */
                        last--;
                        /* END FreeBSD sys/dev/usb/storage/umass.c */
                    } else if (last == 0xffffffff && msc->cmdset == CMDSET_SCSI
                            && !msc->pad12) {
                        probe_capacity16(lun);
                        return;
                    }
                } else {
                    last = get_be64(b);
                    lun->block_size = get_be32(b + 8);
                }
                lun->blocks = last + 1;
                if (lun->block_size == 0 || lun->block_size > 0x10000
                 || lun->blocks == 0) {
                    probe_done(lun, false);
                    return;
                }
                lun->tries = 0;
                lun->write_protected = lun->type == TYPE_CDROM;
                if ((lun->type == TYPE_DIRECT || lun->type == TYPE_RBC)
                 && !lun->no_mode_sense) {
                    probe_mode_sense(lun);
                } else {
                    probe_done(lun, true);
                }
                return;
            }
            if (v == V_FAIL && lun->step == LUN_CAPACITY16
             && st == TENDRIL_ERR_INVALID) {
                /*
                 * Without READ CAPACITY (16), the first 2^32 blocks are all
                 * that can be reached.
                 */
                lun->blocks = (uint64_t)0x100000000;
                lun->write_protected = false;
                probe_mode_sense(lun);
                return;
            }
            if (v == V_CHANGED) {
                lun->changed = true;
            }
            if ((v == V_RETRY || v == V_CHANGED)
             && ++lun->tries < PROBE_TRIES) {
                probe_test(lun);
                return;
            }
            if ((v == V_WAIT || v == V_START) && wait_ready(lun)) {
                return;
            }
            probe_done(lun, false);
            return;
        }
        case LUN_MODE_SENSE: {
            if (v == V_OK) {
                size_t off = msc->cmdset == CMDSET_SCSI ? 2 : 3;
                lun->write_protected = cmd->actual > off
                    && (lun->buf[off] & 0x80) != 0;
            } else if ((v == V_RETRY || v == V_CHANGED) && ++lun->tries < 2) {
                probe_mode_sense(lun);
                return;
            } else if (v == V_FAIL) {
                lun->no_mode_sense = true;
            }
            probe_done(lun, true);
            return;
        }
        default: {
            return;
        }
    }
}

/* Requests */

static void io_issue(struct tendril_msc_lun *lun);

static void io_finish(struct tendril_msc_lun *lun, int st) {
    struct tendril_msc_request *req = lun->current;
    lun->current = NULL;
    lun->busy = false;
    req->status = st;
    req->done(req);
    lun_next(lun);
}

/*
 * Fails the current and all queued requests, which were meant for a medium
 * that is gone.
 */
static void io_fail_all(struct tendril_msc_lun *lun, int st) {
    struct tendril_list list;
    struct tendril_msc_request *cur = lun->current;

    list_init(&list);
    while (!list_empty(&lun->requests)) {
        struct tendril_list *node = lun->requests.next;
        list_del(node);
        list_add_tail(&list, node);
    }
    if (cur != NULL) {
        lun->current = NULL;
        lun->busy = false;
        cur->status = st;
        cur->done(cur);
    }
    while (!list_empty(&list)) {
        struct tendril_msc_request *req = CONTAINER_OF(
            list.next, struct tendril_msc_request, internal.node
        );
        list_del(&req->internal.node);
        req->status = st;
        req->done(req);
    }
}

static void io_issue(struct tendril_msc_lun *lun) {
    struct tendril_msc *msc = lun->msc;
    struct tendril_msc_request *req = lun->current;
    uint8_t cdb[16] = { 0 };

    if (req->op == TENDRIL_MSC_FLUSH) {
        if (!lun->present) {
            io_finish(lun, TENDRIL_ERR_NO_MEDIUM);
        } else if (lun->no_sync_cache || (msc->quirks & MQ_NO_SYNC_CACHE) != 0
                || msc->cmdset == CMDSET_UFI || lun->write_protected) {
            io_finish(lun, TENDRIL_OK);
        } else {
            cdb[0] = SCSI_SYNC_CACHE10;
            lun_issue(
                lun, LUN_FLUSH, cdb, 10, NULL, 0, false, LONG_TIMEOUT_MS
            );
        }
        return;
    }
    if (!lun->present) {
        io_finish(lun, TENDRIL_ERR_NO_MEDIUM);
        return;
    }

    bool write = req->op == TENDRIL_MSC_WRITE;
    size_t left = req->count - req->internal.done_blocks;
    uint32_t n = (uint32_t)MIN((size_t)left, msc->max_bytes / lun->block_size);
    uint64_t lba = req->lba + req->internal.done_blocks;
    size_t len;
    if (n == 0) {
        n = 1;
    }
    if (lba + n > (uint64_t)0x100000000 && msc->cmdset == CMDSET_SCSI
     && !msc->pad12) {
        cdb[0] = write ? SCSI_WRITE16 : SCSI_READ16;
        put_be64(cdb + 2, lba);
        put_be32(cdb + 10, n);
        len = 16;
    } else {
        n = MIN(n, (uint32_t)0xffff);
        if (lba + n > (uint64_t)0x100000000) {
            io_finish(lun, TENDRIL_ERR_INVALID);
            return;
        }
        cdb[0] = write ? SCSI_WRITE10 : SCSI_READ10;
        put_be32(cdb + 2, (uint32_t)lba);
        put_be16(cdb + 7, (uint16_t)n);
        len = 10;
    }
    req->internal.chunk = n;
    uint8_t *data = (uint8_t *)req->buffer
        + (size_t)req->internal.done_blocks * lun->block_size;
    lun_issue(
        lun, LUN_IO, cdb, len, data, (size_t)n * lun->block_size, !write,
        CMD_TIMEOUT_MS
    );
}

static void io_result(struct tendril_msc_lun *lun, int v, int st) {
    struct tendril_msc_request *req = lun->current;
    struct msc_cmd *cmd = &lun->cmd;
    struct tendril_msc *msc = lun->msc;

    if (lun->step == LUN_IO_START) {
        io_issue(lun);
        return;
    }
    if (v == V_OK && lun->step == LUN_IO && cmd->actual < cmd->len) {
        v = V_RETRY;
        st = TENDRIL_ERR_SHORT;
    }
    switch (v) {
        case V_OK: {
            lun->ready_deadline = 0;
            req->internal.retries = 0;
            if (lun->step == LUN_FLUSH) {
                io_finish(lun, TENDRIL_OK);
                return;
            }
            req->internal.done_blocks += req->internal.chunk;
            if (req->internal.done_blocks == req->count) {
                io_finish(lun, TENDRIL_OK);
            } else {
                io_issue(lun);
            }
            return;
        }
        case V_RETRY: {
            if (++req->internal.retries <= MAX_RETRIES) {
                io_issue(lun);
            } else {
                io_finish(lun, st);
            }
            return;
        }
        case V_START:
        case V_WAIT: {
            if (v == V_START && (msc->quirks & MQ_NO_START_STOP) == 0
             && ++req->internal.retries <= MAX_RETRIES) {
                issue_start(lun, LUN_IO_START);
                return;
            }
            if (wait_ready(lun)) {
                return;
            }
            io_finish(lun, TENDRIL_ERR_TIMEOUT);
            return;
        }
        case V_NO_MEDIUM:
        case V_CHANGED: {
            /* Whatever was queued was meant for the old medium. */
            lun->changed = true;
            lun->probe_pending = true;
            io_fail_all(lun, st);
            lun_next(lun);
            return;
        }
        default: {
            if (lun->step == LUN_FLUSH && st == TENDRIL_ERR_INVALID) {
                lun->no_sync_cache = true;
                st = TENDRIL_OK;
            } else if (st == TENDRIL_ERR_WRITE_PROTECTED) {
                lun->write_protected = true;
            }
            io_finish(lun, st);
            return;
        }
    }
}

static void lun_cmd_done(struct msc_cmd *cmd) {
    struct tendril_msc_lun *lun = cmd->ctx;
    int st;
    int v = judge(cmd, &st);

    if (st == TENDRIL_ERR_NO_DEVICE) {
        /*
         * Gone, or its controller died: nothing more will run, though the
         * disconnection may come later.
         */
        lun->msc->stopped = true;
        io_fail_all(lun, st);
        if (lun->check_done != NULL) {
            void (*done)(void *, int) = lun->check_done;
            lun->check_done = NULL;
            done(lun->check_ctx, st);
        }
        return;
    }
    if (lun->step >= LUN_IO) {
        io_result(lun, v, st);
    } else {
        probe_result(lun, v, st);
    }
}

static void lun_timer_work(struct tendril_work *work) {
    struct tendril_msc_lun *lun = CONTAINER_OF(
        work, struct tendril_msc_lun, timer.work
    );
    if (lun->msc->stopped) {
        return;
    }
    if (lun->step >= LUN_IO) {
        io_issue(lun);
    } else {
        probe_test(lun);
    }
}

static void lun_next(struct tendril_msc_lun *lun) {
    if (lun->busy || lun->msc->stopped) {
        return;
    }
    if (lun->probe_pending) {
        lun->probe_pending = false;
        probe_start(lun, false);
        return;
    }
    if (list_empty(&lun->requests)) {
        return;
    }
    lun->current = CONTAINER_OF(
        lun->requests.next, struct tendril_msc_request, internal.node
    );
    list_del(&lun->current->internal.node);
    lun->busy = true;
    lun->ready_deadline = 0;
    io_issue(lun);
}

/* Setup */

static void setup_step(struct tendril_msc *msc, int st);
static void get_max_lun(struct tendril_msc *msc);

static void setup_done(void *ctx, int st) {
    setup_step(ctx, st);
}

static void setup_async_work(struct tendril_work *work) {
    struct tendril_msc *msc = CONTAINER_OF(
        work, struct tendril_msc, async.work
    );
    setup_step(msc, msc->async.status);
}

static void setup_fail(struct tendril_msc *msc, int st, const char *what) {
    if (st != TENDRIL_ERR_NO_DEVICE) {
        LOG_WARN("MSC: %s failed: %s", what, tendril_status_str(st));
    }
    msc->stopped = true;
}

/* BEGIN FreeBSD sys/dev/usb/storage/umass.c */
static bool command_set(int subclass, uint32_t quirks, int *cs) {
    if ((quirks & MQ_CMD_SCSI) != 0) {
        *cs = CMDSET_SCSI;
    } else if ((quirks & MQ_CMD_ATAPI) != 0) {
        *cs = CMDSET_ATAPI;
    } else if ((quirks & MQ_CMD_UFI) != 0) {
        *cs = CMDSET_UFI;
    } else if ((quirks & MQ_CMD_RBC) != 0) {
        *cs = CMDSET_RBC;
    } else {
        switch (subclass) {
            case SUBCLASS_NOT_REPORTED:
            case SUBCLASS_SCSI: {
                *cs = CMDSET_SCSI;
                break;
            }
            case SUBCLASS_RBC: {
                *cs = CMDSET_RBC;
                break;
            }
            case SUBCLASS_MMC:
            case SUBCLASS_QIC157:
            case SUBCLASS_SFF8070I: {
                *cs = CMDSET_ATAPI;
                break;
            }
            case SUBCLASS_UFI: {
                *cs = CMDSET_UFI;
                break;
            }
            default: {
                return false;
            }
        }
    }
    return true;
}

static bool wire_protocol(int protocol, uint32_t quirks, int *t) {
    if ((quirks & MQ_WIRE_BBB) != 0) {
        *t = TRANSPORT_BBB;
    } else if ((quirks & MQ_WIRE_CBI) != 0) {
        *t = TRANSPORT_CBI;
    } else if ((quirks & MQ_WIRE_CB) != 0) {
        *t = TRANSPORT_CB;
    } else if (protocol == PROTO_BBB || protocol == PROTO_BBB_OLD) {
        *t = TRANSPORT_BBB;
    } else if (protocol == PROTO_CBI) {
        *t = TRANSPORT_CBI;
    } else if (protocol == PROTO_CB) {
        *t = TRANSPORT_CB;
    } else {
        return false;
    }
    return true;
}
/* END FreeBSD sys/dev/usb/storage/umass.c */

/* The pipe usage IDs found in an alternate setting, as a mask. */
/* BEGIN SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */
static uint32_t alt_pipes(struct tendril_interface *intf, size_t alt) {
    const uint8_t *p = (const uint8_t *)intf->alts[alt];
    const uint8_t *end = p + intf->alt_sizes[alt];
    uint32_t mask = 0;
    for (p = tendril_desc_next(p, end); p != NULL;
         p = tendril_desc_next(p, end)) {
        if (p[1] == DT_PIPE_USAGE && p[0] >= 4 && p[2] >= PIPE_COMMAND
         && p[2] <= PIPE_DATA_OUT) {
            mask |= PIPE_BIT(p[2]);
        }
    }
    return mask;
}

/* The pipe usage ID following an endpoint's descriptor, or 0. */
static uint8_t pipe_id(
    struct tendril_interface *intf, struct tendril_endpoint *ep
) {
    size_t size;
    const uint8_t *start = tendril_interface_descriptors(intf, &size);
    const uint8_t *end = start + size;
    const uint8_t *first = tendril_desc_next((const uint8_t *)ep->desc, end);
    for (const uint8_t *p = first; p != NULL; p = tendril_desc_next(p, end)) {
        if (p[1] == TENDRIL_DT_ENDPOINT || p[1] == TENDRIL_DT_INTERFACE) {
            break;
        }
        if (p[1] == DT_PIPE_USAGE && p[0] >= 4) {
            return p[2];
        }
    }
    return 0;
}
/* END SerenityOS Kernel/Devices/Storage/USB/UAS/Structures.h */

static size_t max_transfer(struct tendril_msc *msc) {
    bool ss = tendril_device_is_ss(msc->dev);
    if (msc->transport == TRANSPORT_UAS) {
        /*
         * Within a 256-TRB stream ring even when every page is mapped apart.
         */
        return ss ? 512 * 1024 : 256 * 1024;
    }
    /* BEGIN FreeBSD sys/dev/usb/storage/umass.c, OpenBSD sys/dev/usb/umass.c */
    /*
     * What FreeBSD's umass.c allows: maxphys (1 MiB) at SuperSpeed, else CAM's
     * 64 KiB default.
     */
    return ss ? 1024 * 1024 : 64 * 1024;
    /* END FreeBSD sys/dev/usb/storage/umass.c, OpenBSD sys/dev/usb/umass.c */
}

static void create_luns(struct tendril_msc *msc, size_t count) {
    msc->max_bytes = max_transfer(msc);
    msc->num_luns = count;
    const char *transport = msc->transport == TRANSPORT_UAS
        ? (msc->streams ? "UAS with streams" : "UAS")
        : msc->transport == TRANSPORT_BBB ? "Bulk-Only"
        : msc->transport == TRANSPORT_CBI ? "CBI"
        : "CB";
    LOG_INFO(
        "MSC: interface %u: %s, %zu LUN%s", msc->intf->number, transport,
        count, count == 1 ? "" : "s"
    );
    for (size_t i = 0; i < count; i++) {
        struct tendril_msc_lun *lun = tendril_zalloc(
            sizeof(struct tendril_msc_lun)
        );
        if (lun == NULL) {
            break;
        }
        lun->msc = msc;
        lun->number = (uint8_t)i;
        list_init(&lun->requests);
        list_init(&lun->cmd.node);
        tendril_timer_init(&lun->timer, lun_timer_work);
        msc->luns[i] = lun;
    }
    for (size_t i = 0; i < count && !msc->stopped; i++) {
        if (msc->luns[i] != NULL) {
            probe_start(msc->luns[i], true);
        }
    }
}

static bool legacy_endpoints(struct tendril_msc *msc) {
    struct tendril_interface *intf = msc->intf;
    msc->bulk_in = tendril_interface_find_endpoint(
        intf, TENDRIL_ENDPOINT_XFER_BULK, TENDRIL_DIR_IN
    );
    msc->bulk_out = tendril_interface_find_endpoint(
        intf, TENDRIL_ENDPOINT_XFER_BULK, TENDRIL_DIR_OUT
    );
    msc->intr_in = tendril_interface_find_endpoint(
        intf, TENDRIL_ENDPOINT_XFER_INT, TENDRIL_DIR_IN
    );
    if (msc->transport == TRANSPORT_CBI && msc->intr_in == NULL) {
        msc->transport = TRANSPORT_CB;
    }
    return msc->bulk_in != NULL && msc->bulk_out != NULL;
}

static bool uas_endpoints(struct tendril_msc *msc) {
    struct tendril_interface *intf = msc->intf;
    msc->cmd_ep = NULL;
    msc->status_ep = NULL;
    msc->data_in_ep = NULL;
    msc->data_out_ep = NULL;
    for (size_t e = 0; e < intf->num_endpoints; e++) {
        struct tendril_endpoint *ep = &intf->endpoints[e];
        bool in = (ep->address & TENDRIL_ENDPOINT_DIR_MASK) != 0;
        if (ep->type != TENDRIL_ENDPOINT_XFER_BULK) {
            continue;
        }
        switch (pipe_id(intf, ep)) {
            case PIPE_COMMAND: {
                msc->cmd_ep = in ? NULL : ep;
                break;
            }
            case PIPE_STATUS: {
                msc->status_ep = in ? ep : NULL;
                break;
            }
            case PIPE_DATA_IN: {
                msc->data_in_ep = in ? ep : NULL;
                break;
            }
            case PIPE_DATA_OUT: {
                msc->data_out_ep = in ? NULL : ep;
                break;
            }
            default: {
                break;
            }
        }
    }
    return msc->cmd_ep != NULL && msc->status_ep != NULL
        && msc->data_in_ep != NULL && msc->data_out_ep != NULL;
}

static void setup_luns(struct tendril_msc *msc) {
    if ((msc->quirks & MQ_SHUTTLE_INIT) != 0) {
        msc->setup = SETUP_SHUTTLE;
        int st = tendril_msc_shuttle_init(
            msc->dev, msc->intf->number, &msc->ctrl, msc->ctrl_buf,
            sizeof(msc->ctrl_buf), &msc->async
        );
        if (st != TENDRIL_OK) {
            setup_step(msc, st);
        }
        return;
    }
    get_max_lun(msc);
}

/* BEGIN FreeBSD sys/dev/usb/usb_msctest.c, usb_device.c */
static const uint8_t pre_inquiry_cdb[6] = { SCSI_INQUIRY, 0, 0, 0, 36, 0 };

static void pre_done(struct msc_cmd *cmd);

static void pre_issue(
    struct tendril_msc *msc, int step, const uint8_t *cdb, void *data,
    size_t len
) {
    struct msc_cmd *cmd = &msc->pre_cmd;

    msc->pre = step;
    memset(cmd->cdb, 0, sizeof(cmd->cdb));
    memcpy(cmd->cdb, cdb, 6);
    cmd->cdb_len = 6;
    cmd->lun = 0;
    cmd->in = len != 0;
    cmd->data = data;
    cmd->len = len;
    cmd->timeout_ms = PRE_TIMEOUT_MS;
    cmd->done = pre_done;
    cmd->ctx = msc;
    cmd_queue(msc, cmd);
}

/* Starts the next usb_msctest.c check; false once none is left. */
static bool pre_start(struct tendril_msc *msc) {
    static const uint8_t test_cdb[6] = { SCSI_TEST_UNIT_READY, 0, 0, 0, 0, 0 };

    if (msc->pre_cdrom) {
        msc->pre_cdrom = false;
        msc->pre_tries = 0;
        pre_issue(
            msc, PRE_CDROM, pre_inquiry_cdb, msc->pre_buf, sizeof(msc->pre_buf)
        );
        return true;
    }
    if (msc->pre_eject) {
        msc->pre_eject = false;
        pre_issue(msc, PRE_EJECT_TEST, test_cdb, NULL, 0);
        return true;
    }
    return false;
}

static void pre_timer_work(struct tendril_work *work) {
    struct tendril_msc *msc = CONTAINER_OF(
        work, struct tendril_msc, pre_timer.work
    );
    if (!msc->stopped) {
        pre_issue(
            msc, PRE_CDROM, pre_inquiry_cdb, msc->pre_buf, sizeof(msc->pre_buf)
        );
    }
}

/*
 * Moves on to configuration index 1, as usb_alloc_device() does past an
 * auto-install CD-ROM.
 */
static bool skip_cdrom(struct tendril_msc *msc) {
    struct tendril_device *dev = msc->dev;

    if (dev->num_configs < 2 || dev->configs[1] == NULL) {
        return false;
    }
    const struct tendril_config_descriptor *cd = (const void *)dev->configs[1];
    int st = tendril_device_set_configuration(
        dev, cd->configuration_value, NULL, NULL
    );
    if (st != TENDRIL_OK) {
        LOG_WARN(
            "MSC: leaving the auto-install CD-ROM configuration failed: %s",
            tendril_status_str(st)
        );
        return false;
    }
    LOG_INFO(
        "MSC: interface %u looks like an auto-install CD-ROM; trying "
        "configuration %u",
        msc->intf->number, cd->configuration_value
    );
    msc->stopped = true;
    return true;
}

static void pre_done(struct msc_cmd *cmd) {
    static const uint8_t stop_cdb[6] = {
        SCSI_START_STOP_UNIT, 0, 0, 0, 0x02, 0
    };
    struct tendril_msc *msc = cmd->ctx;
    struct tendril_device *dev = msc->dev;

    if (cmd->status == TENDRIL_ERR_NO_DEVICE || msc->stopped || dev->gone) {
        msc->stopped = true;
        return;
    }
    switch (msc->pre) {
        case PRE_CDROM: {
            bool good = cmd->status == TENDRIL_OK
                && cmd->scsi_status == STATUS_GOOD;
            /*
             * A command that failed on the device is tried again a second
             * later, three times in all.
             */
            if (cmd->status == TENDRIL_OK && !good
             && ++msc->pre_tries < PRE_TRIES) {
                hc_lock(msc->hc);
                tendril_timer_arm(
                    msc->hc, &msc->pre_timer, PRE_RETRY_MS * NS_PER_MS
                );
                hc_unlock(msc->hc);
                return;
            }
            if (good && cmd->actual > 0
             && (msc->pre_buf[0] & 0x1f) == TYPE_CDROM && skip_cdrom(msc)) {
                return;
            }
            break;
        }
        case PRE_EJECT_TEST: {
            /* What either command comes to does not matter. */
            pre_issue(msc, PRE_EJECT_STOP, stop_cdb, NULL, 0);
            return;
        }
        case PRE_EJECT_STOP: {
            LOG_INFO(
                "MSC: interface %u: ejected, for the device to come back in "
                "its other mode",
                msc->intf->number
            );
            msc->stopped = true;
            int st = tendril_device_set_configuration(dev, 0, NULL, NULL);
            if (st != TENDRIL_OK) {
                LOG_WARN(
                    "MSC: unconfiguring the ejected device failed: %s",
                    tendril_status_str(st)
                );
            }
            return;
        }
        default: {
            return;
        }
    }
    msc->pre = PRE_NONE;
    if (!pre_start(msc)) {
        setup_luns(msc);
    }
}
/* END FreeBSD sys/dev/usb/usb_msctest.c, usb_device.c */

/*
 * Bulk-Only and CBI setup past the alternate setting: endpoints, the
 * usb_msctest.c checks, and the LUNs.
 */
static void setup_legacy(struct tendril_msc *msc) {
    /* BEGIN FreeBSD sys/dev/usb/storage/umass.c */
    /*
     * UFI and ATAPI commands are 12 bytes long; RBC ones only with
     * MQ_PAD_TO_12, as in FreeBSD's umass.c.
     */
    msc->pad12 = msc->cmdset == CMDSET_ATAPI || msc->cmdset == CMDSET_UFI
        || (msc->cmdset == CMDSET_RBC && (msc->quirks & MQ_PAD_TO_12) != 0);
    /* END FreeBSD sys/dev/usb/storage/umass.c */
    if (!legacy_endpoints(msc)) {
        setup_fail(msc, TENDRIL_ERR_UNSUPPORTED, "finding the bulk endpoints");
        return;
    }
    if (msc->transport != TRANSPORT_BBB || !pre_start(msc)) {
        setup_luns(msc);
    }
}

static void get_max_lun(struct tendril_msc *msc) {
    msc->setup = SETUP_MAX_LUN;
    if (msc->transport != TRANSPORT_BBB
     || (msc->quirks & MQ_NO_GETMAXLUN) != 0) {
        create_luns(msc, 1);
        return;
    }
    msc->ctrl_buf[0] = 0;
    int st = tendril_control_async(
        msc->dev, &msc->ctrl,
        TENDRIL_DIR_IN | TENDRIL_TYPE_CLASS | TENDRIL_RECIP_INTERFACE,
        REQ_GET_MAX_LUN, 0, msc->intf->number, msc->ctrl_buf, 1, &msc->async
    );
    if (st != TENDRIL_OK) {
        setup_step(msc, st);
    }
}

static void fall_back(struct tendril_msc *msc, int st) {
    if (msc->fallback_alt < 0) {
        setup_fail(msc, st, "UAS setup");
        return;
    }
    LOG_INFO(
        "MSC: UAS setup failed (%s), using Bulk-Only", tendril_status_str(st)
    );
    msc->transport = msc->fallback_transport;
    msc->cmdset = msc->fallback_cmdset;
    msc->alt = (size_t)msc->fallback_alt;
    msc->fallback_alt = -1;
    msc->streams = false;
    msc->setup = SETUP_FALLBACK;
    st = tendril_interface_set_alt(
        msc->intf, msc->intf->alts[msc->alt]->alternate_setting, setup_done,
        msc
    );
    if (st != TENDRIL_OK) {
        setup_fail(msc, st, "selecting the Bulk-Only setting");
    }
}

static void uas_ready(struct tendril_msc *msc) {
    if (msc->streams) {
        /*
         * Stream IDs 1 to n: one per command tag, and the last for task
         * management.
         */
        msc->num_tags = MIN((size_t)msc->num_streams - 1, (size_t)MAX_TAGS);
        msc->tm_tag = (uint16_t)(msc->num_tags + 1);
    } else {
        msc->num_tags = MAX_TAGS;
        msc->tm_tag = MAX_TAGS + 1;
    }
    msc->next_tag = 1;
    create_luns(msc, 1);
}

static void setup_step(struct tendril_msc *msc, int st) {
    struct tendril_interface *intf = msc->intf;

    if (msc->stopped) {
        return;
    }
    if (st == TENDRIL_ERR_NO_DEVICE || intf->dev->gone) {
        msc->stopped = true;
        return;
    }
    switch (msc->setup) {
        case SETUP_ALT: {
            if (msc->transport != TRANSPORT_UAS) {
                if (st != TENDRIL_OK) {
                    setup_fail(msc, st, "selecting the alternate setting");
                } else {
                    setup_legacy(msc);
                }
                break;
            }
            if (st != TENDRIL_OK || !uas_endpoints(msc)) {
                fall_back(
                    msc, st != TENDRIL_OK ? st : TENDRIL_ERR_UNSUPPORTED
                );
                break;
            }
            if (!tendril_device_is_ss(msc->dev)) {
                uas_ready(msc);
                break;
            }
            struct tendril_endpoint *eps[3] = {
                msc->status_ep, msc->data_in_ep, msc->data_out_ep
            };
            msc->num_streams = MAX_TAGS + 1;
            msc->setup = SETUP_STREAMS;
            st = tendril_endpoint_alloc_streams(
                eps, 3, &msc->num_streams, setup_done, msc
            );
            if (st == TENDRIL_OK && msc->num_streams < 2) {
                st = TENDRIL_ERR_UNSUPPORTED;
            }
            if (st != TENDRIL_OK) {
                fall_back(msc, st);
            }
            break;
        }
        case SETUP_STREAMS: {
            if (st != TENDRIL_OK) {
                fall_back(msc, st);
                break;
            }
            msc->streams = true;
            uas_ready(msc);
            break;
        }
        case SETUP_FALLBACK: {
            if (st != TENDRIL_OK) {
                setup_fail(msc, st, "selecting the Bulk-Only setting");
                break;
            }
            setup_legacy(msc);
            break;
        }
        case SETUP_SHUTTLE: {
            get_max_lun(msc);
            break;
        }
        case SETUP_MAX_LUN: {
            /*
             * Bulk-Only 1.0, 3.2: devices with one LUN may stall the request.
             */
            size_t max = st == TENDRIL_OK && msc->ctrl.actual == 1
                ? msc->ctrl_buf[0]
                : 0;
            create_luns(msc, max < MAX_LUNS ? max + 1 : 1);
            break;
        }
        default: {
            break;
        }
    }
}

static struct tendril_driver msc_driver;

static bool msc_probe(struct tendril_interface *intf) {
    const struct tendril_interface_descriptor *id
        = tendril_interface_descriptor(intf);
    struct tendril_device *dev = intf->dev;
    struct tendril_hc *hc = dev->hc;

    if (id->interface_class != TENDRIL_CLASS_MASS_STORAGE) {
        return false;
    }
    uint32_t quirks = tendril_id_quirks(
        msc_quirks, ARRAY_SIZE(msc_quirks), &dev->desc
    );
    if ((quirks & MQ_IGNORE) != 0) {
        return false;
    }

    /*
     * The Bulk-Only or CBI setting: the current one, or alternate setting 1
     * with MQ_ALT_IFACE_1.
     */
    size_t legacy = intf->cur_alt;
    if ((quirks & MQ_ALT_IFACE_1) != 0 && intf->num_alts > 1) {
        legacy = 1;
    }
    const struct tendril_interface_descriptor *ld = intf->alts[legacy];
    int wire = TRANSPORT_BBB;
    int cmdset = CMDSET_SCSI;
    bool have_legacy = ld->interface_class == TENDRIL_CLASS_MASS_STORAGE
        && wire_protocol(ld->interface_protocol, quirks, &wire)
        && command_set(ld->interface_subclass, quirks, &cmdset);
    if (!have_legacy) {
        /* Failing that, any alternate setting speaking Bulk-Only or CBI. */
        for (size_t a = 0; a < intf->num_alts && !have_legacy; a++) {
            const struct tendril_interface_descriptor *d = intf->alts[a];
            if (d->interface_class == TENDRIL_CLASS_MASS_STORAGE
             && wire_protocol(d->interface_protocol, 0, &wire)
             && command_set(d->interface_subclass, quirks, &cmdset)) {
                legacy = a;
                have_legacy = true;
            }
        }
    }

    /*
     * UAS needs high speed, and streams at SuperSpeed; forced wire protocols
     * rule it out.
     */
    int uas = -1;
    bool streams_ok = hc->ops->alloc_streams != NULL
        && (hc->quirks & TENDRIL_HC_QUIRK_NO_STREAMS) == 0;
    bool uas_ok = dev->speed >= TENDRIL_SPEED_HIGH
        && (!tendril_device_is_ss(dev) || streams_ok)
        && (quirks & (MQ_WIRE_BBB | MQ_WIRE_CB | MQ_WIRE_CBI)) == 0;
    for (size_t a = 0; a < intf->num_alts && uas_ok && uas < 0; a++) {
        const struct tendril_interface_descriptor *d = intf->alts[a];
        if (d->interface_class == TENDRIL_CLASS_MASS_STORAGE
         && d->interface_subclass == SUBCLASS_SCSI
         && d->interface_protocol == PROTO_UAS
         && alt_pipes(intf, a) == UAS_PIPES) {
            uas = (int)a;
        }
    }
    if (uas < 0 && !have_legacy) {
        return false;
    }

    struct tendril_msc *msc = tendril_zalloc(sizeof(struct tendril_msc));
    if (msc == NULL) {
        return false;
    }
    msc->intf = intf;
    msc->dev = dev;
    msc->hc = hc;
    msc->quirks = quirks;
    msc->fallback_alt = -1;
    list_init(&msc->queue);
    list_init(&msc->pre_cmd.node);
    tendril_async_init(&msc->async, setup_async_work);
    tendril_timer_init(&msc->pre_timer, pre_timer_work);
    /* BEGIN FreeBSD sys/dev/usb/usb_msctest.c, usb_device.c */
    /*
     * usb_msctest.c only checks a first interface that speaks Bulk-Only with
     * these command sets (bbb_attach).
     */
    if (intf == &dev->interfaces[0] && uas < 0) {
        bool bbb = (id->interface_protocol == PROTO_BBB
            || id->interface_protocol == PROTO_BBB_OLD)
            && (id->interface_subclass == SUBCLASS_SCSI
                || id->interface_subclass == SUBCLASS_UFI
                || id->interface_subclass == SUBCLASS_MMC
                || id->interface_subclass == SUBCLASS_SFF8070I);
        msc->pre_cdrom = bbb && dev->cdrom_check && dev->active_config == 0
            && (quirks & MQ_NO_INQUIRY) == 0;
        msc->pre_eject = bbb && (quirks & MQ_SCSI_EJECT) != 0;
    }
    /* END FreeBSD sys/dev/usb/usb_msctest.c, usb_device.c */
    if (intf == &dev->interfaces[0]) {
        dev->cdrom_check = false;
    }
    if (uas >= 0) {
        msc->transport = TRANSPORT_UAS;
        msc->cmdset = CMDSET_SCSI;
        msc->alt = (size_t)uas;
        if (have_legacy) {
            msc->fallback_alt = (int)legacy;
            msc->fallback_transport = wire;
            msc->fallback_cmdset = cmdset;
        }
    } else {
        msc->transport = wire;
        msc->cmdset = cmdset;
        msc->alt = legacy;
    }
    tendril_interface_set_data(intf, msc);

    msc->setup = SETUP_ALT;
    if (msc->alt == intf->cur_alt) {
        setup_step(msc, TENDRIL_OK);
        return true;
    }
    int st = tendril_interface_set_alt(
        intf, intf->alts[msc->alt]->alternate_setting, setup_done, msc
    );
    if (st != TENDRIL_OK) {
        setup_step(msc, st);
    }
    return true;
}

static void msc_disconnect(struct tendril_interface *intf) {
    struct tendril_msc *msc = tendril_interface_data(intf);
    if (msc == NULL) {
        return;
    }
    msc->stopped = true;
    hc_lock(msc->hc);
    tendril_timer_cancel(msc->hc, &msc->pre_timer);
    hc_unlock(msc->hc);
    while (!list_empty(&msc->queue)) {
        struct msc_cmd *cmd = CONTAINER_OF(
            msc->queue.next, struct msc_cmd, node
        );
        list_del(&cmd->node);
        cmd->status = TENDRIL_ERR_NO_DEVICE;
        cmd->done(cmd);
    }
    for (size_t i = 0; i < msc->num_luns; i++) {
        struct tendril_msc_lun *lun = msc->luns[i];
        if (lun == NULL) {
            continue;
        }
        hc_lock(msc->hc);
        tendril_timer_cancel(msc->hc, &lun->timer);
        hc_unlock(msc->hc);
        io_fail_all(lun, TENDRIL_ERR_NO_DEVICE);
        if (lun->check_done != NULL) {
            lun->check_done(lun->check_ctx, TENDRIL_ERR_NO_DEVICE);
            lun->check_done = NULL;
        }
        if (lun->attached && callbacks->detach != NULL) {
            callbacks->detach(lun);
        }
        tendril_free(lun, sizeof(struct tendril_msc_lun));
        msc->luns[i] = NULL;
    }
    tendril_interface_set_data(intf, NULL);
    tendril_free(msc, sizeof(struct tendril_msc));
}

/*
 * Set up, with no command, request, or medium check outstanding: nothing is
 * cut short by a suspend.
 */
static bool msc_idle(struct tendril_msc *msc) {
    if (msc->num_luns == 0 || msc->active != NULL || !list_empty(&msc->queue)
     || msc->in_flight != 0 || msc->tm_busy) {
        return false;
    }
    for (size_t i = 0; i < msc->num_luns; i++) {
        struct tendril_msc_lun *lun = msc->luns[i];
        if (lun != NULL
         && (lun->busy || lun->probe_pending || lun->current != NULL
          || !list_empty(&lun->requests) || lun->check_done != NULL
          || tendril_timer_armed(&lun->timer))) {
            return false;
        }
    }
    return true;
}

static bool msc_suspend(struct tendril_interface *intf) {
    struct tendril_msc *msc = tendril_interface_data(intf);
    if (msc == NULL || msc->stopped || msc->broken) {
        return true;
    }
    hc_lock(msc->hc);
    bool idle = msc_idle(msc);
    hc_unlock(msc->hc);
    if (!idle) {
        return false;
    }
    msc->suspended = true;
    return true;
}

static void msc_resume(struct tendril_interface *intf) {
    struct tendril_msc *msc = tendril_interface_data(intf);
    if (msc != NULL) {
        /*
         * A medium swapped meanwhile shows up as a unit attention on the next
         * command.
         */
        msc->suspended = false;
    }
}

static struct tendril_driver msc_driver = {
    .name = "msc",
    .probe = msc_probe,
    .disconnect = msc_disconnect,
    .suspend = msc_suspend,
    .resume = msc_resume
};

void tendril_msc_register(const struct tendril_msc_callbacks *cb) {
    callbacks = cb;
    tendril_driver_register(&msc_driver);
}

/* Accessors */

struct tendril_device *tendril_msc_device(struct tendril_msc_lun *lun) {
    return lun->msc->dev;
}

unsigned int tendril_msc_lun_number(struct tendril_msc_lun *lun) {
    return lun->number;
}

int tendril_msc_type(struct tendril_msc_lun *lun) {
    return lun->type;
}

bool tendril_msc_removable(struct tendril_msc_lun *lun) {
    return lun->removable;
}

bool tendril_msc_present(struct tendril_msc_lun *lun) {
    return lun->present;
}

bool tendril_msc_write_protected(struct tendril_msc_lun *lun) {
    return lun->write_protected;
}

uint64_t tendril_msc_blocks(struct tendril_msc_lun *lun) {
    return lun->blocks;
}

size_t tendril_msc_block_size(struct tendril_msc_lun *lun) {
    return lun->block_size;
}

const char *tendril_msc_vendor(struct tendril_msc_lun *lun) {
    return lun->vendor;
}

const char *tendril_msc_product(struct tendril_msc_lun *lun) {
    return lun->product;
}

const char *tendril_msc_revision(struct tendril_msc_lun *lun) {
    return lun->revision;
}

void tendril_msc_set_user(struct tendril_msc_lun *lun, void *user) {
    lun->user = user;
}

void *tendril_msc_user(struct tendril_msc_lun *lun) {
    return lun->user;
}

/* Requests */

int tendril_msc_submit(
    struct tendril_msc_lun *lun, struct tendril_msc_request *req
) {
    struct tendril_msc *msc = lun->msc;

    if (msc->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (msc->suspended) {
        return TENDRIL_ERR_SUSPENDED;
    }
    if (req->op > TENDRIL_MSC_FLUSH || req->done == NULL) {
        return TENDRIL_ERR_INVALID;
    }
    if (req->op != TENDRIL_MSC_FLUSH) {
        if (!lun->present) {
            return TENDRIL_ERR_NO_MEDIUM;
        }
        if (req->count == 0 || req->buffer == NULL || req->lba >= lun->blocks
         || req->count > lun->blocks - req->lba) {
            return TENDRIL_ERR_INVALID;
        }
        if (req->op == TENDRIL_MSC_WRITE && lun->write_protected) {
            return TENDRIL_ERR_WRITE_PROTECTED;
        }
    }
    req->status = TENDRIL_OK;
    req->internal.lun = lun;
    req->internal.done_blocks = 0;
    req->internal.chunk = 0;
    req->internal.retries = 0;
    list_add_tail(&lun->requests, &req->internal.node);
    lun_next(lun);
    return TENDRIL_OK;
}

struct sync_wait {
    volatile bool done;
    int status;
};

static void sync_request_done(struct tendril_msc_request *req) {
    struct sync_wait *w = req->context;
    w->status = req->status;
    w->done = true;
}

static int sync_request(
    struct tendril_msc_lun *lun, int op, uint64_t lba, size_t count, void *buf
) {
    struct tendril_hc *hc = lun->msc->hc;
    struct tendril_msc_request req;
    struct sync_wait w;

    memset(&req, 0, sizeof(struct tendril_msc_request));
    w.done = false;
    w.status = TENDRIL_OK;
    req.op = op;
    req.lba = lba;
    req.count = count;
    req.buffer = buf;
    req.done = sync_request_done;
    req.context = &w;
    int st = tendril_msc_submit(lun, &req);
    if (st != TENDRIL_OK) {
        return st;
    }
    tendril_hc_wait(hc, &w.done);
    return w.status;
}

int tendril_msc_read(
    struct tendril_msc_lun *lun, uint64_t lba, size_t count, void *buf
) {
    return sync_request(lun, TENDRIL_MSC_READ, lba, count, buf);
}

int tendril_msc_write(
    struct tendril_msc_lun *lun, uint64_t lba, size_t count, const void *buf
) {
    return sync_request(
        lun, TENDRIL_MSC_WRITE, lba, count, (void *)(uintptr_t)buf
    );
}

int tendril_msc_flush(struct tendril_msc_lun *lun) {
    return sync_request(lun, TENDRIL_MSC_FLUSH, 0, 0, NULL);
}

/* Commands from the embedder */

struct passthrough {
    struct msc_cmd cmd;
    uint8_t *sense;
    void (*done)(void *ctx, int status, int scsi_status, size_t actual);
    void *ctx;
};

static void passthrough_done(struct msc_cmd *cmd) {
    struct passthrough *p = CONTAINER_OF(cmd, struct passthrough, cmd);
    void (*done)(void *, int, int, size_t) = p->done;
    void *ctx = p->ctx;
    int st = cmd->status;
    int scsi_status = cmd->scsi_status;
    size_t actual = cmd->actual;

    if (p->sense != NULL && st == TENDRIL_OK && scsi_status == STATUS_CHECK) {
        memset(p->sense, 0, sizeof(cmd->sense));
        memcpy(p->sense, cmd->sense, cmd->sense_len);
    }
    tendril_free(p, sizeof(struct passthrough));
    if (done != NULL) {
        done(ctx, st, scsi_status, actual);
    }
}

int tendril_msc_command(
    struct tendril_msc_lun *lun, const uint8_t *cdb, size_t cdb_len,
    void *data, size_t len, bool in, uint8_t *sense,
    void (*done)(void *ctx, int status, int scsi_status, size_t actual),
    void *ctx
) {
    struct tendril_msc *msc = lun->msc;

    if (cdb_len == 0 || cdb_len > 16 || (len != 0 && data == NULL)) {
        return TENDRIL_ERR_INVALID;
    }
    if (msc->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (msc->suspended) {
        return TENDRIL_ERR_SUSPENDED;
    }
    struct passthrough *p = tendril_zalloc(sizeof(struct passthrough));
    if (p == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    p->sense = sense;
    p->done = done;
    p->ctx = ctx;
    memcpy(p->cmd.cdb, cdb, cdb_len);
    if (msc->cmdset == CMDSET_UFI) {
        p->cmd.cdb[1] = (uint8_t)((p->cmd.cdb[1] & 0x1f) | lun->number << 5);
    }
    p->cmd.cdb_len = (uint8_t)cdb_len;
    p->cmd.lun = lun->number;
    p->cmd.in = in;
    p->cmd.data = data;
    p->cmd.len = len;
    p->cmd.timeout_ms = LONG_TIMEOUT_MS;
    p->cmd.done = passthrough_done;
    list_init(&p->cmd.node);
    cmd_queue(msc, &p->cmd);
    return TENDRIL_OK;
}

int tendril_msc_check_medium(
    struct tendril_msc_lun *lun, void (*done)(void *ctx, int status), void *ctx
) {
    if (lun->msc->stopped) {
        return TENDRIL_ERR_NO_DEVICE;
    }
    if (lun->msc->suspended) {
        return TENDRIL_ERR_SUSPENDED;
    }
    if (lun->check_done != NULL) {
        return TENDRIL_ERR_BUSY;
    }
    lun->check_done = done;
    lun->check_ctx = ctx;
    lun->probe_pending = true;
    lun_next(lun);
    return TENDRIL_OK;
}
