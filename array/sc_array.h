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

#ifndef SC_ARRAY_H
#define SC_ARRAY_H

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * Internals, do not use
 */
struct sc_array
{
    size_t size;
    size_t cap;
    uint8_t elems[];
};

#define sc_array_meta(arr)                                                     \
    ((struct sc_array *) ((char *) (arr) -offsetof(struct sc_array, elems)))

bool sc_array_init(void **arr, size_t elem_size, size_t cap);
void sc_array_term(void **arr);
bool sc_array_expand(void **arr, size_t elem_size);

/**
 * Internal End.
 */


/**
 * If you want to log or abort on out of memory, put your error function here.
 * It will be called with printf like error msg.
 *
 * my_on_oom(const char* fmt, ...);
 */
#define sc_array_on_error(...)

/**
 * Configure memory allocators here. You can plug your allocator if you want,
 * replace 'realloc' and 'free' with your allocator, make sure you include
 * new allocator header.
 */
#define sc_array_realloc realloc
#define sc_array_free    free

/**
 *   @param arr Array pointer
 *   @param cap Initial capacity, '0' is accepted, then no memory allocation
 *              will be made until first 'add' operation.
 *   @return   'true' on success, 'false' on out of memory
 */
#define sc_array_create(arr, cap)                                              \
    sc_array_init((void **) &(arr), sizeof(*(arr)), cap)

/**
 *   @param arr Array pointer
 */
#define sc_array_destroy(arr) sc_array_term((void **) &(arr));

/**
 *   @param arr Array pointer
 *   @return    Current allocated capacity
 */
#define sc_array_cap(arr) (sc_array_meta((arr))->cap)

/**
 *   @param arr Array pointer
 *   @return    Current element count
 */
#define sc_array_size(arr) (sc_array_meta((arr))->size)

/**
 *   Removes items from the list without deallocating underlying memory
 *
 *   @param arr Array pointer
 */
#define sc_array_clear(arr) (sc_array_meta((arr))->size = 0)

/**
 *   @param v    Array pointer
 *   @param elem Element to be appended
 *   @return     'true' on success, 'false' if memory allocation fails if we try
 *               to expand underlying memory.
 */
#define sc_array_add(arr, elem)                                                \
    sc_array_expand((void **) &((arr)), sizeof(*(arr))) == true ?              \
            (arr)[sc_array_meta(arr)->size++] = (elem),                        \
            true : false


/**
 *   Removes the element at index i, moves elements to fill the space
 *   unless removed element is the last element
 *
 *   vec[a,b,c,d,e,f] -> sc_array_remove(vec, 2) - > vec[a,b,d,f,e]
 *
 *   @param arr Array pointer
 *   @param i   Element index to be removed
 *
 *   If 'i' is out of the range, result is undefined.
 */
#define sc_array_remove(arr, i)                                                \
    do {                                                                       \
        assert((i) < sc_array_meta(arr)->size);                                \
        const size_t to_move = sc_array_size(arr) - (i) -1;                    \
        if (to_move > 0) {                                                     \
            memmove(&(arr)[i], &(arr)[(i) + 1], to_move * sizeof(*(arr)));     \
        }                                                                      \
        sc_array_meta((arr))->size--;                                          \
    } while (0)

/**
 *   Removes an element at index i, replaces last element with removed element
 *   unless removed element is the last element. This is faster than moving
 *   elements but elements will no longer be in the push order
 *
 *   arr[a,b,c,d,e,f] -> sc_array_remove_unordered(vec, 2) - > arr[a,b,f,d,e]
 *
 *   @param arr Array pointer
 *   @param i   Element index to be removed
 *
 *   If 'i' is out of the range, result is undefined.
 */
#define sc_array_remove_unordered(arr, i)                                      \
    do {                                                                       \
        assert((i) < sc_array_meta(arr)->size);                                \
        (arr)[i] = (arr)[(--sc_array_meta((arr))->size)];                      \
    } while (0)


/**
 *   Remove the last element. If there is no element, result is undefined.
 *   @param arr Array pointer
 */
#define sc_array_remove_last(arr)                                              \
    do {                                                                       \
        assert(sc_array_meta(arr)->size != 0);                                 \
        sc_array_meta(arr)->size--;                                            \
    } while (0)

/**
 *   Sorts the array using qsort()
 *   @param arr Array pointer
 *   @param cmp Comparator, check qsort docs online for details
 */
#define sc_array_sort(arr, cmp)                                                \
    (qsort((arr), sc_array_size((arr)), sizeof(*(arr)), cmp))

/**
 *  @param arr   Array Pointer
 *  @param value Value
 */
#define sc_array_foreach(arr, value)                                           \
    if (sc_array_size(arr) > 0) {                                              \
        (value) = (arr)[0];                                                    \
    }                                                                          \
    for (int _i = 0; _i < sc_array_size(arr); _i++, (value) = (arr)[_i])

/**
 * Return last element. If array is empty, result is undefined.
 */
#define sc_array_last(arr)                                                     \
    assert(sc_array_size(arr) > 0), (arr)[sc_array_size(arr) - 1]

#endif
