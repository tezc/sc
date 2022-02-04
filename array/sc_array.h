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

#ifndef SC_ARRAY_H
#define SC_ARRAY_H

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define SC_ARRAY_VERSION "2.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_array_realloc realloc
#define sc_array_free free
#endif

#ifndef SC_ARRAY_MAX
#define SC_ARRAY_MAX SIZE_MAX
#endif

#define sc_array_def(T, name)                                                  \
	struct sc_array_##name {                                               \
		bool oom;                                                      \
		size_t cap;                                                    \
		size_t size;                                                   \
		/* NOLINTNEXTLINE */                                           \
		T *elems;                                                      \
	}
/**
 * Init array
 * @param a array
 */
#define sc_array_init(a)                                                       \
	do {                                                                   \
		memset((a), 0, sizeof(*(a)));                                  \
	} while (0)

/**
 * Term array
 * @param a array
 */
#define sc_array_term(a)                                                       \
	do {                                                                   \
		sc_array_free((a)->elems);                                     \
		sc_array_init(a);                                              \
	} while (0)

/**
 * Add elem to array, call sc_array_oom(v) to see if 'add' failed because of out
 * of memory.
 *
 * @param a array
 * @param k elem
 */
#define sc_array_add(a, k)                                                     \
	do {                                                                   \
		const size_t _max = SC_ARRAY_MAX / sizeof(*(a)->elems);        \
		size_t _cap;                                                   \
		void *_p;                                                      \
                                                                               \
		if ((a)->cap == (a)->size) {                                   \
			if ((a)->cap > _max / 2) {                             \
				(a)->oom = true;                               \
				break;                                         \
			}                                                      \
			_cap = (a)->cap == 0 ? 8 : (a)->cap * 2;               \
			_p = sc_array_realloc((a)->elems,                      \
					      _cap * sizeof(*((a)->elems)));   \
			if (_p == NULL) {                                      \
				(a)->oom = true;                               \
				break;                                         \
			}                                                      \
			(a)->cap = _cap;                                       \
			(a)->elems = _p;                                       \
		}                                                              \
		(a)->oom = false;                                              \
		(a)->elems[(a)->size++] = k;                                   \
	} while (0)

/**
 * Deletes items from the array without deallocating underlying memory
 * @param a array
 */
#define sc_array_clear(a)                                                      \
	do {                                                                   \
		(a)->cap = 0;                                                  \
		(a)->size = 0;                                                 \
		(a)->oom = false;                                              \
	} while (0)

/**
 * @param a array
 * @return true if last add operation failed, false otherwise.
 */
#define sc_array_oom(a) ((a)->oom)

/**
 * Get element at index i, if 'i' is out of range, result is undefined.
 *
 * @param a array
 * @param i index
 * @return element at index 'i'
 */
#define sc_array_at(a, i) ((a)->elems[i])

/**
 * @param a array
 * @return element count
 */
#define sc_array_size(a) ((a)->size)

/**
 *   @param a array
 *   @param i element index, If 'i' is out of the range, result is undefined.
 */
#define sc_array_del(a, i)                                                     \
	do {                                                                   \
		assert((i) < (a)->size);                                       \
		const size_t _cnt = (a)->size - (i) -1;                        \
		if (_cnt > 0) {                                                \
			memmove(&((a)->elems[i]), &((a)->elems[(i) + 1]),      \
				_cnt * sizeof(*((a)->elems)));                 \
		}                                                              \
		(a)->size--;                                                   \
	} while (0)

/**
 * Deletes the element at index i, replaces last element with the deleted
 * element unless deleted element is the last element. This is faster than
 * moving elements but elements will no longer be in the 'add order'
 *
 * arr[a,b,c,d,e,f] -> sc_array_del_unordered(arr, 2) - > arr[a,b,f,d,e]
 *
 * @param a array
 * @param i index. If 'i' is out of the range, result is undefined.
 */
#define sc_array_del_unordered(a, i)                                           \
	do {                                                                   \
		assert((i) < (a)->size);                                       \
		(a)->elems[i] = (a)->elems[(--(a)->size)];                     \
	} while (0)

/**
 * Deletes the last element. If current size is zero, result is undefined.
 * @param a array
 */
#define sc_array_del_last(a)                                                   \
	do {                                                                   \
		assert((a)->size != 0);                                        \
		(a)->size--;                                                   \
	} while (0)

/**
 * Sorts the array using qsort()
 * @param a   array
 * @param cmp comparator, check qsort() documentation for details
 */
#define sc_array_sort(a, cmp)                                                  \
	(qsort((a)->elems, (a)->size, sizeof(*(a)->elems), cmp))

/**
 * Returns last element. If array is empty, result is undefined.
 * @param a array
 */
#define sc_array_last(a) (a)->elems[(a)->size - 1]

/**
 * @param a    array
 * @param elem elem
 */
#define sc_array_foreach(a, elem)                                              \
	for (size_t _k = 1, _i = 0; _k && _i != (a)->size; _k = !_k, _i++)     \
		for ((elem) = (a)->elems[_i]; _k; _k = !_k)

//        (type, name)
sc_array_def(int, int);
sc_array_def(unsigned int, uint);
sc_array_def(long, long);
sc_array_def(unsigned long, ulong);
sc_array_def(unsigned long long, ull);
sc_array_def(uint32_t, 32);
sc_array_def(uint64_t, 64);
sc_array_def(double, double);
sc_array_def(const char *, str);
sc_array_def(void *, ptr);

#endif
