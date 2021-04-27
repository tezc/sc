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
