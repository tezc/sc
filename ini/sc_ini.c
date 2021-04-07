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
 *
 * Based on : https://github.com/benhoyt/inih
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright (C) 2009-2020, Ben Hoyt
 */

#include "sc_ini.h"

#include <ctype.h>
#include <stdint.h>
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

	diff = (size_t)(t - str);
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
