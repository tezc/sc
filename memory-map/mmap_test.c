#include "sc_mmap.h"

#include <assert.h>
#include <stdio.h>


void test1()
{
    const int wflags =
            SC_MMAP_FILE_READWRITE | SC_MMAP_FILE_CREATE | SC_MMAP_FILE_TRUNC;
    const int mwflags = SC_MMAP_MAP_READ | SC_MMAP_MAP_WRITE;
    const int rflags = SC_MMAP_FILE_READ;
    const int mrflags = SC_MMAP_MAP_READ;
    int rc;
    char* p;
    struct sc_mmap mmap;

    rc = sc_mmap_init(&mmap, "/tmp/x.txt", wflags, mwflags, true, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_msync(&mmap, 0, 4096);
    assert(rc == 0);
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);

    rc = sc_mmap_init(&mmap, "/tmp/x.txt", wflags, mwflags, true, 0, 8192);
    assert(rc == 0);
    assert(mmap.len == 8192);
    p = mmap.ptr;
    *p = 'x';
    rc = sc_mmap_msync(&mmap, 0, 4096);
    assert(rc == 0);
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);
    rc = sc_mmap_init(&mmap, "/tmp/x.txt", rflags, mrflags, true, 0, 4095);
    assert(rc == 0);
    p = mmap.ptr;
    assert(*p == 'x');
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);

    rc = sc_mmap_init(&mmap, "/tmp/x.txt", rflags, mwflags, true, 0, 4095);
    assert(rc == -1);
}

int main(int argc, char *argv[])
{
    test1();

    return 0;
}
