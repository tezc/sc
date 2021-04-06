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
