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
