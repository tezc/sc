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
#ifndef SC_H
#define SC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SC_VERSION "2.0.0"

#define sc_max(a, b) (((a) > (b)) ? (a) : (b))
#define sc_min(a, b) (((a) > (b)) ? (b) : (a))

struct sc_rand {
	unsigned char i;
	unsigned char j;
	unsigned char init[256];
};

/**
 * Pseudo random number generator - RC4
 *   e.g :
 *      char buf[256], tmp1[16], tmp2[4];
 *
 *      int fd = open("/dev/urandom", O_RDONLY);
 *      if (fd < 0) {
 *          return; // Error
 *      }
 *
 *  retry:
 *      ssize_t sz = read(fd, buf, size);
 *      if (sz < 0 && errno == EINTR) {
 *          goto retry;
 *      }
 *      close(fd);
 *
 *      struct sc_rand rnd;
 *      sc_rand_init(&rnd, buf); // Init generator
 *
 *      sc_rand_read(&rnd, tmp, sizeof(tmp); //  Read random
 *      sc_rand_read(&rnd, tmp, sizeof(tmp2); // Read random
 *
 * @param r  rand
 * @param init rand source, possibly from /dev/urandom, must be 256 bytes long.
 */
void sc_rand_init(struct sc_rand *r, const unsigned char *init);
void sc_rand_read(struct sc_rand *r, void *buf, int size);

/**
 * @param num num
 * @return    'true' if num is power of 2
 */
bool sc_is_pow2(uint64_t num);

/**
 * @param num num
 * @return    next nearest power of two of size
 */
uint64_t sc_to_pow2(uint64_t size);

/**
 * Bytes to human readable form, e.g 1024 bytes  to "1 KB".
 *
 * @param buf  buf to write output
 * @param len  buf len
 * @param val  val to be converted into human readable form
 * @return     'buf' on success, 'NULL' on failure.
 */
char *sc_bytes_to_size(char *buf, size_t len, uint64_t val);

/**
 * Human readable string to bytes, e.g "1 KB" to 1024 bytes.
 *
 * @param buf buf to write output
 * @return    positive value on success, '-1' on error
 */
int64_t sc_size_to_bytes(const char *buf);

#endif
