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

#ifndef SC_INI_H
#define SC_INI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define SC_INI_VERSION "2.0.0"

// Set max line length. If a line is longer, it will be truncated silently.
#define SC_INI_MAX_LINE_LEN 1024

/**
 * @param arg      user arg
 * @param line     current line number
 * @param section  section
 * @param key      key
 * @param value    value
 * @return         Return '0' on success, any other value will make parser
 *                 stop and return error
 */
typedef int (*sc_ini_on_item)(void *arg, int line, const char *section,
			      const char *key, const char *value);

/**
 * @param arg      user data to be passed to 'on_item' callback
 * @param on_item  callback
 * @param filename filename
 * @return         - '0' on success,
 *                 - '-1' on file IO error.
 *                 - positive line number on parsing error
 *                 - 'on_item' return value if it returns other than '0'
 */
int sc_ini_parse_file(void *arg, sc_ini_on_item on_item, const char *filename);

/**
 * @param arg      user data to be passed to 'on_item' callback.
 * @param on_item  callback
 * @param str      string to parse
 * @return         - '0' on success,
 *                 - positive line number on parsing error
 *                 - "on_item's return value" if it returns other than '0'
 */
int sc_ini_parse_string(void *arg, sc_ini_on_item on_item, const char *str);

#endif
