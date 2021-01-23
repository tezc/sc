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
#ifndef SC_URL_H
#define SC_URL_H

#ifdef SC_HAVE_CONFIG_H
#include "sc_config.h"
#else
#define sc_url_malloc malloc
#define sc_url_free   free
#endif

#include <stdbool.h>
#include <stdlib.h>

/**
 * Based on RFC 3986
 *
 *    The following is a example URIs and their component parts:
 *
 *      foo://user:password@example.com:8042/over/there?name=ferret#nose
 *      \_/   \____________________________/\_________/ \_________/ \__/
 *       |                |                     |            |       |
 *     scheme        authority                path        query   fragment
 *
 *
 *      user:password@example.com:8042
 *      \__________/ \_________/ \__/
 *           |            |       |
 *       userinfo       host     port
 * --------------------------------------------------------------------
 *
 */

struct sc_url
{
    const char *str;
    const char *scheme;
    const char *host;
    const char *userinfo;
    const char *port;
    const char *path;
    const char *query;
    const char *fragment;

    char buf[];
};

struct sc_url *sc_url_create(const char *str);
void sc_url_destroy(struct sc_url *url);

#endif
