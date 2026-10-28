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
    if (dma32 && *dma + size > 0x100000000ull) {
        LOG_ERROR("tendril: DMA memory at 0x%llx is above 4 GiB", (unsigned long long)*dma);
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
    struct tendril_dma_chunk *chunk = tendril_zalloc(sizeof(*chunk));
    if (chunk == NULL) {
        return false;
    }
    chunk->virt = tendril_dma_alloc(pool->hc, CHUNK_SIZE, CHUNK_SIZE, &chunk->dma);
    if (chunk->virt == NULL) {
        tendril_free(chunk, sizeof(*chunk));
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
        tendril_free(chunk, sizeof(*chunk));
        chunk = next;
    }
    pool->chunks = NULL;
    pool->free = NULL;
}
