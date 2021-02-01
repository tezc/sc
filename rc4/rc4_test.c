

#include "sc_rc4.h"

#include <assert.h>
#include <memory.h>

void test()
{
    unsigned char tmp[256] = {0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 1, 5, 3, 5, 5, 6};
    unsigned char tmp2[256] = {0, 1, 2, 3, 4, 3, 6, 6, 6, 6, 1, 2, 3, 5, 5, 6};

    unsigned char out1[256];
    unsigned char out2[256];

    struct sc_rc4 rc4_1;
    struct sc_rc4 rc4_2;

    sc_rc4_init(&rc4_1, tmp);
    sc_rc4_init(&rc4_2, tmp);

    sc_rc4_random(&rc4_1, out1, sizeof(out1));
    sc_rc4_random(&rc4_2, out2, sizeof(out2));

    assert(memcmp(out1, out2, sizeof(out1)) == 0);

    sc_rc4_init(&rc4_1, tmp);
    sc_rc4_init(&rc4_2, tmp2);

    sc_rc4_random(&rc4_1, out1, sizeof(out1));
    sc_rc4_random(&rc4_2, out2, sizeof(out2));

    assert(memcmp(out1, out2, sizeof(out1)) != 0);

    sc_rc4_random(&rc4_1, out1, 0);
    sc_rc4_random(&rc4_1, NULL, 0);
    sc_rc4_random(&rc4_1, NULL, 10);
}

int main(int argc, char *argv[])
{
    test();
    return 0;
}
