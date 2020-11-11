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

#include "sc_url.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct sc_url *url_create(const char *str, const char *default_scheme,
                          const char *default_port)
{
    size_t total_len = 0;
    size_t str_len = 0;
    bool ipv6 = false;

    size_t scheme_len = 0;
    size_t user_len = 0;
    size_t host_len = 0;
    size_t port_len = 0;
    size_t path_len = 0;
    size_t query_len = 0;
    size_t fragment_len = 0;

    char *scheme = (char *) (default_scheme != NULL ? default_scheme : "");
    char *userinfo = "";
    char *host = NULL;
    char *port = (char *) (default_port != NULL ? default_port : "");
    char *path = "";
    char *query = "";
    char *fragment = "";

    char *pos, *ptr;
    const char *end;

    struct sc_url *url;

    end = str + strlen(str);
    pos = (char *) str;

    ptr = strstr(pos, "://");
    if (ptr != NULL) {
        scheme = pos;
        scheme_len = ptr - str;
        pos = ptr + strlen("://");
    }

    ptr = strchr(pos, '@');
    if (ptr != NULL) {
        userinfo = pos;
        user_len = ptr - pos;
        pos = ptr + strlen("@");
    }

    ptr = strchr(pos, '[');
    if (ptr != NULL) {
        char *bracket = strrchr(pos, ']');
        if (bracket == NULL) {
            return NULL;
        }

        host = ptr + strlen("[");
        host_len = bracket - ptr - 1;
        pos = bracket + 1;
        ipv6 = true;
    }

    ptr = strrchr(pos, ':');
    if (ptr != NULL) {
        char *parse_end;
        unsigned long val;

        if (*(ptr + 1) == '\0') {
            return NULL;
        }

        errno = 0;
        val = strtoul(ptr + 1, &parse_end, 10);
        if (errno != 0 || val > 65536) {
            return NULL;
        }

        port = ptr + 1;
        port_len = parse_end - ptr - 1;
    } else {
        ptr = (char *) end;
    }

    ptr = strchr(pos, '/');
    if (ptr != NULL) {
        path = pos + strcspn(pos, "?");
        path_len = path - pos;
    }

    if (host == NULL) {
        host = pos;
        host_len = ptr - pos;
    }

    const char *s1 = "%.*s://%.*s%.*s:%.*s";
    const char *s2 = "%.*s://%.*s[%.*s]:%.*s";
    const char *s3 = "%.*s://%.*s@%.*s:%.*s";
    const char *s4 = "%.*s://%.*s@[%.*s]:%.*s";
    const char *n1 = "%.*s%c://%.*s%.*s%c:%.*s";
    const char *n2 = "%.*s%c://%.*s[%.*s]%c:%.*s";
    const char *n3 = "%.*s%c://%.*s%c@%.*s%c:%.*s";
    const char *n4 = "%.*s%c://%.*s%c@[%.*s]%c:%.*s";
    const char *s5;
    const char *n5;

    if (ipv6) {
        if (*userinfo == '\0') {
            s5 = s2;
            n5 = n2;
        } else {
            s5 = s4;
            n5 = n4;
        }
    } else {
        if (*userinfo == '\0') {
            s5 = s1;
            n5 = n1;
        } else {
            s5 = s3;
            n5 = n3;
        }
    }

    str_len = scheme_len + strlen("://") + user_len + (user_len ? 1 : 0) +
              host_len + port_len + 1;
    total_len = str_len + 1 + 1 + 1;

    url = sc_url_malloc(sizeof(struct sc_url) + total_len + str_len);
    if (url == NULL) {
        return NULL;
    }

    sprintf(url->buf, s5, scheme_len, scheme, user_len, userinfo, host_len,
            host, port_len, port);
    char *dest = url->buf + strlen(url->buf) + 1;
    sprintf(dest, n5, scheme_len, scheme, 0, user_len, userinfo, 0, host_len,
            host, 0, port_len, port);

    url->str = url->buf;
    url->scheme = dest;
    url->userinfo = dest + scheme_len + strlen("://") + 1;
    url->host = url->userinfo + user_len + 1 + (*userinfo ? 1 : 0);
    url->port = url->host + host_len + 1 + (strlen(":"));
    url->ipv6 = ipv6;

    return url;
}

void sc_url_destroy(struct sc_url *url)
{
    sc_url_free(url);
}
