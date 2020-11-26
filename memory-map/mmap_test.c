#include "sc_mmap.h"

#include <assert.h>
#include <stdio.h>


void test1()
{
    int rc;
    char* p;
    struct sc_mmap mmap;

    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC, PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_msync(&mmap, 0, 4096);
    assert(rc == 0);
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);

    rc = sc_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC, PROT_READ | PROT_WRITE, MAP_SHARED, 0, 8192);
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

    rc = sc_mmap_init(&mmap, "x.txt" ,O_RDWR | O_CREAT | O_TRUNC, PROT_READ | PROT_WRITE, MAP_SHARED, 3, 0);
    assert(rc == -1);

    rc = sc_mmap_init(&mmap, "x.txt" ,O_RDWR | O_CREAT | O_TRUNC, PROT_READ | PROT_WRITE, MAP_SHARED, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_mlock(&mmap, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_munlock(&mmap, 0, 4095);
    assert(rc == 0);
    rc = sc_mmap_term(&mmap);
    assert(rc == 0);
}

int main(int argc, char *argv[])
{
    test1();

    return 0;
}
