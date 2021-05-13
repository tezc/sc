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

#include "sc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * RC4 random is based on https://sqlite.org/src/file?name=src/random.c
 */
void sc_rand_init(struct sc_rand *r, const unsigned char *init)
{
	unsigned char t;

	*r = (struct sc_rand){0};

	memcpy(r->init, init, 256);

	for (int i = 0; i < 256; i++) {
		r->j += r->init[i] + init[i];
		t = r->init[r->j];
		r->init[r->j] = r->init[i];
		r->init[i] = t;
	}
}

void sc_rand_read(struct sc_rand *r, void *buf, int size)
{
	unsigned char t;
	unsigned char *p = buf;

	if (size <= 0 || buf == NULL) {
		return;
	}

	do {
		r->i++;
		t = r->init[r->i];
		r->j += t;
		r->init[r->i] = r->init[r->j];
		r->init[r->j] = t;
		t += r->init[r->i];
		*(p++) = r->init[t];
	} while (--size);
}

bool sc_is_pow2(uint64_t num)
{
	return (num != 0) && (num & (num - 1)) == 0;
}

uint64_t sc_to_pow2(uint64_t size)
{
	if (size == 0) {
		return 1;
	}

	size--;

	for (uint32_t i = 1; i < sizeof(size) * 8; i *= 2) {
		size |= size >> i;
	}

	size++;

	return size;
}

char *sc_bytes_to_size(char *buf, size_t len, uint64_t val)
{
	const char *suffix[] = {"KB", "MB", "GB", "TB", "PB", "EB"};
	int n = 0, wr;
	uint64_t count = val;

	if (val < 1024) {
		wr = snprintf(buf, len, "%d B", (int) val);
		if (wr <= 0 || (size_t) wr >= len) {
			return NULL;
		}
		return buf;
	}

	for (int i = 40; i >= 0 && val > 0xfffccccccccccccUL >> i; i -= 10) {
		n++;
		count >>= 10;
	}

	wr = snprintf(buf, len, "%.02lf %s", (double) count / 1024, suffix[n]);
	if (wr <= 0 || (size_t) wr >= len) {
		return NULL;
	}

	return buf;
}

int64_t sc_size_to_bytes(const char *buf)
{
	const int64_t kb = (int64_t) 1024;
	const int64_t mb = (int64_t) 1024 * 1024;
	const int64_t gb = (int64_t) 1024 * 1024 * 1024;
	const int64_t tb = (int64_t) 1024 * 1024 * 1024 * 1024;
	const int64_t pb = (int64_t) 1024 * 1024 * 1024 * 1024 * 1024;
	const int64_t eb = (int64_t) 1024 * 1024 * 1024 * 1024 * 1024 * 1024;

	int count;
	int64_t val;
	char *parse_end;
	const char *end = (char *) (buf + strlen(buf));

	errno = 0;
	val = strtoll(buf, &parse_end, 10);
	if (errno != 0 || parse_end == buf) {
		return -1;
	}

	count = (int) (end - parse_end);

	switch (count) {
	case 0:
		return val;
	case 1:
		break;
	case 2:
		if (tolower(parse_end[1]) != 'b') {
			return -1;
		}
		break;
	default:
		return -1;
	}

	switch (tolower(parse_end[0])) {
	case 'b':
		break;
	case 'k':
		val = (val > INT64_MAX / kb) ? -1 : val * kb;
		break;
	case 'm':
		val = (val > INT64_MAX / mb) ? -1 : val * mb;
		break;
	case 'g':
		val = (val > INT64_MAX / gb) ? -1 : val * gb;
		break;
	case 't':
		val = (val > INT64_MAX / tb) ? -1 : val * tb;
		break;
	case 'p':
		val = (val > INT64_MAX / pb) ? -1 : val * pb;
		break;
	case 'e':
		val = (val > INT64_MAX / eb) ? -1 : val * eb;
		break;
	default:
		return -1;
	}

	return val;
}
