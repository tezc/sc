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

#include "sc_time.h"

#if defined(_WIN32) || defined(_WIN64)
#include <assert.h>
#include <windows.h>
#else
#include <assert.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#endif

uint64_t sc_time_ms()
{
#if defined(_WIN32) || defined(_WIN64)
	FILETIME ft;
	ULARGE_INTEGER dateTime;

	GetSystemTimeAsFileTime(&ft);
	dateTime.LowPart = ft.dwLowDateTime;
	dateTime.HighPart = ft.dwHighDateTime;

	return (dateTime.QuadPart / 10000);
#else
	int rc;
	struct timespec ts;

	rc = clock_gettime(CLOCK_REALTIME, &ts);
	assert(rc == 0);
	(void) rc;

	return ts.tv_sec * 1000 + (uint64_t)(ts.tv_nsec / 10e6);
#endif
}

uint64_t sc_time_ns()
{
#if defined(_WIN32) || defined(_WIN64)
	FILETIME ft;
	ULARGE_INTEGER dateTime;

	GetSystemTimeAsFileTime(&ft);
	dateTime.LowPart = ft.dwLowDateTime;
	dateTime.HighPart = ft.dwHighDateTime;

	return (dateTime.QuadPart * 100);
#else
	int rc;
	struct timespec ts;

	rc = clock_gettime(CLOCK_REALTIME, &ts);
	assert(rc == 0);
	(void) rc;

	return ts.tv_sec * (uint64_t) 1000000000 + ts.tv_nsec;
#endif
}

uint64_t sc_time_mono_ms()
{
#if defined(_WIN32) || defined(_WIN64)
	//  System frequency does not change at run-time, cache it
	static int64_t frequency = 0;
	if (frequency == 0) {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		assert(freq.QuadPart != 0);
		frequency = freq.QuadPart;
	}
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return (int64_t)(count.QuadPart * 1000) / frequency;
#else
	int rc;
	struct timespec ts;

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	assert(rc == 0);
	(void) rc;

	return (uint64_t)((uint64_t) ts.tv_sec * 1000 +
			  (uint64_t) ts.tv_nsec / 1000000);
#endif
}

uint64_t sc_time_mono_ns()
{
#if defined(_WIN32) || defined(_WIN64)
	static int64_t frequency = 0;
	if (frequency == 0) {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		assert(freq.QuadPart != 0);
		frequency = freq.QuadPart;
	}
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return (uint64_t)(count.QuadPart * 1000000000) / frequency;
#else
	int rc;
	struct timespec ts;

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	assert(rc == 0);
	(void) rc;

	return ((uint64_t) ts.tv_sec * 1000000000 + (uint64_t) ts.tv_nsec);
#endif
}

int sc_time_sleep(uint64_t millis)
{
#if defined(_WIN32) || defined(_WIN64)
	Sleep((DWORD) millis);
	return 0;
#else
	int rc;
	struct timespec t, rem;

	rem.tv_sec = millis / 1000;
	rem.tv_nsec = (millis % 1000) * 1000000;

	do {
		t = rem;
		rc = nanosleep(&t, &rem);
	} while (rc != 0 && errno == EINTR);

	return rc;
#endif
}
