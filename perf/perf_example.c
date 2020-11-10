#include "sc_perf.h"
#include <time.h>

int main(int argc, char *argv[])
{
    size_t total = 0;

    sc_perf_start();
    for (int i = 0; i < 100000000; i++) {
        total += (rand() % 331) ^ 33;
    }
    sc_perf_pause();

    for (int i = 0; i < 100000000; i++) {
        total += (rand() % 327) ^ 37;
    }

    sc_perf_end();

    return total;
}
