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

#ifndef SC_URI_H
#define SC_URI_H

#define SC_URI_VERSION "2.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_uri_malloc malloc
#define sc_uri_free free
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

struct sc_uri {
	const char *str; // Full string
	const char *scheme;
	const char *host;
	const char *userinfo;
	const char *port;
	const char *path;
	const char *query;
	const char *fragment;

	char buf[];
};

/**
 * Parse uri.
 *
 * Internally, it does a single allocation but each part is also represented as
 * NULL ended string.
 *
 * E.g :
 *
 * struct sc_uri* uri;
 *
 * struct sc_uri* uri;
 * uri =
 * sc_uri_create("http://user:pass@any.com:8042/over/there?name=jane#doe");
 *
 * printf("%s \n", uri->str);       // prints full string.
 * printf("%s \n", uri->scheme);    // prints "http"
 * printf("%s \n", uri->host);      // prints "any.com"
 * printf("%s \n", uri->userinfo);  // prints "user:pass"
 * printf("%s \n", uri->port);      // prints "8042"
 * printf("%s \n", uri->path);      // prints "/over/there"
 * printf("%s \n", uri->query);     // prints "name=jane"
 * printf("%s \n", uri->fragment);  // prints "doe"
 *
 *
 * @param str uri string
 * @return    uri, NULL on error
 */
struct sc_uri *sc_uri_create(const char *str);

/**
 * @param uri uri
 */
void sc_uri_destroy(struct sc_uri **uri);

#endif
