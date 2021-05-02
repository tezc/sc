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

	return ts.tv_sec * 1000 + (uint64_t) (ts.tv_nsec / 10e6);
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
	return (int64_t) (count.QuadPart * 1000) / frequency;
#else
	int rc;
	struct timespec ts;

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	assert(rc == 0);
	(void) rc;

	return (uint64_t) ((uint64_t) ts.tv_sec * 1000 +
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
	return (uint64_t) (count.QuadPart * 1000000000) / frequency;
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
