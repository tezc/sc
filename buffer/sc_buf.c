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
#include <memory.h>
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

void sc_buf_init(struct sc_buf *buf, uint32_t cap)
{
    void *mem = sc_buf_malloc(cap);
    *buf = sc_buf_wrap(mem, cap, false);
}

struct sc_buf sc_buf_wrap(void *data, uint32_t len, bool ref)
{
    struct sc_buf buf = {
            .mem = data,
            .cap = len,
            .limit = UINT32_MAX,
            .write_pos = 0,
            .read_pos = 0,
            .ref = ref,
            .corrupt = false,
            .oom = false,
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
    assert(limit < 1 * 1024 * 1024 * 1024);

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

static bool sc_buf_reserve(struct sc_buf *buf, uint32_t len)
{
    uint32_t size;

    if (buf->write_pos + len > buf->cap) {
        sc_buf_compact(buf);

        if (buf->write_pos + len > buf->cap) {
            size = ((buf->cap + len + 4095) / 4096) * 4096;
            if (size > buf->limit) {
                buf->corrupt = true;
                buf->oom = true;
                return false;
            }

            buf->cap = size;
            buf->mem = sc_buf_realloc(buf->mem, buf->cap);
        }
    }

    return true;
}

bool sc_buf_is_valid(struct sc_buf *buf)
{
    return !buf->corrupt;
}

uint32_t sc_buf_quota(struct sc_buf *buf)
{
    return buf->cap - buf->write_pos;
}

uint32_t sc_buf_count(struct sc_buf *buf)
{
    return buf->write_pos - buf->read_pos;
}

void sc_buf_clear(struct sc_buf *buf)
{
    buf->read_pos = 0;
    buf->write_pos = 0;
}

void sc_buf_mark_read(struct sc_buf *buf, uint32_t len)
{
    buf->read_pos += len;
}

void sc_buf_mark_write(struct sc_buf *buf, uint32_t len)
{
    buf->write_pos += len;
}

uint32_t sc_buf_get_read_pos(struct sc_buf *buf)
{
    return buf->read_pos;
}

void sc_buf_set_read_pos(struct sc_buf *buf, uint32_t pos)
{
    assert(buf->write_pos >= pos);
    buf->read_pos = pos;
}

void sc_buf_set_write_pos(struct sc_buf *buf, uint32_t pos)
{
    assert(pos <= buf->cap);
    buf->write_pos = pos;
}

uint32_t sc_buf_get_write_pos(struct sc_buf *buf)
{
    return buf->write_pos;
}

void *sc_buf_read_buf(struct sc_buf *buf)
{
    return buf->mem + buf->read_pos;
}

void *sc_buf_write_buf(struct sc_buf *buf)
{
    return buf->mem + buf->write_pos;
}

void sc_buf_memset(struct sc_buf *buf, int val, uint32_t offset, uint32_t len)
{
    assert(offset + len < buf->cap);

    memset(buf->mem + offset, val, len);
}

void sc_buf_compact(struct sc_buf *buf)
{
    uint32_t copy;

    if (buf->read_pos == buf->write_pos) {
        buf->read_pos = 0;
        buf->write_pos = 0;
    }

    if (buf->read_pos != 0) {
        copy = buf->write_pos - buf->read_pos;
        memmove(buf->mem, buf->mem + buf->read_pos, copy);
        buf->read_pos = 0;
        buf->write_pos = copy;
    }
}

uint32_t sc_buf_peek_data(struct sc_buf *buf, uint32_t offset, void *dest,
                          uint32_t len)
{
    if (buf->corrupt || (offset + len > buf->write_pos)) {
        buf->corrupt = true;
        memset(dest, 0, len);
        return 0;
    }

    memcpy(dest, &buf->mem[offset], len);

    return len;
}

uint32_t sc_buf_set_data(struct sc_buf *buf, uint32_t offset, const void *src,
                         uint32_t len)
{
    if (buf->corrupt || (offset + len > buf->cap)) {
        buf->corrupt = true;
        return 0;
    }

    memcpy(&buf->mem[offset], src, len);
    return len;
}

void sc_buf_get_data(struct sc_buf *buf, void *dest, uint32_t len)
{
    if (buf->read_pos + len > buf->write_pos) {
        buf->corrupt = true;
        memset(dest, 0, len);
        return;
    }

    buf->read_pos += sc_buf_peek_data(buf, buf->read_pos, dest, len);
}

void sc_buf_put_data(struct sc_buf *buf, const void *ptr, uint32_t len)
{
    if (!sc_buf_reserve(buf, len)) {
        return;
    }

    buf->write_pos += sc_buf_set_data(buf, buf->write_pos, ptr, len);
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

    sc_buf_get_data(buf, &val, 1);

    return val;
}

void sc_buf_put_8(struct sc_buf *buf, uint8_t byte)
{
    sc_buf_put_data(buf, &byte, sizeof(byte));
}

uint16_t sc_buf_get_16(struct sc_buf *buf)
{
    uint16_t val;

    sc_buf_get_data(buf, &val, sizeof(val));

    return sc_swap16(val);
}

void sc_buf_put_16(struct sc_buf *buf, uint16_t val)
{
    uint16_t sw = sc_swap16(val);
    sc_buf_put_data(buf, &sw, sizeof(sw));
}

uint32_t sc_buf_peek_32_at(struct sc_buf *buf, uint32_t pos)
{
    uint32_t val;

    sc_buf_peek_data(buf, pos, &val, 4);

    return sc_swap32(val);
}

uint32_t sc_buf_peek_32(struct sc_buf *buf)
{
    return sc_buf_peek_32_at(buf, buf->read_pos);
}

uint32_t sc_buf_get_32(struct sc_buf *buf)
{
    uint32_t val;

    sc_buf_get_data(buf, &val, sizeof(val));

    return sc_swap32(val);
}

void sc_buf_set_32_at(struct sc_buf *buf, uint32_t pos, uint32_t val)
{
    uint32_t sw = sc_swap32(val);
    sc_buf_set_data(buf, pos, &sw, sizeof(sw));
}

void sc_buf_set_32(struct sc_buf *buf, uint32_t val)
{
    uint32_t sw = sc_swap32(val);
    sc_buf_set_data(buf, buf->write_pos, &sw, sizeof(sw));
}

void sc_buf_put_32(struct sc_buf *buf, uint32_t val)
{
    uint32_t sw = sc_swap32(val);
    sc_buf_put_data(buf, &sw, sizeof(sw));
}

uint64_t sc_buf_peek_64_at(struct sc_buf *buf, uint32_t pos)
{
    uint64_t val;

    sc_buf_peek_data(buf, pos, &val, sizeof(val));

    return sc_swap64(val);
}

uint64_t sc_buf_get_64(struct sc_buf *buf)
{
    uint64_t val;

    sc_buf_get_data(buf, &val, sizeof(val));

    return sc_swap64(val);
}

void sc_buf_set_64_at(struct sc_buf *buf, uint32_t pos, uint64_t val)
{
    uint64_t sw = sc_swap64(val);
    sc_buf_set_data(buf, pos, &sw, sizeof(sw));
}

void sc_buf_put_64(struct sc_buf *buf, uint64_t val)
{
    uint64_t sw = sc_swap64(val);
    sc_buf_put_data(buf, &sw, sizeof(sw));
}

double sc_buf_get_double(struct sc_buf *buf)
{
    double d;
    uint64_t val;

    sc_buf_get_data(buf, &val, 8);
    val = sc_swap64(val);

    memcpy(&d, &val, 8);
    return d;
}

void sc_buf_put_double(struct sc_buf *buf, double val)
{
    uint64_t sw;
    memcpy(&sw, &val, 8);

    sw = sc_swap64(sw);
    sc_buf_put_data(buf, &sw, 8);
}

uint32_t sc_buf_peek_strlen(struct sc_buf *buf)
{
    int len;

    len = sc_buf_peek_32(buf);
    if (len == -1) {
        return 0;
    }

    return len;
}

const char *sc_buf_get_str(struct sc_buf *buf)
{
    int len;
    const char *str;

    len = sc_buf_get_32(buf);
    if (len == -1) {
        return NULL;
    }

    str = (char *) buf->mem + buf->read_pos;
    buf->read_pos += len + 1;

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
    sc_buf_put_data(buf, str, size + sc_buf_8bit_len('\0'));
}

void sc_buf_put_as_str(struct sc_buf *buf, const char *fmt, ...)
{
    int rc;
    va_list args;
    void *mem = (char *) sc_buf_write_buf(buf) + sc_buf_32bit_len(0);
    uint32_t pos = sc_buf_get_write_pos(buf);
    uint32_t quota = sc_buf_quota(buf) - sc_buf_32bit_len(0);

    va_start(args, fmt);
    rc = vsnprintf(mem, quota, fmt, args);
    va_end(args);

    if (rc >= quota) {
        buf->corrupt = true;
        return;
    }

    sc_buf_set_32_at(buf, pos, rc);
    sc_buf_mark_write(buf, rc + sc_buf_32bit_len(0) + sc_buf_8bit_len('\0'));
}

void *sc_buf_get_blob(struct sc_buf *buf, uint32_t len)
{
    if (len == 0) {
        return NULL;
    }

    void *blob = buf->mem + buf->read_pos;
    buf->read_pos += len;

    return blob;
}

void sc_buf_put_blob(struct sc_buf *buf, const void *ptr, uint32_t len)
{
    sc_buf_put_32(buf, len);
    sc_buf_put_data(buf, ptr, len);
}

void sc_buf_move(struct sc_buf *dest, struct sc_buf *src)
{
    uint32_t size = sy_buf_min(sc_buf_quota(dest), sc_buf_count(src));

    sc_buf_put_data(dest, &src->mem[sc_buf_get_read_pos(src)], size);
    src->read_pos += size;
}

// clang-format off
uint32_t sc_buf_bool_len(bool val) { return 1;}
uint32_t sc_buf_8bit_len(uint8_t val) {return 1;}
uint32_t sc_buf_16bit_len(uint16_t val){return 2;}
uint32_t sc_buf_32bit_len(uint32_t val){return 4;}
uint32_t sc_buf_64bit_len(uint64_t val){return 8;}
uint32_t sc_buf_double_len(double val){return 8;}
// clang-format on

uint32_t sc_buf_strlen(const char *str)
{
    size_t size;

    if (str == NULL) {
        return sc_buf_32bit_len(-1);
    }

    size = strlen(str);

    return sc_buf_32bit_len(size) + size + sc_buf_8bit_len('\0');
}

uint32_t sc_buf_blob_len(void *ptr, uint32_t len)
{
    return len + sc_buf_32bit_len(len);
}
