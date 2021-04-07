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

#include "sc_array.h"

#ifndef SC_ARRAY_MAX
#define SC_ARRAY_MAX SIZE_MAX
#endif

/**
 * Empty array instance.
 * Zero element arrays point at it to avoid initial allocation, so unused
 * arrays will not allocate memory.
 */
static const struct sc_array sc_empty = {.size = 0, .cap = 0};

bool sc_array_init(void *a, size_t elem_size, size_t cap)
{
	const size_t max = SC_ARRAY_MAX / elem_size;
	const size_t bytes = sizeof(struct sc_array) + (elem_size * cap);

	void **p = a;
	struct sc_array *m;

	if (cap == 0) {
		*p = (void *) sc_empty.elems;
		return true;
	}

	if (cap > max) {
		*p = NULL;
		return false;
	}

	m = sc_array_realloc(NULL, bytes);
	if (m == NULL) {
		*p = NULL;
		return false;
	}

	m->size = 0;
	m->cap = cap;
	*p = m->elems;

	return true;
}

void sc_array_term(void *a)
{
	void **p = a;
	struct sc_array *m = sc_array_meta(*p);

	if (m != &sc_empty) {
		sc_array_free(m);
	}

	*p = NULL;
}

bool sc_array_expand(void *a, size_t elem_size)
{
	const size_t max = SC_ARRAY_MAX / elem_size;

	size_t size, cap, bytes;
	void **p = a;
	struct sc_array *prev;
	struct sc_array *m = sc_array_meta(*p);

	if (m->size == m->cap) {
		if (m->cap > max / 2) {
			return false;
		}

		size = m->size;
		cap = (m != &sc_empty) ? m->cap * 2 : 2;
		prev = (m != &sc_empty) ? m : NULL;

		bytes = sizeof(*m) + (elem_size * cap);
		m = sc_array_realloc(prev, bytes);
		if (m == NULL) {
			return false;
		}

		m->size = size;
		m->cap = cap;
		*p = m->elems;
	}

	return true;
}
