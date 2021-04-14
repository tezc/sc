#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_timer.h"

#include <errno.h>
#include <stdio.h>
#include <time.h>

uint64_t time_ms();
void sleep_ms(uint64_t milliseconds);

void callback(void *arg, uint64_t timeout, uint64_t type, void *data)
{
	(void) type;

	struct sc_timer *timer = arg;
	char *timer_name = data;

	printf("timeout : %lu, data : %s \n", (unsigned long) timeout,
	       timer_name);
	// Schedule back
	sc_timer_add(timer, 1000, 1, "timer1");
}

int main()
{
	uint64_t next_timeout;
	struct sc_timer timer;

	sc_timer_init(&timer, time_ms());
	sc_timer_add(&timer, 1000, 1, "timer1");

	while (true) {
		next_timeout =
			sc_timer_timeout(&timer, time_ms(), &timer, callback);
		sleep_ms(next_timeout);
	}

	return 0;
}

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/time.h>
#endif

uint64_t time_ms()
{
#if defined(_WIN32) || defined(_WIN64)
	//  System frequency does not change at run-time, cache it
	static int64_t frequency = 0;
	if (frequency == 0) {
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		frequency = freq.QuadPart;
	}
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return (int64_t) (count.QuadPart * 1000) / frequency;
#else
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

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
