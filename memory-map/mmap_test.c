#define _XOPEN_SOURCE 700

#include "sc_mmap.h"

#include <assert.h>
#include <stdio.h>


void test1()
{
    int rc;
    unsigned char *p;
    struct sc_mmap mmap;

    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_msync(&mmap, 0, 4096);
    assert(rc == 0);
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);

    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 8192);
    assert(rc == 0);
    assert(mmap.len == 8192);
    p = mmap.ptr;
    *p = 'x';
    rc = sc_mmap_msync(&mmap, 0, 4096);
    assert(rc == 0);
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);
    rc = sc_mmap_init(&mmap, "x.txt", O_RDONLY, PROT_READ, MAP_SHARED, 0, 0);
    assert(rc == 0);
    p = mmap.ptr;
    assert(*p == 'x');
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);

    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 3, 0);
    assert(rc == -1);

    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_mlock(&mmap, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_munlock(&mmap, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);
}

#ifdef SC_HAVE_WRAP

bool fail_open;
extern int __real_open(const char *pathname, int flags, mode_t mode);
int __wrap_open(const char *pathname, int flags, mode_t mode) {
    if (fail_open) {
        return -1;
    }

    return __real_open(pathname, flags, mode);
}

bool fail_stat;
extern int __real_stat(const char *pathname, struct stat *statbuf);
int __wrap_stat(const char *pathname, struct stat *statbuf) {
    if (fail_stat) {
        return -1;
    }

    return __real_stat(pathname, statbuf);
}

bool fail_mmap;
extern void *__real_mmap(void *addr, size_t length, int prot, int flags,
                  int fd, off_t offset);
void* __wrap_mmap(void *addr, size_t length, int prot, int flags,
              int fd, off_t offset) {
    if (fail_mmap) {
        return MAP_FAILED;
    }

    return __real_mmap(addr, length, prot, flags ,fd, offset);
}

bool fail_mlock;
extern int __real_mlock (const void *addr, size_t len);
int __wrap_mlock (const void *addr, size_t len) {
    if (fail_mlock) {
        return -1;
    }

    return __real_mlock(addr, len);
}

bool fail_munlock;
extern int __real_munlock (const void *addr, size_t len);
int __wrap_munlock (const void *addr, size_t len) {
    if (fail_munlock) {
        return -1;
    }

    return __real_munlock(addr, len);
}

bool fail_msync;
extern int __real_msync (void *addr, size_t len, int flags);
int __wrap_msync (void *addr, size_t len, int flags) {
    if (fail_msync) {
        return -1;
    }

    return __real_msync(addr, len, flags);
}

bool fail_munmap;
extern int __real_munmap (void *addr, size_t len);
int __wrap_munmap(void *addr, size_t len) {
    if (fail_munmap) {
        return -1;
    }

    return __real_munmap(addr, len);
}

void fail_test()
{
    int rc;
    struct sc_mmap mmap;

    fail_open = true;
    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == -1);
    assert(sc_mmap_err(&mmap) != NULL);
    fail_open = false;

    mmap = (struct sc_mmap) {0};
    fail_stat = true;
    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == -1);
    assert(sc_mmap_err(&mmap) != NULL);
    fail_stat = false;

    mmap = (struct sc_mmap) {0};
    fail_mmap = true;
    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == -1);
    assert(sc_mmap_err(&mmap) != NULL);
    fail_mmap = false;

    mmap = (struct sc_mmap) {0};
    fail_munmap = true;
    rc = sc_mmap_term(&mmap);
    assert(rc == -1);
    assert(sc_mmap_err(&mmap) != NULL);
    fail_munmap = false;

    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == 0);
    fail_mlock = true;
    rc = sc_mmap_mlock(&mmap, 0, 4096);
    assert(rc == -1);
    assert(sc_mmap_err(&mmap) != NULL);
    fail_mlock = false;

    rc = sc_mmap_mlock(&mmap, 0, 4096);
    assert(rc == 0);

    fail_munlock = true;
    rc = sc_mmap_munlock(&mmap, 0, 4096);
    assert(rc == -1);
    fail_munlock = false;
    rc = sc_mmap_munlock(&mmap, 0, 4096);
    assert(rc == 0);

    fail_msync = true;
    rc = sc_mmap_msync(&mmap, 0, 4096);
    assert(rc == -1);
    fail_msync = false;
    rc = sc_mmap_msync(&mmap, 0, 4096);
    assert(rc == 0);

    rc = sc_mmap_term(&mmap);
    assert(rc == 0);
}
#else
void fail_test()
{

}
#endif

int main()
{
    test1();
    fail_test();

    return 0;
}
