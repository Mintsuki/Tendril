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

#ifndef TENDRIL_GLUE_H
#define TENDRIL_GLUE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Functions the embedder provides. Every function taking a `ctx` receives the
 * `ctx` given in the struct tendril_hc_config of the controller concerned. See
 * docs/API.md for the full contract of each function.
 */

#define TENDRIL_LOG_ERROR 0
#define TENDRIL_LOG_WARN 1
#define TENDRIL_LOG_INFO 2
#define TENDRIL_LOG_DEBUG 3
#define TENDRIL_LOG_TRACE 4

#define TENDRIL_DMA_TO_DEVICE 0
#define TENDRIL_DMA_FROM_DEVICE 1
#define TENDRIL_DMA_BIDIRECTIONAL 2

/* General purpose memory. */
void *tendril_glue_alloc(size_t size);
void tendril_glue_free(void *ptr, size_t size);

/*
 * Memory coherent with the controller, aligned to `align` (a power of two),
 * below 4 GiB in bus address space if `dma32` is set.
 */
void *tendril_glue_dma_alloc(void *ctx, size_t size, size_t align, bool dma32, uint64_t *dma);
void tendril_glue_dma_free(void *ctx, void *ptr, size_t size);

/*
 * Makes `size` bytes at `ptr` accessible to the controller and returns how
 * many of them, from the start, are contiguous in bus address space at
 * `*dma`. A short return must end on a 4 KiB boundary of bus address space.
 * Returning 0 makes the library bounce the transfer through
 * tendril_glue_dma_alloc() memory instead.
 */
size_t tendril_glue_dma_map(void *ctx, void *ptr, size_t size, int dir, uint64_t *dma);
void tendril_glue_dma_unmap(void *ctx, void *ptr, size_t size, int dir, uint64_t dma);

/* Orders all earlier memory accesses before all later ones, as seen by DMA. */
void tendril_glue_dma_barrier(void);

/*
 * Controller register access, at `offset` into the register window: BAR 4
 * (I/O space) for UHCI, BAR 0 for the others. Values are in CPU byte order,
 * the registers being little-endian. Writes are ordered after earlier memory
 * writes and reads before later memory reads.
 */
uint16_t tendril_glue_read16(void *ctx, uint32_t offset);
uint32_t tendril_glue_read32(void *ctx, uint32_t offset);
void tendril_glue_write16(void *ctx, uint32_t offset, uint16_t value);
void tendril_glue_write32(void *ctx, uint32_t offset, uint32_t value);

/* PCI configuration space, only used for controllers registered as PCI. */
uint8_t tendril_glue_pci_read8(void *ctx, uint16_t offset);
uint16_t tendril_glue_pci_read16(void *ctx, uint16_t offset);
uint32_t tendril_glue_pci_read32(void *ctx, uint16_t offset);
void tendril_glue_pci_write8(void *ctx, uint16_t offset, uint8_t value);
void tendril_glue_pci_write16(void *ctx, uint16_t offset, uint16_t value);
void tendril_glue_pci_write32(void *ctx, uint16_t offset, uint32_t value);
/*
 * Reads byte `offset` of the configuration space of the first PCI function
 * with these vendor and device IDs, which some controllers' chipset fixes
 * depend on; false if there is none, or the embedder cannot look.
 */
bool tendril_glue_pci_find_read8(void *ctx, uint16_t vendor, uint16_t device, uint16_t offset, uint8_t *value);

/* Monotonic time in nanoseconds. */
uint64_t tendril_glue_time_ns(void);

/* Per-controller lock, never taken recursively. */
void tendril_glue_lock(void *ctx);
void tendril_glue_unlock(void *ctx);

/* `message` is one line, without a trailing newline. */
void tendril_glue_log(int level, const char *message);

#ifdef __cplusplus
}
#endif

#endif
