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

#include "sc_thread.h"

#include <string.h>

void sc_thread_init(struct sc_thread *t)
{
	t->id = 0;
}

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)

#include <process.h>

static void sc_thread_errstr(struct sc_thread *t)
{
	int rc;
	DWORD err = GetLastError();
	LPSTR errstr = 0;

	rc = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				    FORMAT_MESSAGE_FROM_SYSTEM,
			    NULL, err, 0, (LPSTR) &errstr, 0, NULL);
	if (rc != 0) {
		strncpy(t->err, errstr, sizeof(t->err) - 1);
		LocalFree(errstr);
	}
}

unsigned int __stdcall sc_thread_fn(void *arg)
{
	struct sc_thread *t = arg;

	t->ret = t->fn(t->arg);
	return 0;
}

int sc_thread_start(struct sc_thread *t, void *(*fn)(void *), void *arg)
{
	int rc = 0;

	t->fn = fn;
	t->arg = arg;

	t->id = (HANDLE) _beginthreadex(NULL, 0, sc_thread_fn, t, 0, NULL);
	if (t->id == 0) {
		sc_thread_errstr(t);
		rc = -1;
	}

	return rc;
}

int sc_thread_join(struct sc_thread *t, void **ret)
{
	int rc = 0;
	void *val = NULL;
	DWORD rv;
	BOOL brc;

	if (t->id == 0) {
		goto out;
	}

	rv = WaitForSingleObject(t->id, INFINITE);
	if (rv == WAIT_FAILED) {
		sc_thread_errstr(t);
		rc = -1;
	}

	brc = CloseHandle(t->id);
	if (!brc) {
		sc_thread_errstr(t);
		rc = -1;
	}

	val = t->ret;
	t->id = 0;

out:
	if (ret != NULL) {
		*ret = t->ret;
	}

	return rc;
}
#else

int sc_thread_start(struct sc_thread *t, void *(*fn)(void *), void *arg)
{
	int rc;
	pthread_attr_t attr;

	rc = pthread_attr_init(&attr);
	if (rc != 0) {
		strncpy(t->err, strerror(rc), sizeof(t->err) - 1);
		return -1;
	}

	// This may only fail with EINVAL.
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	rc = pthread_create(&t->id, &attr, fn, arg);
	if (rc != 0) {
		strncpy(t->err, strerror(rc), sizeof(t->err) - 1);
	}

	// This may only fail with EINVAL.
	pthread_attr_destroy(&attr);

	return rc;
}

int sc_thread_join(struct sc_thread *t, void **ret)
{
	int rc = 0;
	void *val = NULL;

	if (t->id == 0) {
		goto out;
	}

	rc = pthread_join(t->id, &val);
	if (rc != 0) {
		strncpy(t->err, strerror(rc), sizeof(t->err) - 1);
	}

	t->id = 0;

out:
	if (ret != NULL) {
		*ret = val;
	}

	return rc;
}

#endif

int sc_thread_term(struct sc_thread *t)
{
	return sc_thread_join(t, NULL);
}

const char *sc_thread_err(struct sc_thread *t)
{
	return t->err;
}
