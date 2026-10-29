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

/* Chipset fixes for PCI EHCI controllers. */

#include "../internal.h"

void tendril_ehci_chipset_fixes(void *ctx) {
    /* Type 0 header (PCI Local Bus Specification Revision 3.0, 6.2.1). */
    uint16_t vendor = tendril_glue_pci_read16(ctx, 0x00);
    uint16_t device = tendril_glue_pci_read16(ctx, 0x02);
    uint8_t revision = tendril_glue_pci_read8(ctx, 0x08);

    /*
     * The SB600's and SB700's EHCI controllers, the SB700's only with some
     * revisions of its SMBus controller. The bit is undocumented: the SB700
     * register reference guide calls it reserved (clean-room specification).
     */
    if (vendor == 0x1002 && (device == 0x4386 || device == 0x4396)) {
        if (device == 0x4396) {
            uint8_t smbus_revision;
            if (!tendril_glue_pci_find_read8(ctx, 0x1002, 0x4385, 0x08, &smbus_revision)
             || (smbus_revision != 0x3a && smbus_revision != 0x3b)) {
                return;
            }
        }
        uint8_t value = tendril_glue_pci_read8(ctx, 0x53);
        if (!(value & 0x08)) {
            tendril_glue_pci_write8(ctx, 0x53, (uint8_t)(value | 0x08));
            LOG_INFO("EHCI: applied the SB600/SB700 fix");
        }
    }

    /*
     * The bit sets the controller's schedule sleep time to 10 microseconds
     * (clean-room specification).
     */
    if (vendor == 0x1106 && device == 0x3104 && (revision & 0xf0) == 0x60) {
        uint8_t value = tendril_glue_pci_read8(ctx, 0x4b);
        if (!(value & 0x20)) {
            tendril_glue_pci_write8(ctx, 0x4b, (uint8_t)(value | 0x20));
            LOG_INFO("EHCI: applied the VIA fix");
        }
    }
}
