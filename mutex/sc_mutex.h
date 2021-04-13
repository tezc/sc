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

#ifndef SC_MUTEX_H
#define SC_MUTEX_H

#define SC_MUTEX_VERSION "2.0.0"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <pthread.h>
#endif

struct sc_mutex {
#if defined(_WIN32) || defined(_WIN64)
	CRITICAL_SECTION mtx;
#else
	pthread_mutex_t mtx;
#endif
};

/**
 * Create mutex.
 *
 * Be warned on Windows, mutexes are recursive, on Posix default
 * mutex type is not recursive. Edit code if that bothers you. Pass
 * PTHREAD_MUTEX_RECURSIVE instead of PTHREAD_MUTEX_NORMAL.
 *
 * @param mtx mtx
 * @return    '0' on success, '-1' on error.
 */
int sc_mutex_init(struct sc_mutex *mtx);

/**
 * Destroy mutex
 *
 * @param mtx mtx
 * @return    '0' on success, '-1' on error.
 */
int sc_mutex_term(struct sc_mutex *mtx);

/**
 * @param mtx mtx
 */
void sc_mutex_lock(struct sc_mutex *mtx);

/**
 * @param mtx mtx
 */
void sc_mutex_unlock(struct sc_mutex *mtx);

#endif
