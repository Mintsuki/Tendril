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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../../include/tendril/glue.h"
#include "../internal.h"

#define CHUNK_SIZE 4096

struct tendril_dma_chunk {
    struct tendril_dma_chunk *next;
    void *virt;
    uint64_t dma;
};

struct tendril_dma_free {
    struct tendril_dma_free *next;
    uint64_t dma;
};

void *tendril_zalloc(size_t size) {
    void *p = tendril_glue_alloc(size);
    if (p != NULL) {
        memset(p, 0, size);
    }
    return p;
}

void tendril_free(void *ptr, size_t size) {
    if (ptr != NULL) {
        tendril_glue_free(ptr, size);
    }
}

size_t tendril_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') {
        n++;
    }
    return n;
}

char *tendril_strdup(const char *s) {
    size_t n = tendril_strlen(s) + 1;
    char *p = tendril_glue_alloc(n);
    if (p != NULL) {
        memcpy(p, s, n);
    }
    return p;
}

void tendril_strfree(char *s) {
    if (s != NULL) {
        tendril_glue_free(s, tendril_strlen(s) + 1);
    }
}

void *tendril_dma_alloc(struct tendril_hc *hc, size_t size, size_t align, uint64_t *dma) {
    return tendril_dma_alloc_limit(hc, size, align, hc->dma32, dma);
}

void *tendril_dma_alloc_limit(struct tendril_hc *hc, size_t size, size_t align, bool dma32, uint64_t *dma) {
    void *p = tendril_glue_dma_alloc(hc->ctx, size, align, dma32, dma);
    if (p == NULL) {
        return NULL;
    }
    if (dma32 && *dma + size > (uint64_t)0x100000000) {
        LOG_ERROR("DMA memory at 0x%llx is above 4 GiB", (unsigned long long)*dma);
        tendril_glue_dma_free(hc->ctx, p, size);
        return NULL;
    }
    memset(p, 0, size);
    return p;
}

void tendril_dma_free(struct tendril_hc *hc, void *ptr, size_t size) {
    if (ptr != NULL) {
        tendril_glue_dma_free(hc->ctx, ptr, size);
    }
}

void tendril_dma_pool_init(struct tendril_dma_pool *pool, struct tendril_hc *hc, size_t size, size_t align) {
    pool->hc = hc;
    pool->align = align;
    pool->size = ALIGN_UP(MAX(size, sizeof(struct tendril_dma_free)), MAX(align, (size_t)8));
    pool->chunks = NULL;
    pool->free = NULL;
}

/*
 * A free block keeps its list node at its end and is zero elsewhere: hardware
 * descriptors start their blocks, and a controller that still reads one after
 * it is freed must find an inactive descriptor, not the bits of a pointer.
 */
static struct tendril_dma_free *free_node(struct tendril_dma_pool *pool, void *block) {
    return (struct tendril_dma_free *)(void *)((char *)block + pool->size - sizeof(struct tendril_dma_free));
}

static void *node_block(struct tendril_dma_pool *pool, struct tendril_dma_free *f) {
    return (char *)f - (pool->size - sizeof(struct tendril_dma_free));
}

static void push_free(struct tendril_dma_pool *pool, void *block, uint64_t dma) {
    memset(block, 0, pool->size);
    struct tendril_dma_free *f = free_node(pool, block);
    f->dma = dma;
    f->next = pool->free;
    pool->free = f;
}

static bool pool_grow(struct tendril_dma_pool *pool) {
    if (pool->size > CHUNK_SIZE) {
        return false;
    }
    struct tendril_dma_chunk *chunk = tendril_zalloc(sizeof(struct tendril_dma_chunk));
    if (chunk == NULL) {
        return false;
    }
    chunk->virt = tendril_dma_alloc(pool->hc, CHUNK_SIZE, CHUNK_SIZE, &chunk->dma);
    if (chunk->virt == NULL) {
        tendril_free(chunk, sizeof(struct tendril_dma_chunk));
        return false;
    }
    chunk->next = pool->chunks;
    pool->chunks = chunk;

    for (size_t off = 0; off + pool->size <= CHUNK_SIZE; off += pool->size) {
        push_free(pool, (char *)chunk->virt + off, chunk->dma + off);
    }
    return true;
}

void *tendril_dma_pool_alloc(struct tendril_dma_pool *pool, uint64_t *dma) {
    if (pool->free == NULL && !pool_grow(pool)) {
        return NULL;
    }
    struct tendril_dma_free *f = pool->free;
    void *block = node_block(pool, f);
    pool->free = f->next;
    *dma = f->dma;
    memset(block, 0, pool->size);
    return block;
}

void tendril_dma_pool_free(struct tendril_dma_pool *pool, void *ptr, uint64_t dma) {
    if (ptr != NULL) {
        push_free(pool, ptr, dma);
    }
}

void tendril_dma_pool_destroy(struct tendril_dma_pool *pool) {
    struct tendril_dma_chunk *chunk = pool->chunks;
    while (chunk != NULL) {
        struct tendril_dma_chunk *next = chunk->next;
        tendril_dma_free(pool->hc, chunk->virt, CHUNK_SIZE);
        tendril_free(chunk, sizeof(struct tendril_dma_chunk));
        chunk = next;
    }
    pool->chunks = NULL;
    pool->free = NULL;
}
