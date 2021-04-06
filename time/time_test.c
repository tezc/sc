#define _GNU_SOURCE
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_time.h"

#include <assert.h>

void test1(void)
{
	assert(sc_time_ns() != 0);
	assert(sc_time_ms() != 0);

	for (int i = 0; i < 100000; i++) {
		uint64_t t1 = sc_time_mono_ms();
		uint64_t t2 = sc_time_mono_ms();

		assert(t2 >= t1);
	}

	for (int i = 0; i < 100000; i++) {
		uint64_t t1 = sc_time_mono_ns();
		uint64_t t2 = sc_time_mono_ns();

		assert(t2 >= t1);
	}
}

void test2(void)
{
	uint64_t t1, t2;

	t1 = sc_time_mono_ms();
	sc_time_sleep(1000);
	t2 = sc_time_mono_ms();

	assert(t2 > t1);

	t1 = sc_time_mono_ns();
	sc_time_sleep(1000);
	t2 = sc_time_mono_ns();

	assert(t2 > t1);
}

#ifdef SC_HAVE_WRAP

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

void sig_handler(int signum)
{
	printf("signal : %d", signum);
}

void test3(void)
{
	uint64_t t1, t2;

	signal(SIGALRM, sig_handler);
	printf("%d \n", alarm(2));

	t1 = sc_time_mono_ms();
	sc_time_sleep(4000);
	t2 = sc_time_mono_ms();

	assert(t2 > t1);

	printf("%d \n", alarm(2));
	t1 = sc_time_mono_ns();
	sc_time_sleep(4000);
	t2 = sc_time_mono_ns();

	assert(t2 > t1);
}

bool fail_nanosleep = false;
int __real_nanosleep(const struct timespec *__requested_time,
		     struct timespec *__remaining);
int __wrap_nanosleep(const struct timespec *__requested_time,
		     struct timespec *__remaining)
{
	if (fail_nanosleep) {
		errno = ERANGE;
		return -1;
	}

	return __real_nanosleep(__requested_time, __remaining);
}

void test4(void)
{
	fail_nanosleep = true;
	assert(sc_time_sleep(100) != 0);
	fail_nanosleep = false;
}

#else
void test3(void)
{
}

void test4(void)
{
}
#endif

int main()
{
	test1();
	test2();
	test3();
	test4();

	return 0;
}
