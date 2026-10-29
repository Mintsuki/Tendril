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

#include "../internal.h"

int tendril_log_threshold = TENDRIL_LOG_INFO;

void tendril_set_log_level(int level) {
    tendril_log_threshold = level;
}

struct out {
    char *buf;
    size_t size;
    size_t len;
};

static void out_char(struct out *o, char c) {
    if (o->len + 1 < o->size) {
        o->buf[o->len] = c;
    }
    o->len++;
}

static void out_pad(struct out *o, char c, int count) {
    while (count-- > 0) {
        out_char(o, c);
    }
}

#define FLAG_LEFT 0x01
#define FLAG_ZERO 0x02
#define FLAG_PLUS 0x04
#define FLAG_SPACE 0x08
#define FLAG_ALT 0x10

static void out_number(
    struct out *o, uint64_t value, bool negative, unsigned int base, bool upper, int flags, int width, int precision
) {
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    char tmp[24];
    int n = 0;

    while (value != 0) {
        tmp[n++] = digits[value % base];
        value /= base;
    }
    if (precision < 0) {
        precision = 1;
    } else {
        flags &= ~FLAG_ZERO;
    }

    char sign = 0;
    if (negative) {
        sign = '-';
    } else if (flags & FLAG_PLUS) {
        sign = '+';
    } else if (flags & FLAG_SPACE) {
        sign = ' ';
    }

    const char *prefix = "";
    if ((flags & FLAG_ALT) && base == 16 && n != 0) {
        prefix = upper ? "0X" : "0x";
    }
    int prefix_len = (int)tendril_strlen(prefix);
    int zeros = precision > n ? precision - n : 0;
    int total = n + zeros + prefix_len + (sign ? 1 : 0);

    if (!(flags & FLAG_LEFT)) {
        if (flags & FLAG_ZERO) {
            zeros += width - total;
        } else {
            out_pad(o, ' ', width - total);
        }
    }
    if (sign) {
        out_char(o, sign);
    }
    for (size_t i = 0; prefix[i] != '\0'; i++) {
        out_char(o, prefix[i]);
    }
    out_pad(o, '0', zeros);
    while (n > 0) {
        out_char(o, tmp[--n]);
    }
    if (flags & FLAG_LEFT) {
        out_pad(o, ' ', width - total);
    }
}

#define LEN_DEFAULT 0
#define LEN_CHAR 1
#define LEN_SHORT 2
#define LEN_LONG 3
#define LEN_LONG_LONG 4
#define LEN_SIZE 5
#define LEN_MAX 6
#define LEN_PTRDIFF 7

static uint64_t fetch_unsigned(va_list *args, int len) {
    switch (len) {
        case LEN_CHAR: {
            return (unsigned char)va_arg(*args, unsigned int);
        }
        case LEN_SHORT: {
            return (unsigned short)va_arg(*args, unsigned int);
        }
        case LEN_LONG: {
            return va_arg(*args, unsigned long);
        }
        case LEN_LONG_LONG: {
            return va_arg(*args, unsigned long long);
        }
        case LEN_SIZE: {
            return va_arg(*args, size_t);
        }
        case LEN_MAX: {
            return va_arg(*args, uintmax_t);
        }
        case LEN_PTRDIFF: {
            return (uint64_t)va_arg(*args, ptrdiff_t);
        }
        default: {
            return va_arg(*args, unsigned int);
        }
    }
}

static int64_t fetch_signed(va_list *args, int len) {
    switch (len) {
        case LEN_CHAR: {
            return (signed char)va_arg(*args, int);
        }
        case LEN_SHORT: {
            return (short)va_arg(*args, int);
        }
        case LEN_LONG: {
            return va_arg(*args, long);
        }
        case LEN_LONG_LONG: {
            return va_arg(*args, long long);
        }
        case LEN_SIZE: {
            return (int64_t)va_arg(*args, size_t);
        }
        case LEN_MAX: {
            return va_arg(*args, intmax_t);
        }
        case LEN_PTRDIFF: {
            return va_arg(*args, ptrdiff_t);
        }
        default: {
            return va_arg(*args, int);
        }
    }
}

int tendril_vsnprintf(char *buf, size_t size, const char *fmt, va_list args_in) {
    struct out o = { buf, size, 0 };
    va_list args;

    va_copy(args, args_in);
    while (*fmt != '\0') {
        if (*fmt != '%') {
            out_char(&o, *fmt++);
            continue;
        }
        fmt++;

        int flags = 0;
        for (;;) {
            if (*fmt == '-') {
                flags |= FLAG_LEFT;
            } else if (*fmt == '0') {
                flags |= FLAG_ZERO;
            } else if (*fmt == '+') {
                flags |= FLAG_PLUS;
            } else if (*fmt == ' ') {
                flags |= FLAG_SPACE;
            } else if (*fmt == '#') {
                flags |= FLAG_ALT;
            } else {
                break;
            }
            fmt++;
        }

        int width = 0;
        if (*fmt == '*') {
            width = va_arg(args, int);
            if (width < 0) {
                flags |= FLAG_LEFT;
                width = -width;
            }
            fmt++;
        } else {
            while (*fmt >= '0' && *fmt <= '9') {
                width = width * 10 + (*fmt++ - '0');
            }
        }

        int precision = -1;
        if (*fmt == '.') {
            fmt++;
            precision = 0;
            if (*fmt == '*') {
                precision = va_arg(args, int);
                fmt++;
            } else {
                while (*fmt >= '0' && *fmt <= '9') {
                    precision = precision * 10 + (*fmt++ - '0');
                }
            }
        }

        int len = LEN_DEFAULT;
        if (*fmt == 'h') {
            fmt++;
            len = LEN_SHORT;
            if (*fmt == 'h') {
                fmt++;
                len = LEN_CHAR;
            }
        } else if (*fmt == 'l') {
            fmt++;
            len = LEN_LONG;
            if (*fmt == 'l') {
                fmt++;
                len = LEN_LONG_LONG;
            }
        } else if (*fmt == 'z') {
            fmt++;
            len = LEN_SIZE;
        } else if (*fmt == 'j') {
            fmt++;
            len = LEN_MAX;
        } else if (*fmt == 't') {
            fmt++;
            len = LEN_PTRDIFF;
        }

        char conv = *fmt;
        if (conv == '\0') {
            break;
        }
        fmt++;

        switch (conv) {
            case 'd':
            case 'i': {
                int64_t v = fetch_signed(&args, len);
                uint64_t mag = v < 0 ? (uint64_t)0 - (uint64_t)v : (uint64_t)v;
                out_number(&o, mag, v < 0, 10, false, flags, width, precision);
                break;
            }
            case 'u': {
                out_number(&o, fetch_unsigned(&args, len), false, 10, false, flags, width, precision);
                break;
            }
            case 'o': {
                out_number(&o, fetch_unsigned(&args, len), false, 8, false, flags, width, precision);
                break;
            }
            case 'x':
            case 'X': {
                out_number(&o, fetch_unsigned(&args, len), false, 16, conv == 'X', flags, width, precision);
                break;
            }
            case 'p': {
                uintptr_t p = (uintptr_t)va_arg(args, void *);
                out_number(&o, p, false, 16, false, flags | FLAG_ALT, width, precision);
                break;
            }
            case 'c': {
                if (!(flags & FLAG_LEFT)) {
                    out_pad(&o, ' ', width - 1);
                }
                out_char(&o, (char)va_arg(args, int));
                if (flags & FLAG_LEFT) {
                    out_pad(&o, ' ', width - 1);
                }
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                if (s == NULL) {
                    s = "(null)";
                }
                size_t n = 0;
                while (s[n] != '\0' && (precision < 0 || n < (size_t)precision)) {
                    n++;
                }
                if (!(flags & FLAG_LEFT)) {
                    out_pad(&o, ' ', width - (int)n);
                }
                for (size_t i = 0; i < n; i++) {
                    out_char(&o, s[i]);
                }
                if (flags & FLAG_LEFT) {
                    out_pad(&o, ' ', width - (int)n);
                }
                break;
            }
            case '%': {
                out_char(&o, '%');
                break;
            }
            default: {
                out_char(&o, '%');
                out_char(&o, conv);
                break;
            }
        }
    }
    va_end(args);

    if (size != 0) {
        buf[o.len < size ? o.len : size - 1] = '\0';
    }
    return (int)o.len;
}

int tendril_snprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = tendril_vsnprintf(buf, size, fmt, args);
    va_end(args);
    return ret;
}

void tendril_vlog(int level, const char *fmt, va_list args) {
    char line[256];

    if (level > tendril_log_threshold) {
        return;
    }
    tendril_vsnprintf(line, sizeof(line), fmt, args);
    tendril_glue_log(level, line);
}

void tendril_log(int level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    tendril_vlog(level, fmt, args);
    va_end(args);
}

const char *tendril_status_str(int status) {
    switch (status) {
        case TENDRIL_OK: {
            return "success";
        }
        case TENDRIL_ERR_NO_MEMORY: {
            return "out of memory";
        }
        case TENDRIL_ERR_INVALID: {
            return "invalid argument";
        }
        case TENDRIL_ERR_UNSUPPORTED: {
            return "unsupported";
        }
        case TENDRIL_ERR_BUSY: {
            return "busy";
        }
        case TENDRIL_ERR_TIMEOUT: {
            return "timed out";
        }
        case TENDRIL_ERR_CANCELLED: {
            return "cancelled";
        }
        case TENDRIL_ERR_NO_DEVICE: {
            return "no device";
        }
        case TENDRIL_ERR_STALL: {
            return "stalled";
        }
        case TENDRIL_ERR_IO: {
            return "transaction error";
        }
        case TENDRIL_ERR_BABBLE: {
            return "babble";
        }
        case TENDRIL_ERR_OVERRUN: {
            return "data overrun or underrun";
        }
        case TENDRIL_ERR_SHORT: {
            return "short packet";
        }
        case TENDRIL_ERR_BANDWIDTH: {
            return "insufficient bandwidth";
        }
        case TENDRIL_ERR_MISSED: {
            return "missed service interval";
        }
        case TENDRIL_ERR_CONTROLLER: {
            return "host controller error";
        }
        case TENDRIL_ERR_PROTOCOL: {
            return "protocol error";
        }
        case TENDRIL_ERR_NO_MEDIUM: {
            return "no medium";
        }
        case TENDRIL_ERR_MEDIUM_CHANGED: {
            return "medium changed";
        }
        case TENDRIL_ERR_WRITE_PROTECTED: {
            return "write protected";
        }
        case TENDRIL_ERR_SUSPENDED: {
            return "device suspended";
        }
        case TENDRIL_ERR_NO_POWER: {
            return "not enough bus power";
        }
    }
    return "unknown error";
}

const char *tendril_speed_str(int speed) {
    switch (speed) {
        case TENDRIL_SPEED_LOW: {
            return "low-speed";
        }
        case TENDRIL_SPEED_FULL: {
            return "full-speed";
        }
        case TENDRIL_SPEED_HIGH: {
            return "high-speed";
        }
        case TENDRIL_SPEED_SUPER: {
            return "SuperSpeed";
        }
        case TENDRIL_SPEED_SUPER_PLUS: {
            return "SuperSpeedPlus";
        }
        case TENDRIL_SPEED_SUPER_PLUS_X2: {
            return "SuperSpeedPlus x2";
        }
    }
    return "unknown speed";
}

const char *tendril_hc_type_str(int type) {
    switch (type) {
        case TENDRIL_HC_UHCI: {
            return "UHCI";
        }
        case TENDRIL_HC_OHCI: {
            return "OHCI";
        }
        case TENDRIL_HC_EHCI: {
            return "EHCI";
        }
        case TENDRIL_HC_XHCI: {
            return "xHCI";
        }
    }
    return "unknown";
}
