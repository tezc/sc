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
    void* ptr;
    size_t len;
    char err[64];
};

int sc_mmap_init(struct sc_mmap* m, const char* name, int file_flags, int prot,
                 int map_flags, size_t offset, size_t len);
int sc_mmap_msync(struct sc_mmap* m, size_t offset, size_t len);
int sc_mmap_mlock(struct sc_mmap* m, size_t offset, size_t len);
int sc_mmap_munlock(struct sc_mmap* m, size_t offset, size_t len);
int sc_mmap_term(struct sc_mmap* m);

#endif
