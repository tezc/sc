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
