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

#include "sc_mutex.h"

#include <assert.h>
#include <stdbool.h>

#ifdef SC_HAVE_WRAP
bool mock_attrinit = false;
extern int __real_pthread_mutexattr_init(pthread_mutexattr_t *attr);
int __wrap_pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
	if (!mock_attrinit) {
		return __real_pthread_mutexattr_init(attr);
	}

	return -1;
}

bool mock_mutexdestroy = false;
extern int __real_pthread_mutex_destroy(pthread_mutex_t *m);
int __wrap_pthread_mutex_destroy(pthread_mutex_t *m)
{
	int rc;

	rc = __real_pthread_mutex_destroy(m);

	return mock_mutexdestroy ? -1 : rc;
}

bool mock_mutexinit = false;
extern int __real_pthread_mutex_init(pthread_mutex_t *__mutex,
				     const pthread_mutexattr_t *__mutexattr);
int __wrap_pthread_mutex_init(pthread_mutex_t *__mutex,
			      const pthread_mutexattr_t *__mutexattr)
{
	if (!mock_mutexinit) {
		return __real_pthread_mutex_init(__mutex, __mutexattr);
	}

	return -1;
}

#endif

int main()
{
	struct sc_mutex mutex;
#ifdef SC_HAVE_WRAP
	mock_attrinit = true;
	assert(sc_mutex_init(&mutex) != 0);
	mock_attrinit = false;
	assert(sc_mutex_init(&mutex) == 0);
	mock_mutexdestroy = true;
	assert(sc_mutex_term(&mutex) == -1);
	mock_mutexdestroy = false;
	mock_mutexinit = true;
	assert(sc_mutex_init(&mutex) != 0);
	mock_mutexinit = false;
	assert(sc_mutex_term(&mutex) == 0);

#endif
	assert(sc_mutex_init(&mutex) == 0);
	sc_mutex_lock(&mutex);
	sc_mutex_unlock(&mutex);
	assert(sc_mutex_term(&mutex) == 0);

	return 0;
}
