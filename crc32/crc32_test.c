#include "sc_crc32.h"

#include <assert.h>

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
    
    uint32_t crc1, crc2, crc3;
    uint8_t buf[128] = {1, 1, 2, 3};
    uint8_t buf2[4096 * 8] = {2 , 5, 6 ,5};

    sc_crc32_init();

    crc1 = sc_crc32(0, buf, 100);
    crc2 = sc_crc32(crc1, buf + 100, 28);
    crc3 = sc_crc32(0, buf, 128);

    assert(crc2 == crc3);

    crc1 = sc_crc32(0, buf2, 4096 * 4);
    crc2 = sc_crc32(crc1, buf2 + (4096 * 4), 4096 * 4);
    crc3 = sc_crc32(0, buf2, 4096 * 8);

    assert(crc2 == crc3);

    crc1 = sc_crc32(100, buf, 8);
    crc2 = sc_crc32(100, buf, 7);
    assert(crc1 != crc2);

    crc2 = sc_crc32(100, buf + 7, 7);
    assert(crc1 != crc2);

    crc2 = sc_crc32(100, buf + 8, 7);
    assert(crc1 != crc2);

    crc2 = sc_crc32(100, buf + 8, 8);
    assert(crc1 != crc2);

    crc2 = sc_crc32(100, buf + 8, 0);
    assert(crc1 != crc2);
}
