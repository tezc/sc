#include "sc_signal.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void test1()
{
    char tmp[128] = "";

    sc_signal_snprintf(tmp, 0, "%s", "test");
    assert(strcmp(tmp, "") == 0);

    sc_signal_snprintf(tmp, sizeof(tmp), "%s", "test");
    assert(strcmp(tmp, "test") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%s", NULL);
    assert(strcmp(tmp, "(null)") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%d", -3);
    assert(strcmp(tmp, "-3") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%u", 3);
    assert(strcmp(tmp, "3") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%ld", -100000000000000l);
    assert(strcmp(tmp, "-100000000000000") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%lld", -1000000000000000000ll);
    assert(strcmp(tmp, "-1000000000000000000") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%lu", 100000000000000l);
    assert(strcmp(tmp, "100000000000000") == 0);
    sc_signal_snprintf(tmp, sizeof(tmp), "%llu", 1000000000000000000ll);
    assert(strcmp(tmp, "1000000000000000000") == 0);

    char* x = (char*)0xabcdef;
    sc_signal_snprintf(tmp, sizeof(tmp), "%p", x);
    assert(strcmp(tmp, "0xabcdef") == 0);

    sc_signal_snprintf(tmp, sizeof(tmp), "%%p", x);
    assert(strcmp(tmp, "%p") == 0);

    assert(sc_signal_snprintf(tmp, sizeof(tmp), "%c", 3) == -1);
    assert(sc_signal_snprintf(tmp, sizeof(tmp), "%llx", 3) == -1);
    assert(sc_signal_snprintf(tmp, sizeof(tmp), "%lx", 3) == -1);

    sc_signal_log(STDOUT_FILENO, tmp, sizeof(tmp), "%s", "test");
}

void test2()
{
    assert(sc_signal_init() == 0);
}


int main()
{


    test1();
    test2();

    return 0;
}
