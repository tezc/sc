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
