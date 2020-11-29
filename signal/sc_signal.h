/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
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

#if defined(_WIN32)
    #include <WinSock2.h>
volatile SOCKET sc_signal_shutdown_fd;
#else
volatile sig_atomic_t sc_signal_shutdown_fd;
#endif

volatile sig_atomic_t sc_signal_log_fd;
volatile sig_atomic_t sc_signal_will_shutdown;

int sc_signal_init();
int sc_signal_vsnprintf(char *buf, size_t size, const char *fmt, va_list va);
int sc_signal_snprintf(char *buf, size_t size, const char *fmt, ...);

#endif
