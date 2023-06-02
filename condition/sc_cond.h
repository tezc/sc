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

#ifndef SC_COND_H
#define SC_COND_H

#include <stdbool.h>

#define SC_COND_VERSION "2.0.0"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

struct sc_cond {
	bool init;
	bool done;
	void *data;

#if defined(_WIN32) || defined(_WIN64)
	CONDITION_VARIABLE cond;
	CRITICAL_SECTION mtx;
#else
	pthread_cond_t cond;
	pthread_mutex_t mtx;
#endif
};

/**
 * Init cond object
 *
 * @param c cond
 * @return     '0' on success, negative on error, errno will be set.
 */
int sc_cond_init(struct sc_cond *c);

/**
 * Destroy cond object
 *
 * @param c cond
 * @return     '0' on success, negative on error, errno will be set.
 */
int sc_cond_term(struct sc_cond *c);

/**
 * @param c cond
 * @param data data to pass to thread which will call 'sc_cond_wait'.
 */
void sc_cond_signal(struct sc_cond *c, void *data);

/**
 * @param c cond
 * @return     'user data'.'data' argument on previous sc_cond_signal() call
 */
void *sc_cond_wait(struct sc_cond *c);

#endif
