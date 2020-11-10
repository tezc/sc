/*
 * MIT License
 *
 * Copyright (c) 2020 Ozan Tezcan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "sc_perf.h"

#include <sys/prctl.h>

#define ITEMS_SIZE (sizeof(sc_perf_hw) / sizeof(struct sc_perf_event))

static int initialized = 0;
static int running = 0;
static uint64_t total = 0;
static uint64_t start = 0;

struct sc_perf_item
{
    struct sc_perf_event event;
    double value;
    double active;
    int fd;
};


static struct sc_perf_item sc_perf_items[ITEMS_SIZE];

#define sc_perf_assert(val)                                                    \
    do {                                                                       \
        if (!(val)) {                                                          \
            fprintf(stderr, "%s:%d: error", __FILE__, __LINE__);               \
            if (errno) {                                                       \
                fprintf(stderr, " (%s)", strerror(errno));                     \
            }                                                                  \
            abort();                                                           \
        }                                                                      \
    } while (0)

static void sc_perf_set(struct sc_perf_item *items, size_t size)
{
    const uint64_t flags =
            PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
    int fd;

    for (int i = 0; i < size; i++) {
        struct perf_event_attr p = {.size = sizeof(struct perf_event_attr),
                                     .read_format = flags,
                                     .type = items[i].event.type,
                                     .config = items[i].event.config,
                                     .disabled = 1,
                                     .inherit = 1,
                                     .inherit_stat = 0,
                                     .exclude_kernel = false,
                                     .exclude_hv = false};

        fd = syscall(__NR_perf_event_open, &p, 0, -1, -1, PERF_FLAG_FD_CLOEXEC);
        if (fd == -1) {
            fprintf(stderr,
                    "Failed to set counter : %s , probably your system does "
                    "not support it! \n",
                    items[i].event.name);
            abort();
        }

        items[i].fd = fd;
    }
}

static void sc_read(struct sc_perf_item *items, size_t size)
{
    struct read_format
    {
        uint64_t value;
        uint64_t time_enabled;
        uint64_t time_running;
    } fmt;

    for (int i = 0; i < size; i++) {
        double n = 1.0;

        sc_perf_assert(read(items[i].fd, &fmt, sizeof(fmt)) == sizeof(fmt));

        if (fmt.time_enabled > 0 && fmt.time_running > 0) {
            n = (double) fmt.time_running / (double) fmt.time_enabled;
            items[i].active = n;
        }

        items[i].value += fmt.value * n;
    }
}

static void sc_perf_clear(void)
{
    total = 0;
    start = 0;
    running = 0;
    initialized = 0;

    for (int i = 0; i < ITEMS_SIZE; i++) {
        sc_perf_items[i].event = sc_perf_hw[i];
        sc_perf_items[i].value = 0;
        sc_perf_items[i].active = 0;
        sc_perf_items[i].fd = -1;
    }
}

static uint64_t sy_time_nano(void)
{
    int rc;
    struct timespec ts;

    rc = clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    if (rc == -1) {
        abort();
    }

    return ((uint64_t)(ts.tv_nsec + (ts.tv_sec * 1000 * 1000 * 1000)));
}

void sc_perf_start(void)
{
    if (!initialized) {
        sc_perf_clear();
        sc_perf_set(sc_perf_items, ITEMS_SIZE);
        initialized = 1;
    }

    sc_perf_assert(prctl(PR_TASK_PERF_EVENTS_ENABLE) == 0);

    start = sy_time_nano();
    running = 1;
}

void sc_perf_pause(void)
{
    sc_perf_assert(initialized);

    if (!running) {
        return;
    }

    sc_perf_assert(prctl(PR_TASK_PERF_EVENTS_DISABLE) == 0);

    total += sy_time_nano() - start;
    running = 0;
}

void sc_perf_end(void)
{
    sc_perf_assert(initialized);

    sc_perf_pause();
    sc_read(sc_perf_items, ITEMS_SIZE);

    for (int i = 0; i < ITEMS_SIZE; i++) {
        close(sc_perf_items[i].fd);
    }

    printf("\n| %-25s | %-18s | %s  \n", "Event", "Value", "Measurement time");
    printf("---------------------------------------------------------------\n");
    printf("| %-25s | %-18.2f | %s  \n", "time (seconds)",
           ((double) total / 1e9), "(100,00%)");

    for (int i = 0; i < ITEMS_SIZE; i++) {
        printf("| %-25s | %-18.2f | (%.2f%%)  \n", sc_perf_items[i].event.name,
               sc_perf_items[i].value, sc_perf_items[i].active * 100);
    }

    sc_perf_clear();
}
