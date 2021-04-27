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

struct sc_heap_data *sc_heap_peek(struct sc_heap *h)
{
	if (h->size == 0) {
		return NULL;
	}

	// Top element is always at heap->elems[1].
	return &h->elems[1];
}

struct sc_heap_data *sc_heap_pop(struct sc_heap *h)
{
	size_t i = 1, child = 2;
	struct sc_heap_data last;

	if (h->size == 0) {
		return NULL;
	}

	// Top element is always at heap->elems[1].
	h->elems[0] = h->elems[1];

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

	return &h->elems[0];
}
