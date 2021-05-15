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

#ifndef SC_MMAP_H
#define SC_MMAP_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

#define SC_MMAP_VERSION "2.0.0"

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
	long page_size;     // os page size
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
