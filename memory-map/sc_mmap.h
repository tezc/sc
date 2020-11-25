#ifndef SC_MMAP_H
#define SC_MMAP_H

#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SC_MMAP_FILE_READ      O_RDONLY
#define SC_MMAP_FILE_READWRITE O_RDWR
#define SC_MMAP_FILE_CREATE    O_CREAT
#define SC_MMAP_FILE_TRUNC     O_TRUNC

#define SC_MMAP_MAP_READ  PROT_READ
#define SC_MMAP_MAP_WRITE PROT_WRITE
#define SC_MMAP_MAP_EXEC  PROT_EXEC


struct sc_mmap
{
    void *ptr;
    size_t len;
    char err[64];
};

int sc_mmap_init(struct sc_mmap *m, const char *name, int file_flags, int map_flags,
                 bool shared, size_t offset, size_t len);

int sc_mmap_msync(struct sc_mmap *m, size_t offset, size_t len);
int sc_mmap_term(struct sc_mmap *m);


#endif
