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

#include "sc_heap.h"

#include <stdlib.h>

#ifndef SC_HEAP_MAX
#define SC_HEAP_MAX SIZE_MAX / sizeof(struct sc_heap_data)
#endif

bool sc_heap_init(struct sc_heap *h, size_t cap)
{
	void *e;
	const size_t sz = cap * sizeof(struct sc_heap_data);

	*h = (struct sc_heap){0};

	if (cap == 0) {
		return true;
	}

	if (cap > SC_HEAP_MAX || (e = sc_heap_malloc(sz)) == NULL) {
		return false;
	}

	h->elems = e;
	h->cap = cap;

	return true;
}

void sc_heap_term(struct sc_heap *h)
{
	sc_heap_free(h->elems);

	*h = (struct sc_heap){
		.elems = NULL,
	};
}

size_t sc_heap_size(struct sc_heap *h)
{
	return h->size;
}

void sc_heap_clear(struct sc_heap *h)
{
	h->size = 0;
}

bool sc_heap_add(struct sc_heap *h, int64_t key, void *data)
{
	size_t i, cap, m;
	void *exp;

	if (++h->size >= h->cap) {
		cap = h->cap != 0 ? h->cap * 2 : 4;
		m = cap * 2 * sizeof(*h->elems);

		if (h->cap >= SC_HEAP_MAX / 2 ||
		    (exp = sc_heap_realloc(h->elems, m)) == NULL) {
			return false;
		}

		h->elems = exp;
		h->cap = cap;
	}

	i = h->size;
	while (i != 1 && key < h->elems[i / 2].key) {
		h->elems[i] = h->elems[i / 2];
		i /= 2;
	}

	h->elems[i].key = key;
	h->elems[i].data = data;

	return true;
}

bool sc_heap_peek(struct sc_heap *h, int64_t *key, void **data)
{
	if (h->size == 0) {
		return false;
	}

	// Top element is always at heap->elems[1].
	*key = h->elems[1].key;
	*data = h->elems[1].data;

	return true;
}

bool sc_heap_pop(struct sc_heap *h, int64_t *key, void **data)
{
	size_t i = 1, child = 2;
	struct sc_heap_data last;

	if (h->size == 0) {
		return false;
	}

	// Top element is always at heap->elems[1].
	*key = h->elems[1].key;
	*data = h->elems[1].data;

	last = h->elems[h->size--];
	while (child <= h->size) {
		if (child < h->size &&
		    h->elems[child].key > h->elems[child + 1].key) {
			child++;
		};

		if (last.key <= h->elems[child].key) {
			break;
		}

		h->elems[i] = h->elems[child];

		i = child;
		child *= 2;
	}

	h->elems[i] = last;

	return true;
}
