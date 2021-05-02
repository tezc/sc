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

#ifndef SC_LIST_H
#define SC_LIST_H

#include <stdbool.h>
#include <stddef.h>

#define SC_LIST_VERSION "2.0.0"

struct sc_list {
	struct sc_list *next;
	struct sc_list *prev;
};

#define sc_list_entry(ptr, type, elem)                                         \
	((type *) ((char *) (ptr) -offsetof(type, elem)))

/**
 * Call once to initialize.
 * @param l list
 */
void sc_list_init(struct sc_list *l);

/**
 * @param l list
 */
void sc_list_clear(struct sc_list *l);

/**
 * @param l list
 * @return     'true' if empty
 */
bool sc_list_is_empty(struct sc_list *l);

/**
 * @param l list
 * @return     element count in the list, beware this is a log(n) operation.
 */
size_t sc_list_count(struct sc_list *l);

/**
 * @param l list
 * @return  returns head. If list is empty, returns NULL.
 */
struct sc_list *sc_list_head(struct sc_list *l);

/**
 * @param l list
 * @return  returns tail. If list is empty, returns NULL.
 */
struct sc_list *sc_list_tail(struct sc_list *l);

/**
 *  before : [head]item1 -> [tail]item2
 *  after  : [head]'elem' -> item1 -> [tail]item2
 *
 * @param l    list
 * @param elem elem to add to the head
 */
void sc_list_add_head(struct sc_list *l, struct sc_list *elem);

/**
 *  before : [head]item1 -> item2 -> item3
 *  after  : [head]item2 -> item3
 *
 * @param l list
 * @return  head element, if list is empty, returns NULL.
 */
struct sc_list *sc_list_pop_head(struct sc_list *l);

/**
 *  before : [head]item1 -> [tail]item2
 *  after  : [head]item1 -> item2 -> [tail]'elem'
 *
 * @param l    list
 * @param elem elem to append to the tail
 */
void sc_list_add_tail(struct sc_list *l, struct sc_list *elem);

/**
 *  before : [head]item1 -> item2 -> item3
 *  after  : [head]item1 -> item2
 *
 * @param l list
 * @return  head element, if list is empty, returns NULL.
 */
struct sc_list *sc_list_pop_tail(struct sc_list *l);

/**
 *  before : item1 -> 'prev' -> item2
 *  after  : item1 -> 'prev' -> 'elem' -> item2
 *
 * @param l    list
 * @param prev previous element of the 'elem'
 * @param elem elem to be added after 'prev'
 */
void sc_list_add_after(struct sc_list *l, struct sc_list *prev,
		       struct sc_list *elem);

/**
 *  before : item1 -> 'next' -> item2
 *  after  : item1 -> 'elem'-> 'next' -> item2
 *
 * @param l    list
 * @param next next element of the 'elem'
 * @param elem elem to be added before 'next'
 */
void sc_list_add_before(struct sc_list *l, struct sc_list *next,
			struct sc_list *elem);

/**
 *  before : item1 -> 'elem' -> item2
 *  after  : item1 -> item2
 *
 * @param l    list
 * @param elem elem to be deleted
 */
void sc_list_del(struct sc_list *l, struct sc_list *elem);

/**
 * struct container {
 *      struct sc_list others;
 * };
 *
 * struct container *container; // User object
 * struct sc_list *list;        // List pointer, should already be initialized.
 * struct sc_list *it;          // Iterator
 *
 * sc_list_foreach (list, it) {
 *       container = sc_list_entry(it, struct container, others);
 * }
 */
#define sc_list_foreach(list, elem)                                            \
	for ((elem) = (list)->next; (elem) != (list); (elem) = (elem)->next)

/**
 *  It is safe to delete items from the list while using this iterator.
 *
 * struct container {
 *      struct sc_list others;
 * };
 *
 * struct container *container; // User object
 * struct sc_list *list;        // List pointer, should already be initialized.
 * struct sc_list *tmp;         // Variable for loop, user should not use this.
 * struct sc_list *it;          // Iterator
 *
 * sc_list_foreach_safe (list, it) {
 *       container = sc_list_entry(it, struct container, others);
 *       sc_list_del(list, &container->others);
 * }
 */
#define sc_list_foreach_safe(list, n, elem)                                    \
	for ((elem) = (list)->next, (n) = (elem)->next; (elem) != (list);      \
	     (elem) = (n), (n) = (elem)->next)

/**
 * Reverse iterator
 *
 * struct container {
 *      struct sc_list others;
 * };
 *
 * struct container *container; // User object
 * struct sc_list *list;        // List pointer, should already be initialized.
 * struct sc_list *it;          // Iterator
 *
 * sc_list_foreach_r (list, it) {
 *       container = sc_list_entry(it, struct container, others);
 * }
 */
#define sc_list_foreach_r(list, elem)                                          \
	for ((elem) = (list)->prev; (elem) != (list); (elem) = (elem)->prev)

/**
 *  Reverse iterator.
 *
 *  It is safe to delete items from the list while using
 *  this iterator.
 *
 * struct container {
 *      struct sc_list others;
 * };
 *
 * struct container *container; // User object
 * struct sc_list *list;        // List pointer, should already be initialized.
 * struct sc_list *tmp;         // Variable for loop, user should not use this.
 * struct sc_list *it;          // Iterator
 *
 * sc_list_foreach_safe_r (list, tmp, it) {
 *       container = sc_list_entry(it, struct container, others);
 *       sc_list_del(list, &container->others);
 * }
 */
#define sc_list_foreach_safe_r(list, n, elem)                                  \
	for ((elem) = (list)->prev, (n) = (elem)->prev; (elem) != (list);      \
	     (elem) = (n), (n) = (elem)->prev)

#endif
