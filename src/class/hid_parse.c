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
#include "../internal.h"
#include <tendril/hid.h>
#include <tendril/tendril.h>

/* HID 1.11 6.2.2: report descriptor items. */

#define TYPE_MAIN 0
#define TYPE_GLOBAL 1
#define TYPE_LOCAL 2

#define MAIN_INPUT 0x8
#define MAIN_OUTPUT 0x9
#define MAIN_COLLECTION 0xa
#define MAIN_FEATURE 0xb
#define MAIN_END_COLLECTION 0xc

#define GLOBAL_USAGE_PAGE 0x0
#define GLOBAL_LOGICAL_MIN 0x1
#define GLOBAL_LOGICAL_MAX 0x2
#define GLOBAL_PHYSICAL_MIN 0x3
#define GLOBAL_PHYSICAL_MAX 0x4
#define GLOBAL_UNIT_EXPONENT 0x5
#define GLOBAL_UNIT 0x6
#define GLOBAL_REPORT_SIZE 0x7
#define GLOBAL_REPORT_ID 0x8
#define GLOBAL_REPORT_COUNT 0x9
#define GLOBAL_PUSH 0xa
#define GLOBAL_POP 0xb

#define LOCAL_USAGE 0x0
#define LOCAL_USAGE_MIN 0x1
#define LOCAL_USAGE_MAX 0x2
#define LOCAL_DELIMITER 0xa

#define MAX_STACK 8
#define MAX_COLLECTIONS 16
#define MAX_RANGES 256
#define MAX_FIELDS 1024
#define MAX_EXPANDED 2048
/* Reports beyond 64 KiB are taken as a broken descriptor. */
#define MAX_REPORT_BITS ((uint32_t)65536 * 8)

struct globals {
    uint32_t usage_page;
    int32_t logical_min;
    int32_t logical_max;
    uint32_t logical_max_raw;
    int32_t physical_min;
    int32_t physical_max;
    int32_t unit_exponent;
    uint32_t unit;
    size_t report_size;
    size_t report_count;
    uint8_t report_id;
};

struct usage_range {
    uint32_t min;
    uint32_t max;
};

struct parser {
    struct globals g;
    struct globals stack[MAX_STACK];
    size_t sp;
    struct usage_range ranges[MAX_RANGES];
    size_t num_ranges;
    bool have_min;
    uint32_t pending_min;
    bool delimiter;
    bool delimiter_used;
    uint32_t collections[MAX_COLLECTIONS];
    int collection_types[MAX_COLLECTIONS];
    /* Collections open, which may be more than MAX_COLLECTIONS keeps. */
    size_t depth;
    struct tendril_hid_report_desc *rd;
    size_t cap_fields;
    size_t cap_reports;
};

static bool grow(void **array, size_t *cap, size_t count, size_t elem) {
    if (count < *cap) {
        return true;
    }
    size_t ncap = *cap == 0 ? 8 : *cap * 2;
    void *n = tendril_zalloc(ncap * elem);
    if (n == NULL) {
        return false;
    }
    if (*array != NULL) {
        memcpy(n, *array, *cap * elem);
        tendril_free(*array, *cap * elem);
    }
    *array = n;
    *cap = ncap;
    return true;
}

static int trim(void **array, size_t cap, size_t count, size_t elem) {
    if (cap == count) {
        return TENDRIL_OK;
    }
    void *n = NULL;
    if (count != 0) {
        n = tendril_zalloc(count * elem);
        if (n == NULL) {
            return TENDRIL_ERR_NO_MEMORY;
        }
        memcpy(n, *array, count * elem);
    }
    tendril_free(*array, cap * elem);
    *array = n;
    return TENDRIL_OK;
}

static struct tendril_hid_report_info *report_info(
    struct parser *p, int type, uint8_t id
) {
    struct tendril_hid_report_desc *rd = p->rd;
    for (size_t i = 0; i < rd->num_reports; i++) {
        if (rd->reports[i].type == type && rd->reports[i].id == id) {
            return &rd->reports[i];
        }
    }
    bool grown = grow(
        (void **)&rd->reports, &p->cap_reports, rd->num_reports,
        sizeof(struct tendril_hid_report_info)
    );
    if (!grown) {
        return NULL;
    }
    struct tendril_hid_report_info *r = &rd->reports[rd->num_reports++];
    r->type = type;
    r->id = id;
    r->bits = 0;
    return r;
}

static void clear_locals(struct parser *p) {
    p->num_ranges = 0;
    p->have_min = false;
    p->delimiter_used = false;
}

static void add_usage(struct parser *p, uint32_t min, uint32_t max) {
    /* Inside a delimiter set, only the first usage counts. */
    if (p->delimiter) {
        if (p->delimiter_used) {
            return;
        }
        p->delimiter_used = true;
    }
    if (p->num_ranges < MAX_RANGES) {
        p->ranges[p->num_ranges].min = min;
        p->ranges[p->num_ranges].max = max;
        p->num_ranges++;
    }
}

static uint32_t first_usage(struct parser *p) {
    return p->num_ranges != 0 ? p->ranges[0].min : 0;
}

static uint32_t application(struct parser *p) {
    for (size_t i = MIN(p->depth, (size_t)MAX_COLLECTIONS); i-- > 0; ) {
        if (p->collection_types[i] == 0x01) {
            return p->collections[i];
        }
    }
    return 0;
}

static int add_field(struct parser *p, int type, uint32_t flags) {
    struct globals *g = &p->g;
    struct tendril_hid_report_desc *rd = p->rd;
    struct tendril_hid_report_info *r = report_info(p, type, g->report_id);
    if (r == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    uint64_t bits64 = (uint64_t)g->report_size * g->report_count;
    if (bits64 > MAX_REPORT_BITS - r->bits) {
        return TENDRIL_ERR_PROTOCOL;
    }
    uint32_t bits = (uint32_t)bits64;
    if (g->report_size > 32) {
        /*
         * Larger elements cannot be read as values; keep the layout right
         * anyway.
         */
        r->bits += bits;
        return TENDRIL_OK;
    }
    if (bits == 0) {
        return TENDRIL_OK;
    }
    if (rd->num_fields >= MAX_FIELDS) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    bool grown = grow(
        (void **)&rd->fields, &p->cap_fields, rd->num_fields,
        sizeof(struct tendril_hid_field)
    );
    if (!grown) {
        return TENDRIL_ERR_NO_MEMORY;
    }

    struct tendril_hid_field *f = &rd->fields[rd->num_fields];
    memset(f, 0, sizeof(struct tendril_hid_field));
    f->type = type;
    f->report_id = g->report_id;
    f->flags = (uint16_t)flags;
    f->bit_offset = r->bits;
    f->bit_size = (uint16_t)g->report_size;
    f->count = (uint16_t)MIN(g->report_count, (uint32_t)0xffff);
    f->logical_min = g->logical_min;
    f->logical_max = g->logical_max;
    /* A maximum below the minimum was meant as unsigned. */
    if (g->logical_min >= 0 && g->logical_max < g->logical_min) {
        f->logical_max = (int32_t)g->logical_max_raw;
    }
    f->physical_min = g->physical_min;
    f->physical_max = g->physical_max;
    if (f->physical_min == 0 && f->physical_max == 0) {
        f->physical_min = f->logical_min;
        f->physical_max = f->logical_max;
    }
    f->unit_exponent = g->unit_exponent;
    f->unit = g->unit;
    f->application = application(p);
    r->bits += bits;

    if (p->num_ranges == 1) {
        f->usage_min = p->ranges[0].min;
        f->usage_max = p->ranges[0].max;
    } else if (p->num_ranges > 1) {
        uint64_t total = 0;
        for (size_t i = 0; i < p->num_ranges; i++) {
            total += (uint64_t)p->ranges[i].max - p->ranges[i].min + 1;
        }
        size_t n = (size_t)MIN(total, (uint64_t)MAX_EXPANDED);
        f->usages = tendril_zalloc(n * sizeof(uint32_t));
        if (f->usages == NULL) {
            return TENDRIL_ERR_NO_MEMORY;
        }
        size_t k = 0;
        for (size_t i = 0; i < p->num_ranges && k < n; i++) {
            for (uint32_t u = p->ranges[i].min; k < n; u++) {
                f->usages[k++] = u;
                if (u == p->ranges[i].max) {
                    break;
                }
            }
        }
        f->num_usages = (uint16_t)n;
        f->usage_min = f->usages[0];
        f->usage_max = f->usages[n - 1];
    }
    rd->num_fields++;
    return TENDRIL_OK;
}

static int32_t sign_extend(uint32_t v, size_t size) {
    switch (size) {
        case 1: {
            return (int8_t)v;
        }
        case 2: {
            return (int16_t)v;
        }
        default: {
            return (int32_t)v;
        }
    }
}

void tendril_hid_free(struct tendril_hid_report_desc *rd) {
    if (rd == NULL) {
        return;
    }
    for (size_t i = 0; i < rd->num_fields; i++) {
        tendril_free(
            rd->fields[i].usages, rd->fields[i].num_usages * sizeof(uint32_t)
        );
    }
    tendril_free(
        rd->fields, rd->num_fields * sizeof(struct tendril_hid_field)
    );
    tendril_free(
        rd->reports, rd->num_reports * sizeof(struct tendril_hid_report_info)
    );
    tendril_free(rd, sizeof(struct tendril_hid_report_desc));
}

int tendril_hid_parse(
    const uint8_t *desc, size_t size, struct tendril_hid_report_desc **out
) {
    struct parser *p = tendril_zalloc(sizeof(struct parser));
    if (p == NULL) {
        return TENDRIL_ERR_NO_MEMORY;
    }
    p->rd = tendril_zalloc(sizeof(struct tendril_hid_report_desc));
    if (p->rd == NULL) {
        tendril_free(p, sizeof(struct parser));
        return TENDRIL_ERR_NO_MEMORY;
    }

    int st = TENDRIL_OK;
    size_t i = 0;
    while (i < size && st == TENDRIL_OK) {
        uint8_t prefix = desc[i++];
        if (prefix == 0xfe) {
            /* Long item: no long item tags are defined, skip its data. */
            if (i + 2 > size) {
                break;
            }
            i += 2 + desc[i];
            continue;
        }
        size_t bsize = prefix & 3;
        size_t len = bsize == 3 ? 4 : bsize;
        int type = (prefix >> 2) & 3;
        int tag = prefix >> 4;
        if (i + len > size) {
            break;
        }
        uint32_t v = 0;
        for (size_t b = 0; b < len; b++) {
            v |= (uint32_t)desc[i + b] << (8 * b);
        }
        i += len;
        struct globals *g = &p->g;

        if (type == TYPE_MAIN) {
            switch (tag) {
                case MAIN_INPUT: {
                    st = add_field(p, TENDRIL_HID_INPUT, v);
                    break;
                }
                case MAIN_OUTPUT: {
                    st = add_field(p, TENDRIL_HID_OUTPUT, v);
                    break;
                }
                case MAIN_FEATURE: {
                    st = add_field(p, TENDRIL_HID_FEATURE, v);
                    break;
                }
                case MAIN_COLLECTION: {
                    if (p->depth < MAX_COLLECTIONS) {
                        p->collections[p->depth] = first_usage(p);
                        p->collection_types[p->depth] = (uint8_t)v;
                    }
                    p->depth++;
                    break;
                }
                case MAIN_END_COLLECTION: {
                    if (p->depth > 0) {
                        p->depth--;
                    }
                    break;
                }
                default: {
                    break;
                }
            }
            clear_locals(p);
            continue;
        }

        if (type == TYPE_GLOBAL) {
            switch (tag) {
                case GLOBAL_USAGE_PAGE: {
                    g->usage_page = v & 0xffff;
                    break;
                }
                case GLOBAL_LOGICAL_MIN: {
                    g->logical_min = sign_extend(v, len);
                    break;
                }
                case GLOBAL_LOGICAL_MAX: {
                    g->logical_max = sign_extend(v, len);
                    g->logical_max_raw = v;
                    break;
                }
                case GLOBAL_PHYSICAL_MIN: {
                    g->physical_min = sign_extend(v, len);
                    break;
                }
                case GLOBAL_PHYSICAL_MAX: {
                    g->physical_max = sign_extend(v, len);
                    break;
                }
                case GLOBAL_UNIT_EXPONENT: {
                    /* HID 1.11 6.2.2.7: a 4-bit two's complement value. */
                    g->unit_exponent = v <= 0xf
                        ? (v < 8 ? (int32_t)v : (int32_t)v - 16)
                        : sign_extend(v, len);
                    break;
                }
                case GLOBAL_UNIT: {
                    g->unit = v;
                    break;
                }
                case GLOBAL_REPORT_SIZE: {
                    g->report_size = v;
                    break;
                }
                case GLOBAL_REPORT_ID: {
                    g->report_id = (uint8_t)v;
                    p->rd->numbered = true;
                    break;
                }
                case GLOBAL_REPORT_COUNT: {
                    g->report_count = v;
                    break;
                }
                case GLOBAL_PUSH: {
                    if (p->sp < MAX_STACK) {
                        p->stack[p->sp++] = *g;
                    }
                    break;
                }
                case GLOBAL_POP: {
                    if (p->sp > 0) {
                        *g = p->stack[--p->sp];
                    }
                    break;
                }
                default: {
                    break;
                }
            }
            continue;
        }

        if (type == TYPE_LOCAL) {
            /*
             * 16-bit usages take the current usage page; 32-bit ones carry
             * their own.
             */
            uint32_t usage = len == 4 ? v
                : (g->usage_page << 16) | (v & 0xffff);
            switch (tag) {
                case LOCAL_USAGE: {
                    add_usage(p, usage, usage);
                    break;
                }
                case LOCAL_USAGE_MIN: {
                    p->pending_min = usage;
                    p->have_min = true;
                    break;
                }
                case LOCAL_USAGE_MAX: {
                    if (p->have_min && usage >= p->pending_min) {
                        add_usage(p, p->pending_min, usage);
                    }
                    p->have_min = false;
                    break;
                }
                case LOCAL_DELIMITER: {
                    p->delimiter = v != 0;
                    p->delimiter_used = false;
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    /* Trim the arrays to their counts, which tendril_hid_free() relies on. */
    struct tendril_hid_report_desc *rd = p->rd;
    if (st == TENDRIL_OK) {
        st = trim(
            (void **)&rd->fields, p->cap_fields, rd->num_fields,
            sizeof(struct tendril_hid_field)
        );
    }
    if (st == TENDRIL_OK) {
        st = trim(
            (void **)&rd->reports, p->cap_reports, rd->num_reports,
            sizeof(struct tendril_hid_report_info)
        );
    }
    if (st != TENDRIL_OK) {
        for (size_t f = 0; f < rd->num_fields; f++) {
            tendril_free(
                rd->fields[f].usages,
                rd->fields[f].num_usages * sizeof(uint32_t)
            );
        }
        tendril_free(
            rd->fields, p->cap_fields * sizeof(struct tendril_hid_field)
        );
        tendril_free(
            rd->reports,
            p->cap_reports * sizeof(struct tendril_hid_report_info)
        );
        tendril_free(rd, sizeof(struct tendril_hid_report_desc));
        tendril_free(p, sizeof(struct parser));
        return st;
    }
    tendril_free(p, sizeof(struct parser));
    *out = rd;
    return TENDRIL_OK;
}

size_t tendril_hid_report_size(
    const struct tendril_hid_report_desc *rd, int type, uint8_t id
) {
    for (size_t i = 0; i < rd->num_reports; i++) {
        if (rd->reports[i].type == type && rd->reports[i].id == id) {
            return (rd->reports[i].bits + 7) / 8;
        }
    }
    return 0;
}

uint32_t tendril_hid_extract(
    const uint8_t *data, size_t size, size_t bit_offset, size_t bit_size
) {
    uint32_t v = 0;
    for (size_t b = 0; b < bit_size && b < 32; b++) {
        size_t bit = bit_offset + b;
        if (bit / 8 >= size) {
            break;
        }
        if ((data[bit / 8] & ((uint32_t)1 << (bit % 8))) != 0) {
            v |= (uint32_t)1 << b;
        }
    }
    return v;
}

void tendril_hid_insert(
    uint8_t *data, size_t size, size_t bit_offset, size_t bit_size,
    uint32_t value
) {
    for (size_t b = 0; b < bit_size && b < 32; b++) {
        size_t bit = bit_offset + b;
        if (bit / 8 >= size) {
            break;
        }
        if ((value & ((uint32_t)1 << b)) != 0) {
            data[bit / 8] |= (uint8_t)((uint32_t)1 << (bit % 8));
        } else {
            data[bit / 8] &= (uint8_t)~((uint32_t)1 << (bit % 8));
        }
    }
}

int32_t tendril_hid_field_get(
    const struct tendril_hid_field *f, const uint8_t *data, size_t size,
    size_t index
) {
    uint32_t v = tendril_hid_extract(
        data, size, f->bit_offset + (uint32_t)index * f->bit_size, f->bit_size
    );
    if (f->logical_min < 0 && f->bit_size < 32
     && (v & ((uint32_t)1 << (f->bit_size - 1))) != 0) {
        v |= ~(uint32_t)0 << f->bit_size;
    }
    return (int32_t)v;
}

void tendril_hid_field_set(
    const struct tendril_hid_field *f, uint8_t *data, size_t size,
    size_t index, int32_t value
) {
    tendril_hid_insert(
        data, size, f->bit_offset + (uint32_t)index * f->bit_size, f->bit_size,
        (uint32_t)value
    );
}

uint32_t tendril_hid_field_usage(
    const struct tendril_hid_field *f, size_t index
) {
    if (f->num_usages != 0) {
        return f->usages[MIN(index, (size_t)f->num_usages - 1)];
    }
    if (f->usage_max < f->usage_min) {
        return f->usage_min;
    }
    return index > f->usage_max - f->usage_min ? f->usage_max
        : f->usage_min + (uint32_t)index;
}

uint32_t tendril_hid_array_usage(
    const struct tendril_hid_field *f, int32_t value
) {
    if (value < f->logical_min || value > f->logical_max) {
        return 0;
    }
    uint32_t index = (uint32_t)((int64_t)value - f->logical_min);
    if (f->num_usages != 0) {
        return index < f->num_usages ? f->usages[index] : 0;
    }
    if (f->usage_max < f->usage_min || index > f->usage_max - f->usage_min) {
        return 0;
    }
    return f->usage_min + index;
}

const struct tendril_hid_field *tendril_hid_find(
    const struct tendril_hid_report_desc *rd, int type, uint32_t usage,
    size_t *index
) {
    for (size_t i = 0; i < rd->num_fields; i++) {
        const struct tendril_hid_field *f = &rd->fields[i];
        if (f->type != type || (f->flags & TENDRIL_HID_VARIABLE) == 0
         || (f->flags & TENDRIL_HID_CONSTANT) != 0) {
            continue;
        }
        for (size_t e = 0; e < f->count; e++) {
            if (tendril_hid_field_usage(f, e) == usage) {
                if (index != NULL) {
                    *index = e;
                }
                return f;
            }
        }
    }
    return NULL;
}
