#include "sc_signal.h"

#include <assert.h>
#include <string.h>

void test1()
{
	char tmp[128] = "";

	sc_signal_snprintf(tmp, 0, "%s", "test");
	assert(strcmp(tmp, "") == 0);

	sc_signal_snprintf(tmp, sizeof(tmp), "%s", "test");
	assert(strcmp(tmp, "test") == 0);
	sc_signal_snprintf(tmp, sizeof(tmp), "%s", NULL);
	assert(strcmp(tmp, "(null)") == 0);
	sc_signal_snprintf(tmp, sizeof(tmp), "%d", -3);
	assert(strcmp(tmp, "-3") == 0);
	sc_signal_snprintf(tmp, sizeof(tmp), "%u", 3);
	assert(strcmp(tmp, "3") == 0);
	sc_signal_snprintf(tmp, sizeof(tmp), "%ld", -1000000000l);
	assert(strcmp(tmp, "-1000000000") == 0);
	sc_signal_snprintf(tmp, sizeof(tmp), "%lld", -100000000000ll);
	assert(strcmp(tmp, "-100000000000") == 0);
	sc_signal_snprintf(tmp, sizeof(tmp), "%lu", 1000000000l);
	assert(strcmp(tmp, "1000000000") == 0);
	sc_signal_snprintf(tmp, sizeof(tmp), "%llu", 100000000000ll);
	assert(strcmp(tmp, "100000000000") == 0);

	char *x = (char *) 0xabcdef;
	sc_signal_snprintf(tmp, sizeof(tmp), "%p", x);
	assert(strcmp(tmp, "0xabcdef") == 0);

	sc_signal_snprintf(tmp, sizeof(tmp), "%%p", x);
	assert(strcmp(tmp, "%p") == 0);

	assert(sc_signal_snprintf(tmp, sizeof(tmp), "%c", 3) == -1);
	assert(sc_signal_snprintf(tmp, sizeof(tmp), "%llx", 3) == -1);
	assert(sc_signal_snprintf(tmp, sizeof(tmp), "%lx", 3) == -1);

	sc_signal_log(1, tmp, sizeof(tmp), "%s", "test");
}

void test2()
{
	assert(sc_signal_init() == 0);
}

#ifdef SC_HAVE_WRAP
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

bool fail_signal;
extern void (*__real_signal(int sig, void (*func)(int)))(int);
void (*__wrap_signal(int sig, void (*func)(int)))(int)
{
	if (fail_signal) {
		return SIG_ERR;
	}

	return __real_signal(sig, func);
}

void sig_handler(int signum)
{
	(void) signum;
}

void test3x(int signal)
{
	pid_t pid = fork();
	if (pid == -1) {
		assert(true);
	} else if (pid) {
		int status = 0;
		wait(&status);
		if (WEXITSTATUS(status) == -2) {
			return;
		} else {
			assert(true);
		}
	} else {
		printf("Running child \n");
		raise(signal);
		printf("Child done \n");
		exit(-2);
	}
}

void test3()
{
	test3x(SIGSEGV);
	test3x(SIGABRT);
	test3x(SIGBUS);
	test3x(SIGFPE);
	sc_signal_log_fd = STDOUT_FILENO;
	test3x(SIGILL);
	sc_signal_log_fd = -1;
	test3x(SIGUSR1);
	test3x(SIGSYS);
}

void test4x(int signal)
{
	pid_t pid = fork();
	if (pid == -1) {
		assert(true);
	} else if (pid) {
		int status = 0;
		wait(&status);
		if (WEXITSTATUS(status) == -2) {
			return;
		} else {
			assert(true);
		}
	} else {
		assert(sc_signal_init() == 0);
		sc_signal_log_fd = STDOUT_FILENO;
		sc_signal_shutdown_fd = STDOUT_FILENO;
		printf("Running child \n");
		raise(signal);
		sleep(1);
		printf("Child done \n");
		exit(-2);
	}
}

void test4()
{
	test4x(SIGINT);
	test4x(SIGTERM);
	test4x(SIGUSR2);
}

void test5()
{
	pid_t pid = fork();
	if (pid == -1) {
		assert(true);
	} else if (pid) {
		int status = 0;
		wait(&status);
		if (WEXITSTATUS(status) == -2) {
			return;
		} else {
			assert(true);
		}
	} else {
		assert(sc_signal_init() == 0);
		sc_signal_shutdown_fd = STDOUT_FILENO;
		printf("Running child \n");
		raise(SIGINT);
		raise(SIGINT);
		sleep(1);
		printf("Child done \n");
		exit(-2);
	}
}

void test6()
{
	pid_t pid = fork();
	if (pid == -1) {
		assert(true);
	} else if (pid) {
		int status = 0;
		wait(&status);
		if (WEXITSTATUS(status) == -2) {
			return;
		} else {
			assert(true);
		}
	} else {
		assert(sc_signal_init() == 0);
		sc_signal_shutdown_fd = 1222;
		printf("Running child \n");
		raise(SIGINT);
		sleep(1);
		printf("Child done \n");
		exit(-2);
	}
}

void test7()
{
	pid_t pid = fork();
	if (pid == -1) {
		assert(true);
	} else if (pid) {
		int status = 0;
		wait(&status);
		if (WEXITSTATUS(status) == -2) {
			return;
		} else {
			assert(true);
		}
	} else {
		assert(sc_signal_init() == 0);
		printf("Running child \n");
		raise(SIGINT);
		sleep(1);
		printf("Child done \n");
		exit(-2);
	}
}

void test8()
{
	pid_t pid = fork();
	if (pid == -1) {
		assert(true);
	} else if (pid) {
		int status = 0;
		wait(&status);
		if (WEXITSTATUS(status) == -2) {
			return;
		} else {
			assert(true);
		}
	} else {
		assert(sc_signal_init() == 0);
		sc_signal_shutdown_fd = -1;
		printf("Running child \n");
		raise(SIGINT);
		sleep(1);
		printf("Child done \n");
		exit(-2);
	}
}

void test9()
{
	fail_signal = true;
	assert(sc_signal_init() != 0);
	fail_signal = false;
}

#else
void test3()
{
}
void test4()
{
}
void test5()
{
}
void test6()
{
}
void test7()
{
}
void test8()
{
}
void test9()
{
}
#endif

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();

	return 0;
}
