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

#include "sc_buf.h"

#include <stdio.h>

#ifndef SC_BUF_MAX
#define SC_BUF_MAX UINT64_MAX-4096
#endif

#define NULL_LEN SC_BUF_MAX

bool sc_buf_init(struct sc_buf *b, uint64_t cap)
{
	void *m = NULL;

	*b = (struct sc_buf){0};

	if (cap > 0) {
		m = sc_buf_calloc(1, cap);
		if (m == NULL) {
			return false;
		}
	}

	*b = sc_buf_wrap(m, cap, 0);

	return true;
}

struct sc_buf sc_buf_wrap(void *data, uint64_t len, int flag)
{
	struct sc_buf b = {
		.mem = data,
		.cap = len,
		.limit = flag & SC_BUF_REF ? len : SC_BUF_MAX,
		.wpos = flag & SC_BUF_DATA ? len : 0,
		.rpos = 0,
		.ref = (bool) (flag & SC_BUF_REF),
		.err = 0,
	};

	return b;
}

void sc_buf_term(struct sc_buf *b)
{
	if (!b->ref) {
		sc_buf_free(b->mem);
	}

	sc_buf_init(b, 0);
}

void sc_buf_limit(struct sc_buf *b, uint64_t limit)
{
	b->limit = limit;
}

void *sc_buf_at(struct sc_buf *b, uint64_t pos)
{
	return b->mem + pos;
}

uint64_t sc_buf_cap(struct sc_buf *b)
{
	return b->cap;
}

bool sc_buf_reserve(struct sc_buf *b, uint64_t len)
{
	uint64_t size;
	void *m;

	if (b->wpos + len > b->cap) {
		if (b->ref) {
			goto err;
		}

		size = ((b->cap + len + 4095) / 4096) * 4096;
		if (size > b->limit || b->cap >= SC_BUF_MAX - 4096) {
			goto err;
		}

		m = sc_buf_realloc(b->mem, size);
		if (m == NULL) {
			goto err;
		}

		b->cap = size;
		b->mem = m;
	}

	return true;

err:
	b->err |= SC_BUF_OOM;
	return false;
}

bool sc_buf_shrink(struct sc_buf *b, uint64_t len)
{
	void *tmp;

	sc_buf_compact(b);

	if (len > b->cap || b->wpos >= len) {
		return true;
	}

	len = ((len + 4095) / 4096) * 4096;

	tmp = sc_buf_realloc(b->mem, len);
	if (tmp == NULL) {
		b->err |= SC_BUF_OOM;
		return false;
	}

	b->cap = len;
	b->mem = tmp;

	return true;
}

bool sc_buf_valid(struct sc_buf *b)
{
	return b->err == 0;
}

uint64_t sc_buf_quota(struct sc_buf *b)
{
	return b->cap - b->wpos;
}

uint64_t sc_buf_size(struct sc_buf *b)
{
	return b->wpos - b->rpos;
}

void sc_buf_clear(struct sc_buf *b)
{
	b->rpos = 0;
	b->wpos = 0;
	b->err = 0;
}

void sc_buf_mark_read(struct sc_buf *b, uint64_t len)
{
	b->rpos += len;
}

void sc_buf_mark_write(struct sc_buf *b, uint64_t len)
{
	b->wpos += len;
}

uint64_t sc_buf_rpos(struct sc_buf *b)
{
	return b->rpos;
}

void sc_buf_set_rpos(struct sc_buf *b, uint64_t pos)
{
	if (pos > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		return;
	}

	b->rpos = pos;
}

void sc_buf_set_wpos(struct sc_buf *b, uint64_t pos)
{
	if (pos > b->cap) {
		b->err |= SC_BUF_CORRUPT;
		return;
	}

	b->wpos = pos;
}

uint64_t sc_buf_wpos(struct sc_buf *b)
{
	return b->wpos;
}

void *sc_buf_rbuf(struct sc_buf *b)
{
	return b->mem + b->rpos;
}

void *sc_buf_wbuf(struct sc_buf *b)
{
	return b->mem + b->wpos;
}

void sc_buf_compact(struct sc_buf *b)
{
	uint64_t copy;

	if (b->rpos == b->wpos) {
		b->rpos = 0;
		b->wpos = 0;
	}

	if (b->rpos != 0) {
		copy = b->wpos - b->rpos;
		memmove(b->mem, b->mem + b->rpos, copy);
		b->rpos = 0;
		b->wpos = copy;
	}
}

void sc_buf_move(struct sc_buf *dest, struct sc_buf *src)
{
	uint64_t quota = sc_buf_quota(dest);
	uint64_t size = sc_buf_size(src);
	uint64_t copy = quota < size ? quota : size;
	void *from = &src->mem[src->rpos];

	sc_buf_put_raw(dest, from, copy);
	src->rpos += copy;
}

static uint16_t sc_buf_peek_8_pos(struct sc_buf *b, uint64_t pos, uint8_t *val)
{
	if (b->err != 0 || pos + sizeof(*val) > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		*val = 0;
		return 0;
	}

	*val = b->mem[pos];

	return sizeof(*val);
}

static uint16_t sc_buf_peek_16_pos(struct sc_buf *b, uint64_t pos,
				   uint16_t *val)
{
	unsigned char *p;

	if (b->err != 0 || pos + sizeof(*val) > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		*val = 0;
		return 0;
	}

	p = &b->mem[pos];

	*val = (uint16_t) p[0] << 0 | (uint16_t) p[1] << 8;

	return sizeof(*val);
}

static uint64_t sc_buf_peek_32_pos(struct sc_buf *b, uint64_t pos,
				   uint32_t *val)
{
	unsigned char *p;

	if (b->err != 0 || pos + sizeof(*val) > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		*val = 0;
		return 0;
	}

	p = &b->mem[pos];

	*val = (uint32_t) p[0] << 0 | (uint32_t) p[1] << 8 |
	       (uint32_t) p[2] << 16 | (uint32_t) p[3] << 24;

	return sizeof(*val);
}

static uint64_t sc_buf_peek_64_pos(struct sc_buf *b, uint64_t pos,
				   uint64_t *val)
{
	unsigned char *p;

	if (b->err != 0 || pos + sizeof(*val) > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		*val = 0;
		return 0;
	}

	p = &b->mem[pos];

	*val = (uint64_t) p[0] << 0 | (uint64_t) p[1] << 8 |
	       (uint64_t) p[2] << 16 | (uint64_t) p[3] << 24 |
	       (uint64_t) p[4] << 32 | (uint64_t) p[5] << 40 |
	       (uint64_t) p[6] << 48 | (uint64_t) p[7] << 56;

	return sizeof(*val);
}

static uint64_t sc_buf_set_8_pos(struct sc_buf *b, uint64_t pos,
				 const uint8_t *val)
{
	if (b->err != 0 || (pos + sizeof(*val) > b->cap)) {
		b->err |= SC_BUF_CORRUPT;
		return 0;
	}

	b->mem[pos] = *val;

	return sizeof(*val);
}

static uint64_t sc_buf_set_16_pos(struct sc_buf *b, uint64_t pos,
				  const uint16_t *val)
{
	unsigned char *p;

	if (b->err != 0 || (pos + sizeof(*val) > b->cap)) {
		b->err |= SC_BUF_CORRUPT;
		return 0;
	}

	p = &b->mem[pos];

	p[0] = (unsigned char) (*val >> 0);
	p[1] = (unsigned char) (*val >> 8);

	return sizeof(*val);
}

static uint64_t sc_buf_set_32_pos(struct sc_buf *b, uint64_t pos,
				  const uint32_t *val)
{
	unsigned char *p;

	if (b->err != 0 || (pos + sizeof(*val) > b->cap)) {
		b->err |= SC_BUF_CORRUPT;
		return 0;
	}

	p = &b->mem[pos];

	p[0] = (unsigned char) (*val >> 0);
	p[1] = (unsigned char) (*val >> 8);
	p[2] = (unsigned char) (*val >> 16);
	p[3] = (unsigned char) (*val >> 24);

	return sizeof(*val);
}

static uint64_t sc_buf_set_64_pos(struct sc_buf *b, uint64_t pos,
				  const uint64_t *val)
{
	unsigned char *p;

	if (b->err != 0 || (pos + sizeof(*val) > b->cap)) {
		b->err |= SC_BUF_CORRUPT;
		return 0;
	}

	p = &b->mem[pos];

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

uint8_t sc_buf_peek_8_at(struct sc_buf *b, uint64_t pos)
{
	uint8_t val;

	sc_buf_peek_8_pos(b, pos, &val);
	return val;
}

uint16_t sc_buf_peek_16_at(struct sc_buf *b, uint64_t pos)
{
	uint16_t val;

	sc_buf_peek_16_pos(b, pos, &val);
	return val;
}

uint32_t sc_buf_peek_32_at(struct sc_buf *b, uint64_t pos)
{
	uint32_t val;

	sc_buf_peek_32_pos(b, pos, &val);
	return val;
}

uint64_t sc_buf_peek_64_at(struct sc_buf *b, uint64_t pos)
{
	uint64_t val;

	sc_buf_peek_64_pos(b, pos, &val);
	return val;
}

uint8_t sc_buf_peek_8(struct sc_buf *b)
{
	return sc_buf_peek_8_at(b, b->rpos);
}

uint16_t sc_buf_peek_16(struct sc_buf *b)
{
	return sc_buf_peek_16_at(b, b->rpos);
}

uint32_t sc_buf_peek_32(struct sc_buf *b)
{
	return sc_buf_peek_32_at(b, b->rpos);
}

uint64_t sc_buf_peek_64(struct sc_buf *b)
{
	return sc_buf_peek_64_at(b, b->rpos);
}

uint64_t sc_buf_peek_data(struct sc_buf *b, uint64_t pos, unsigned char *dest,
			  uint64_t len)
{
	if (b->err != 0 || (pos + len > b->wpos)) {
		b->err |= SC_BUF_CORRUPT;
		memset(dest, 0, len);
		return 0;
	}

	memcpy(dest, &b->mem[pos], len);

	return len;
}

void sc_buf_set_8_at(struct sc_buf *b, uint64_t pos, uint8_t val)
{
	sc_buf_set_8_pos(b, pos, &val);
}

void sc_buf_set_16_at(struct sc_buf *b, uint64_t pos, uint16_t val)
{
	sc_buf_set_16_pos(b, pos, &val);
}

void sc_buf_set_32_at(struct sc_buf *b, uint64_t pos, uint32_t val)
{
	sc_buf_set_32_pos(b, pos, &val);
}

void sc_buf_set_64_at(struct sc_buf *b, uint64_t pos, uint64_t val)
{
	sc_buf_set_64_pos(b, pos, &val);
}

void sc_buf_set_8(struct sc_buf *b, uint8_t val)
{
	sc_buf_set_8_at(b, b->wpos, val);
}

void sc_buf_set_16(struct sc_buf *b, uint16_t val)
{
	sc_buf_set_16_at(b, b->wpos, val);
}

void sc_buf_set_32(struct sc_buf *b, uint32_t val)
{
	sc_buf_set_32_at(b, b->wpos, val);
}

void sc_buf_set_64(struct sc_buf *b, uint64_t val)
{
	sc_buf_set_64_at(b, b->wpos, val);
}

uint64_t sc_buf_set_data(struct sc_buf *b, uint64_t offset, const void *src,
			 uint64_t len)
{
	if (b->err != 0 || (offset + len > b->cap)) {
		b->err |= SC_BUF_CORRUPT;
		return 0;
	}

	if (len == 0) {
		return 0;
	}

	memcpy(&b->mem[offset], src, len);
	return len;
}

bool sc_buf_get_bool(struct sc_buf *b)
{
	return sc_buf_get_8(b);
}

uint8_t sc_buf_get_8(struct sc_buf *b)
{
	uint8_t val;

	b->rpos += sc_buf_peek_8_pos(b, b->rpos, &val);

	return val;
}

uint16_t sc_buf_get_16(struct sc_buf *b)
{
	uint16_t val;

	b->rpos += sc_buf_peek_16_pos(b, b->rpos, &val);

	return val;
}

uint32_t sc_buf_get_32(struct sc_buf *b)
{
	uint32_t val;

	b->rpos += sc_buf_peek_32_pos(b, b->rpos, &val);

	return val;
}

uint64_t sc_buf_get_64(struct sc_buf *b)
{
	uint64_t val;

	b->rpos += sc_buf_peek_64_pos(b, b->rpos, &val);

	return val;
}

double sc_buf_get_double(struct sc_buf *b)
{
	double d;
	uint64_t val;

	val = sc_buf_get_64(b);
	memcpy(&d, &val, 8);

	return d;
}

const char *sc_buf_get_str(struct sc_buf *b)
{
	uint64_t len;
	const char *str;

	len = sc_buf_get_64(b);
	if (len == NULL_LEN) {
		return NULL;
	}

	if (!sc_buf_valid(b)) {
		return NULL;
	}

	if (b->rpos + len + 1 > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		return NULL;
	}

	str = (char *) b->mem + b->rpos;
	b->rpos += len + 1;

	return str;
}

void *sc_buf_get_blob(struct sc_buf *b, uint64_t len)
{
	void *blob;

	if (len == 0) {
		return NULL;
	}

	if (b->rpos + len > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		return NULL;
	}

	blob = b->mem + b->rpos;
	b->rpos += len;

	return blob;
}

void sc_buf_get_data(struct sc_buf *b, void *dest, uint64_t len)
{
	if (b->rpos + len > b->wpos) {
		b->err |= SC_BUF_CORRUPT;
		memset(dest, 0, len);
		return;
	}

	b->rpos += sc_buf_peek_data(b, b->rpos, dest, len);
}

void sc_buf_put_raw(struct sc_buf *b, const void *ptr, uint64_t len)
{
	if (!sc_buf_reserve(b, len)) {
		return;
	}

	b->wpos += sc_buf_set_data(b, b->wpos, ptr, len);
}

void sc_buf_put_bool(struct sc_buf *b, bool val)
{
	sc_buf_put_8(b, (uint8_t) val);
}

void sc_buf_put_8(struct sc_buf *b, uint8_t val)
{
	if (!sc_buf_reserve(b, sizeof(val))) {
		return;
	}

	b->wpos += sc_buf_set_8_pos(b, b->wpos, &val);
}

void sc_buf_put_16(struct sc_buf *b, uint16_t val)
{
	if (!sc_buf_reserve(b, sizeof(val))) {
		return;
	}

	b->wpos += sc_buf_set_16_pos(b, b->wpos, &val);
}

void sc_buf_put_32(struct sc_buf *b, uint32_t val)
{
	if (!sc_buf_reserve(b, sizeof(val))) {
		return;
	}

	b->wpos += sc_buf_set_32_pos(b, b->wpos, &val);
}

void sc_buf_put_64(struct sc_buf *b, uint64_t val)
{
	if (!sc_buf_reserve(b, sizeof(val))) {
		return;
	}

	b->wpos += sc_buf_set_64_pos(b, b->wpos, &val);
}

void sc_buf_put_double(struct sc_buf *b, double val)
{
	uint64_t sw;

	memcpy(&sw, &val, 8);
	sc_buf_put_64(b, sw);
}

void sc_buf_put_str(struct sc_buf *b, const char *str)
{
	uint64_t sz;

	if (str == NULL) {
		sc_buf_put_64(b, NULL_LEN);
		return;
	}

	sz = (uint64_t) strlen(str);
	if (sz >= SC_BUF_MAX) {
		b->err |= SC_BUF_CORRUPT;
		return;
	}

	sc_buf_put_64(b, sz);
	sc_buf_put_raw(b, str, sz + sc_buf_8_len('\0'));
}

void sc_buf_put_str_len(struct sc_buf *b, const char *str, uint64_t len)
{
	if (str == NULL) {
		sc_buf_put_64(b, NULL_LEN);
		return;
	}

	sc_buf_put_64(b, len);
	sc_buf_put_raw(b, str, len);
	sc_buf_put_8(b, '\0');
}

void sc_buf_put_fmt(struct sc_buf *b, const char *fmt, ...)
{
	const uint64_t len_bytes = sc_buf_64_len(0);
	const uint64_t pos = sc_buf_wpos(b);

	int rc;
	uint64_t wr;
	uint64_t quota = sc_buf_quota(b);
	void *mem = (char *) sc_buf_wbuf(b) + sc_buf_64_len(0);
	va_list args;

	if (quota > len_bytes) {
		quota -= len_bytes;
	} else {
		quota = 0;
	}

	va_start(args, fmt);
	rc = vsnprintf(mem, quota, fmt, args);
	va_end(args);

	if (rc < 0) {
		b->err |= SC_BUF_CORRUPT;
		return;
	}

	wr = (uint64_t) rc;

	if (wr >= quota) {
		if (!sc_buf_reserve(b, wr + len_bytes)) {
			return;
		}

		mem = (char *) sc_buf_wbuf(b) + len_bytes;
		quota = sc_buf_quota(b) - len_bytes;

		va_start(args, fmt);
		rc = vsnprintf(mem, quota, fmt, args);
		va_end(args);

		if (rc < 0 || (uint64_t) rc >= quota) {
			b->err |= SC_BUF_OOM;
			return;
		}

		wr = (uint64_t) rc;
	}

	sc_buf_set_64_at(b, pos, wr);
	sc_buf_mark_write(b, wr + len_bytes + 1);
}

void sc_buf_put_text(struct sc_buf *b, const char *fmt, ...)
{
	int rc;
	int off = sc_buf_size(b) > 0 ? 1 : 0;
	uint64_t wr, quota;
	char *dst;
	va_list va;

	dst = (char *) sc_buf_wbuf(b) - off;
	quota = sc_buf_quota(b);

	va_start(va, fmt);
	rc = vsnprintf(dst, quota, fmt, va);
	va_end(va);

	if (rc < 0) {
		b->err |= SC_BUF_CORRUPT;
		goto clean_up;
	}

	wr = (uint64_t) rc;
	if (wr >= quota) {
		if (!sc_buf_reserve(b, wr + 1)) {
			goto clean_up;
		}

		dst = (char *) sc_buf_wbuf(b) - off;
		quota = sc_buf_quota(b);

		va_start(va, fmt);
		rc = vsnprintf(dst, quota, fmt, va);
		va_end(va);

		if (rc < 0 || (uint64_t) rc >= quota) {
			b->err = SC_BUF_OOM;
			goto clean_up;
		}
	}

	sc_buf_mark_write(b, rc - off + 1);
	return;

clean_up:
	sc_buf_set_wpos(b, 0);
}

void sc_buf_put_blob(struct sc_buf *b, const void *ptr, uint64_t len)
{
	sc_buf_put_64(b, len);
	sc_buf_put_raw(b, ptr, len);
}
