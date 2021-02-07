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

#define TICK        16u
#define WHEEL_COUNT 16u

#ifndef SC_SIZE_MAX
    #define SC_SIZE_MAX UINT32_MAX
#endif

#define SC_CAP_MAX (SC_SIZE_MAX / sizeof(struct sc_timer_data)) / WHEEL_COUNT

bool sc_timer_init(struct sc_timer *timer, uint64_t timestamp)
{
    const uint32_t wheel_cap = 4;
    const uint32_t cap = WHEEL_COUNT * wheel_cap;
    const size_t size = cap * sizeof(struct sc_timer_data);

    timer->count = 0;
    timer->head = 0;
    timer->wheel = wheel_cap;
    timer->timestamp = timestamp;

    timer->list = sc_timer_malloc(size);
    if (timer->list == NULL) {
        return false;
    }

    for (uint32_t i = 0; i < cap; i++) {
        timer->list[i].timeout = UINT64_MAX;
        timer->list[i].data = NULL;
    }

    return true;
}

void sc_timer_term(struct sc_timer *timer)
{
    sc_timer_free(timer->list);
}

void sc_timer_clear(struct sc_timer *timer)
{
    const uint32_t cap = timer->wheel * WHEEL_COUNT;

    timer->count = 0;
    timer->head = 0;

    for (uint32_t i = 0; i < cap; i++) {
        timer->list[i].timeout = UINT64_MAX;
        timer->list[i].data = NULL;
    }
}

static bool expand(struct sc_timer *timer)
{
    uint32_t cap = timer->wheel * WHEEL_COUNT * 2;
    size_t size = cap * sizeof(struct sc_timer_data);
    struct sc_timer_data *alloc;

    // Check overflow
    if (timer->wheel > SC_CAP_MAX / 2) {
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
        void *dest = &alloc[(i * timer->wheel * 2)];
        void *src = &timer->list[(i * timer->wheel)];
        size_t copy = sizeof(struct sc_timer_data) * timer->wheel;

        memcpy(dest, src, copy);
    }

    sc_timer_free(timer->list);

    timer->list = alloc;
    timer->wheel *= 2;

    return true;
}

uint64_t sc_timer_add(struct sc_timer *timer, uint64_t timeout, uint64_t type,
                      void *data)
{
    const uint32_t offset = (uint32_t)(timeout / TICK + timer->head);
    const uint32_t pos = offset & (WHEEL_COUNT - 1);
    uint64_t id;
    uint32_t seq, index, wheel_pos;

    assert(timeout < UINT64_MAX);

    timer->count++;

    wheel_pos = (pos * timer->wheel);
    for (seq = 0; seq < timer->wheel; seq++) {
        index = wheel_pos + seq;
        if (timer->list[index].timeout == UINT64_MAX) {
            goto out;
        }
    }

    if (!expand(timer)) {
        return SC_TIMER_INVALID;
    }

    index = (pos * timer->wheel) + (seq);
    assert(timer->list[index].timeout == UINT64_MAX);

out:
    timer->list[index].timeout = timeout + timer->timestamp;
    timer->list[index].type = type;
    timer->list[index].data = data;

    id = (((uint64_t) seq) << 32u) | pos;
    assert(id != SC_TIMER_INVALID);

    return id;
}

void sc_timer_cancel(struct sc_timer *timer, uint64_t *id)
{
    uint64_t pos;

    if (*id == SC_TIMER_INVALID) {
        return;
    }

    timer->count--;
    pos = (((uint32_t) *id) * timer->wheel) + (*id >> 32u);

    assert(timer->list[pos].timeout != UINT64_MAX);
    timer->list[pos].timeout = UINT64_MAX;
    *id = SC_TIMER_INVALID;
}

#define sc_timer_min(a, b) (a) < (b) ? (a) : (b)

uint64_t sc_timer_timeout(struct sc_timer *timer, uint64_t timestamp, void *arg,
                          void (*callback)(void *, uint64_t, uint64_t, void *))
{
    const uint64_t time = timestamp - timer->timestamp;
    uint32_t wheel, base;
    uint32_t head = timer->head;
    uint32_t wheels = (uint32_t) (sc_timer_min(time / TICK, WHEEL_COUNT));

    if (wheels == 0) {
        return sc_timer_min(TICK - time, TICK);
    }

    timer->timestamp = timestamp;
    timer->head = (timer->head + wheels) & (WHEEL_COUNT - 1);

    while (wheels-- > 0) {
        wheel = timer->wheel;
        base = wheel * head;

        for (uint32_t i = 0; i < wheel; i++) {
            struct sc_timer_data *item = &timer->list[base + i];

            if (item->timeout <= timer->timestamp) {
                uint64_t timeout = item->timeout;
                item->timeout = UINT64_MAX;

                timer->count--;
                callback(arg, timeout, item->type, item->data);

                // Recalculates position each time because there might be newly
                // added timers in the callback and it might require expansion
                // of the list.
                base = timer->wheel * head;
            }
        }

        head = (head + 1) & (WHEEL_COUNT - 1);
    }

    return sc_timer_min(TICK - time, TICK);
}
