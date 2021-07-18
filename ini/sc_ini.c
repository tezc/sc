/*
 * BSD-3-Clause
 *
 * Based on : https://github.com/benhoyt/inih
 * Copyright (C) 2009-2020,   Ben Hoyt
 *               2020-present Ozan Tezcan
 *
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

#include "sc_ini.h"

#include <ctype.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#pragma warning(disable : 4996)
#endif

static char *trim_space(char *str)
{
	char *end;

	while (isspace((unsigned char) *str)) {
		str++;
	}

	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char) *end)) {
		end--;
	}

	end[1] = '\0';

	return str;
}

static char *trim_comment(char *str)
{
	char *s = str;

	if (*s == '\0' || *s == ';' || *s == '#') {
		*s = '\0';
		return str;
	}

	while (*s && (s = strchr(s, ' ')) != NULL) {
		s++;
		if (*s == ';' || *s == '#') {
			*s = '\0';
			break;
		}
	}

	return str;
}

static char *trim_bom(char *str)
{
	unsigned char *p = (unsigned char *) str;

	if (strlen(str) >= 3) {
		if (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
			str += 3;
		}
	}

	return str;
}

int sc_ini_parse(void *arg, sc_ini_on_item cb, void *arg1,
		 char *(*next_line)(void *, char *, size_t))
{
	int rc = 0, line = 0;
	char buf[SC_INI_MAX_LINE_LEN];
	char section[256] = {0}, key[256] = {0};
	char *head, *end;

	while ((head = next_line(arg1, buf, sizeof(buf) - 1)) != NULL) {
		if (++line == 1) {
			head = trim_bom(head);
		}

		head = trim_space(trim_comment(head));
		if (*head == '\0') {
			continue;
		}

		if (head > buf && *key) {
			rc = cb(arg, line, section, key, head);
		} else if (*head == '[') {
			if ((end = strchr(head, ']')) == NULL) {
				return line;
			}

			*key = '\0';
			*end = '\0';
			strncpy(section, head + 1, sizeof(section) - 1);
		} else {
			if ((end = strpbrk(head, "=:")) == NULL) {
				return line;
			}

			*end = '\0';
			trim_space(head);
			strncpy(key, head, sizeof(key) - 1);
			rc = cb(arg, line, section, head, trim_space(end + 1));
		}

		if (rc != 0) {
			return line;
		}
	}

	return 0;
}

static char *file_line(void *p, char *buf, size_t size)
{
	return fgets(buf, (int) size, (FILE *) p);
}

static char *string_line(void *p, char *buf, size_t size)
{
	size_t len, diff;
	char *t, *str = (*(char **) p);

	if (str == NULL || *str == '\0') {
		return NULL;
	}

	t = strchr(str, '\n');
	if (t == NULL) {
		t = str + strlen(str);
	}

	diff = (size_t) (t - str);
	len = diff < size ? diff : size;
	memcpy(buf, str, len);
	buf[len] = '\0';

	*(char **) p = (*t == '\0') ? '\0' : t + 1;

	return buf;
}

int sc_ini_parse_file(void *arg, sc_ini_on_item cb, const char *filename)
{
	int rc;
	FILE *file;

	file = fopen(filename, "rb");
	if (!file) {
		return -1;
	}

	rc = sc_ini_parse(arg, cb, file, file_line);
	if (rc == 0) {
		rc = ferror(file) != 0 ? -1 : 0;
	}

	fclose(file);

	return rc;
}

int sc_ini_parse_string(void *arg, sc_ini_on_item cb, const char *str)
{
	char *ptr = (char *) str;
	return sc_ini_parse(arg, cb, &ptr, string_line);
}
