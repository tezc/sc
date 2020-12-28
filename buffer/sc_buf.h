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

#include <assert.h>
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

/**
 * Create buffer
 *
 * @param buf buffer
 * @param cap initial capacity
 * @return    'false' on out of memory.
 */
bool sc_buf_init(struct sc_buf *buf, uint32_t cap);

/**
 * Destroy buffer
 * @param buf  buffer
 */
void sc_buf_term(struct sc_buf *buf);

/**
 * Wrap memory into buffer, write position will be '0' after this call.
 * Useful for using preallocated or stack allocated buffers.
 *
 * @param data data pointer
 * @param len  len
 * @param ref  if set 'true', buffer will not try to expand itself and
 *             'sc_buf_term' will not try to 'free()' buffer.
 * @return
 */
struct sc_buf sc_buf_wrap(void *data, uint32_t len, bool ref);

/**
 * Set limit of the buffer, when buffer reaches limit, it will set buffer's
 * 'out of memory' flag. Default is UINT32_MAX.
 *
 * @param buf   buffer
 * @param limit limit
 */
void sc_buf_limit(struct sc_buf *buf, uint32_t limit);

/**
 *
 * @param buf buffer
 * @param pos pos
 * @return    buffer address at 'pos'
 */
void *sc_buf_at(struct sc_buf *buf, uint32_t pos);

/**
 *
 * @param buf buffer
 * @return    current capacity.
 */
uint32_t sc_buf_cap(struct sc_buf *buf);

/**
 *
 * @param buf buffer
 * @param len len
 * @return    'false' on out of memory or hits the 'limit'.
 *            'out memory flag' will be set to check it later.
 */
bool sc_buf_reserve(struct sc_buf *buf, uint32_t len);

/**
 *
 * @param buf buffer
 * @return    'true' if buffer is valid. Buffer can be invalid either on
 *            allocation failure or when you try to read data more than buffer
 *            has.
 */
bool sc_buf_valid(struct sc_buf *buf);

/**
 * @param buf buffer
 * @return    current remaining space to write
 */
uint32_t sc_buf_quota(struct sc_buf *buf);

/**
 * @param buf buffer
 * @return    current byte count in the buffer
 */
uint32_t sc_buf_size(struct sc_buf *buf);

/**
 * Set read and write position to '0', clear error flag.
 * @param buf buffer
 */
void sc_buf_clear(struct sc_buf *buf);

/**
 * Compact buffer, e.g if bytes in buffer at [3, 20],
 * it will be moved to [0, 17]
 * @param buf buffer
 */
void sc_buf_compact(struct sc_buf *buf);

/**
 * Advance read position, useful when you pass underlying array to another
 * function which operates on void*. e.g socket write() call.
 * @param buf buffer
 * @param len len
 */
void sc_buf_mark_read(struct sc_buf *buf, uint32_t len);

/**
 * Advance read position, useful when you pass underlying array to another
 * function which operates on void*. e.g socket read() call.
 * @param buf buffer
 * @param len len
 */
void sc_buf_mark_write(struct sc_buf *buf, uint32_t len);

/**
 * @param buf buffer
 * @return    current read position
 */
uint32_t sc_buf_rpos(struct sc_buf *buf);

/**
 * @param buf buffer
 * @return    new read position
 */
void sc_buf_set_rpos(struct sc_buf *buf, uint32_t pos);

/**
 * @param buf buffer
 * @return    current write position
 */
uint32_t sc_buf_wpos(struct sc_buf *buf);

/**
 * @param buf buffer
 * @return    new write position
 */
void sc_buf_set_wpos(struct sc_buf *buf, uint32_t pos);

/**
 * Get address of read position. Useful for e.g : write(fd, sc_buf_rbuf(buf) ..)
 * @param buf buffer
 * @return    read address
 */
void *sc_buf_rbuf(struct sc_buf *buf);

/**
 * Get address of write position. Useful for e.g : read(fd, sc_buf_wbuf(buf) ..)
 * @param buf buffer
 * @return    write address
 */
void *sc_buf_wbuf(struct sc_buf *buf);

/**
 * Read data without advancing read position. Useful when you want to read from
 * buffer without moving altering it.
 *
 * @param buf    buffer
 * @param pos    read position
 * @param dest   dest
 * @param len    bytes to read
 * @return       returns 'len' on success, '0' on failure and error flags will
 *               be set.
 */
uint32_t sc_buf_peek_data(struct sc_buf *buf, uint32_t pos, void *dest,
                          uint32_t len);

/**
 * Syntactic sugar for 32 bit 'sc_buf_peek_data()'
 *
 * @param buf buffer
 * @param pos pos
 * @return    32 bit value
 */
uint32_t sc_buf_peek_32_at(struct sc_buf *buf, uint32_t pos);

/**
 * Syntactic sugar for 32 bit 'sc_buf_peek_data()'
 *
 * @param buf buffer
 * @return    32 bit value from current read position
 */
uint32_t sc_buf_peek_32(struct sc_buf *buf);

/**
 * Syntactic sugar for 64 bit 'sc_buf_peek_data()'
 *
 * @param buf buffer
 * @param pos pos
 * @return    64 bit value
 */
uint64_t sc_buf_peek_64_at(struct sc_buf *buf, uint32_t pos);

/**
 * Syntactic sugar for 64 bit 'sc_buf_peek_data()'
 *
 * @param buf buffer
 * @return    64 bit value from current read position
 */
uint64_t sc_buf_peek_64(struct sc_buf *buf);

/**
 * Write bytes without advancing write position, if pos + len if out of bounds,
 * error flags will be set.
 *
 * @param buf buffer
 * @param pos write position
 * @param src source
 * @param len source len
 * @return    'len'
 */
uint32_t sc_buf_set_data(struct sc_buf *buf, uint32_t pos, const void *src,
                         uint32_t len);

/**
 * Syntactic sugar for 'sc_buf_set_data()'
 *
 * @param buf  buffer
 * @param val  value to write at current write position without advancing write
 *             position
 */
void sc_buf_set_32(struct sc_buf *buf, uint32_t val);

/**
 * Syntactic sugar for 'sc_buf_set_data()'
 *
 * @param buf  buffer
 * @param val  value to write at current write position without advancing write
 *             position
 */
void sc_buf_set_64(struct sc_buf *buf, uint64_t val);


/**
 * Syntactic sugar for 'sc_buf_set_data()'
 *
 * @param buf  buffer
 * @param pos  write position
 * @param val  value to write at current write position without advancing write
 *             position
 */
void sc_buf_set_8_at(struct sc_buf *buf, uint32_t pos, uint8_t val);

/**
 * Syntactic sugar for 'sc_buf_set_data()'
 *
 * @param buf  buffer
 * @param pos  write position
 * @param val  value to write at current write position without advancing write
 *             position
 */
void sc_buf_set_32_at(struct sc_buf *buf, uint32_t pos, uint32_t val);

/**
 * Syntactic sugar for 'sc_buf_set_data()'
 *
 * @param buf  buffer
 * @param pos  write position
 * @param val  value to write at current write position without advancing write
 *             position
 */
void sc_buf_set_64_at(struct sc_buf *buf, uint32_t pos, uint64_t val);

/**
 * Read 'len' bytes from buffer, if buffer doesn't have 'len' bytes, error
 * flag will be set.
 *
 * @param buf  buffer
 * @param dest dest
 * @param len  len
 */
void sc_buf_get_raw(struct sc_buf *buf, void *dest, uint32_t len);

/**
 * Write 'len' bytes to buffer, on out of memory, error flag will be set.
 *
 * @param buf  buffer
 * @param dest dest
 * @param len  len
 */
void sc_buf_put_raw(struct sc_buf *buf, const void *ptr, uint32_t len);

/**
 * Read boolean value
 *
 * @param buf buffer
 * @return    boolean value
 */
bool sc_buf_get_bool(struct sc_buf *buf);

/**
 * Write boolean value
 *
 * @param buf buffer
 * @param    boolean value
 */
void sc_buf_put_bool(struct sc_buf *buf, bool val);

/**
 * Read 8 bit value
 * @param buf buffer
 * @return    8 bit value
 */
uint8_t sc_buf_get_8(struct sc_buf *buf);

/**
 * Write 8 bit value
 * @param buf   buffer
 * @param val  8 bit value
 */
void sc_buf_put_8(struct sc_buf *buf, uint8_t val);

/**
 * Read 16 bit value
 * @param buf  buffer
 * @return     16 bit value
 */
uint16_t sc_buf_get_16(struct sc_buf *buf);

/**
 * Write 16 bit value
 * @param buf   buffer
 * @param val  16 bit value
 */
void sc_buf_put_16(struct sc_buf *buf, uint16_t val);

/**
 * Read 32 bit value
 * @param buf  buffer
 * @return     32 bit value
 */
uint32_t sc_buf_get_32(struct sc_buf *buf);

/**
 * Write 32 bit value
 * @param buf   buffer
 * @param val  32 bit value
 */
void sc_buf_put_32(struct sc_buf *buf, uint32_t val);

/**
 * Read 64 bit value
 * @param buf  buffer
 * @return     64 bit value
 */
uint64_t sc_buf_get_64(struct sc_buf *buf);

/**
 * Write 64 bit value
 * @param buf   buffer
 * @param val  64 bit value
 */
void sc_buf_put_64(struct sc_buf *buf, uint64_t val);

/**
 * Read double (8 bytes)
 * @param buf  buffer
 * @return     double
 */
double sc_buf_get_double(struct sc_buf *buf);

/**
 * Write double (8 bytes)
 * @param buf  buffer
 * @param val  double
 */
void sc_buf_put_double(struct sc_buf *buf, double val);

/**
 * Read string. Strings are stored as [4 bytes length][string bytes]['\0'].
 * So you can use return value without copying/freeing until you overwrite it
 * in the buffer.
 *
 * @param buf buffer
 * @return    Pointer to string, possibly NULL if NULL has been put before.
 */
const char *sc_buf_get_str(struct sc_buf *buf);

/**
 * Write string. Strings are stored as [4 bytes length][string bytes]['\0'].
 * NULL values are accepted, stored as [4 bytes length] = -1.
 *
 * @param buf buffer
 * @param str string
 */
void sc_buf_put_str(struct sc_buf *buf, const char *str);

/**
 * Put formatted string, passes arguments to vsnprintf
 * @param buf buffer
 * @param fmt fmt
 * @param ... arguments
 */
void sc_buf_put_fmt(struct sc_buf *buf, const char *fmt, ...);


/**
 * Put formatted string, passes arguments to vsnprintf but concatenates strings.
 * Only useful if you want to append strings. It doesn't store string as length
 * prefixed string.
 *
 * @param buf buffer
 * @param fmt fmt
 * @param ... arguments
 */
void sc_buf_put_text(struct sc_buf *buf, const char *fmt, ...);

/**
 *  Put binary data, it will store len in 4 bytes first, then binary data.
 *
 * @param buf buffer
 * @param ptr data
 * @param len data len
 */
void sc_buf_put_blob(struct sc_buf *buf, const void *ptr, uint32_t len);

/**
 * Get binary data, returned pointer is valid until buffer is altered.
 *
 * @param buf buffer
 * @param len data len
 * @return    pointer to data.
 */
void *sc_buf_get_blob(struct sc_buf *buf, uint32_t len);

/**
 * Copy from src to dest, it will copy maximum amount of bytes without expanding
 * dest buffer size
 *
 * @param dest Destination buffer
 * @param src  Source buffer
 */
void sc_buf_move(struct sc_buf *dest, struct sc_buf *src);


/**
 *  Get encoded length of the variables.
 */

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
    assert(len <= INT32_MAX - 4);

    return len + sc_buf_32_len(len);
}

static inline uint32_t sc_buf_str_len(const char *str)
{
    if (str == NULL) {
        return sc_buf_32_len(-1);
    }

    assert(strlen(str) <= INT32_MAX - 5);

    return sc_buf_32_len(-1) + strlen(str) + sc_buf_8_len('\0');
}

#endif
