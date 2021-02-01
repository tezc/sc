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

#ifndef SC_QUEUE_H
#define SC_QUEUE_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef SC_HAVE_CONFIG_H
    #include "sc_config.h"
#else
    #define sc_queue_realloc realloc
    #define sc_queue_free    free
#endif

/**
 * Internals, do not use
 */
struct sc_queue
{
    size_t cap;
    size_t first;
    size_t last;
    unsigned char elems[];
};

#define sc_queue_meta(q)                                                       \
    ((struct sc_queue *) ((char *) (q) -offsetof(struct sc_queue, elems)))

static inline size_t sc_queue_inc_first(void *q)
{
    struct sc_queue *meta = sc_queue_meta(q);
    size_t tmp = meta->first;

    meta->first = (meta->first + 1) & (meta->cap - 1);
    return tmp;
}

static inline size_t sc_queue_inc_last(void *q)
{
    struct sc_queue *meta = sc_queue_meta(q);
    size_t tmp = meta->last;

    meta->last = (meta->last + 1) & (meta->cap - 1);
    return tmp;
}

static inline size_t sc_queue_dec_first(void *q)
{
    struct sc_queue *meta = sc_queue_meta(q);

    meta->first = (meta->first - 1) & (meta->cap - 1);
    return meta->first;
}

static inline size_t sc_queue_dec_last(void *q)
{
    struct sc_queue *meta = sc_queue_meta(q);

    meta->last = (meta->last - 1) & (meta->cap - 1);
    return meta->last;
}

bool sc_queue_init(void *q, size_t elem_size, size_t cap);
void sc_queue_term(void *q);
bool sc_queue_expand(void *q, size_t elem_size);

/**
 *   @param q     Queue pointer
 *   @param count Initial capacity, '0' is a valid value if you don't want to
 *                allocate memory immediately.
 */
#define sc_queue_create(q, count) sc_queue_init(&(q), sizeof(*(q)), count)

/**
 *   Deallocate underlying memory.
 *   @param q Queue pointer
 */
#define sc_queue_destroy(q) sc_queue_term((&(q)))

/**
 *   @param q Queue pointer
 *   @return  Current capacity
 */
#define sc_queue_cap(q) (sc_queue_meta((q))->cap)

/**
 *   @param q Queue pointer
 *   @return  Element count
 */
#define sc_queue_size(q)                                                       \
    ((sc_queue_meta(q)->last - sc_queue_meta(q)->first) &                      \
     (sc_queue_meta(q)->cap - 1))

/**
 *   @param q Queue pointer
 *   @return true if queue is empty
 */
#define sc_queue_empty(q) ((sc_queue_meta(q)->last == sc_queue_meta(q)->first))

/**
 * Clear the queue without deallocating underlying memory.
 * @param q Queue pointer
 */
#define sc_queue_clear(q)                                                      \
    do {                                                                       \
        sc_queue_meta(q)->first = 0;                                           \
        sc_queue_meta(q)->last = 0;                                            \
    } while (0)

/**
 * @param q Queue pointer
 * @return  Index of the first element. If queue is empty, result is undefined.
 */
#define sc_queue_first(q) (sc_queue_meta(q)->first)

/**
 * @param q Queue pointer
 * @return  Index of the last element. If queue is empty, result is undefined.
 */
#define sc_queue_last(q) (sc_queue_meta(q)->last)

/**
 * @return Index of the next element after i, if there is no element after i
 *            result is undefined.
 */
#define sc_queue_next(q, i) (((i) + 1) & (sc_queue_meta(q)->cap - 1))

/**
 * Returns element at index 'i', so regular loops are possible :
 *
 *      for (size_t i = 0; i < sc_queue_size(q); i++) {
 *           printf("%d" \n, sc_queue_at(q, i));
 *      }
 *
 *   @param q Queue pointer
 *   @return element at index i
 */
#define sc_queue_at(q, i)                                                      \
    (q)[((sc_queue_meta(q)->first) + (i)) & (sc_queue_cap(q) - 1)]

/**
 *   @param q Queue pointer
 *   @return  First element without removing from the queue.
 *            If queue is empty, result is undefined
 */
#define sc_queue_peek_first(q) ((q)[sc_queue_meta(q)->first])

/**
 *   @param q Queue pointer
 *   @return  Last element without removing from the queue.
 *            If queue is empty, result is undefined
 */
#define sc_queue_peek_last(q)                                                  \
    (q)[(sc_queue_meta(q)->last - 1) & (sc_queue_meta(q)->cap - 1)]

/**
 * @param q    Queue pointer
 * @param elem Elem to be added at the end of the list
 * @return     'true' on success, 'false' on out of memory.
 */
#define sc_queue_add_last(q, elem)                                             \
    sc_queue_expand(&(q), sizeof(*(q))) == true ?                              \
            (q)[sc_queue_inc_last((q))] = (elem),                              \
            true : false

/**
 * @param q Queue pointer
 * @return  Delete the last element from the queue and return its value.
 *            If queue is empty, result is undefined.
 */
#define sc_queue_del_last(q) ((q)[sc_queue_dec_last((q))])

/**
 * @param q    Queue pointer.
 * @param elem Elem to be added at the head of the list.
 * @return     'true' on success, 'false' on out of memory.
 */
#define sc_queue_add_first(q, elem)                                            \
    sc_queue_expand(&(q), sizeof(*(q))) == true ?                              \
            (q)[sc_queue_dec_first((q))] = (elem),                             \
            true : false

/**
 * @param q Queue pointer
 * @return  Delete the first element from the queue and return its value.
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
    for (int _k = 1, _i = sc_queue_first(q); _k && _i != sc_queue_last(q);     \
         _k = !_k, _i = sc_queue_next(q, _i))                                  \
        for ((elem) = (q)[_i]; _k; _k = !_k)

#endif
