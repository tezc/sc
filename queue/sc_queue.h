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
#define sc_queue_realloc realloc
#define sc_queue_free	 free
#endif

// Internals
struct sc_queue {
	size_t cap;
	size_t first;
	size_t last;
	unsigned char elems[];
};

#define sc_queue_meta(q)                                                       \
	((struct sc_queue *) ((char *) (q) -offsetof(struct sc_queue, elems)))

static inline size_t sc_queue_inc_first(void *q)
{
	struct sc_queue *m = sc_queue_meta(q);
	size_t tmp = m->first;

	m->first = (m->first + 1) & (m->cap - 1);
	return tmp;
}

static inline size_t sc_queue_inc_last(void *q)
{
	struct sc_queue *m = sc_queue_meta(q);
	size_t tmp = m->last;

	m->last = (m->last + 1) & (m->cap - 1);
	return tmp;
}

static inline size_t sc_queue_dec_first(void *q)
{
	struct sc_queue *m = sc_queue_meta(q);

	m->first = (m->first - 1) & (m->cap - 1);
	return m->first;
}

static inline size_t sc_queue_dec_last(void *q)
{
	struct sc_queue *m = sc_queue_meta(q);

	m->last = (m->last - 1) & (m->cap - 1);
	return m->last;
}

bool sc_queue_init(void *q, size_t elem_size, size_t cap);
void sc_queue_term(void *q);
bool sc_queue_expand(void *q, size_t elem_size);
#define sc_queue_sizeof(a) (sizeof(a)) // NOLINT

/**
 *   @param q     queue
 *   @param count initial capacity, '0' is accepted.
 *   @return      'true' on success, 'false' on out of memory.
 */
#define sc_queue_create(q, count)                                              \
	sc_queue_init(&(q), sc_queue_sizeof(*(q)), count)

/**
 *   Destroy queue
 *   @param q queue
 */
#define sc_queue_destroy(q) sc_queue_term((&(q)))

/**
 *   @param q queue
 *   @return  current capacity
 */
#define sc_queue_cap(q) (sc_queue_meta((q))->cap)

/**
 *   @param q queue
 *   @return  element count
 */
#define sc_queue_size(q)                                                       \
	((sc_queue_meta(q)->last - sc_queue_meta(q)->first) &                  \
	 (sc_queue_meta(q)->cap - 1))

/**
 *   @param q queue
 *   @return true if queue is empty
 */
#define sc_queue_empty(q) ((sc_queue_meta(q)->last == sc_queue_meta(q)->first))

/**
 * Clear the queue without deallocating underlying memory.
 * @param q queue
 */
#define sc_queue_clear(q)                                                      \
	do {                                                                   \
		sc_queue_meta(q)->first = 0;                                   \
		sc_queue_meta(q)->last = 0;                                    \
	} while (0)

/**
 * @param q queue
 * @return  index of the first element. If queue is empty, result is undefined.
 */
#define sc_queue_first(q) (sc_queue_meta(q)->first)

/**
 * @param q queue
 * @return  index of the last element. If queue is empty, result is undefined.
 */
#define sc_queue_last(q) (sc_queue_meta(q)->last)

/**
 * @return index of the next element after i, if i is the last element,
 *            result is undefined.
 */
#define sc_queue_next(q, i) (((i) + 1) & (sc_queue_meta(q)->cap - 1))

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
#define sc_queue_at(q, i)                                                      \
	(q)[((sc_queue_meta(q)->first) + (i)) & (sc_queue_cap(q) - 1)]

/**
 *   @param q queue
 *   @return  peek first element, if queue is empty, result is undefined
 */
#define sc_queue_peek_first(q) ((q)[sc_queue_meta(q)->first])

/**
 *   @param q queue
 *   @return  peek last element, if queue is empty, result is undefined
 */
#define sc_queue_peek_last(q)                                                  \
	(q)[(sc_queue_meta(q)->last - 1) & (sc_queue_meta(q)->cap - 1)]

/**
 * @param q    queue
 * @param elem elem to be added at the end of the list
 * @return     'true' on success, 'false' on out of memory.
 */
#define sc_queue_add_last(q, elem)                                             \
	sc_queue_expand(&(q), sc_queue_sizeof(*(q))) == true ?                 \
		      (q)[sc_queue_inc_last((q))] = (elem), true : false

/**
 * @param q queue
 * @return  delete the last element from the queue and return its value.
 *          If queue is empty, result is undefined.
 */
#define sc_queue_del_last(q) ((q)[sc_queue_dec_last((q))])

/**
 * @param q    queue.
 * @param elem elem to be added at the head of the list.
 * @return     'true' on success, 'false' on out of memory.
 */
#define sc_queue_add_first(q, elem)                                            \
	sc_queue_expand(&(q), sc_queue_sizeof(*(q))) == true ?                 \
		      (q)[sc_queue_dec_first((q))] = (elem), true : false

/**
 * @param q queue
 * @return  delete the first element from the queue and return its value.
 *          If queue is empty, result is undefined.
 */
#define sc_queue_del_first(q) (q)[sc_queue_inc_first((q))]

/**
 *  For each loop,
 *
 *  int *queue;
 *  sc_queue_create(queue, 4);
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
		for ((elem) = (q)[_i]; _k; _k = !_k)

#endif
