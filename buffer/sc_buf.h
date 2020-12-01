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

#define SC_BUF_CORRUPT 1u
#define SC_BUF_OOM 3u

struct sc_buf
{
    unsigned char *mem;
    uint32_t cap;
    uint32_t limit;
    uint32_t read_pos;
    uint32_t write_pos;
    unsigned int error;

    bool ref;
};

#define sc_buf_malloc malloc
#define sc_buf_realloc realloc
#define sc_buf_free free

int sc_buf_init(struct sc_buf *buf, uint32_t cap);
void sc_buf_term(struct sc_buf *buf);

struct sc_buf sc_buf_wrap(void *data, uint32_t len, bool ref);

void sc_buf_limit(struct sc_buf *buf, uint32_t limit);

void *sc_buf_at(struct sc_buf *buf, uint32_t pos);
uint32_t sc_buf_cap(struct sc_buf *buf);

bool sc_buf_valid(struct sc_buf *buf);
uint32_t sc_buf_quota(struct sc_buf *buf);
uint32_t sc_buf_size(struct sc_buf *buf);
void sc_buf_clear(struct sc_buf *buf);
void sc_buf_compact(struct sc_buf *buf);

void sc_buf_mark_read(struct sc_buf *buf, uint32_t len);
void sc_buf_mark_write(struct sc_buf *buf, uint32_t len);

uint32_t sc_buf_get_read_pos(struct sc_buf *buf);
void sc_buf_set_read_pos(struct sc_buf *buf, uint32_t pos);

uint32_t sc_buf_get_write_pos(struct sc_buf *buf);
void sc_buf_set_write_pos(struct sc_buf *buf, uint32_t pos);

void *sc_buf_read_buf(struct sc_buf *buf);
void *sc_buf_write_buf(struct sc_buf *buf);


uint32_t sc_buf_peek_data(struct sc_buf *buf, uint32_t offset, void *dest,
                       uint32_t len);
uint32_t sc_buf_set_data(struct sc_buf *buf, uint32_t offset, const void *src,
                      uint32_t len);

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

uint32_t sc_buf_bool_len(bool val);
uint32_t sc_buf_8bit_len(uint8_t val);
uint32_t sc_buf_16bit_len(uint16_t val);
uint32_t sc_buf_32bit_len(uint32_t val);
uint32_t sc_buf_64bit_len(uint64_t val);
uint32_t sc_buf_double_len(double val);
uint32_t sc_buf_strlen(const char *str);
uint32_t sc_buf_blob_len(void *ptr, uint32_t len);

#endif
