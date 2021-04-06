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

#include "sc_option.h"

#include <string.h>

char sc_option_at(struct sc_option *opt, int index, char **value)
{
	char id = '?';
	size_t len;
	char *pos;
	const char *curr, *name;

	pos = opt->argv[index];
	*value = NULL;

	if (*pos != '-') {
		return id;
	}

	pos++; // Skip first '-'
	if (*pos != '-') {
		for (int i = 0; i < opt->count; i++) {
			if (*pos == opt->options[i].letter &&
			    strchr("= \0", *(pos + 1)) != NULL) {
				id = *pos;
				pos++; // skip letter
				*value = pos + (*pos != '=' ? 0 : 1);
				break;
			}
		}
	} else {
		while (*pos && *pos != '=') {
			pos++;
		}

		for (int i = 0; i < opt->count; i++) {
			curr = opt->argv[index] + 2; // Skip '--'
			name = opt->options[i].name;
			len = (pos - curr);

			if (name == NULL) {
				continue;
			}

			if (len == strlen(name) &&
			    memcmp(name, curr, len) == 0) {
				id = opt->options[i].letter;
				*value = pos + (*pos != '=' ? 0 : 1);
				break;
			}
		}
	}

	return id;
}
