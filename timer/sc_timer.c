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

#include "sc_timer.h"

#include <assert.h>
#include <memory.h>

#define TICK 16u
#define WHEEL_COUNT 16u

#ifndef SC_TIMER_MAX
#define SC_TIMER_MAX (UINT32_MAX / sizeof(struct sc_timer_data)) / WHEEL_COUNT
#endif

void sc_timer_init(struct sc_timer *t, uint64_t timestamp)
{
	*t = (struct sc_timer){
		.timestamp = timestamp,
	};
}

void sc_timer_term(struct sc_timer *t)
{
	sc_timer_free(t->list);

	*t = (struct sc_timer){
		.timestamp = t->timestamp,
	};
}

void sc_timer_clear(struct sc_timer *t)
{
	const uint32_t cap = t->wheel * WHEEL_COUNT;

	t->head = 0;

	for (uint32_t i = 0; i < cap; i++) {
		t->list[i].timeout = SC_TIMER_INVALID;
		t->list[i].data = NULL;
	}
}

static bool expand(struct sc_timer *t)
{
	uint32_t wheel = t->wheel != 0 ? t->wheel * 2 : 4;
	uint32_t cap = wheel * WHEEL_COUNT * 2;
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
		alloc[i].timeout = SC_TIMER_INVALID;
		alloc[i].data = NULL;
	}

	// Copy from old list to new list
	if (t->wheel != 0) {
		for (uint32_t i = 0; i < WHEEL_COUNT; i++) {
			void *dest = &alloc[(i * t->wheel * 2)];
			void *src = &t->list[(i * t->wheel)];
			size_t copy = sizeof(struct sc_timer_data) * t->wheel;

			memcpy(dest, src, copy);
		}
	}

	sc_timer_free(t->list);

	t->list = alloc;
	t->wheel = wheel;

	return true;
}

uint64_t sc_timer_add(struct sc_timer *t, uint64_t timeout, uint64_t type,
		      void *data)
{
	const uint32_t offset = (uint32_t) (timeout / TICK + t->head);
	const uint32_t pos = offset & (WHEEL_COUNT - 1);
	uint64_t id;
	uint32_t seq, index, wheel_pos;

	wheel_pos = (pos * t->wheel);
	for (seq = 0; seq < t->wheel; seq++) {
		index = wheel_pos + seq;
		if (t->list[index].timeout == SC_TIMER_INVALID) {
			goto out;
		}
	}

	if (!expand(t)) {
		return SC_TIMER_INVALID;
	}

	index = (pos * t->wheel) + (seq);
	assert(t->list[index].timeout == SC_TIMER_INVALID);

out:
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

	pos = (((uint32_t) *id) * t->wheel) + (*id >> 32u);

	t->list[pos].timeout = SC_TIMER_INVALID;
	*id = SC_TIMER_INVALID;
}

#define sc_timer_min(a, b) (a) < (b) ? (a) : (b)

uint64_t sc_timer_timeout(struct sc_timer *t, uint64_t timestamp, void *arg,
			  void (*callback)(void *, uint64_t, uint64_t, void *))
{
	const uint64_t time = timestamp - t->timestamp;
	uint32_t wheel, base, timeout;
	uint32_t head = t->head;
	uint32_t wheels = (uint32_t) (sc_timer_min(time / TICK, WHEEL_COUNT));
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
				timeout = item->timeout;
				item->timeout = SC_TIMER_INVALID;

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
