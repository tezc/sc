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
#ifndef SC_TIME_H
#define SC_TIME_H

#define SC_TIME_VERSION "2.0.0"

#include <stdint.h>

/**
 * Wall clock time. Gets CLOCK_REALTIME on Posix.
 * @return current timestamp in milliseconds.
 */
uint64_t sc_time_ms();

/**
 * Wall clock time. Gets CLOCK_REALTIME on Posix.
 * @return current timestamp in nanoseconds.
 */
uint64_t sc_time_ns();

/**
 * Monotonic timer. Gets CLOCK_MONOTONIC on Posix
 * @return current timestamp in milliseconds.
 */
uint64_t sc_time_mono_ms();

/**
 * Monotonic timer. Gets CLOCK_MONOTONIC on Posix
 * @return Current timestamp in nanoseconds.
 */
uint64_t sc_time_mono_ns();

/**
 * @param millis milliseconds to sleep.
 * @return '0' on success, negative on failure.
 */
int sc_time_sleep(uint64_t millis);

#endif
