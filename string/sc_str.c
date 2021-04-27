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

#include "sc_str.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * String with 'length' at the start of the allocated memory
 *  e.g :
 *  -----------------------------------------------
 *  | 0 | 0 | 0 | 4 | 'T' | 'E' | 'S' | 'T' | '\0'|
 *  -----------------------------------------------
 *
 *  User can keep pointer to first character, so it's like C style strings with
 *  additional functionality when it's used with these functions here.
 */
struct sc_str {
	uint32_t len;
	char buf[];
};

#define sc_str_meta(str)                                                       \
	((struct sc_str *) ((char *) (str) -offsetof(struct sc_str, buf)))

#define sc_str_bytes(n) ((n) + sizeof(struct sc_str) + 1)

#ifndef SC_STR_MAX
#define SC_STR_MAX (UINT32_MAX - sizeof(struct sc_str) - 1)
#endif

char *sc_str_create(const char *str)
{
	size_t len;

	if (str == NULL || (len = strlen(str)) > SC_STR_MAX) {
		return NULL;
	}

	return sc_str_create_len(str, (uint32_t) len);
}

char *sc_str_create_len(const char *str, uint32_t len)
{
	struct sc_str *c;

	if (str == NULL) {
		return NULL;
	}

	c = sc_str_malloc(sc_str_bytes(len));
	if (c == NULL) {
		return NULL;
	}

	memcpy(c->buf, str, len);
	c->buf[len] = '\0';
	c->len = len;

	return c->buf;
}

char *sc_str_create_va(const char *fmt, va_list va)
{
	int rc;
	char tmp[1024];
	struct sc_str *str;
	va_list args;

	va_copy(args, va);
	rc = vsnprintf(tmp, sizeof(tmp), fmt, args);
	if (rc < 0) {
		va_end(args);
		return NULL;
	}
	va_end(args);

	str = sc_str_malloc(sc_str_bytes(rc));
	if (str == NULL) {
		return NULL;
	}

	str->len = (uint32_t) rc;

	if (rc < (int) sizeof(tmp)) {
		memcpy(str->buf, tmp, str->len + 1);
	} else {
		va_copy(args, va);
		rc = vsnprintf(str->buf, str->len + 1, fmt, args);
		va_end(args);

		if (rc < 0 || (uint32_t) rc > str->len) {
			sc_str_free(str);
			return NULL;
		}
	}

	return str->buf;
}

char *sc_str_create_fmt(const char *fmt, ...)
{
	char *str;
	va_list args;

	va_start(args, fmt);
	str = sc_str_create_va(fmt, args);
	va_end(args);

	return str;
}

void sc_str_destroy(char **str)
{
	if (str == NULL || *str == NULL) {
		return;
	}

	sc_str_free(sc_str_meta(*str));
	*str = NULL;
}

int64_t sc_str_len(const char *str)
{
	if (str == NULL) {
		return -1;
	}

	return sc_str_meta(str)->len;
}

char *sc_str_dup(const char *str)
{
	struct sc_str *m;

	if (str == NULL) {
		return NULL;
	}

	m = sc_str_meta(str);
	return sc_str_create_len(str, m->len);
}

bool sc_str_set(char **str, const char *param)
{
	char *c;

	c = sc_str_create(param);
	if (c == NULL) {
		return false;
	}

	sc_str_destroy(str);
	*str = c;

	return true;
}

bool sc_str_set_fmt(char **str, const char *fmt, ...)
{
	char *ret;
	va_list args;

	va_start(args, fmt);
	ret = sc_str_create_va(fmt, args);
	va_end(args);

	if (ret != NULL) {
		sc_str_destroy(str);
		*str = ret;
	}

	return ret != NULL;
}

bool sc_str_append(char **str, const char *param)
{
	size_t len, alloc;
	struct sc_str *m;

	if (*str == NULL) {
		*str = sc_str_create(param);
		return *str != NULL;
	}

	m = sc_str_meta(*str);
	len = strlen(param);
	alloc = sc_str_bytes(m->len + len);

	if (len > SC_STR_MAX - m->len ||
	    (m = sc_str_realloc(m, alloc)) == NULL) {
		return false;
	}

	memcpy(&m->buf[m->len], param, len);
	m->len += (uint32_t) len;
	m->buf[m->len] = '\0';
	*str = m->buf;

	return true;
}

bool sc_str_cmp(const char *str, const char *other)
{
	struct sc_str *s1 = sc_str_meta(str);
	struct sc_str *s2 = sc_str_meta(other);

	return s1->len == s2->len && !memcmp(s1->buf, s2->buf, s1->len);
}

static void swap(char *str, char *d)
{
	char tmp;
	char *c = str + sc_str_meta(str)->len;

	tmp = *c;
	*c = *d;
	*d = tmp;
}

const char *sc_str_token_begin(char *str, char **save, const char *delim)
{
	char *it = str;

	if (str == NULL) {
		return NULL;
	}

	if (*save != NULL) {
		it = *save;
		swap(str, it);
		if (*it == '\0') {
			return NULL;
		}
		it++;
	}

	*save = it + strcspn(it, delim);
	swap(str, *save);

	return it;
}

void sc_str_token_end(char *str, char **save)
{
	char *end;

	if (str == NULL) {
		return;
	}

	end = str + sc_str_meta(str)->len;
	if (*end == '\0') {
		return;
	}

	swap(str, *save);
}

bool sc_str_trim(char **str, const char *list)
{
	uint32_t diff;
	size_t len;
	char *head, *end;

	if (*str == NULL) {
		return true;
	}

	len = sc_str_meta(*str)->len;
	head = *str + strspn(*str, list);
	end = (*str) + len;

	while (end > head) {
		end--;
		if (!strchr(list, *end)) {
			end++;
			break;
		}
	}

	if (head != *str || end != (*str) + len) {
		diff = (uint32_t) (end - head);
		head = sc_str_create_len(head, diff);
		if (head == NULL) {
			return false;
		}

		sc_str_destroy(str);
		*str = head;
	}

	return true;
}

bool sc_str_substring(char **str, uint32_t start, uint32_t end)
{
	char *c;
	struct sc_str *meta;

	if (*str == NULL) {
		return false;
	}

	meta = sc_str_meta(*str);
	if (start > meta->len || end > meta->len || start > end) {
		return false;
	}

	c = sc_str_create_len(*str + start, end - start);
	if (c == NULL) {
		return false;
	}

	sc_str_destroy(str);
	*str = c;

	return true;
}

bool sc_str_replace(char **str, const char *replace, const char *with)
{
	assert(replace != NULL && with != NULL);

	int64_t diff;
	size_t rep_len, with_len;
	size_t len_unmatch, count, size;
	struct sc_str *dest, *meta;
	char *tmp, *orig, *orig_end;

	if (*str == NULL) {
		return true;
	}

	rep_len = strlen(replace);
	with_len = strlen(with);

	if (rep_len >= UINT32_MAX || with_len >= UINT32_MAX) {
		return false;
	}

	meta = sc_str_meta(*str);
	orig = *str;
	orig_end = *str + meta->len;
	diff = (int64_t) with_len - (int64_t) rep_len;

	// Fast path, same size replacement.
	if (diff == 0) {
		while ((orig = strstr(orig, replace)) != NULL) {
			memcpy(orig, with, rep_len);
			orig += rep_len;
		}

		return true;
	}

	// Calculate new string size.
	tmp = orig;
	size = meta->len;
	for (count = 0; (tmp = strstr(tmp, replace)) != NULL; count++) {
		tmp += rep_len;
		// Check overflow.
		if ((int64_t) size > (int64_t) SC_STR_MAX - diff) {
			return false;
		}
		size += diff;
	}

	// No match.
	if (count == 0) {
		return true;
	}

	dest = sc_str_malloc(sc_str_bytes(size));
	if (!dest) {
		return false;
	}

	dest->len = (uint32_t) size;
	tmp = dest->buf;

	while (count--) {
		len_unmatch = strstr(orig, replace) - orig;
		memcpy(tmp, orig, len_unmatch);
		tmp += len_unmatch;

		// Copy extra '\0' byte just to silence sanitizer.
		memcpy(tmp, with, with_len + 1);
		tmp += with_len;
		orig += len_unmatch + rep_len;
	}

	memcpy(tmp, orig, orig_end - orig + 1);

	sc_str_destroy(str);
	*str = dest->buf;

	return true;
}
