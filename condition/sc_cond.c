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
