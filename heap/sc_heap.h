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

#ifndef SC_HEAP_H
#define SC_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef SC_HAVE_CONFIG_H
    #include "sc_config.h"
#else
    #define sc_heap_malloc  malloc
    #define sc_heap_realloc realloc
    #define sc_heap_free    free
#endif


struct sc_heap_data
{
    int64_t key;
    void *data;
};

struct sc_heap
{
    size_t cap;
    size_t size;
    struct sc_heap_data *elems;
};

/**
 * @param heap heap
 * @param cap  initial capacity, pass '0' for no initial memory allocation
 * @return     'true' on success, 'false' on out of memory
 */
bool sc_heap_init(struct sc_heap *heap, size_t cap);

/**
 * Destroys heap, frees memory
 * @param heap heap
 */
void sc_heap_term(struct sc_heap *heap);

/**
 * @param heap heap
 * @return     element count
 */
size_t sc_heap_size(struct sc_heap *heap);

/**
 * Clears elements from the queue, does not free the allocated memory.
 * @param heap heap
 */
void sc_heap_clear(struct sc_heap *heap);

/**
 * @param heap heap
 * @param key  key
 * @param data data
 * @return     'false' on out of memory.
 */
bool sc_heap_add(struct sc_heap *heap, int64_t key, void *data);

/**
 * Read top element without removing from the heap.
 *
 * @param heap heap
 * @param key  [out] key
 * @param data [out] data
 * @return     'false' if there is no element in the heap.
 */
bool sc_heap_peek(struct sc_heap *heap, int64_t *key, void **data);

/**
 * Read top element and remove it from the heap.
 *
 * @param heap heap
 * @param key  [out] key
 * @param data [out] data
 * @return     'false' if there is no element in the heap.
 */
bool sc_heap_pop(struct sc_heap *heap, int64_t *key, void **data);


#endif
