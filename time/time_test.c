#include "sc_time.h"

#include <assert.h>

void test1(void)
{
    assert(sc_time_ns() != 0);
    assert(sc_time_ms() != 0);

    for (int i = 0; i < 100000; i++) {
        uint64_t t1 = sc_time_mono_ms();
        uint64_t t2 = sc_time_mono_ms();

        assert(t2 >= t1);
    }

    for (int i = 0; i < 100000; i++) {
        uint64_t t1 = sc_time_mono_ns();
        uint64_t t2 = sc_time_mono_ns();

        assert(t2 >= t1);
    }
}

void test2(void)
{
    uint64_t t1, t2;

    t1 = sc_time_mono_ms();
    sc_time_sleep(1000);
    t2 = sc_time_mono_ms();

    assert(t2 > t1);

    t1 = sc_time_mono_ns();
    sc_time_sleep(1000);
    t2 = sc_time_mono_ns();

    assert(t2 > t1);
}

int main(int argc, char *argv[])
{
    test1();
    test2();
    return 0;
}
