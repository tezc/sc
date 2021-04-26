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

#ifndef SC_QUEUE_H
#define SC_QUEUE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SC_QUEUE_VERSION "2.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_queue_calloc calloc
#define sc_queue_free free
#endif

#ifndef SC_QUEUE_MAX
#define SC_QUEUE_MAX (SIZE_MAX)
#endif

#define sc_queue_def(T, name)                                                  \
	struct sc_queue_##name {                                               \
		bool oom;                                                      \
		size_t cap;                                                    \
		size_t first;                                                  \
		size_t last;                                                   \
		/* NOLINTNEXTLINE */                                           \
		T *elems;                                                      \
	}

#define sc_queue_expand(q)                                                     \
	do {                                                                   \
		size_t _cap, _len, _off;                                       \
		size_t _pos = ((q)->last + 1) & ((q)->cap - 1);                \
		void *_dst, *_src;                                             \
                                                                               \
		if (_pos == (q)->first) {                                      \
			if ((q)->cap > SC_QUEUE_MAX / 2ul) {                   \
				(q)->oom = true;                               \
				break;                                         \
			}                                                      \
			_cap = (q)->cap * 2;                                   \
			_dst = sc_queue_calloc(_cap, sizeof(*((q)->elems)));   \
			if (_dst == NULL) {                                    \
				(q)->oom = true;                               \
				break;                                         \
			}                                                      \
			_len = ((q)->cap - (q)->first) * sizeof(*(q)->elems);  \
			_off = ((q)->first * sizeof(*((q)->elems)));           \
			_src = ((char *) (q)->elems) + _off;                   \
                                                                               \
			memcpy(_dst, _src, _len);                              \
			memcpy(((char *) _dst) + _len, (q)->elems, _off);      \
			(q)->oom = false;                                      \
			(q)->last = (q)->cap - 1;                              \
			(q)->first = 0;                                        \
			(q)->cap = _cap;                                       \
			sc_queue_free((q)->elems);                             \
			(q)->elems = _dst;                                     \
		}                                                              \
	} while (0)

/**
 *   Init queue. Call sc_queue_oom(q) to see if memory allocation succeeded.
 *   @param q queue
 */
#define sc_queue_init(q)                                                       \
	do {                                                                   \
		(q)->oom = false;                                              \
		(q)->cap = 8;                                                  \
		(q)->first = 0;                                                \
		(q)->last = 0;                                                 \
		(q)->elems = sc_queue_calloc(1, sizeof(*(q)->elems) * 8);      \
		if ((q)->elems == NULL) {                                      \
			(q)->oom = true;                                       \
		}                                                              \
	} while (0)

/**
 *   Term queue
 *   @param q queue
 */
#define sc_queue_term(q)                                                       \
	do {                                                                   \
		sc_queue_free((q)->elems);                                     \
		(q)->elems = NULL;                                             \
		(q)->cap = 0;                                                  \
		(q)->first = 0;                                                \
		(q)->last = 0;                                                 \
		(q)->oom = false;                                              \
	} while (0)

/**
 * @param q queue
 * @return true if last add operation failed, false otherwise.
 */
#define sc_queue_oom(q) ((q)->oom)

/**
 *   @param q queue
 *   @return  element count
 */
#define sc_queue_size(q) (((q)->last - (q)->first) & ((q)->cap - 1))

/**
 * Clear the queue without deallocating underlying memory.
 * @param q queue
 */
#define sc_queue_clear(q)                                                      \
	do {                                                                   \
		(q)->first = 0;                                                \
		(q)->last = 0;                                                 \
		(q)->oom = false;                                              \
	} while (0)

/**
 *   @param q queue
 *   @return true if queue is empty
 */
#define sc_queue_empty(q) (((q)->last == (q)->first))

/**
 * @param q queue
 * @return  index of the first element. If queue is empty, result is undefined.
 */
#define sc_queue_first(q) ((q)->first)

/**
 * @param q queue
 * @return  index of the last element. If queue is empty, result is undefined.
 */
#define sc_queue_last(q) ((q)->last)

/**
 * @param q queue
 * @param i index
 * @return  index of the next element after i, if i is the last element,
 *            result is undefined.
 */
#define sc_queue_next(q, i) (((i) + 1) & ((q)->cap - 1))

/**
 *   Returns element at index 'i', so regular loops are possible :
 *
 *   for (size_t i = 0; i < sc_queue_size(q); i++) {
 *        printf("%d" \n, sc_queue_at(q, i));
 *   }
 *
 *   @param q queue
 *   @return element at index i
 */
#define sc_queue_at(q, i) (q)->elems[(((q)->first) + (i)) & ((q)->cap - 1)]

/**
 *   @param q queue
 *   @return  peek first element, if queue is empty, result is undefined
 */
#define sc_queue_peek_first(q) ((q)->elems[(q)->first])

/**
 *   @param q queue
 *   @return  peek last element, if queue is empty, result is undefined
 */
#define sc_queue_peek_last(q) (q)->elems[((q)->last - 1) & ((q)->cap - 1)]

/**
 * Call sc_queue_oom(q) after this function to check out of memory condition.
 *
 * @param q    queue
 * @param elem elem to be added at the end of the list
 */
#define sc_queue_add_last(q, elem)                                             \
	do {                                                                   \
		sc_queue_expand(q);                                            \
		if ((q)->oom) {                                                \
			break;                                                 \
		}                                                              \
		(q)->oom = false;                                              \
		(q)->elems[(q)->last] = elem;                                  \
		(q)->last = ((q)->last + 1) & ((q)->cap - 1);                  \
	} while (0)

/**
 * @param q queue
 * @return  delete the last element from the queue and return its value.
 *          If queue is empty, result is undefined.
 */
#define sc_queue_del_last(q)                                                   \
	((q)->elems[((q)->last = ((q)->last - 1) & ((q)->cap - 1))])

/**
 * Call sc_queue_oom(q) after this function to check out of memory condition.
 *
 * @param q    queue.
 * @param elem elem to be added at the head of the list.
 */
#define sc_queue_add_first(q, elem)                                            \
	do {                                                                   \
		sc_queue_expand(q);                                            \
		if ((q)->oom) {                                                \
			break;                                                 \
		}                                                              \
		(q)->oom = false;                                              \
		(q)->first = ((q)->first - 1) & ((q)->cap - 1);                \
		(q)->elems[(q)->first] = elem;                                 \
	} while (0)

static inline size_t sc_queue_inc_first(size_t *first, size_t cap)
{
	size_t tmp = *first;

	*first = (*first + 1) & (cap - 1);
	return tmp;
}

/**
 * @param q queue
 * @return  delete the first element from the queue and return its value.
 *          If queue is empty, result is undefined.
 */
#define sc_queue_del_first(q)                                                  \
	(q)->elems[sc_queue_inc_first(&(q)->first, (q)->cap)]

/**
 *  For each loop,
 *
 *  int *queue;
 *  sc_queue_create(queue, 4);"
 *
 *  int elem;
 *  sc_queue_foreach(queue, elem) {
 *      printf("Elem : %d \n, elem);
 *  }
 */
#define sc_queue_foreach(q, elem)                                              \
	for (size_t _k = 1, _i = sc_queue_first(q);                            \
	     _k && _i != sc_queue_last(q);                                     \
	     _k = !_k, _i = sc_queue_next(q, _i))                              \
		for ((elem) = (q)->elems[_i]; _k; _k = !_k)

//        (type, name)
sc_queue_def(int, int);
sc_queue_def(unsigned int, uint);
sc_queue_def(long, long);
sc_queue_def(unsigned long, ulong);
sc_queue_def(uint32_t, 32);
sc_queue_def(uint64_t, 64);
sc_queue_def(double, double);
sc_queue_def(const char *, str);
sc_queue_def(void *, ptr);

#endif
