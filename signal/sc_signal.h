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

#ifndef SC_SIGNAL_H
#define SC_SIGNAL_H

#include <signal.h>
#include <stdarg.h>
#include <stddef.h>

#define SC_SIGNAL_VERSION "2.0.0"

/**
 * Set shutdown fd here. When shutdown signal is received e.g SIGINT, SIGTERM.
 * Signal handler will write 1 byte to shutdown fd. So, your app can detect
 * shutdown command received and shutdown properly (Assuming you observe this fd
 * with select() like function). Before app shutdowns, if another shutdown
 * signal is received, _Exit() is called without waiting.
 * e.g CTRL+C to shutdown, twice CTRL+C means 'I don't want to wait anything'.
 */
#if defined(_WIN32)
#include <WinSock2.h>
extern volatile SOCKET sc_signal_shutdown_fd;
#else
extern volatile sig_atomic_t sc_signal_shutdown_fd;
#endif

/**
 * Set log file fd here, logging will be redirected to this fd, otherwise
 * STDERR_FILENO or STDOUT_FILENO will be used.
 */
extern volatile sig_atomic_t sc_signal_log_fd;

// Internal variable to handle twice shutdown signal.
extern volatile sig_atomic_t sc_signal_will_shutdown;

/**
 * Init signal handler, hooks for shutdown signals and some fatal signals.
 * @return '0' on success, '-1' on failure
 */
int sc_signal_init();

/**
 * Signal safe logging
 *
 * @param fd   fd to write log
 * @param buf  buf
 * @param size size
 * @param fmt  fmt
 * @param ...  args
 */
void sc_signal_log(int fd, char *buf, size_t size, char *fmt, ...);

/**
 * Signal safe vsnprintf.
 * Supports only "%s, "%u", "%lu", "%llu", "%d", "%ld", "%lld" and "%p"
 *
 * @param buf  buf
 * @param size size
 * @param fmt  fmt
 * @param va   va_list
 * @return     number of characters written on success, '-1' on failure.
 */
int sc_signal_vsnprintf(char *buf, size_t size, const char *fmt, va_list va);

/**
 * Signal safe snprintf.
 * Supports only "%s, "%u", "%lu", "%llu", "%d", "%ld", "%lld" and "%p"
 *
 * @param buf  buf
 * @param size size
 * @param fmt  fmt
 * @param ...  args
 * @return     number of characters written on success, '-1' on failure.
 */
int sc_signal_snprintf(char *buf, size_t size, const char *fmt, ...);

#endif
