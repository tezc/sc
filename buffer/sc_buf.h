/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef SC_BUF_H
#define SC_BUF_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SC_BUF_CORRUPT 1u
#define SC_BUF_OOM     3u

struct sc_buf
{
    unsigned char *mem;
    uint32_t cap;
    uint32_t limit;
    uint32_t rpos;
    uint32_t wpos;
    unsigned int error;

    bool ref;
};

#define sc_buf_malloc  malloc
#define sc_buf_realloc realloc
#define sc_buf_free    free

int sc_buf_init(struct sc_buf *buf, uint32_t cap);
void sc_buf_term(struct sc_buf *buf);

struct sc_buf sc_buf_wrap(void *data, uint32_t len, bool ref);

void sc_buf_limit(struct sc_buf *buf, uint32_t limit);

void *sc_buf_at(struct sc_buf *buf, uint32_t pos);
uint32_t sc_buf_cap(struct sc_buf *buf);
bool sc_buf_reserve(struct sc_buf *buf, uint32_t len);

bool sc_buf_valid(struct sc_buf *buf);
uint32_t sc_buf_quota(struct sc_buf *buf);
uint32_t sc_buf_size(struct sc_buf *buf);
void sc_buf_clear(struct sc_buf *buf);
void sc_buf_compact(struct sc_buf *buf);

void sc_buf_mark_read(struct sc_buf *buf, uint32_t len);
void sc_buf_mark_write(struct sc_buf *buf, uint32_t len);

uint32_t sc_buf_rpos(struct sc_buf *buf);
void sc_buf_set_rpos(struct sc_buf *buf, uint32_t pos);

uint32_t sc_buf_wpos(struct sc_buf *buf);
void sc_buf_set_wpos(struct sc_buf *buf, uint32_t pos);

void *sc_buf_rbuf(struct sc_buf *buf);
void *sc_buf_wbuf(struct sc_buf *buf);


uint32_t sc_buf_peek_data(struct sc_buf *buf, uint32_t offset, void *dest,
                          uint32_t len);

uint32_t sc_buf_peek_32_at(struct sc_buf *buf, uint32_t pos);
uint32_t sc_buf_peek_32(struct sc_buf *buf);

uint64_t sc_buf_peek_64_at(struct sc_buf *buf, uint32_t pos);
uint64_t sc_buf_peek_64(struct sc_buf *buf);

uint32_t sc_buf_set_data(struct sc_buf *buf, uint32_t offset, const void *src,
                         uint32_t len);

void sc_buf_set_32(struct sc_buf *buf, uint32_t val);
void sc_buf_set_64(struct sc_buf *buf, uint64_t val);

void sc_buf_set_8_at(struct sc_buf *buf, uint32_t pos, uint8_t val);
void sc_buf_set_32_at(struct sc_buf *buf, uint32_t pos, uint32_t val);
void sc_buf_set_64_at(struct sc_buf *buf, uint32_t pos, uint64_t val);

void sc_buf_get_raw(struct sc_buf *buf, void *dest, uint32_t len);
void sc_buf_put_raw(struct sc_buf *buf, const void *ptr, uint32_t len);

bool sc_buf_get_bool(struct sc_buf *buf);
void sc_buf_put_bool(struct sc_buf *buf, bool val);

uint8_t sc_buf_get_8(struct sc_buf *buf);
void sc_buf_put_8(struct sc_buf *buf, uint8_t byte);

uint16_t sc_buf_get_16(struct sc_buf *buf);
void sc_buf_put_16(struct sc_buf *buf, uint16_t val);

uint32_t sc_buf_get_32(struct sc_buf *buf);
void sc_buf_put_32(struct sc_buf *buf, uint32_t val);

uint64_t sc_buf_get_64(struct sc_buf *buf);
void sc_buf_put_64(struct sc_buf *buf, uint64_t val);

double sc_buf_get_double(struct sc_buf *buf);
void sc_buf_put_double(struct sc_buf *buf, double val);

const char *sc_buf_get_str(struct sc_buf *buf);
void sc_buf_put_str(struct sc_buf *buf, const char *str);
void sc_buf_put_fmt(struct sc_buf *buf, const char *fmt, ...);

void sc_buf_put_text(struct sc_buf *buf, const char *fmt, ...);

void sc_buf_put_blob(struct sc_buf *buf, const void *ptr, uint32_t len);
void *sc_buf_get_blob(struct sc_buf *buf, uint32_t len);

void sc_buf_move(struct sc_buf *dest, struct sc_buf *src);


// clang-format off
static inline uint32_t sc_buf_bool_len(bool val) { return 1;}
static inline uint32_t sc_buf_8_len(uint8_t val) {return 1;}
static inline uint32_t sc_buf_16_len(uint16_t val){return 2;}
static inline uint32_t sc_buf_32_len(uint32_t val){return 4;}
static inline uint32_t sc_buf_64_len(uint64_t val){return 8;}
static inline uint32_t sc_buf_double_len(double val){return 8;}
// clang-format on

static inline uint32_t sc_buf_blob_len(void *ptr, uint32_t len)
{
    return len + sc_buf_32_len(len);
}

static inline uint32_t sc_buf_str_len(const char *str)
{
    return str == NULL ? sc_buf_32_len(-1) :
           sc_buf_32_len(-1) + strlen(str) + sc_buf_8_len('\0');
}

#endif
