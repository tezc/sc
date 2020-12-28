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

#include "sc_ini.h"

#include <ctype.h>
#include <stdint.h>
#include <string.h>

static char *trim_space(char *str)
{
    char *end;

    while (isspace(*str)) {
        str++;
    }

    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) {
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
    if (str != NULL && strlen(str) >= 3) {
        if ((uint8_t) str[0] == 0xEF && (uint8_t) str[1] == 0xBB &&
            (uint8_t) str[2] == 0xBF) {
            str += 3;
        }
    }

    return str;
}

int sc_ini_parse(void *arg, sc_ini_on_item on_item, void *arg1,
                 char *(*next_line)(void *arg, char *buf, size_t size))
{
    int rc = 0, line = 0;
    char buf[SC_INI_MAX_LINE_LEN];
    char section[256] = {0}, prev_key[256] = {0};
    char *head, *end;

    while ((head = next_line(arg1, buf, sizeof(buf) - 1)) != NULL) {
        if (++line == 1) {
            // Skip byte order mark
            head = trim_bom(head);
        }

        head = trim_space(trim_comment(head));
        if (*head == '\0') {
            continue;
        }

        if (head > buf && *prev_key) {
            // Multi-line case. This line is another value to previous key.
            rc = on_item(arg, line, section, prev_key, head);
        } else if (*head == '[') {
            if ((end = strchr(head, ']')) == NULL) {
                return line;
            }

            *prev_key = '\0';
            *end = '\0';
            strncpy(section, head + 1, sizeof(section) - 1);
        } else {
            if ((end = strpbrk(head, "=:")) == NULL) {
                return line;
            }

            *end = '\0';
            trim_space(head);
            strncpy(prev_key, head, sizeof(prev_key) - 1);
            rc = on_item(arg, line, section, head, trim_space(end + 1));
        }

        if (rc != 0) {
            return line;
        }
    }

    return 0;
}

static char *file_next_line(void *p, char *buf, size_t size)
{
    return fgets(buf, size, (FILE *) p);
}

static char *string_next_line(void *p, char *buf, size_t size)
{
    size_t len;
    char *t;
    char *str = (*(char **) p);

    if (str == NULL || *str == '\0') {
        return NULL;
    }

    t = strchr(str, '\n');
    if (t == NULL) {
        t = str + strlen(str);
    }

    len = (t - str) < size ? (t - str) : size;
    memcpy(buf, str, len);
    buf[len] = '\0';

    *(char **) p = *t == '\0' ? '\0' : t + 1;

    return buf;
}

int sc_ini_parse_file(void *arg, sc_ini_on_item on_item, const char *filename)
{
    int rc;
    FILE *file;

    file = fopen(filename, "rb");
    if (!file) {
        return -1;
    }

    rc = sc_ini_parse(arg, on_item, file, file_next_line);
    if (rc == 0) {
        rc = ferror(file) != 0 ? -1 : 0;
    }

    fclose(file);

    return rc;
}

int sc_ini_parse_string(void *arg, sc_ini_on_item on_item, const char *str)
{
    char *ptr = (char *) str;

    return sc_ini_parse(arg, on_item, &ptr, string_next_line);
}
