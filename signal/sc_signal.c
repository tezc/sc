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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_signal.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SC_SIGNAL_TEST
#define sc_exit(n) return
#else
#define sc_exit(n) _Exit(n)
#endif

#if defined(_WIN32)
#include <WinSock2.h>
volatile SOCKET sc_signal_shutdown_fd;
#else
volatile sig_atomic_t sc_signal_shutdown_fd;
#endif

#if defined(__APPLE__) && defined(__arm64__)
#include <mach/mach.h>
#endif

/**
 * Set log file fd here, logging will be redirected to this fd,
 * otherwise STDERR_FILENO or STDOUT_FILENO will be used.
 */
volatile sig_atomic_t sc_signal_log_fd;

/**
 * Internal variable to handle twice shutdown signal.
 */
volatile sig_atomic_t sc_signal_will_shutdown;

#define get_uint(va, size)                                                     \
	(size) == 3 ? va_arg(va, unsigned long long) :                         \
	(size) == 2 ? va_arg(va, unsigned long) :                              \
			    va_arg(va, unsigned int)

#define get_int(va, size)                                                      \
	(size) == 3 ? va_arg(va, long long) :                                  \
	(size) == 2 ? va_arg(va, long) :                                       \
			    va_arg(va, int)

#define PSIZE sizeof(void *) == sizeof(unsigned long long) ? 3 : 2

int sc_signal_vsnprintf(char *buf, size_t sz, const char *fmt, va_list va)
{
	const char *arr = "0123456789abcdef";

	char c;
	int64_t i;
	uint64_t u;
	size_t len;
	size_t out_cap = sz == 0 ? 0 : sz - 1;
	char *str, *out = buf;
	char *pos = (char *) fmt;
	char dst[32];

	while (true) {
		char *orig = pos;

		switch (*pos) {
		case '\0': {
			*out = '\0';
			return (int) (out - buf);
		}
		case '%': {
			pos++;
			switch (*pos) {
			case 's':
				str = (char *) va_arg(va, const char *);
				str = (str == NULL) ? "(null)" : str;
				len = strlen(str);
				break;
			case 'l':
				pos += (*(pos + 1) == 'l') ? 2 : 1;
				// fall through
			case 'd':
			case 'u':
				len = 0;

				if (*pos == 'u') {
					u = get_uint(va, pos - orig);

					do {
						c = (char) ('0' + (u % 10));
						dst[31 - (len++)] = c;
					} while (u /= 10UL);

				} else if (*pos == 'd') {
					i = get_int(va, pos - orig);
					u = (uint64_t) (i < 0 ? -i : i);

					do {
						c = (char) ('0' + (u % 10));
						dst[31 - (len++)] = c;
					} while (u /= 10UL);

					if (i < 0) {
						dst[31 - (len++)] = '-';
					}
				} else {
					return -1;
				}

				str = &dst[32 - len];
				break;
			case 'p':
				len = 0;
				u = get_uint(va, PSIZE);

				do {
					dst[31 - (len++)] = arr[u % 16];
				} while (u /= 16UL);

				dst[31 - (len++)] = 'x';
				dst[31 - (len++)] = '0';
				str = &dst[32 - len];
				break;
			case '%':
				str = "%";
				len = 1;
				break;
			default:
				return -1;
			}
			pos++;
		} break;
		default: {
			while (*pos != '\0' && *pos != '%') {
				pos++;
			}
			str = orig;
			len = pos - orig;
			break;
		}
		}

		len = len < out_cap ? len : out_cap;
		memcpy(out, str, len);

		out += len;
		out_cap -= len;
	}
}

int sc_signal_snprintf(char *buf, size_t sz, const char *fmt, ...)
{
	int ret;
	va_list args;

	va_start(args, fmt);
	ret = sc_signal_vsnprintf(buf, sz, fmt, args);
	va_end(args);

	return ret;
}

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN

#include <Ws2tcpip.h>
#include <io.h>
#include <signal.h>
#include <windows.h>

#pragma warning(disable : 4996)
#pragma comment(lib, "Ws2_32.lib")

BOOL WINAPI sc_console_handler(DWORD type)
{
	int rc;
	char *err;
	char buf[128];
	int fd = sc_signal_log_fd != -1 ? sc_signal_log_fd : _fileno(stdout);

	switch (type) {
	case CTRL_C_EVENT:
		err = "CTRL_C event";
		break;
	case CTRL_BREAK_EVENT:
		err = "CTRL_BREAK event";
		break;
	default:
		sc_signal_log(fd, buf, sizeof(buf),
			      "Unknown console event [%d], shutting down! \n",
			      type);
		_Exit(1);
	}

	sc_signal_log(fd, buf, sizeof(buf), "Received : %s, (%d) \n", err,
		      type);

	if (sc_signal_will_shutdown != 0) {
		sc_signal_log(fd, buf, sizeof(buf), "Forcing shut down! \n");
		_Exit(1);
	}

	sc_signal_will_shutdown = 1;

	if (sc_signal_shutdown_fd != INVALID_SOCKET) {
		sc_signal_log(fd, buf, sizeof(buf),
			      "Sending shutdown command. \n");
		rc = send(sc_signal_shutdown_fd, (void *) &(int){1}, 1, 0);
		if (rc != 1) {
			sc_signal_log(fd, buf, sizeof(buf),
				      "Failed to send shutdown command, "
				      "shutting down immediately! \n");
			_Exit(1);
		}
	} else {
		sc_signal_log(fd, buf, sizeof(buf),
			      "No shutdown handler, shutting down! \n");
		_Exit(0);
	}

	return TRUE;
}

LONG WINAPI sc_signal_on_fatal(PEXCEPTION_POINTERS info)
{
	char buf[128];
	int fd = sc_signal_log_fd != -1 ? sc_signal_log_fd : _fileno(stderr);

	sc_signal_log(fd, buf, sizeof(buf),
		      "Fatal signal : %d, shutting down! \n",
		      info->ExceptionRecord->ExceptionCode);

	return 0;
}

void sc_signal_std_on_fatal(int sig)
{
	char buf[128];
	int fd = sc_signal_log_fd != -1 ? sc_signal_log_fd : _fileno(stderr);
	char *sig_str = "unknown signal";

	if (sig == SIGSEGV) {
		sig_str = "SIGSEGV";
	} else if (sig == SIGABRT) {
		sig_str = "SIGABRT";
	} else if (sig == SIGFPE) {
		sig_str = "SIGFPE";
	} else if (sig == SIGILL) {
		sig_str = "SIGILL";
	}

	sc_signal_log(fd, buf, sizeof(buf),
		      "Fatal signal : [%s][%d], shutting down! \n", sig_str,
		      sig);

	_Exit(1);
}

void sc_signal_std_on_shutdown(int type)
{
	sc_console_handler(CTRL_C_EVENT);
}
int sc_signal_init()
{
	BOOL b;
	sc_signal_log_fd = -1;
	sc_signal_shutdown_fd = -1;

	b = SetConsoleCtrlHandler(sc_console_handler, TRUE);
	if (!b) {
		return -1;
	}

	SetUnhandledExceptionFilter(sc_signal_on_fatal);
	signal(SIGABRT, sc_signal_std_on_fatal);
	signal(SIGINT, sc_signal_std_on_shutdown);

	return 0;
}

#else

#include <errno.h>
#include <unistd.h>

#ifdef HAVE_BACKTRACE
#include <execinfo.h>

// clang-format off
static void *sc_instruction(ucontext_t *uc)
{
	(void) uc;
	void *p = NULL;

#if defined(__APPLE__) && defined(MAC_OS_X_VERSION_10_6)
	#if defined(_STRUCT_X86_THREAD_STATE64) && !defined(__i386__)
		p = (void *) uc->uc_mcontext->__ss.__rip;
	#elif defined(__i386__)
		p = (void *) uc->uc_mcontext->__ss.__eip;
	#else
		p = (void *) (uintptr_t) arm_thread_state64_get_pc(uc->uc_mcontext->__ss);
	#endif
#elif defined(__linux__)
	#if defined(__i386__) || ((defined(__x86_64__)) && defined(__ILP32__))
		p = (void *) uc->uc_mcontext.gregs[REG_EIP];
	#elif defined(__x86_64__)
		p = (void *) uc->uc_mcontext.gregs[REG_RIP];
	#elif defined(__ia64__)
		p = (void *) uc->uc_mcontext.sc_ip;
	#elif defined(__arm__)
		p = (void *) uc->uc_mcontext.arm_pc;
	#elif defined(__aarch64__)
		p = (void *) uc->uc_mcontext.pc;
	#endif
#elif defined(__FreeBSD__)
	#if defined(__i386__)
		p = (void *) uc->uc_mcontext.mc_eip;
	#elif defined(__x86_64__)
		p = (void *) uc->uc_mcontext.mc_rip;
	#endif
#elif defined(__OpenBSD__)
	#if defined(__i386__)
		p = (void *) uc->sc_eip;
	#elif defined(__x86_64__)
		p = (void *) uc->sc_rip;
	#endif
#elif defined(__NetBSD__)
	#if defined(__i386__)
		p = (void *) uc->uc_mcontext.__gregs[_REG_EIP];
	#elif defined(__x86_64__)
		p = (void *) uc->uc_mcontext.__gregs[_REG_RIP];
	#endif
#elif defined(__DragonFly__)
	p = (void *) uc->uc_mcontext.mc_rip;
#endif
	return p;
}

#endif

// clang-format on

static void sc_signal_on_shutdown(int sig)
{
	int rc, saved_errno = errno;
	int fd = sc_signal_log_fd != -1 ? sc_signal_log_fd : STDOUT_FILENO;
	char buf[4096], *str = "Shutdown signal";

	if (sig == SIGINT) {
		str = "SIGINT";
	} else if (sig == SIGTERM) {
		str = "SIGTERM";
	}

	sc_signal_log(fd, buf, sizeof(buf), "Recv : %s, (%d) \n", str, sig);

	if (sc_signal_will_shutdown != 0) {
		sc_signal_log(fd, buf, sizeof(buf), "Forcing shut down! \n");
		sc_exit(1);
	}

	sc_signal_will_shutdown = 1;

	if (sc_signal_shutdown_fd != -1) {
		sc_signal_log(fd, buf, sizeof(buf),
			      "Sending shutdown command. \n");
		rc = (int) write(sc_signal_shutdown_fd, (void *) &(int){1}, 1);
		if (rc != 1) {
			sc_signal_log(fd, buf, sizeof(buf),
				      "Failed to send shutdown command, "
				      "shutting down immediately! \n");
			sc_exit(1);
		}
	} else {
		sc_signal_log(fd, buf, sizeof(buf),
			      "No shutdown handler, shutting down! \n");
		sc_exit(0);
	}

	errno = saved_errno;
}

static void sc_signal_on_fatal(int sig, siginfo_t *info, void *context)
{
	(void) info;

	int fd = sc_signal_log_fd != -1 ? sc_signal_log_fd : STDERR_FILENO;

	char buf[4096], *str = "unknown signal";
	struct sigaction act;

	if (sig == SIGSEGV) {
		str = "SIGSEGV";
	} else if (sig == SIGABRT) {
		str = "SIGABRT";
	} else if (sig == SIGBUS) {
		str = "SIGBUS";
	} else if (sig == SIGFPE) {
		str = "SIGFPE";
	} else if (sig == SIGILL) {
		str = "SIGILL";
	}

	sc_signal_log(fd, buf, sizeof(buf), "\nSignal : [%d][%s] \n", sig, str);
	sc_signal_log(fd, buf, sizeof(buf),
		      "\n----------------- CRASH REPORT ---------------- \n");

#ifdef HAVE_BACKTRACE
	void *caller = sc_instruction((ucontext_t *) context);
	int trace_size;
	void *trace[100];

	sc_signal_log(fd, buf, sizeof(buf), "\n Caller [%p] \n\n", caller);

	trace_size = backtrace(trace, 100);
	backtrace_symbols_fd(trace, trace_size, fd);
#else
	(void) context;
#endif
	sc_signal_log(fd, buf, sizeof(buf),
		      "\n--------------- CRASH REPORT END -------------- \n");

	sc_signal_log(fd, buf, sizeof(buf), "\nSignal handler completed! \n");
	close(fd);

	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
	act.sa_handler = SIG_DFL;
	sigaction(sig, &act, NULL);

#ifndef SC_SIGNAL_TEST
	kill(getpid(), sig);
#endif
}

int sc_signal_init()
{
	bool rc = true;
	struct sigaction action;

	sc_signal_log_fd = -1;
	sc_signal_shutdown_fd = -1;

	rc &= (signal(SIGHUP, SIG_IGN) != SIG_ERR);
	rc &= (signal(SIGPIPE, SIG_IGN) != SIG_ERR);
	rc &= (sigemptyset(&action.sa_mask) == 0);

	action.sa_flags = 0;
	action.sa_handler = sc_signal_on_shutdown;

	rc &= (sigaction(SIGTERM, &action, NULL) == 0);
	rc &= (sigaction(SIGINT, &action, NULL) == 0);

#ifdef SC_SIGNAL_TEST
	rc &= (sigaction(SIGUSR2, &action, NULL) == 0);
#endif
	rc &= (sigemptyset(&action.sa_mask) == 0);
	action.sa_flags = SA_NODEFER | SA_RESETHAND | SA_SIGINFO;
	action.sa_sigaction = sc_signal_on_fatal;

	rc &= (sigaction(SIGABRT, &action, NULL) == 0);
	rc &= (sigaction(SIGSEGV, &action, NULL) == 0);
	rc &= (sigaction(SIGBUS, &action, NULL) == 0);
	rc &= (sigaction(SIGFPE, &action, NULL) == 0);
	rc &= (sigaction(SIGILL, &action, NULL) == 0);

#ifdef SC_SIGNAL_TEST
	rc &= (sigaction(SIGSYS, &action, NULL) == 0);
#endif

	return rc ? 0 : -1;
}

#endif

void sc_signal_log(int fd, char *buf, size_t sz, char *fmt, ...)
{
	int wr, rc;
	va_list args;

	va_start(args, fmt);
	wr = sc_signal_vsnprintf(buf, sz, fmt, args);
	va_end(args);

	rc = write(fd, buf, (size_t) wr);
	(void) rc;
}
