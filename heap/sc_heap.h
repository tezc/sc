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

#ifndef SC_HEAP_H
#define SC_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SC_HEAP_VERSION "2.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_heap_malloc malloc
#define sc_heap_realloc realloc
#define sc_heap_free free
#endif

struct sc_heap_data {
	int64_t key;
	void *data;
};

struct sc_heap {
	size_t cap;
	size_t size;
	struct sc_heap_data *elems;
};

/**
 * @param h   heap
 * @param cap initial capacity, pass '0' for no initial memory allocation
 * @return    'true' on success, 'false' on out of memory
 */
bool sc_heap_init(struct sc_heap *h, size_t cap);

/**
 * Destroys heap, frees memory
 * @param h heap
 */
void sc_heap_term(struct sc_heap *h);

/**
 * @param h heap
 * @return  element count
 */
size_t sc_heap_size(struct sc_heap *h);

/**
 * Clears elements from the queue, does not free the allocated memory.
 * @param h heap
 */
void sc_heap_clear(struct sc_heap *h);

/**
 * @param h    heap
 * @param key  key
 * @param data data
 * @return     'false' on out of memory.
 */
bool sc_heap_add(struct sc_heap *h, int64_t key, void *data);

/**
 * Read top element without removing from the heap.
 *
 * @param h    heap
 * @return     pointer to data holder(valid until next heap operation)
 *             NULL if heap is empty.
 */
struct sc_heap_data *sc_heap_peek(struct sc_heap *h);

/**
 * Read top element and remove it from the heap.
 *
 * @param h    heap
 * @return     pointer to data holder(valid until next heap operation)
 *             NULL if heap is empty.
 */
struct sc_heap_data *sc_heap_pop(struct sc_heap *h);

#endif
