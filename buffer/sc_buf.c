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

#include "sc_buf.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define IS_BIG_ENDIAN                                                          \
    (!(union sc_end {                                                          \
          uint16_t u16;                                                        \
          unsigned char c;                                                     \
      }){.u16 = 1}                                                             \
              .c)

#ifdef _MSC_VER
#include <stdlib.h>
    #define bswap_16(x) _byteswap_ushort(x)
    #define bswap_32(x) _byteswap_ulong(x)
    #define bswap_64(x) _byteswap_uint64(x)
#elif defined(__APPLE__)
// Mac OS X / Darwin features
    #include <libkern/OSByteOrder.h>
    #define bswap_16(x) OSSwapInt16(x)
    #define bswap_32(x) OSSwapInt32(x)
    #define bswap_64(x) OSSwapInt64(x)
#elif defined(__sun) || defined(sun)
#include <sys/byteorder.h>
    #define bswap_16(x) BSWAP_16(x)
    #define bswap_32(x) BSWAP_32(x)
    #define bswap_64(x) BSWAP_64(x)
#elif defined(__FreeBSD__)
#include <sys/endian.h>
    #define bswap_16(x) bswap16(x)
    #define bswap_32(x) bswap32(x)
    #define bswap_64(x) bswap64(x)
#elif defined(__OpenBSD__)
#include <sys/types.h>
    #define bswap_16(x) swap16(x)
    #define bswap_32(x) swap32(x)
    #define bswap_64(x) swap64(x)
#elif defined(__NetBSD__)
#include <machine/bswap.h>
    #include <sys/types.h>
    #if defined(__BSWAP_RENAME) && !defined(__bswap_32)
        #define bswap_16(x) bswap16(x)
        #define bswap_32(x) bswap32(x)
        #define bswap_64(x) bswap64(x)
    #endif
#else
#include <byteswap.h>
#endif

#define sc_swap16(n) (IS_BIG_ENDIAN ? bswap_16(n) : (n))
#define sc_swap32(n) (IS_BIG_ENDIAN ? bswap_32(n) : (n))
#define sc_swap64(n) (IS_BIG_ENDIAN ? bswap_64(n) : (n))

#define sy_buf_min(a, b) ((a) > (b) ? (b) : (a))

int sc_buf_init(struct sc_buf *buf, uint32_t cap)
{
    void *mem;

    mem = sc_buf_malloc(cap);
    if (mem == NULL) {
        *buf = (struct sc_buf) {0};
        return -1;
    }

    *buf = sc_buf_wrap(mem, cap, false);

    return 0;
}

struct sc_buf sc_buf_wrap(void *data, uint32_t len, bool ref)
{
    struct sc_buf buf = {
            .mem = data,
            .cap = len,
            .limit = ref ? len : UINT32_MAX,
            .wpos = 0,
            .rpos = 0,
            .ref = ref,
            .error = 0
    };

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
        sc_buf_compact(buf);

        if (buf->wpos + len > buf->cap) {
            size = ((buf->cap + len + 4095) / 4096) * 4096;
            if (size > buf->limit) {
                buf->error = SC_BUF_OOM;
                return false;
            }

            tmp = sc_buf_realloc(buf->mem, size);
            if (tmp == NULL) {
                return false;
            }

            buf->cap = size;
            buf->mem = tmp;
        }
    }

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
    assert(buf->wpos >= pos);
    buf->rpos = pos;
}

void sc_buf_set_wpos(struct sc_buf *buf, uint32_t pos)
{
    assert(pos <= buf->cap);
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

uint32_t sc_buf_peek_data(struct sc_buf *buf, uint32_t offset, void *dest,
                          uint32_t len)
{
    if (buf->error != 0 || (offset + len > buf->wpos)) {
        buf->error |= SC_BUF_CORRUPT;
        memset(dest, 0, len);
        return 0;
    }

    memcpy(dest, &buf->mem[offset], len);

    return len;
}

uint32_t sc_buf_peek_32_at(struct sc_buf *buf, uint32_t pos)
{
    uint32_t val;

    sc_buf_peek_data(buf, pos, &val, 4);

    return val;
}

uint32_t sc_buf_peek_32(struct sc_buf *buf)
{
    return sc_buf_peek_32_at(buf, buf->rpos);
}

uint64_t sc_buf_peek_64_at(struct sc_buf *buf, uint32_t pos)
{
    uint64_t val;

    sc_buf_peek_data(buf, pos, &val, 8);

    return val;
}

uint64_t sc_buf_peek_64(struct sc_buf *buf)
{
    return sc_buf_peek_64_at(buf, buf->rpos);
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
void sc_buf_set_32(struct sc_buf *buf, uint32_t val)
{
    sc_buf_set_data(buf, buf->wpos, &val, 4);
}

void sc_buf_set_64(struct sc_buf *buf, uint64_t val)
{
    sc_buf_set_data(buf, buf->wpos, &val, 8);
}

void sc_buf_set_8_at(struct sc_buf *buf, uint32_t pos, uint8_t val)
{
    sc_buf_set_data(buf, pos, &val, 1);
}

void sc_buf_set_32_at(struct sc_buf *buf, uint32_t pos, uint32_t val)
{
    sc_buf_set_data(buf, pos, &val, 4);
}

void sc_buf_set_64_at(struct sc_buf *buf, uint32_t pos, uint64_t val)
{
    sc_buf_set_data(buf, pos, &val, 8);
}

void sc_buf_get_raw(struct sc_buf *buf, void *dest, uint32_t len)
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

bool sc_buf_get_bool(struct sc_buf *buf)
{
    return sc_buf_get_8(buf);
}

void sc_buf_put_bool(struct sc_buf *buf, bool val)
{
    sc_buf_put_8(buf, val);
}

uint8_t sc_buf_get_8(struct sc_buf *buf)
{
    uint8_t val;

    sc_buf_get_raw(buf, &val, 1);

    return val;
}

void sc_buf_put_8(struct sc_buf *buf, uint8_t byte)
{
    sc_buf_put_raw(buf, &byte, sizeof(byte));
}

uint16_t sc_buf_get_16(struct sc_buf *buf)
{
    uint16_t val;

    sc_buf_get_raw(buf, &val, sizeof(val));

    return sc_swap16(val);
}

void sc_buf_put_16(struct sc_buf *buf, uint16_t val)
{
    uint16_t sw = sc_swap16(val);
    sc_buf_put_raw(buf, &sw, sizeof(sw));
}

uint32_t sc_buf_get_32(struct sc_buf *buf)
{
    uint32_t val;

    sc_buf_get_raw(buf, &val, sizeof(val));

    return sc_swap32(val);
}

void sc_buf_put_32(struct sc_buf *buf, uint32_t val)
{
    uint32_t sw = sc_swap32(val);
    sc_buf_put_raw(buf, &sw, sizeof(sw));
}

uint64_t sc_buf_get_64(struct sc_buf *buf)
{
    uint64_t val;

    sc_buf_get_raw(buf, &val, sizeof(val));

    return sc_swap64(val);
}

void sc_buf_put_64(struct sc_buf *buf, uint64_t val)
{
    uint64_t sw = sc_swap64(val);
    sc_buf_put_raw(buf, &sw, sizeof(sw));
}

double sc_buf_get_double(struct sc_buf *buf)
{
    double d;
    uint64_t val;

    sc_buf_get_raw(buf, &val, 8);
    val = sc_swap64(val);

    memcpy(&d, &val, 8);
    return d;
}

void sc_buf_put_double(struct sc_buf *buf, double val)
{
    uint64_t sw;
    memcpy(&sw, &val, 8);

    sw = sc_swap64(sw);
    sc_buf_put_raw(buf, &sw, 8);
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

void sc_buf_put_str(struct sc_buf *buf, const char *str)
{
    uint32_t size;

    if (str == NULL) {
        sc_buf_put_32(buf, -1);
        return;
    }

    size = strlen(str);

    sc_buf_put_32(buf, size);
    sc_buf_put_raw(buf, str, size + sc_buf_8_len('\0'));
}

void sc_buf_put_fmt(struct sc_buf *buf, const char *fmt, ...)
{
    int rc;
    uint32_t tmp;
    va_list args;
    void *mem = (char *) sc_buf_wbuf(buf) + sc_buf_32_len(0);
    uint32_t pos = sc_buf_wpos(buf);
    uint32_t quota = sc_buf_quota(buf) - sc_buf_32_len(0);

    va_start(args, fmt);
    rc = vsnprintf(mem, quota, fmt, args);
    va_end(args);

    if (rc < 0) {
        buf->error |= SC_BUF_CORRUPT;
        return;
    }

    if (rc >= quota) {
        if (!sc_buf_reserve(buf, rc)) {
            return;
        }

        mem = (char *) sc_buf_wbuf(buf) + sc_buf_32_len(0);
        quota = sc_buf_quota(buf) - sc_buf_32_len(0);

        va_start(args, fmt);
        rc = vsnprintf(mem, quota, fmt, args);
        va_end(args);

        if (rc < 0 || rc >= quota) {
            buf->error |= SC_BUF_OOM;
            return;
        }
    }

    tmp = sc_swap32(rc);
    sc_buf_set_data(buf, pos, &tmp, sizeof(tmp));
    sc_buf_mark_write(buf, rc + sc_buf_32_len(0) + sc_buf_8_len('\0'));
}

void sc_buf_put_text(struct sc_buf *buf, const char *fmt, ...)
{
    int rc;
    va_list args;
    uint32_t pos = 0;
    int offset = sc_buf_size(buf) > 0 ? 1 : 0;
    uint32_t quota = sc_buf_quota(buf);

    va_start(args, fmt);
    rc = vsnprintf((char*) sc_buf_wbuf(buf) - offset, quota, fmt, args);
    va_end(args);

    if (rc < 0) {
        buf->error |= SC_BUF_CORRUPT;
        sc_buf_set_wpos(buf, pos);
        return;
    }

    if (rc >= quota) {
        if (!sc_buf_reserve(buf, rc)) {
            sc_buf_set_wpos(buf, pos);
            return;
        }

        va_start(args, fmt);
        rc = vsnprintf((char*) sc_buf_wbuf(buf) - offset, quota, fmt, args);
        va_end(args);

        if (rc < 0 || rc >= quota) {
            sc_buf_set_wpos(buf, pos);
            buf->error = SC_BUF_OOM;
            return;
        }
    }

    sc_buf_mark_write(buf, rc + sc_buf_8_len('\0') - offset);
}

void *sc_buf_get_blob(struct sc_buf *buf, uint32_t len)
{
    if (len == 0) {
        return NULL;
    }

    void *blob = buf->mem + buf->rpos;
    buf->rpos += len;

    return blob;
}

void sc_buf_put_blob(struct sc_buf *buf, const void *ptr, uint32_t len)
{
    sc_buf_put_32(buf, len);
    sc_buf_put_raw(buf, ptr, len);
}

void sc_buf_move(struct sc_buf *dest, struct sc_buf *src)
{
    uint32_t size = sy_buf_min(sc_buf_quota(dest), sc_buf_size(src));

    sc_buf_put_raw(dest, &src->mem[sc_buf_rpos(src)], size);
    src->rpos += size;
}
