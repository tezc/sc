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

#ifndef SC_INI_H
#define SC_INI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define SC_INI_VERSION "1.0.0"

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
