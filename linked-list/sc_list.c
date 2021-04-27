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

#include "sc_list.h"

void sc_list_init(struct sc_list *l)
{
	l->next = l;
	l->prev = l;
}

void sc_list_clear(struct sc_list *l)
{
	struct sc_list *tmp, *elem;

	sc_list_foreach_safe (l, tmp, elem) {
		sc_list_del(NULL, elem);
	}
}

bool sc_list_is_empty(struct sc_list *l)
{
	return l->next == l;
}

size_t sc_list_count(struct sc_list *l)
{
	size_t count = 0;
	struct sc_list *elem;

	sc_list_foreach (l, elem) {
		count++;
	}

	return count;
}

struct sc_list *sc_list_head(struct sc_list *l)
{
	return l->next != l ? l->next : NULL;
}

struct sc_list *sc_list_tail(struct sc_list *l)
{
	return l->prev != l ? l->prev : NULL;
}

void sc_list_add_tail(struct sc_list *l, struct sc_list *elem)
{
	struct sc_list *prev;

	// Delete if exists to prevent adding same item twice
	sc_list_del(l, elem);

	prev = l->prev;
	l->prev = elem;
	elem->next = l;
	elem->prev = prev;
	prev->next = elem;
}

struct sc_list *sc_list_pop_tail(struct sc_list *l)
{
	struct sc_list *tail = l->prev;

	if (sc_list_is_empty(l)) {
		return NULL;
	}

	sc_list_del(l, l->prev);

	return tail;
}

void sc_list_add_head(struct sc_list *l, struct sc_list *elem)
{
	struct sc_list *next;

	// Delete if exists to prevent adding same item twice
	sc_list_del(l, elem);

	next = l->next;
	l->next = elem;
	elem->prev = l;
	elem->next = next;
	next->prev = elem;
}

struct sc_list *sc_list_pop_head(struct sc_list *l)
{
	struct sc_list *head = l->next;

	if (sc_list_is_empty(l)) {
		return NULL;
	}

	sc_list_del(l, l->next);

	return head;
}

void sc_list_add_after(struct sc_list *l, struct sc_list *prev,
		       struct sc_list *elem)
{
	(void) l;
	struct sc_list *next;

	// Delete if exists to prevent adding same item twice
	sc_list_del(l, elem);

	next = prev->next;
	prev->next = elem;
	elem->next = next;
	elem->prev = prev;
	next->prev = elem;
}

void sc_list_add_before(struct sc_list *l, struct sc_list *next,
			struct sc_list *elem)
{
	(void) l;
	struct sc_list *prev;

	// Delete if exists to prevent adding same item twice
	sc_list_del(l, elem);

	prev = next->prev;
	next->prev = elem;
	elem->next = next;
	elem->prev = prev;
	prev->next = elem;
}

void sc_list_del(struct sc_list *l, struct sc_list *elem)
{
	(void) (l);

	elem->prev->next = elem->next;
	elem->next->prev = elem->prev;
	elem->next = elem;
	elem->prev = elem;
}
