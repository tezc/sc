/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
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

#include <stdbool.h>
#include <stddef.h>

#ifndef SC_SIZE_MAX
    #define SC_SIZE_MAX SIZE_MAX
#endif


/**
 * Empty array instance.
 * Zero element arrays point at it to avoid initial allocation, so unused
 * arrays will not allocate memory.
 */
static const struct sc_array sc_empty = {.size = 0, .cap = 0};

bool sc_array_init(void **arr, size_t elem_size, size_t cap)
{
    struct sc_array *meta;

    if (cap == 0) {
        *arr = (void *) sc_empty.elems;
        return true;
    }

    // Check overflow
    if (cap > SC_SIZE_MAX / elem_size) {
        *arr = NULL;
        return false;
    }

    meta = sc_array_realloc(NULL, sizeof(*meta) + (elem_size * cap));
    if (meta == NULL) {
        *arr = NULL;
        return false;
    }

    meta->size = 0;
    meta->cap = cap;
    *arr = meta->elems;

    return true;
}

void sc_array_term(void **arr)
{
    struct sc_array *meta = sc_array_meta(*arr);

    if (meta != &sc_empty) {
        sc_array_free(meta);
    }

    *arr = NULL;
}

bool sc_array_expand(void **arr, size_t elem_size)
{
    size_t size, cap;
    struct sc_array *prev, *meta = sc_array_meta(*arr);

    if (meta->size == meta->cap) {

        // Check overflow
        if (meta->cap > SC_SIZE_MAX / elem_size / 2) {
            return false;
        }

        size = meta->size;
        cap = (meta != &sc_empty) ? meta->cap * 2 : 2;
        prev = (meta != &sc_empty) ? meta : NULL;

        meta = sc_array_realloc(prev, sizeof(*meta) + (elem_size * cap));
        if (meta == NULL) {
            return false;
        }

        meta->size = size;
        meta->cap = cap;
        *arr = meta->elems;
    }

    return true;
}
