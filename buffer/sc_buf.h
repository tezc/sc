/*
 * BSD-3-Clause
 *
 * Copyright 2021 Ozan Tezcan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SC_BUF_H
#define SC_BUF_H

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SC_BUF_VERSION "2.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_buf_calloc calloc
#define sc_buf_realloc realloc
#define sc_buf_free free
#endif

#define SC_BUF_CORRUPT 1u
#define SC_BUF_OOM 3u

#define SC_BUF_REF 8
#define SC_BUF_DATA 16
#define SC_BUF_READ (SC_BUF_REF | SC_BUF_DATA)

struct sc_buf {
	unsigned char *mem;
	uint64_t cap;
	uint64_t limit;
	uint64_t rpos;
	uint64_t wpos;

	unsigned int err;
	bool ref;
};

/**
 * Create buffer
 *
 * @param buf buf
 * @param cap initial capacity
 * @return    'false' on out of memory.
 */
bool sc_buf_init(struct sc_buf *b, uint64_t cap);

/**
 * Destroy buffer
 *
 * @param buf buf
 */
void sc_buf_term(struct sc_buf *b);

/**
 * Wrap memory into buffer, write position will be '0' after this call.
 * Useful for using with pre-allocated/stack allocated buffers.
 *
 * @param data data pointer
 * @param len  len
 * @param flags if set 'SC_BUF_REF', buffer will not try to expand itself and
 *             'sc_buf_term' will not try to 'free()' buffer.
 *             if set 'SC_BUF_DATA', buffer wpos will be 'len'.
 *             flags can be combined : SC_BUF_REF | SC_BUF_DATA
 * @return     buf
 */
struct sc_buf sc_buf_wrap(void *data, uint64_t len, int flags);

/**
 * Set limit of the buffer, when buffer reaches limit, it will set buffer's
 * 'out of memory' flag. Default is UINT64_MAX.
 *
 * @param buf   buf
 * @param limit limit
 */
void sc_buf_limit(struct sc_buf *b, uint64_t limit);

/**
 * @param buf buf
 * @param pos pos
 * @return    buf address at 'pos'
 */
void *sc_buf_at(struct sc_buf *b, uint64_t pos);

/**
 * @param buf buf
 * @return    current capacity.
 */
uint64_t sc_buf_cap(struct sc_buf *b);

/**
 * Reserve space
 *
 * @param buf buf
 * @param len len
 * @return    'false' on out of memory or hits the 'limit'.
 *            'out memory flag' will be set to check it later.
 */
bool sc_buf_reserve(struct sc_buf *b, uint64_t len);

/**
 * Shrink buffer. Shrinks only if empty space > 4096 bytes
 *
 * @param b   buf
 * @param len desired size
 * @return    'false' on out of memory, true otherwise.
 */
bool sc_buf_shrink(struct sc_buf *b, uint64_t len);

/**
 * @param buf buf
 * @return    'true' if buffer is valid. Buffer becomes invalid on out of
 *            memory, on buffer overflow or on buffer underflow.
 */
bool sc_buf_valid(struct sc_buf *b);

/**
 * @param buf buf
 * @return    current remaining space to write
 */
uint64_t sc_buf_quota(struct sc_buf *b);

/**
 * @param buf buf
 * @return    current byte count in the buffer
 */
uint64_t sc_buf_size(struct sc_buf *b);

/**
 * Set read and write position to '0', clear error flag.
 * @param buf buf
 */
void sc_buf_clear(struct sc_buf *b);

/**
 * Compact buf, e.g if bytes in buffer at [3, 20], it will be moved to [0, 17].
 * @param buf buf
 */
void sc_buf_compact(struct sc_buf *b);

/**
 * Advance read position, useful when you pass underlying array to another
 * function which operates on void*. e.g socket write() call.
 *
 * @param buf buf
 * @param len len
 */
void sc_buf_mark_read(struct sc_buf *b, uint64_t len);

/**
 * Advance read position, useful when you pass underlying array to another
 * function which operates on void*. e.g socket read() call.
 *
 * @param buf buf
 * @param len len
 */
void sc_buf_mark_write(struct sc_buf *b, uint64_t len);

/**
 * @param buf buf
 * @return    current read position
 */
uint64_t sc_buf_rpos(struct sc_buf *b);

/**
 * @param buf buf
 * @return    new read position
 */
void sc_buf_set_rpos(struct sc_buf *b, uint64_t pos);

/**
 * @param buf buf
 * @return    current write position
 */
uint64_t sc_buf_wpos(struct sc_buf *b);

/**
 * @param buf buf
 * @return    new write position
 */
void sc_buf_set_wpos(struct sc_buf *b, uint64_t pos);

/**
 * Get address of read position. Useful for e.g : write(fd, sc_buf_rbuf(buf) ..)
 *
 * @param b   buffer
 * @return    read address
 */
void *sc_buf_rbuf(struct sc_buf *b);

/**
 * Get address of write position. Useful for e.g : read(fd, sc_buf_wbuf(buf) ..)
 *
 * @param buf buf
 * @return    write address
 */
void *sc_buf_wbuf(struct sc_buf *b);

/**
 * Copy from src to dest, it will copy maximum amount of bytes without expanding
 * dest buffer size
 *
 * @param dest Destination buffer
 * @param src  Source buffer
 */
void sc_buf_move(struct sc_buf *dest, struct sc_buf *src);

/**
 * Read from buffer without advancing read position. 'peek' functions will set
 * error flags if buffer does not have required amount of data.
 */
uint8_t sc_buf_peek_8_at(struct sc_buf *b, uint64_t pos);
uint16_t sc_buf_peek_16_at(struct sc_buf *b, uint64_t pos);
uint32_t sc_buf_peek_32_at(struct sc_buf *b, uint64_t pos);
uint64_t sc_buf_peek_64_at(struct sc_buf *b, uint64_t pos);

uint8_t sc_buf_peek_8(struct sc_buf *b);
uint16_t sc_buf_peek_16(struct sc_buf *b);
uint32_t sc_buf_peek_32(struct sc_buf *b);
uint64_t sc_buf_peek_64(struct sc_buf *b);
uint64_t sc_buf_peek_data(struct sc_buf *b, uint64_t pos, unsigned char *dest,
			  uint64_t len);

/**
 * Set value at current write position. 'set' functions will not try to expand
 * buffer if there is no space and error flags will be set.
 */
void sc_buf_set_8_at(struct sc_buf *b, uint64_t pos, uint8_t val);
void sc_buf_set_16_at(struct sc_buf *b, uint64_t pos, uint16_t val);
void sc_buf_set_32_at(struct sc_buf *b, uint64_t pos, uint32_t val);
void sc_buf_set_64_at(struct sc_buf *b, uint64_t pos, uint64_t val);

void sc_buf_set_8(struct sc_buf *b, uint8_t val);
void sc_buf_set_16(struct sc_buf *b, uint16_t val);
void sc_buf_set_32(struct sc_buf *b, uint32_t val);
void sc_buf_set_64(struct sc_buf *b, uint64_t val);
uint64_t sc_buf_set_data(struct sc_buf *b, uint64_t pos, const void *src,
			 uint64_t len);
/**
 * Get values from buffer, read position will be advanced.
 */
bool sc_buf_get_bool(struct sc_buf *b);
uint8_t sc_buf_get_8(struct sc_buf *b);
uint16_t sc_buf_get_16(struct sc_buf *b);
uint32_t sc_buf_get_32(struct sc_buf *b);
uint64_t sc_buf_get_64(struct sc_buf *b);
double sc_buf_get_double(struct sc_buf *b);

/**
 * Read string. Strings are stored as [8 bytes length][string bytes]['\0'].
 * So you can use return value without copying/freeing until you overwrite it
 * in the buffer.
 *
 * @param b   buffer
 * @return    Pointer to string, possibly NULL if NULL has been put before.
 */
const char *sc_buf_get_str(struct sc_buf *b);

/**
 * Get binary data, returned pointer is valid until buffer is altered.
 *
 * @param b   buffer
 * @param len data len
 * @return    pointer to data.
 */
void *sc_buf_get_blob(struct sc_buf *b, uint64_t len);

/**
 *  Get binary data to destination buffer. If buffer does not have 'len' bytes,
 *  error flags will be set.
 *
 * @param buf  buffer
 * @param dest destination
 * @param len  len
 */
void sc_buf_get_data(struct sc_buf *b, void *dest, uint64_t len);

/**
 * Put functions, 'val' will be copied to the buffer and write position will
 * be advanced.
 */
void sc_buf_put_bool(struct sc_buf *b, bool val);
void sc_buf_put_8(struct sc_buf *b, uint8_t val);
void sc_buf_put_16(struct sc_buf *b, uint16_t val);
void sc_buf_put_32(struct sc_buf *b, uint32_t val);
void sc_buf_put_64(struct sc_buf *b, uint64_t val);
void sc_buf_put_double(struct sc_buf *b, double val);

/**
 * Write string. Strings are stored as [8 bytes length][string bytes]['\0'].
 * NULL values are accepted, stored as [8 bytes length].
 *
 * @param b   buffer
 * @param str string
 */
void sc_buf_put_str(struct sc_buf *b, const char *str);

/**
 * Write string. Strings are stored as [8 bytes length][string bytes]['\0'].
 * NULL values are accepted, stored as [8 bytes length].
 *
 * @param b   buffer
 * @param str string
 * @param len string len excluding '\0' byte at the end
 */
void sc_buf_put_str_len(struct sc_buf *b, const char *str, uint64_t len);

/**
 * Put formatted string, passes arguments to vsnprintf
 *
 * @param b   buffer
 * @param fmt fmt
 * @param ... arguments
 */
void sc_buf_put_fmt(struct sc_buf *b, const char *fmt, ...);

/**
 * Put formatted string, passes arguments to vsnprintf but concatenates strings.
 * Only useful if you want to append strings. It doesn't store string as length
 * prefixed string. So, only valid use case is :
 *
 * e.g
 *
 * char tmp[128];
 * struct sc_buf buf = sc_buf_wrap(tmp, sizeof(tmp), SC_BUF_REF);
 * sc_buf_put_text(&buf, "Hello");
 * sc_buf_put_text(&buf, " world");
 *
 * printf("%s", buf.mem); // Prints "Hello world"
 *
 *
 * @param b   buffer
 * @param fmt fmt
 * @param ... arguments
 */
void sc_buf_put_text(struct sc_buf *b, const char *fmt, ...);

/**
 *  Put binary data, it will store len in 4 bytes first, then binary data.
 *
 * @param b   buffer
 * @param ptr data
 * @param len data len
 */
void sc_buf_put_blob(struct sc_buf *b, const void *ptr, uint64_t len);

/**
 * Write 'len' bytes to buffer, on out of memory, error flag will be set.
 *
 * @param buf  buffer
 * @param dest dest
 * @param len  len
 */
void sc_buf_put_raw(struct sc_buf *b, const void *ptr, uint64_t len);

/**
 *  Get encoded length of the variables.
 */

// clang-format off
static inline uint64_t sc_buf_bool_len(bool val) {(void) val; return 1;}
static inline uint64_t sc_buf_8_len(uint8_t val) {(void) val; return 1;}
static inline uint64_t sc_buf_16_len(uint16_t val){(void) val; return 2;}
static inline uint64_t sc_buf_32_len(uint32_t val){(void) val; return 4;}
static inline uint64_t sc_buf_64_len(uint64_t val){(void) val; return 8;}
static inline uint64_t sc_buf_double_len(double val){(void) val; return 8;}
// clang-format on

static inline uint64_t sc_buf_blob_len(void *ptr, uint64_t len)
{
	(void) ptr;

	return len + sc_buf_64_len(len);
}

static inline uint64_t sc_buf_str_len(const char *str)
{
	const uint64_t bytes = sc_buf_64_len(UINT64_MAX) + sc_buf_8_len('\0');

	if (str == NULL) {
		return sc_buf_64_len(UINT64_MAX);
	}

	return bytes + (uint64_t) strlen(str);
}

#endif
