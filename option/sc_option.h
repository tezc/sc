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
#ifndef SC_OPTION_H
#define SC_OPTION_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define SC_OPTION_VERSION "1.0.0"

struct sc_option_item {
	const char letter;
	const char *name;
};

struct sc_option {
	struct sc_option_item *options;
	int count;
	char **argv;
};

/**
 *
 * @param opt    Already initialized sc_opt struct
 * @param index  Index for argv
 * @param value  [out] Value for the option if exists. It should be after '='
 *               sign. E.g : -key=value or -k=value. If value does not exists
 *               (*value) will point to '\0' character. It won't be NULL itself.
 *
 *               To check if option has value associated : if (*value != '\0')
 *
 * @return       Letter for the option. If option is not known, '?' will be
 *               returned.
 */
char sc_option_at(struct sc_option *opt, int index, char **value);

#endif
