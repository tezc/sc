/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
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

#include "sc_buf.h"

#include <assert.h>
#include <stdio.h>

#ifndef SC_BUF_SIZE_MAX
    #define SC_BUF_SIZE_MAX UINT32_MAX
#endif

#define sc_buf_min(a, b) ((a) > (b) ? (b) : (a))

bool sc_buf_init(struct sc_buf *buf, uint32_t cap)
{
    void *mem = NULL;

    *buf = (struct sc_buf){0};

    if (cap > 0) {
        mem = sc_buf_calloc(1, cap);
        if (mem == NULL) {
            return false;
        }
    }

    *buf = sc_buf_wrap(mem, cap, 0);

    return true;
}

struct sc_buf sc_buf_wrap(void *data, uint32_t len, int flags)
{
    struct sc_buf buf = {.mem = data,
                         .cap = len,
                         .limit = flags & SC_BUF_REF ? len : SC_BUF_SIZE_MAX,
                         .wpos = flags & SC_BUF_DATA ? len : 0,
                         .rpos = 0,
                         .ref = (bool) (flags & SC_BUF_REF),
                         .error = 0};

    return buf;
}

void sc_buf_term(struct sc_buf *buf)
{
    if (!buf->ref) {
        sc_buf_free(buf->mem);
    }
}

void sc_buf_limit(struct sc_buf *buf, uint32_t limit)
{
    buf->limit = limit;
}

void *sc_buf_at(struct sc_buf *buf, uint32_t pos)
{
    return buf->mem + pos;
}

uint32_t sc_buf_cap(struct sc_buf *buf)
{
    return buf->cap;
}

bool sc_buf_reserve(struct sc_buf *buf, uint32_t len)
{
    uint32_t size;
    void *tmp;

    if (buf->wpos + len > buf->cap) {
        if (buf->ref) {
            goto error;
        }

        size = ((buf->cap + len + 4095) / 4096) * 4096;
        if (size > buf->limit || buf->cap >= SC_BUF_SIZE_MAX - 4096) {
            goto error;
        }

        tmp = sc_buf_realloc(buf->mem, size);
        if (tmp == NULL) {
            goto error;
        }

        buf->cap = size;
        buf->mem = tmp;
    }

    return true;

error:
    buf->error |= SC_BUF_OOM;
    return false;
}

bool sc_buf_shrink(struct sc_buf *buf, uint32_t len)
{
    void *tmp;

    sc_buf_compact(buf);

    if (len > buf->cap || buf->wpos >= len) {
        return true;
    }

    len = ((len + 4095) / 4096) * 4096;

    tmp = sc_buf_realloc(buf->mem, len);
    if (tmp == NULL) {
        buf->error |= SC_BUF_OOM;
        return false;
    }

    buf->cap = len;
    buf->mem = tmp;

    return true;
}

bool sc_buf_valid(struct sc_buf *buf)
{
    return buf->error == 0;
}

uint32_t sc_buf_quota(struct sc_buf *buf)
{
    return buf->cap - buf->wpos;
}

uint32_t sc_buf_size(struct sc_buf *buf)
{
    return buf->wpos - buf->rpos;
}

void sc_buf_clear(struct sc_buf *buf)
{
    buf->rpos = 0;
    buf->wpos = 0;
    buf->error = 0;
}

void sc_buf_mark_read(struct sc_buf *buf, uint32_t len)
{
    buf->rpos += len;
}

void sc_buf_mark_write(struct sc_buf *buf, uint32_t len)
{
    buf->wpos += len;
}

uint32_t sc_buf_rpos(struct sc_buf *buf)
{
    return buf->rpos;
}

void sc_buf_set_rpos(struct sc_buf *buf, uint32_t pos)
{
    if (pos > buf->wpos) {
        buf->error |= SC_BUF_CORRUPT;
    }

    buf->rpos = pos;
}

void sc_buf_set_wpos(struct sc_buf *buf, uint32_t pos)
{
    if (pos > buf->cap) {
        buf->error |= SC_BUF_CORRUPT;
    }

    buf->wpos = pos;
}

uint32_t sc_buf_wpos(struct sc_buf *buf)
{
    return buf->wpos;
}

void *sc_buf_rbuf(struct sc_buf *buf)
{
    return buf->mem + buf->rpos;
}

void *sc_buf_wbuf(struct sc_buf *buf)
{
    return buf->mem + buf->wpos;
}

void sc_buf_compact(struct sc_buf *buf)
{
    uint32_t copy;

    if (buf->rpos == buf->wpos) {
        buf->rpos = 0;
        buf->wpos = 0;
    }

    if (buf->rpos != 0) {
        copy = buf->wpos - buf->rpos;
        memmove(buf->mem, buf->mem + buf->rpos, copy);
        buf->rpos = 0;
        buf->wpos = copy;
    }
}

void sc_buf_move(struct sc_buf *dest, struct sc_buf *src)
{
    uint32_t size = sc_buf_min(sc_buf_quota(dest), sc_buf_size(src));

    sc_buf_put_raw(dest, &src->mem[sc_buf_rpos(src)], size);
    src->rpos += size;
}

static uint16_t sc_buf_peek_8_pos(struct sc_buf *buf, uint32_t pos,
                                  uint8_t *val)
{
    if (pos + sizeof(*val) > buf->wpos) {
        buf->error |= SC_BUF_CORRUPT;
        *val = 0;
        return 0;
    }

    *val = buf->mem[pos];

    return sizeof(*val);
}

static uint16_t sc_buf_peek_16_pos(struct sc_buf *buf, uint32_t pos,
                                   uint16_t *val)
{
    unsigned char *p;

    if (pos + sizeof(*val) > buf->wpos) {
        buf->error |= SC_BUF_CORRUPT;
        *val = 0;
        return 0;
    }

    p = &buf->mem[pos];

    *val = (uint16_t) p[0] << 0 | (uint16_t) p[1] << 8;

    return sizeof(*val);
}

static uint32_t sc_buf_peek_32_pos(struct sc_buf *buf, uint32_t pos,
                                   uint32_t *val)
{
    unsigned char *p;

    if (pos + sizeof(*val) > buf->wpos) {
        buf->error |= SC_BUF_CORRUPT;
        *val = 0;
        return 0;
    }

    p = &buf->mem[pos];

    *val = (uint32_t) p[0] << 0 | (uint32_t) p[1] << 8 | (uint32_t) p[2] << 16 |
           (uint32_t) p[3] << 24;

    return sizeof(*val);
}

static uint32_t sc_buf_peek_64_pos(struct sc_buf *buf, uint32_t pos,
                                   uint64_t *val)
{
    unsigned char *p;

    if (pos + sizeof(*val) > buf->wpos) {
        buf->error |= SC_BUF_CORRUPT;
        *val = 0;
        return 0;
    }

    p = &buf->mem[pos];

    *val = (uint64_t) p[0] << 0 | (uint64_t) p[1] << 8 | (uint64_t) p[2] << 16 |
           (uint64_t) p[3] << 24 | (uint64_t) p[4] << 32 |
           (uint64_t) p[5] << 40 | (uint64_t) p[6] << 48 |
           (uint64_t) p[7] << 56;

    return sizeof(*val);
}

static uint32_t sc_buf_set_8_pos(struct sc_buf *buf, uint32_t pos,
                                 const uint8_t *val)
{
    if (buf->error != 0 || (pos + sizeof(*val) > buf->cap)) {
        buf->error |= SC_BUF_CORRUPT;
        return 0;
    }

    buf->mem[pos] = *val;

    return sizeof(*val);
}

static uint32_t sc_buf_set_16_pos(struct sc_buf *buf, uint32_t pos,
                                  const uint16_t *val)
{
    unsigned char *p;

    if (buf->error != 0 || (pos + sizeof(*val) > buf->cap)) {
        buf->error |= SC_BUF_CORRUPT;
        return 0;
    }

    p = &buf->mem[pos];

    p[0] = (unsigned char) (*val >> 0);
    p[1] = (unsigned char) (*val >> 8);

    return sizeof(*val);
}

static uint32_t sc_buf_set_32_pos(struct sc_buf *buf, uint32_t pos,
                                  const uint32_t *val)
{
    unsigned char *p;

    if (buf->error != 0 || (pos + sizeof(*val) > buf->cap)) {
        buf->error |= SC_BUF_CORRUPT;
        return 0;
    }

    p = &buf->mem[pos];

    p[0] = (unsigned char) (*val >> 0);
    p[1] = (unsigned char) (*val >> 8);
    p[2] = (unsigned char) (*val >> 16);
    p[3] = (unsigned char) (*val >> 24);

    return sizeof(*val);
}

static uint32_t sc_buf_set_64_pos(struct sc_buf *buf, uint32_t pos,
                                  const uint64_t *val)
{
    unsigned char *p;

    if (buf->error != 0 || (pos + sizeof(*val) > buf->cap)) {
        buf->error |= SC_BUF_CORRUPT;
        return 0;
    }

    p = &buf->mem[pos];

    p[0] = (unsigned char) (*val >> 0);
    p[1] = (unsigned char) (*val >> 8);
    p[2] = (unsigned char) (*val >> 16);
    p[3] = (unsigned char) (*val >> 24);
    p[4] = (unsigned char) (*val >> 32);
    p[5] = (unsigned char) (*val >> 40);
    p[6] = (unsigned char) (*val >> 48);
    p[7] = (unsigned char) (*val >> 56);

    return sizeof(*val);
}

uint8_t sc_buf_peek_8_at(struct sc_buf *buf, uint32_t pos)
{
    uint8_t val;

    sc_buf_peek_8_pos(buf, pos, &val);
    return val;
}

uint16_t sc_buf_peek_16_at(struct sc_buf *buf, uint32_t pos)
{
    uint16_t val;

    sc_buf_peek_16_pos(buf, pos, &val);
    return val;
}

uint32_t sc_buf_peek_32_at(struct sc_buf *buf, uint32_t pos)
{
    uint32_t val;

    sc_buf_peek_32_pos(buf, pos, &val);
    return val;
}

uint64_t sc_buf_peek_64_at(struct sc_buf *buf, uint32_t pos)
{
    uint64_t val;

    sc_buf_peek_64_pos(buf, pos, &val);
    return val;
}

uint8_t sc_buf_peek_8(struct sc_buf *buf)
{
    return sc_buf_peek_8_at(buf, buf->rpos);
}

uint16_t sc_buf_peek_16(struct sc_buf *buf)
{
    return sc_buf_peek_16_at(buf, buf->rpos);
}

uint32_t sc_buf_peek_32(struct sc_buf *buf)
{
    return sc_buf_peek_32_at(buf, buf->rpos);
}

uint64_t sc_buf_peek_64(struct sc_buf *buf)
{
    return sc_buf_peek_64_at(buf, buf->rpos);
}

uint32_t sc_buf_peek_data(struct sc_buf *buf, uint32_t pos, unsigned char *dest,
                          uint32_t len)
{
    if (buf->error != 0 || (pos + len > buf->wpos)) {
        buf->error |= SC_BUF_CORRUPT;
        memset(dest, 0, len);
        return 0;
    }

    memcpy(dest, &buf->mem[pos], len);

    return len;
}

void sc_buf_set_8_at(struct sc_buf *buf, uint32_t pos, uint8_t val)
{
    sc_buf_set_8_pos(buf, pos, &val);
}

void sc_buf_set_16_at(struct sc_buf *buf, uint32_t pos, uint16_t val)
{
    sc_buf_set_16_pos(buf, pos, &val);
}

void sc_buf_set_32_at(struct sc_buf *buf, uint32_t pos, uint32_t val)
{
    sc_buf_set_32_pos(buf, pos, &val);
}

void sc_buf_set_64_at(struct sc_buf *buf, uint32_t pos, uint64_t val)
{
    sc_buf_set_64_pos(buf, pos, &val);
}

void sc_buf_set_8(struct sc_buf *buf, uint8_t val)
{
    sc_buf_set_8_at(buf, buf->wpos, val);
}

void sc_buf_set_16(struct sc_buf *buf, uint16_t val)
{
    sc_buf_set_16_at(buf, buf->wpos, val);
}

void sc_buf_set_32(struct sc_buf *buf, uint32_t val)
{
    sc_buf_set_32_at(buf, buf->wpos, val);
}

void sc_buf_set_64(struct sc_buf *buf, uint64_t val)
{
    sc_buf_set_64_at(buf, buf->wpos, val);
}

uint32_t sc_buf_set_data(struct sc_buf *buf, uint32_t offset, const void *src,
                         uint32_t len)
{
    if (buf->error != 0 || (offset + len > buf->cap)) {
        buf->error |= SC_BUF_CORRUPT;
        return 0;
    }

    memcpy(&buf->mem[offset], src, len);
    return len;
}

bool sc_buf_get_bool(struct sc_buf *buf)
{
    return sc_buf_get_8(buf);
}

uint8_t sc_buf_get_8(struct sc_buf *buf)
{
    uint8_t val;

    buf->rpos += sc_buf_peek_8_pos(buf, buf->rpos, &val);

    return val;
}

uint16_t sc_buf_get_16(struct sc_buf *buf)
{
    uint16_t val;

    buf->rpos += sc_buf_peek_16_pos(buf, buf->rpos, &val);

    return val;
}

uint32_t sc_buf_get_32(struct sc_buf *buf)
{
    uint32_t val;

    buf->rpos += sc_buf_peek_32_pos(buf, buf->rpos, &val);

    return val;
}

uint64_t sc_buf_get_64(struct sc_buf *buf)
{
    uint64_t val;

    buf->rpos += sc_buf_peek_64_pos(buf, buf->rpos, &val);

    return val;
}

double sc_buf_get_double(struct sc_buf *buf)
{
    double d;
    uint64_t val;

    val = sc_buf_get_64(buf);
    memcpy(&d, &val, 8);

    return d;
}

const char *sc_buf_get_str(struct sc_buf *buf)
{
    int len;
    const char *str;

    len = sc_buf_get_32(buf);
    if (len == -1) {
        return NULL;
    }

    str = (char *) buf->mem + buf->rpos;
    buf->rpos += len + 1;

    return str;
}

void *sc_buf_get_blob(struct sc_buf *buf, uint32_t len)
{
    void *blob;

    if (len == 0) {
        return NULL;
    }

    if (buf->rpos + len > buf->wpos) {
        buf->error |= SC_BUF_CORRUPT;
        return NULL;
    }

    blob = buf->mem + buf->rpos;
    buf->rpos += len;

    return blob;
}

void sc_buf_get_data(struct sc_buf *buf, void *dest, uint32_t len)
{
    if (buf->rpos + len > buf->wpos) {
        buf->error |= SC_BUF_CORRUPT;
        memset(dest, 0, len);
        return;
    }

    buf->rpos += sc_buf_peek_data(buf, buf->rpos, dest, len);
}

void sc_buf_put_raw(struct sc_buf *buf, const void *ptr, uint32_t len)
{
    if (!sc_buf_reserve(buf, len)) {
        return;
    }

    buf->wpos += sc_buf_set_data(buf, buf->wpos, ptr, len);
}

void sc_buf_put_bool(struct sc_buf *buf, bool val)
{
    sc_buf_put_8(buf, (uint8_t) val);
}

void sc_buf_put_8(struct sc_buf *buf, uint8_t val)
{
    if (!sc_buf_reserve(buf, sizeof(val))) {
        return;
    }

    buf->wpos += sc_buf_set_8_pos(buf, buf->wpos, &val);
}

void sc_buf_put_16(struct sc_buf *buf, uint16_t val)
{
    if (!sc_buf_reserve(buf, sizeof(val))) {
        return;
    }

    buf->wpos += sc_buf_set_16_pos(buf, buf->wpos, &val);
}

void sc_buf_put_32(struct sc_buf *buf, uint32_t val)
{
    if (!sc_buf_reserve(buf, sizeof(val))) {
        return;
    }

    buf->wpos += sc_buf_set_32_pos(buf, buf->wpos, &val);
}


void sc_buf_put_64(struct sc_buf *buf, uint64_t val)
{
    if (!sc_buf_reserve(buf, sizeof(val))) {
        return;
    }

    buf->wpos += sc_buf_set_64_pos(buf, buf->wpos, &val);
}

void sc_buf_put_double(struct sc_buf *buf, double val)
{
    uint64_t sw;

    memcpy(&sw, &val, 8);
    sc_buf_put_64(buf, sw);
}

void sc_buf_put_str(struct sc_buf *buf, const char *str)
{
    size_t size;

    if (str == NULL) {
        sc_buf_put_32(buf, UINT32_MAX);
        return;
    }

    size = strlen(str);
    if (size >= UINT32_MAX) {
        buf->error |= SC_BUF_CORRUPT;
        return;
    }

    sc_buf_put_32(buf, (uint32_t) size);
    sc_buf_put_raw(buf, str, (uint32_t) size + sc_buf_8_len('\0'));
}

void sc_buf_put_str_len(struct sc_buf *buf, const char *str, int len)
{
    assert(len >= 0);

    if (str == NULL) {
        sc_buf_put_32(buf, UINT32_MAX);
        return;
    }

    sc_buf_put_32(buf, (uint32_t) len);
    sc_buf_put_raw(buf, str, (uint32_t) len);
    sc_buf_put_8(buf, '\0');
}

void sc_buf_put_fmt(struct sc_buf *buf, const char *fmt, ...)
{
    int rc;
    va_list args;
    void *mem = (char *) sc_buf_wbuf(buf) + sc_buf_32_len(0);
    uint32_t wr;
    uint32_t pos = sc_buf_wpos(buf);
    uint32_t quota = sc_buf_quota(buf);

    if (quota > sc_buf_32_len(0)) {
        quota -= sc_buf_32_len(0);
    } else {
        quota = 0;
    }

    va_start(args, fmt);
    rc = vsnprintf(mem, quota, fmt, args);
    va_end(args);

    if (rc < 0) {
        buf->error |= SC_BUF_CORRUPT;
        return;
    }

    wr = (uint32_t) rc;

    if (wr >= quota) {
        if (!sc_buf_reserve(buf, wr + sc_buf_32_len(0))) {
            return;
        }

        mem = (char *) sc_buf_wbuf(buf) + sc_buf_32_len(0);
        quota = sc_buf_quota(buf) - sc_buf_32_len(0);

        va_start(args, fmt);
        rc = vsnprintf(mem, quota, fmt, args);
        va_end(args);

        if (rc < 0 || (uint32_t) rc >= quota) {
            buf->error |= SC_BUF_OOM;
            return;
        }

        wr = (uint32_t) rc;
    }

    sc_buf_set_32_at(buf, pos, wr);
    sc_buf_mark_write(buf, wr + sc_buf_32_len(0) + sc_buf_8_len('\0'));
}

void sc_buf_put_text(struct sc_buf *buf, const char *fmt, ...)
{
    int rc;
    uint32_t written;
    va_list args;
    int offset = sc_buf_size(buf) > 0 ? 1 : 0;
    uint32_t quota = sc_buf_quota(buf);

    va_start(args, fmt);
    rc = vsnprintf((char *) sc_buf_wbuf(buf) - offset, quota, fmt, args);
    va_end(args);

    if (rc < 0) {
        buf->error |= SC_BUF_CORRUPT;
        sc_buf_set_wpos(buf, 0);
        return;
    }

    written = (uint32_t) rc;

    if (written >= quota) {
        if (!sc_buf_reserve(buf, written + 1)) {
            sc_buf_set_wpos(buf, 0);
            return;
        }

        va_start(args, fmt);
        rc = vsnprintf((char *) sc_buf_wbuf(buf) - offset, quota, fmt, args);
        va_end(args);

        if (rc < 0 || (uint32_t) rc >= sc_buf_quota(buf)) {
            sc_buf_set_wpos(buf, 0);
            buf->error = SC_BUF_OOM;
            return;
        }
    }

    sc_buf_mark_write(buf, rc + sc_buf_8_len('\0') - offset);
}

void sc_buf_put_blob(struct sc_buf *buf, const void *ptr, uint32_t len)
{
    sc_buf_put_32(buf, len);
    sc_buf_put_raw(buf, ptr, len);
}
