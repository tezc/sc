#ifndef SC_MMAP_H
#define SC_MMAP_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <memoryapi.h>


#define PROT_READ  FILE_MAP_READ
#define PROT_WRITE FILE_MAP_WRITE

#define MAP_FIXED     0x01
#define MAP_ANONYMOUS 0x02
#define MAP_SHARED    0x04

#else /*POSIX*/

#include <sys/mman.h>

#endif

struct sc_mmap
{
    int fd;
    unsigned char* ptr;
    size_t len;
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
 * @param prot        prot flags, PROT_READ / PROT_WRITE
 * @param map_flags   mmap flags, e.g : MAP_SHARED
 * @param offset      offset
 * @param len         len
 * @return            '0' on success, '-1' on failure, call sc_mmap_err() for
 *                    error string.
 */
int sc_mmap_init(struct sc_mmap* m, const char* name, int file_flags, int prot,
                 int map_flags, size_t offset, size_t len);

/**
 * @param m mmap
 * @return       '0' on success, '-1' on error, call sc_mmap_err() for
 *               error string.
 */
int sc_mmap_term(struct sc_mmap* m);

/**
 *
 * @param m      mmap
 * @param offset offset
 * @param len    len
 * @return       '0' on success, '-1' on error, call sc_mmap_err() for
 *               error string.
 */
int sc_mmap_msync(struct sc_mmap* m, size_t offset, size_t len);

/**
 * @param m       mmap
 * @param offset  offset
 * @param len     len
 * @return       '0' on success, '-1' on error, call sc_mmap_err() for
 *               error string.
 */
int sc_mmap_mlock(struct sc_mmap* m, size_t offset, size_t len);

/**
 * @param m       mmap
 * @param offset  offset
 * @param len     len
 * @return       '0' on success, '-1' on error, call sc_mmap_err() for
 *               error string.
 */
int sc_mmap_munlock(struct sc_mmap* m, size_t offset, size_t len);

/**
 * Error string of the last error, call after if any mmap function returns '-1'.
 *
 * @param m mmap
 * @return  error string.
 */
const char *sc_mmap_err(struct sc_mmap *m);


#endif
