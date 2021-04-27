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

#include "sc_cond.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)

int sc_cond_init(struct sc_cond *c)
{
	*c = (struct sc_cond){.init = true};

	InitializeCriticalSection(&c->mtx);
	InitializeConditionVariable(&c->cond);

	return 0;
}

int sc_cond_term(struct sc_cond *c)
{
	if (!c->init) {
		return 0;
	}

	DeleteCriticalSection(&c->mtx);
	c->init = false;

	return 0;
}

void sc_cond_signal(struct sc_cond *c, void *var)
{
	EnterCriticalSection(&c->mtx);

	c->data = var;
	c->done = true;

	WakeConditionVariable(&c->cond);
	LeaveCriticalSection(&c->mtx);
}

void *sc_cond_wait(struct sc_cond *c)
{
	BOOL rc;
	void *data;

	EnterCriticalSection(&c->mtx);

	while (c->done == false) {
		// This should not fail as we pass INFINITE.
		rc = SleepConditionVariableCS(&c->cond, &c->mtx, INFINITE);
		assert(rc != 0);
	}

	data = c->data;
	c->data = NULL;
	c->done = false;

	LeaveCriticalSection(&c->mtx);

	return data;
}

#else

int sc_cond_init(struct sc_cond *c)
{
	int rc;

	*c = (struct sc_cond){0};

	pthread_mutexattr_t attr;
	pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

	c->mtx = mut;

	rc = pthread_mutexattr_init(&attr);
	if (rc != 0) {
		goto error;
	}

	// This won't fail as long as we pass correct params.
	rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	assert(rc == 0);

	rc = pthread_mutex_init(&c->mtx, &attr);
	if (rc != 0) {
		goto cleanup_attr;
	}

	rc = pthread_cond_init(&c->cond, NULL);
	if (rc != 0) {
		goto cleanup_mutex;
	}

	pthread_mutexattr_destroy(&attr);
	c->init = true;

	return 0;

cleanup_mutex:
	pthread_mutex_destroy(&c->mtx);
cleanup_attr:
	pthread_mutexattr_destroy(&attr);
error:
	errno = rc;
	return -1;
}

int sc_cond_term(struct sc_cond *c)
{
	int rc;

	errno = 0;

	if (!c->init) {
		return 0;
	}

	rc = pthread_mutex_destroy(&c->mtx);
	if (rc != 0) {
		errno = rc;
	}

	rc = pthread_cond_destroy(&c->cond);
	if (rc != 0 && errno == 0) {
		errno = rc;
	}

	c->init = false;

	return errno;
}

void sc_cond_signal(struct sc_cond *c, void *data)
{
	int rc;

	pthread_mutex_lock(&c->mtx);

	c->data = data;
	c->done = true;

	// This won't fail as long as we pass correct params.
	rc = pthread_cond_signal(&c->cond);
	assert(rc == 0);
	(void) rc;

	pthread_mutex_unlock(&c->mtx);
}

void *sc_cond_wait(struct sc_cond *c)
{
	int rc;
	void *data;

	pthread_mutex_lock(&c->mtx);

	while (c->done == false) {
		// This won't fail as long as we pass correct params.
		rc = pthread_cond_wait(&c->cond, &c->mtx);
		assert(rc == 0);
		(void) rc;
	}

	data = c->data;
	c->data = NULL;
	c->done = false;

	pthread_mutex_unlock(&c->mtx);

	return data;
}

#endif
