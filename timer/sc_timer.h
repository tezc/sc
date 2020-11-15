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

#ifndef SC_TIMER_H
#define SC_TIMER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define SC_TIMER_INVALID UINT64_MAX

struct sc_timer_data
{
    uint64_t timeout;
    void *data;
};

struct sc_timer
{
    uint64_t timestamp;
    uint32_t head;
    uint32_t wheel;
    uint32_t count;
    struct sc_timer_data *list;
};

#define sc_timer_malloc malloc
#define sc_timer_free   free

/**
 * @param timer     Timer
 * @param timestamp Current timestamp. Use monotonic timer source.
 * @return          'false' on out of memory.
 */
bool sc_timer_init(struct sc_timer *timer, uint64_t timestamp);

/**
 * Destroy timer.
 * @param timer Timer
 */
void sc_timer_term(struct sc_timer *timer);

/**
 * Remove all timers without deallocating underlying memory.
 * @param timer
 */
void sc_timer_clear(struct sc_timer *timer);

/**
 * Add timer
 * 'timeout' is relative to latest 'timestamp' value given to 'timer' object.
 *
 * e.g sc_timer_init(&timer, 1000); // Current timestamp is 1000.
 *     sc_timer_add(&timer, arg, 10); // Timeout will be at 1010.
 *     sc_timer_timeout(&timer, 2000, arg, callback); // Timestamp is now 2000.
 *     sc_timer_add(&timer, arg, 10); // Timeout will be at 2010.
 *
 *
 * @param timer   Timer
 * @param data    User data to pass into callback on 'sc_timer_timeout' call.
 * @param timeout Timeout value, this is relative to 'sc_timer_init's timer.
 *                e.g sc_timer_init(&timer, 10); // say, start time 10 milliseconds
 * @return        SC_TIMER_INVALID on out of memory. Otherwise, timer id. You
 *                can cancel this timer via this id.
 */
uint64_t sc_timer_add(struct sc_timer *timer, void *data, uint64_t timeout);

/**
 * uint64_t id = sc_timer_add(&timer, arg, 10);
 * sc_timer_cancel(&timer, &id);
 *
 * @param timer Timer
 * @param id    Timer id
 */
void sc_timer_cancel(struct sc_timer *timer, uint64_t *id);

/**
 * Logical pattern is :
 *
 * e.g:
 * struct sc_timer timer;
 * sc_timer_init(&timer, time_ms());
 * sc_timer_add(&timer, data, 100);
 *
 * while (true) {
 *      uint64_t timeout = sc_timer_timeout(&timer, time_ms(), arg, callback);
 *      sleep(timeout); // select(timeout), epoll_wait(timeout) etc..
 * }
 *
 *
 * @param timer     Timer
 * @param timestamp Current timestamp
 * @param arg       User data to user callback
 * @param callback  'arg' is user data.
 *                  'timeout' is scheduled timeout for that timer.
 *                  'data' is what user passed on 'sc_timer_add'.
 * @return          next timeout.
 */
uint64_t sc_timer_timeout(struct sc_timer *timer, uint64_t timestamp, void *arg,
                          void (*callback)(void *arg, uint64_t timeout,
                                           void *data));
#endif
