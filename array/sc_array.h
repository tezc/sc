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

#ifndef SC_ARRAY_H
#define SC_ARRAY_H

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define SC_ARRAY_VERSION "1.0.0"

#ifdef SC_HAVE_CONFIG_H
    #include "config.h"
#else
    #define sc_array_realloc realloc
    #define sc_array_free    free
#endif

// Internals, do not use
struct sc_array
{
    size_t size;
    size_t cap;
    unsigned char elems[];
};

#define sc_array_meta(arr)                                                     \
    ((struct sc_array *) ((char *) (arr) -offsetof(struct sc_array, elems)))

bool sc_array_init(void *arr, size_t elem_size, size_t cap);
void sc_array_term(void *arr);
bool sc_array_expand(void *arr, size_t elem_size);
#define sc_array_sizeof(a) (sizeof(a)) // NOLINT
// Internals end

/**
 *   @param arr array
 *   @param cap initial capacity. '0' is a valid initial capacity.
 *   @return   'true' on success, 'false' on out of memory
 */
#define sc_array_create(arr, cap)                                              \
    sc_array_init(&(arr), sc_array_sizeof(*(arr)), cap)

/**
 *   @param arr array to be destroyed
 */
#define sc_array_destroy(arr) sc_array_term(&(arr));

/**
 *   @param arr array
 *   @return    current allocated capacity
 */
#define sc_array_cap(arr) (sc_array_meta((arr))->cap)

/**
 *   @param arr array
 *   @return    current element count
 */
#define sc_array_size(arr) (sc_array_meta((arr))->size)

/**
 *   Deletes items from the array without deallocating underlying memory
 *   @param arr array
 */
#define sc_array_clear(arr) (sc_array_meta((arr))->size = 0)

/**
 *   @param arr  array
 *   @param elem element to be appended
 *   @return     'true' on success, 'false' on out of memory.
 */
#define sc_array_add(arr, elem)                                                \
    sc_array_expand(&((arr)), sc_array_sizeof(*(arr))) == true ?               \
            (arr)[sc_array_meta(arr)->size++] = (elem),                        \
            true : false

/**
 *   @param arr array
 *   @param i   element index, If 'i' is out of the range, result is undefined.
 */
#define sc_array_del(arr, i)                                                   \
    do {                                                                       \
        assert((i) < sc_array_meta(arr)->size);                                \
        const size_t to_move = sc_array_size(arr) - (i) -1;                    \
        if (to_move > 0) {                                                     \
            memmove(&(arr)[i], &(arr)[(i) + 1], to_move * sizeof(*(arr)));     \
        }                                                                      \
        sc_array_meta((arr))->size--;                                          \
    } while (0)

/**
 *   Deletes the element at index i, replaces last element with the deleted
 *   element unless deleted element is the last element. This is faster than
 *   moving elements but elements will no longer be in the 'add order'
 *
 *   arr[a,b,c,d,e,f] -> sc_array_del_unordered(vec, 2) - > arr[a,b,f,d,e]
 *
 *   @param arr array
 *   @param i   index. If 'i' is out of the range, result is undefined.
 */
#define sc_array_del_unordered(arr, i)                                         \
    do {                                                                       \
        assert((i) < sc_array_meta(arr)->size);                                \
        (arr)[i] = (arr)[(--sc_array_meta((arr))->size)];                      \
    } while (0)

/**
 *   Deletes the last element. If current size is zero, result is undefined.
 *   @param arr array
 */
#define sc_array_del_last(arr)                                                 \
    do {                                                                       \
        assert(sc_array_meta(arr)->size != 0);                                 \
        sc_array_meta(arr)->size--;                                            \
    } while (0)

/**
 *   Sorts the array using qsort()
 *   @param arr array.
 *   @param cmp comparator, check qsort() documentation for details
 */
#define sc_array_sort(arr, cmp)                                                \
    (qsort((arr), sc_array_size((arr)), sc_array_sizeof(*(arr)), cmp))

/**
 *  @param arr  array
 *  @param elem elem
 */
#define sc_array_foreach(arr, elem)                                            \
    for (size_t _k = 1, _i = 0; _k && _i != sc_array_size(arr);                \
         _k = !_k, _i++)                                                       \
        for ((elem) = (arr)[_i]; _k; _k = !_k)

/**
 * Returns last element. If array is empty, result is undefined.
 */
#define sc_array_last(arr) (arr)[sc_array_size(arr) - 1]

#endif
