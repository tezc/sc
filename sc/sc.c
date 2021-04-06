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

bool sc_is_pow2(size_t num)
{
	return (num != 0) && (num & (num - 1)) == 0;
}

size_t sc_to_pow2(size_t size)
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
