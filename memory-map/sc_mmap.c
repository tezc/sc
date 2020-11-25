#include "sc_mmap.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int sc_mmap_init(struct sc_mmap *m, const char *name, int file_flags,
                 int map_flags, bool shared, size_t offset, size_t len)
{
    const int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int s;

    *m = (struct sc_mmap){0};

    struct stat st;
    int fd, rc, saved_errno;
    size_t size;
    void *p = NULL;

    fd = open(name, file_flags, mode);
    if (fd == -1) {
        goto error;
    }

    rc = stat(name, &st);
    if (rc != 0) {
        goto cleanup_fd;
    }

    size = st.st_size;

    if (len > size) {
        size_t seek, written;
        size_t block = st.st_blksize;
        size_t pos = (size / block) * block + block - 1;

        for (; pos < len + block - 1; pos += block) {
            if (pos >= len) {
                pos = len - 1;
            }

            seek = lseek(fd, pos, SEEK_SET);
            if (seek == -1) {
                goto cleanup_fd;
            }

            written = write(fd, "", 1);
            if (written != 1) {
                goto cleanup_fd;
            }
        }
    }

    s = shared ? MAP_SHARED : MAP_PRIVATE;
    p = mmap(NULL, len, map_flags, s, fd, offset);
    if (p == MAP_FAILED) {
        goto cleanup_fd;
    }

    m->ptr = p;
    m->len = len;

    close(fd);

    return 0;

cleanup_fd:
    saved_errno = errno;
    close(fd);
error:
    strcpy(m->err, strerror(saved_errno));

    return -1;
}

int sc_mmap_msync(struct sc_mmap *m, size_t offset, size_t len)
{
    int rc;
    char *p = (char *) m->ptr + offset;

    rc = msync(p, len, MS_SYNC);
    if (rc != 0) {
        strcpy(m->err, strerror(errno));
    }

    return rc;
}

int sc_mmap_term(struct sc_mmap *m)
{
    int rc;

    rc = munmap(m->ptr, m->len);
    if (rc != 0) {
        strcpy(m->err, strerror(errno));
    }

    return rc;
}
