


#include "sc_math.h"

#include <assert.h>
#include <string.h>

void test1()
{
    uint64_t x;
    char *t;
    char buf[32];

    assert(sc_math_is_pow2(0) == false);

    x = sc_math_to_pow2(0);
    assert(x == 1);
    assert(sc_math_is_pow2(x) == true);
    x = sc_math_to_pow2(1);
    assert(x == 1);
    assert(sc_math_is_pow2(x) == true);
    x = sc_math_to_pow2(1023);
    assert(x == 1024);
    assert(sc_math_is_pow2(x) == true);

    x = sc_math_size_to_bytes("1");
    assert(x == 1);
    x = sc_math_size_to_bytes("1b");
    assert(x == 1);

    x = sc_math_size_to_bytes("1k");
    assert(x == 1 * 1024);
    x = sc_math_size_to_bytes("1kb");
    assert(x == 1 * 1024);

    x = sc_math_size_to_bytes("1m");
    assert(x == 1 * 1024 * 1024);
    x = sc_math_size_to_bytes("1mb");
    assert(x == 1 * 1024 * 1024);

    x = sc_math_size_to_bytes("1g");
    assert(x == 1 * 1024 * 1024 * 1024);
    x = sc_math_size_to_bytes("1gb");
    assert(x == 1 * 1024 * 1024 * 1024);

    x = sc_math_size_to_bytes("1p");
    assert(x == UINT64_C(1 * 1024 * 1024 * 1024 * 1024));
    x = sc_math_size_to_bytes("1pb");
    assert(x == UINT64_C(1 * 1024 * 1024 * 1024 * 1024));

    t = sc_math_bytes_to_size(buf, sizeof(buf), 1024);
    assert(strcmp(t, "1024.00 B") == 0);

    t = sc_math_bytes_to_size(buf, sizeof(buf), 2 * 1024);
    assert(strcmp(t, "2.00 KB") == 0);

    t = sc_math_bytes_to_size(buf, sizeof(buf), 2 * 1024 * 1024);
    assert(strcmp(t, "2.00 MB") == 0);

    t = sc_math_bytes_to_size(buf, sizeof(buf), (uint64_t)2 * 1024 * 1024 * 1024);
    assert(strcmp(t, "2.00 GB") == 0);

    t = sc_math_bytes_to_size(buf, sizeof(buf), (uint64_t)2 * 1024 * 1024 * 1024 * 1024);
    assert(strcmp(t, "2.00 TB") == 0);

    t = sc_math_bytes_to_size(buf, sizeof(buf), (uint64_t)2 * 1024 * 1024 * 1024 * 1024 * 1024);
    assert(strcmp(t, "2.00 PB") == 0);
}


int main()
{
    test1();

    return 0;
}

