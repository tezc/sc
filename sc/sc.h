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
#ifndef SC_H
#define SC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SC_VERSION "1.0.0"

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
bool sc_is_pow2(size_t num);

/**
 * @param size size
 * @return     next nearest power of two of size
 */
size_t sc_to_pow2(size_t size);

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
