#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_timer.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

uint64_t time_ms()
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

	return (int64_t) ((int64_t) ts.tv_sec * 1000 +
			  (int64_t) ts.tv_nsec / 1000000);
#endif
}

void sleep_ms(uint64_t milliseconds)
{
#if defined(_WIN32) || defined(_WIN64)
	Sleep(milliseconds);
#else
	int rc;
	struct timespec t;

	t.tv_sec = milliseconds / 1000;
	t.tv_nsec = (milliseconds % 1000) * 1000000;

	do {
		rc = nanosleep(&t, NULL);
	} while (rc != 0 && errno != EINTR);
#endif
}

int count;
uint64_t ids[1000];

void callback(void *arg, uint64_t timeout, uint64_t type, void *data)
{
	(void) timeout;
	(void) type;

	static int idx = 0;
	count--;
	uint64_t id = (uintptr_t) data;
	assert(ids[id] != SC_TIMER_INVALID);
	ids[id] = SC_TIMER_INVALID;
	assert((int) (uintptr_t) arg == 333);

	idx++;
}

void test1(void)
{
	struct sc_timer timer;

	sc_timer_init(&timer, time_ms());

	for (int i = 0; i < 1000; i++) {
		ids[i] = sc_timer_add(&timer, rand() % 100, i,
				      (void *) (uintptr_t) i);
		count++;
		assert(ids[i] != SC_TIMER_INVALID);
	}

	int t = 10000;
	uint32_t n;
	while (t > 0) {
		n = sc_timer_timeout(&timer, time_ms(),
				     (void *) (uintptr_t) 333, callback);
		if (count == 0) {
			break;
		}
		t -= n;
		sleep_ms(n);
	}

	for (int i = 0; i < 1000; i++) {
		assert(ids[i] == SC_TIMER_INVALID);
	}

	sc_timer_term(&timer);

	for (int i = 0; i < 1000; i++) {
		ids[i] = sc_timer_add(&timer, rand() % 100, i,
				      (void *) (uintptr_t) i);
		count++;
		assert(ids[i] != SC_TIMER_INVALID);
	}

	t = 10000;
	while (t > 0) {
		n = sc_timer_timeout(&timer, time_ms(),
				     (void *) (uintptr_t) 333, callback);
		if (count == 0) {
			break;
		}
		t -= n;
		sleep_ms(n);
	}

	for (int i = 0; i < 1000; i++) {
		assert(ids[i] == SC_TIMER_INVALID);
	}

	sc_timer_term(&timer);
}

void test2(void)
{
	struct sc_timer timer;

	sc_timer_init(&timer, time_ms());

	for (int i = 0; i < 1000; i++) {
		ids[i] = SC_TIMER_INVALID;
		sc_timer_add(&timer, rand() % 100, i, (void *) (uintptr_t) i);
	}

	sc_timer_clear(&timer);

	int t = 10000;
	uint32_t n;
	while (t > 0) {
		n = sc_timer_timeout(&timer, time_ms(),
				     (void *) (uintptr_t) 333, callback);
		if (count == 0) {
			break;
		}

		t -= n;
		sleep_ms(n);
	}

	for (int i = 0; i < 1000; i++) {
		assert(ids[i] == SC_TIMER_INVALID);
	}

	for (int i = 0; i < 1000; i++) {
		ids[i] = sc_timer_add(&timer, rand() % 100, i,
				      (void *) (uintptr_t) i);
		assert(ids[i] != SC_TIMER_INVALID);
		count++;
	}

	t = 10000;
	while (t > 0) {
		n = sc_timer_timeout(&timer, time_ms(),
				     (void *) (uintptr_t) 333, callback);
		if (count == 0) {
			break;
		}
		t -= n;
		sleep_ms(n);
	}

	for (int i = 0; i < 1000; i++) {
		assert(ids[i] == SC_TIMER_INVALID);
	}

	sc_timer_term(&timer);
}

void test3(void)
{
	struct sc_timer timer;

	sc_timer_init(&timer, time_ms());

	for (int i = 0; i < 1000; i++) {
		ids[i] = sc_timer_add(&timer, rand() % 20, i,
				      (void *) (uintptr_t) i);
		assert(ids[i] != SC_TIMER_INVALID);
		count++;
	}

	for (int i = 0; i < 1000; i += 2) {
		sc_timer_cancel(&timer, &ids[i]);
		count--;
	}

	assert(count == 500);
	sc_timer_cancel(&timer, &ids[0]);
	assert(count == 500);

	int t = 10000;
	uint32_t n;
	while (t > 0) {
		n = sc_timer_timeout(&timer, time_ms(),
				     (void *) (uintptr_t) 333, callback);
		if (count == 0) {
			break;
		}
		t -= n;
		sleep_ms(n);
	}

	for (int i = 0; i < 500; i++) {
		assert(ids[i] == SC_TIMER_INVALID);
	}

	sc_timer_term(&timer);
}

void test4(void)
{
	struct sc_timer timer;

	sc_timer_init(&timer, 0);

	for (int i = 0; i < 1000; i++) {
		ids[i] = sc_timer_add(&timer, rand() % 20, i,
				      (void *) (uintptr_t) i);
		assert(ids[i] != SC_TIMER_INVALID);
		count++;
	}

	for (int i = 0; i < 1000; i += 2) {
		sc_timer_cancel(&timer, &ids[i]);
		count--;
	}

	assert(count == 500);
	sc_timer_cancel(&timer, &ids[0]);
	assert(count == 500);

	int t = 10000;
	uint32_t n;
	int x = 0;
	while (t > 0) {
		x += 500;
		n = sc_timer_timeout(&timer, x, (void *) (uintptr_t) 333,
				     callback);
		if (count == 0) {
			break;
		}
		t -= n;
		sleep_ms(n);
	}

	for (int i = 0; i < 500; i++) {
		assert(ids[i] == SC_TIMER_INVALID);
	}

	sc_timer_term(&timer);
}

#ifdef SC_HAVE_WRAP

bool fail_malloc = false;
void *__real_malloc(size_t n);
void *__wrap_malloc(size_t n)
{
	if (fail_malloc) {
		return NULL;
	}

	return __real_malloc(n);
}

void fail_test(void)
{
	size_t max = 50000;
	struct sc_timer timer;

	sc_timer_init(&timer, time_ms());

	uint64_t id;
	for (size_t i = 0; i < max + 100; i++) {
		id = sc_timer_add(&timer, i, i, 0);
		if (id == SC_TIMER_INVALID) {
			break;
		}
	}

	assert(id == SC_TIMER_INVALID);

	sc_timer_term(&timer);

	sc_timer_init(&timer, time_ms());
	fail_malloc = true;

	for (size_t i = 0; i < 65; i++) {
		id = sc_timer_add(&timer, i, i, 0);
		if (id == SC_TIMER_INVALID) {
			break;
		}
	}

	assert(id == SC_TIMER_INVALID);
	fail_malloc = false;

	sc_timer_term(&timer);
}
#else
void fail_test(void)
{
}
#endif

int main()
{
	fail_test();
	test1();
	test2();
	test3();
	test4();

	return 0;
}
