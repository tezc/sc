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

#ifndef SC_MMAP_H
#define SC_MMAP_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

#define SC_MMAP_VERSION "1.0.0"

#if defined(_WIN32)

#include <windows.h>
#include <memoryapi.h>

#define PROT_READ  FILE_MAP_READ
#define PROT_WRITE FILE_MAP_WRITE

#define MAP_FIXED     0x01
#define MAP_ANONYMOUS 0x02
#define MAP_SHARED    0x04

#else /*POSIX*/

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#include <sys/mman.h>

#endif

struct sc_mmap {
	int fd;
	unsigned char *ptr; // memory map start address
	size_t len;	    // memory map length
	char err[128];
};

/**
 * Initializes mmap structure, opens file and maps into memory. If file is
 * smaller than 'offset + len' and PROT_WRITE flag is provided, then the file
 * will be expanded to 'offset + len' bytes.
 *
 * @param m           mmap
 * @param name        file name
 * @param file_flags  flags for open(), e.g : O_RDWR | O_CREAT
 * @param prot        prot flags,       e.g : PROT_READ | PROT_WRITE
 * @param map_flags   mmap flags,       e.g : MAP_SHARED
 * @param offset      offset
 * @param len         len
 * @return            '0' on success, negative on failure,
 *                    call sc_mmap_err() for error string.
 */
int sc_mmap_init(struct sc_mmap *m, const char *name, int file_flags, int prot,
		 int map_flags, size_t offset, size_t len);
/**
 * @param m mmap
 * @return       '0' on success, '-1' on error, call sc_mmap_err() for details.
 */
int sc_mmap_term(struct sc_mmap *m);

/**
 * @param m      mmap
 * @param offset offset
 * @param len    len
 * @return      '0' on success, negative on failure,
 *              call sc_mmap_err() for error string.
 */
int sc_mmap_msync(struct sc_mmap *m, size_t offset, size_t len);

/**
 * @param m       mmap
 * @param offset  offset
 * @param len     len
 * @return        '0' on success, negative on failure,
 *                call sc_mmap_err() for error string.
 */
int sc_mmap_mlock(struct sc_mmap *m, size_t offset, size_t len);

/**
 * @param m       mmap
 * @param offset  offset
 * @param len     len
 * @return        '0' on success, negative on failure,
 *                call sc_mmap_err() for error string.
 */
int sc_mmap_munlock(struct sc_mmap *m, size_t offset, size_t len);

/**
 * @param m mmap
 * @return  last error string.
 */
const char *sc_mmap_err(struct sc_mmap *m);

#endif
