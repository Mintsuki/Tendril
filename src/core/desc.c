#include "../internal.h"

/* Returns the descriptor after `p`, or NULL at the end or on a malformed length. */
const uint8_t *tendril_desc_next(const uint8_t *p, const uint8_t *end) {
    if (p == NULL || p + 2 > end || p[0] < 2) {
        return NULL;
    }
    p += p[0];
    if (p + 2 > end || p[0] < 2 || p + p[0] > end) {
        return NULL;
    }
    return p;
}

/* Returns the first descriptor of `type` at or after `p`. */
const uint8_t *tendril_desc_find(const uint8_t *p, const uint8_t *end, uint8_t type) {
    while (p != NULL) {
        if (p + 2 <= end && p[0] >= 2 && p[1] == type) {
            return p;
        }
        p = tendril_desc_next(p, end);
    }
    return NULL;
}

bool tendril_config_valid(const uint8_t *config, size_t size) {
    if (size < sizeof(struct tendril_config_descriptor) || config[1] != TENDRIL_DT_CONFIGURATION ||
        config[0] < sizeof(struct tendril_config_descriptor)) {
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
