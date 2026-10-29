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
#include <tendril/spec.h>

/*
 * Returns the descriptor after `p`, or NULL at the end or on a malformed
 * length.
 */
const uint8_t *tendril_desc_next(const uint8_t *p, const uint8_t *end) {
    if (p == NULL || end - p < 2 || p[0] < 2 || end - p < p[0]) {
        return NULL;
    }
    p += p[0];
    if (end - p < 2 || p[0] < 2 || end - p < p[0]) {
        return NULL;
    }
    return p;
}

/* Returns the first descriptor of `type` at or after `p`. */
const uint8_t *tendril_desc_find(
    const uint8_t *p, const uint8_t *end, int type
) {
    while (p != NULL) {
        if (end - p >= 2 && p[0] >= 2 && end - p >= p[0] && p[1] == type) {
            return p;
        }
        p = tendril_desc_next(p, end);
    }
    return NULL;
}

bool tendril_config_valid(const uint8_t *config, size_t size) {
    if (size < sizeof(struct tendril_config_descriptor)
     || config[1] != TENDRIL_DT_CONFIGURATION
     || config[0] < sizeof(struct tendril_config_descriptor)) {
        return false;
    }
    /* Walk every descriptor; a bad length anywhere makes the rest unusable. */
    const uint8_t *end = config + size;
    const uint8_t *p = config;
    size_t walked = 0;
    while (p != NULL) {
        walked = (size_t)(p - config) + p[0];
        p = tendril_desc_next(p, end);
    }
    return walked >= sizeof(struct tendril_config_descriptor);
}
