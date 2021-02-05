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

#ifndef SC_SIZE_MAX
    #define SC_SIZE_MAX SIZE_MAX
#endif

#define SC_CAP_MAX SC_SIZE_MAX / sizeof(struct sc_heap_data)

bool sc_heap_init(struct sc_heap *heap, size_t cap)
{
    void *elems;
    const size_t alloc = cap * sizeof(struct sc_heap_data);

    *heap = (struct sc_heap){0};

    if (cap == 0) {
        return true;
    }

    // Check overflow
    if (cap > SC_CAP_MAX || (elems = sc_heap_malloc(alloc)) == NULL) {
        return false;
    }

    heap->elems = elems;
    heap->cap = cap;

    return true;
}

void sc_heap_term(struct sc_heap *heap)
{
    sc_heap_free(heap->elems);
}

size_t sc_heap_size(struct sc_heap *heap)
{
    return heap->size;
}

void sc_heap_clear(struct sc_heap *heap)
{
    heap->size = 0;
}

bool sc_heap_add(struct sc_heap *heap, int64_t key, void *data)
{
    size_t i;
    void *exp;

    if (++heap->size >= heap->cap) {
        const size_t cap = heap->cap != 0 ? heap->cap * 2 : 4;
        const size_t m = cap * 2 * sizeof(struct sc_heap_data);
        // Check overflow
        if (heap->cap >= SC_CAP_MAX / 2 ||
            (exp = sc_heap_realloc(heap->elems, m)) == NULL) {
            return false;
        }

        heap->elems = exp;
        heap->cap = cap;
    }

    i = heap->size;
    while (i != 1 && key < heap->elems[i / 2].key) {
        heap->elems[i] = heap->elems[i / 2];
        i /= 2;
    }

    heap->elems[i].key = key;
    heap->elems[i].data = data;

    return true;
}

bool sc_heap_peek(struct sc_heap *heap, int64_t *key, void **data)
{
    if (heap->size == 0) {
        return false;
    }

    // Top element is always at heap->elems[1].
    *key = heap->elems[1].key;
    *data = heap->elems[1].data;

    return true;
}

bool sc_heap_pop(struct sc_heap *heap, int64_t *key, void **data)
{
    size_t i = 1, child = 2;
    struct sc_heap_data last;

    if (heap->size == 0) {
        return false;
    }

    // Top element is always at heap->elems[1].
    *key = heap->elems[1].key;
    *data = heap->elems[1].data;

    last = heap->elems[heap->size--];
    while (child <= heap->size) {
        if (child < heap->size &&
            heap->elems[child].key > heap->elems[child + 1].key) {
            child++;
        };

        if (last.key <= heap->elems[child].key) {
            break;
        }

        heap->elems[i] = heap->elems[child];

        i = child;
        child *= 2;
    }

    heap->elems[i] = last;

    return true;
}
