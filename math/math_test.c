


#include "sc_math.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void test1()
{
    uint64_t x;
    char *t;
    char buf[32];

    assert(sc_math_min(199, 299) == 199);
    assert(sc_math_max(199, 299) == 299);
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

    x = sc_math_size_to_bytes("1gx");
    assert(x == -1);

    x = sc_math_size_to_bytes("gx");
    assert(x == -1);

    x = sc_math_size_to_bytes("1xgx");
    assert(x == -1);


    x = sc_math_size_to_bytes("1xb");
    assert(x == -1);

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

#ifdef SC_HAVE_WRAP

bool fail_snprintf;

int __wrap_snprintf(char *str, size_t size, const char *format, ...)
{
    int rc;
    va_list va;

    if (!fail_snprintf) {
        va_start(va, format);
        rc = vsnprintf(str, size, format, va);
        va_end(va);

        return rc;
    }

    return -1;
}

void fail_test()
{
    char* t;
    char buf[32];

    fail_snprintf = true;
    t = sc_math_bytes_to_size(buf, sizeof(buf), 2 * 1024);
    assert(t == NULL);
    fail_snprintf = false;
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

