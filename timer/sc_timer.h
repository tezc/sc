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

#ifndef SC_TIMER_H
#define SC_TIMER_H

#define SC_TIMER_VERSION "2.0.0"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_timer_malloc malloc
#define sc_timer_free free
#endif

#define SC_TIMER_INVALID UINT64_MAX

struct sc_timer_data {
	uint64_t timeout;
	uint64_t type;
	void *data;
};

struct sc_timer {
	uint64_t timestamp;
	uint32_t head;
	uint32_t wheel;
	struct sc_timer_data *list;
};

/**
 * Init timer
 *
 * @param t         timer
 * @param timestamp current timestamp. Use monotonic timer source.
 */
void sc_timer_init(struct sc_timer *t, uint64_t timestamp);

/**
 * Destroy timer.
 * @param t timer
 */
void sc_timer_term(struct sc_timer *t);

/**
 * Remove all timers without deallocating underlying memory.
 * @param t timer
 */
void sc_timer_clear(struct sc_timer *t);

/**
 * Add timer
 * 'timeout' is relative to latest 'timestamp' value given to the 'timer'.
 *
 * e.g sc_timer_init(&timer, 1000); // Current timestamp is 1000.
 *     sc_timer_add(&timer, arg, 10); // Timeout will be at 1010.
 *     sc_timer_timeout(&timer, 2000, arg, callback); // Timestamp is now 2000.
 *     sc_timer_add(&timer, arg, 10); // Timeout will be at 2010.
 *
 *
 * @param t       timer
 * @param timeout timeout value, this is relative to 'sc_timer_init's timer.
 *                e.g sc_timer_init(&timer, 10); // say, start time is 10
 * milliseconds
 * @param data    user data to pass into callback on 'sc_timer_timeout' call.
 * @param type    user data to pass into callback on 'sc_timer_timeout' call.
 * @return        SC_TIMER_INVALID on out of memory. Otherwise, timer id. You
 *                can cancel this timer via this id later.
 */
uint64_t sc_timer_add(struct sc_timer *t, uint64_t timeout, uint64_t type,
		      void *data);

/**
 * uint64_t id = sc_timer_add(&timer, arg, 10);
 * sc_timer_cancel(&timer, &id);
 *
 * @param t  timer
 * @param id timer id
 */
void sc_timer_cancel(struct sc_timer *t, uint64_t *id);

/**
 * Checks timeouts and calls 'callback' function for each timeout.
 *
 * Logical pattern is :
 *
 * e.g:
 * struct sc_timer timer;
 * sc_timer_init(&timer, time_ms());
 * sc_timer_add(&timer, data, 100);
 *
 * while (true) {
 *      uint64_t timeout = sc_timer_timeout(&timer, time_ms(), arg, callback);
 *      sleep(timeout); // or select(timeout), epoll_wait(timeout) etc..
 * }
 *
 *
 * @param t         timer
 * @param timestamp current timestamp
 * @param arg       user data to user callback
 * @param callback  'arg' is user data.
 *                  'timeout' is scheduled timeout for that timer.
 *                  'type' is what user passed on 'sc_timer_add'.
 *                  'data' is what user passed on 'sc_timer_add'.
 * @return          next timeout.
 */
uint64_t sc_timer_timeout(struct sc_timer *t, uint64_t timestamp, void *arg,
			  void (*callback)(void *arg, uint64_t timeout,
					   uint64_t type, void *data));
#endif
