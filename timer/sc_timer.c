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

#include "sc_timer.h"

#include <assert.h>
#include <memory.h>

#define TICK	    16u
#define WHEEL_COUNT 16u

#ifndef SC_TIMER_MAX
#define SC_TIMER_MAX (UINT32_MAX / sizeof(struct sc_timer_data)) / WHEEL_COUNT
#endif


bool sc_timer_init(struct sc_timer *t, uint64_t timestamp)
{
	const uint32_t wheel_cap = 4;
	const uint32_t cap = WHEEL_COUNT * wheel_cap;
	const size_t size = cap * sizeof(struct sc_timer_data);

	t->count = 0;
	t->head = 0;
	t->wheel = wheel_cap;
	t->timestamp = timestamp;

	t->list = sc_timer_malloc(size);
	if (t->list == NULL) {
		return false;
	}

	for (uint32_t i = 0; i < cap; i++) {
		t->list[i].timeout = UINT64_MAX;
		t->list[i].data = NULL;
	}

	return true;
}

void sc_timer_term(struct sc_timer *t)
{
	sc_timer_free(t->list);
}

void sc_timer_clear(struct sc_timer *t)
{
	const uint32_t cap = t->wheel * WHEEL_COUNT;

	t->count = 0;
	t->head = 0;

	for (uint32_t i = 0; i < cap; i++) {
		t->list[i].timeout = UINT64_MAX;
		t->list[i].data = NULL;
	}
}

static bool expand(struct sc_timer *t)
{
	uint32_t cap = t->wheel * WHEEL_COUNT * 2;
	size_t size = cap * sizeof(struct sc_timer_data);
	struct sc_timer_data *alloc;

	assert(size != 0);

	if (t->wheel >= SC_TIMER_MAX / 2) {
		return false;
	}

	alloc = sc_timer_malloc(size);
	if (alloc == NULL) {
		return false;
	}

	for (uint32_t i = 0; i < cap; i++) {
		alloc[i].timeout = UINT64_MAX;
		alloc[i].data = NULL;
	}

	// Copy from old list to new list
	for (uint32_t i = 0; i < WHEEL_COUNT; i++) {
		void *dest = &alloc[(i * t->wheel * 2)];
		void *src = &t->list[(i * t->wheel)];
		size_t copy = sizeof(struct sc_timer_data) * t->wheel;

		memcpy(dest, src, copy);
	}

	sc_timer_free(t->list);

	t->list = alloc;
	t->wheel *= 2;

	return true;
}

uint64_t sc_timer_add(struct sc_timer *t, uint64_t timeout, uint64_t type,
		      void *data)
{
	const uint32_t offset = (uint32_t)(timeout / TICK + t->head);
	const uint32_t pos = offset & (WHEEL_COUNT - 1);
	uint64_t id;
	uint32_t seq, index, wheel_pos;


	wheel_pos = (pos * t->wheel);
	for (seq = 0; seq < t->wheel; seq++) {
		index = wheel_pos + seq;
		if (t->list[index].timeout == UINT64_MAX) {
			goto out;
		}
	}

	if (!expand(t)) {
		return SC_TIMER_INVALID;
	}

	index = (pos * t->wheel) + (seq);
	assert(t->list[index].timeout == UINT64_MAX);

out:
	t->count++;
	t->list[index].timeout = timeout + t->timestamp;
	t->list[index].type = type;
	t->list[index].data = data;

	id = (((uint64_t) seq) << 32u) | pos;
	assert(id != SC_TIMER_INVALID);

	return id;
}

void sc_timer_cancel(struct sc_timer *t, uint64_t *id)
{
	uint64_t pos;

	if (*id == SC_TIMER_INVALID) {
		return;
	}

	t->count--;
	pos = (((uint32_t) *id) * t->wheel) + (*id >> 32u);

	assert(t->list[pos].timeout != UINT64_MAX);
	t->list[pos].timeout = UINT64_MAX;
	*id = SC_TIMER_INVALID;
}

#define sc_timer_min(a, b) (a) < (b) ? (a) : (b)

uint64_t sc_timer_timeout(struct sc_timer *t, uint64_t timestamp, void *arg,
			  void (*callback)(void *, uint64_t, uint64_t, void *))
{
	const uint64_t time = timestamp - t->timestamp;
	uint32_t wheel, base;
	uint32_t head = t->head;
	uint32_t wheels = (uint32_t)(sc_timer_min(time / TICK, WHEEL_COUNT));
	struct sc_timer_data *item;

	if (wheels == 0) {
		return sc_timer_min(TICK - time, TICK);
	}

	t->timestamp = timestamp;
	t->head = (t->head + wheels) & (WHEEL_COUNT - 1);

	while (wheels-- > 0) {
		wheel = t->wheel;
		base = wheel * head;

		for (uint32_t i = 0; i < wheel; i++) {
			item = &t->list[base + i];

			if (item->timeout <= t->timestamp) {
				uint64_t timeout = item->timeout;
				item->timeout = UINT64_MAX;

				t->count--;
				callback(arg, timeout, item->type, item->data);

				// Recalculates position each time because there
				// might be newly added timers in the callback
				// and it might require expansion of the list.
				base = t->wheel * head;
			}
		}

		head = (head + 1) & (WHEEL_COUNT - 1);
	}

	return sc_timer_min(TICK - time, TICK);
}
