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

enum tendril_log_level {
    TENDRIL_LOG_ERROR,
    TENDRIL_LOG_WARN,
    TENDRIL_LOG_INFO,
    TENDRIL_LOG_DEBUG,
    TENDRIL_LOG_TRACE
};

enum tendril_dma_dir {
    TENDRIL_DMA_TO_DEVICE,
    TENDRIL_DMA_FROM_DEVICE,
    TENDRIL_DMA_BIDIRECTIONAL
};

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
size_t tendril_glue_dma_map(void *ctx, void *ptr, size_t size, enum tendril_dma_dir dir, uint64_t *dma);
void tendril_glue_dma_unmap(void *ctx, void *ptr, size_t size, enum tendril_dma_dir dir, uint64_t dma);

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

/* Monotonic time in nanoseconds. */
uint64_t tendril_glue_time_ns(void);

/* Per-controller lock, never taken recursively. */
void tendril_glue_lock(void *ctx);
void tendril_glue_unlock(void *ctx);

/* `message` is one line, without a trailing newline. */
void tendril_glue_log(enum tendril_log_level level, const char *message);

#ifdef __cplusplus
}
#endif

#endif
