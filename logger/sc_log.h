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

#ifndef SC_LOG_H
#define SC_LOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SC_LOG_VERSION "2.0.0"

enum sc_log_level
{
	SC_LOG_DEBUG,
	SC_LOG_INFO,
	SC_LOG_WARN,
	SC_LOG_ERROR,
	SC_LOG_OFF,
};

// clang-format off
static const struct sc_log_level_pair
{
    const enum sc_log_level id;
    const char *str;
} sc_log_levels[] = {
        {SC_LOG_DEBUG, "DEBUG"},
        {SC_LOG_INFO,  "INFO" },
        {SC_LOG_WARN,  "WARN" },
        {SC_LOG_ERROR, "ERROR"},
        {SC_LOG_OFF,   "OFF"  },
};
// clang-format on

// Internal function
int sc_log_log(enum sc_log_level level, const char *fmt, ...);

// Max file size to rotate, should not be more than 4 GB.
#define SC_LOG_FILE_SIZE (2 * 1024 * 1024)

// Define SC_LOG_PRINT_FILE_NAME to print file name and line no in the log line.
#ifdef SC_LOG_PRINT_FILE_NAME
#define sc_log_ap(fmt, ...)                                                    \
	"(%s:%d) " fmt, strrchr("/" __FILE__, '/') + 1, __LINE__, __VA_ARGS__
#else
#define sc_log_ap(fmt, ...) fmt, __VA_ARGS__
#endif

/**
 * sc_log_init() call once in your application before using log functions.
 * sc_log_term() call once to clean up, you must not use logger after this call.
 * These functions are not thread-safe, should be called from a single thread.
 *
 * @return '0' on success, negative value on error, errno will be set.
 */
int sc_log_init(void);
int sc_log_term(void);

/**
 * Call once from each thread if you want to set thread name.
 * @param name  Thread name
 */
void sc_log_set_thread_name(const char *name);

/**
 * @param level  One of "DEBUG", "INFO", "WARN", "ERROR", "OFF"
 * @return       '0' on success, negative value on invalid level string
 */
int sc_log_set_level(const char *level);

/**
 * @param enable 'true' to enable, 'false' to disable logging to stdout.
 */
void sc_log_set_stdout(bool enable);

/**
 * Log files will be rotated. Latest logs will always be in the 'current_file'.
 * e.g sc_log_set_file("/tmp/log.0.txt", "/tmp/log-latest.txt");
 *     To disable logging into file : sc_log_set_file(NULL, NULL);
 *
 * @param prev     file path for previous log file, 'NULL' to disable
 * @param current  file path for latest log file, 'NULL' to disable
 * @return         0 on success, -1 on error, errno will be set.
 */
int sc_log_set_file(const char *prev, const char *current);

/**
 * @param arg user arg to callback.
 * @param cb  log callback.
 */
void sc_log_set_callback(void *arg,
			 int (*cb)(void *arg, enum sc_log_level level,
				   const char *fmt, va_list va));

// e.g : sc_log_error("Errno : %d, reason : %s", errno, strerror(errno));
#define sc_log_debug(...) (sc_log_log(SC_LOG_DEBUG, sc_log_ap(__VA_ARGS__, "")))
#define sc_log_info(...) (sc_log_log(SC_LOG_INFO, sc_log_ap(__VA_ARGS__, "")))
#define sc_log_warn(...) (sc_log_log(SC_LOG_WARN, sc_log_ap(__VA_ARGS__, "")))
#define sc_log_error(...) (sc_log_log(SC_LOG_ERROR, sc_log_ap(__VA_ARGS__, "")))

#endif
