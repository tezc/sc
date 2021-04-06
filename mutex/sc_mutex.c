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
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_mutex.h"

#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)

int sc_mutex_init(struct sc_mutex *mtx)
{
	InitializeCriticalSection(&mtx->mtx);
	return 0;
}

int sc_mutex_term(struct sc_mutex *mtx)
{
	DeleteCriticalSection(&mtx->mtx);
	return 0;
}

void sc_mutex_lock(struct sc_mutex *mtx)
{
	EnterCriticalSection(&mtx->mtx);
}

void sc_mutex_unlock(struct sc_mutex *mtx)
{
	LeaveCriticalSection(&mtx->mtx);
}

#else

int sc_mutex_init(struct sc_mutex *mtx)
{
	int rc, rv;
	pthread_mutexattr_t attr;
	pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

	mtx->mtx = mut;

	// May fail on OOM
	rc = pthread_mutexattr_init(&attr);
	if (rc != 0) {
		return -1;
	}

	// This won't fail as long as we pass correct params.
	rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	assert(rc == 0);

	// May fail on OOM
	rc = pthread_mutex_init(&mtx->mtx, &attr);

	// This won't fail as long as we pass correct param.
	rv = pthread_mutexattr_destroy(&attr);
	assert(rv == 0);
	(void) rv;

	return rc != 0 ? -1 : 0;
}

int sc_mutex_term(struct sc_mutex *mtx)
{
	int rc;

	rc = pthread_mutex_destroy(&mtx->mtx);
	return rc != 0 ? -1 : 0;
}

void sc_mutex_lock(struct sc_mutex *mtx)
{
	int rc;

	// This won't fail as long as we pass correct param.
	rc = pthread_mutex_lock(&mtx->mtx);
	assert(rc == 0);
	(void) rc;
}

void sc_mutex_unlock(struct sc_mutex *mtx)
{
	int rc;

	// This won't fail as long as we pass correct param.
	rc = pthread_mutex_unlock(&mtx->mtx);
	assert(rc == 0);
	(void) rc;
}

#endif
