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

#include "sc_uri.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
    #pragma warning(disable : 4996)
#endif


struct sc_uri *sc_uri_create(const char *str)
{
    const char *s1 = "%.*s%.*s%.*s%.*s%.*s%.*s%.*s%.*s";
    const char *s2 = "%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c%.*s%c";
    const char *authority = "//";

    int diff, ret;
    unsigned long val;
    size_t full_len, parts_len;
    size_t scheme_len, authority_len = 0, userinfo_len = 0;
    size_t host_len = 0, port_len = 0, path_len;
    size_t query_len = 0, fragment_len = 0;

    char *scheme, *userinfo = "", *host = "", *port = "";
    char *path, *query = "", *fragment = "";
    char *ptr, *dest, *parse_end;
    char *pos = (char *) str;
    struct sc_uri *uri;

    if (str == NULL || (ptr = strstr(pos, ":")) == NULL) {
        return NULL;
    }

    scheme = pos;
    scheme_len = ptr - str + 1;
    pos = ptr + 1;

    if (*pos == '/' && *(pos + 1) == '/') {
        authority_len = 2;
        pos += authority_len;

        ptr = strchr(pos, '@');
        if (ptr != NULL) {
            userinfo = pos;
            userinfo_len = ptr - pos + strlen("@");
            pos = ptr + 1;
        }

        ptr = pos + strcspn(pos, *pos == '[' ? "]" : ":/?#");
        host = pos;
        host_len = ptr - pos + (*host == '[');
        pos = host + host_len;

        if (*host == '[' && *(host + host_len - 1) != ']') {
            return NULL;
        }

        ptr = strchr(pos, ':');
        if (ptr != NULL) {
            if (*(ptr + 1) == '\0') {
                return NULL;
            }

            errno = 0;
            val = strtoul(ptr + 1, &parse_end, 10);
            if (errno != 0 || val > 65536) {
                return NULL;
            }

            port = ptr;
            port_len = parse_end - ptr;
            pos = port + port_len;
        }
    }

    path = pos;
    path_len = strcspn(path, "?#");
    pos = path + path_len;

    ptr = strchr(pos, '?');
    if (ptr != NULL) {
        query = ptr;
        query_len = strcspn(query, "#");
        pos = query + query_len;
    }

    if (*pos == '#') {
        fragment = pos;
        fragment_len = strlen(pos);
    }

    full_len = scheme_len + authority_len + userinfo_len + host_len + port_len +
               path_len + query_len + fragment_len + 1;

    parts_len = full_len - authority_len;
    parts_len += 7; // NULL characters for each part.
    parts_len -= (scheme_len != 0);
    parts_len -= (userinfo_len != 0);
    parts_len -= (port_len != 0);
    parts_len -= (query_len != 0);
    parts_len -= (fragment_len != 0);

    uri = sc_uri_malloc(sizeof(*uri) + parts_len + full_len);
    if (uri == NULL) {
        return NULL;
    }

    ret = snprintf(uri->buf, full_len, s1, scheme_len, scheme, authority_len,
                   authority, userinfo_len, userinfo, host_len, host, port_len,
                   port, path_len, path, query_len, query, fragment_len,
                   fragment);
    if (ret < 0 || (size_t) ret != full_len - 1) {
        goto error;
    }

    dest = uri->buf + strlen(uri->buf) + 1;

    scheme_len -= (scheme_len != 0);     // Skip ":"
    userinfo_len -= (userinfo_len != 0); // Skip "@"
    diff = port_len != 0;
    port_len -= diff; // Skip ":"
    port += diff;     // Skip ":"
    diff = (query_len != 0);
    query_len -= diff; // Skip "?"
    query += diff;     // Skip "?"
    diff = (fragment_len != 0);
    fragment_len -= diff; // Skip "#"
    fragment += diff;     // Skip "#"

    ret = sprintf(dest, s2, scheme_len, scheme, 0, userinfo_len, userinfo, 0,
                  host_len, host, 0, port_len, port, 0, path_len, path, 0,
                  query_len, query, 0, fragment_len, fragment, 0);
    if (ret < 0 || (size_t) ret != parts_len - 1) {
        goto error;
    }

    uri->str = uri->buf;
    uri->scheme = dest;
    uri->userinfo = dest + scheme_len + 1;
    uri->host = uri->userinfo + userinfo_len + 1;
    uri->port = uri->host + host_len + 1;
    uri->path = uri->port + port_len + 1;
    uri->query = uri->path + path_len + 1;
    uri->fragment = uri->query + query_len + 1;

    return uri;

error:
    sc_uri_free(uri);
    return NULL;
}

void sc_uri_destroy(struct sc_uri *uri)
{
    sc_uri_free(uri);
}
