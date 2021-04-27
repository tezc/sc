/*
 * BSD-3-Clause
 *
 * Copyright 2021 Ozan Tezcan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "sc_perf.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define ITEMS_SIZE (sizeof(sc_perf_hw) / sizeof(struct sc_perf_event))

static int init = 0;
static int running = 0;
static uint64_t total = 0;
static uint64_t start = 0;

struct sc_perf_item {
	struct sc_perf_event event;
	double value;
	double active;
	int fd;
};

static struct sc_perf_item sc_perf_items[ITEMS_SIZE];

#define sc_perf_assert(val)                                                    \
	do {                                                                   \
		if (!(val)) {                                                  \
			fprintf(stderr, "%s:%d: error", __FILE__, __LINE__);   \
			if (errno) {                                           \
				fprintf(stderr, " (%s)", strerror(errno));     \
			}                                                      \
			abort();                                               \
		}                                                              \
	} while (0)

static void sc_perf_set(struct sc_perf_item *items, size_t size)
{
	const uint64_t flags = PERF_FORMAT_TOTAL_TIME_ENABLED |
			       PERF_FORMAT_TOTAL_TIME_RUNNING;
	int fd;

	for (size_t i = 0; i < size; i++) {
		struct perf_event_attr p = {
			.size = sizeof(struct perf_event_attr),
			.read_format = flags,
			.type = items[i].event.type,
			.config = items[i].event.config,
			.disabled = 1,
			.inherit = 1,
			.inherit_stat = 0,
			.exclude_kernel = false,
			.exclude_hv = false};

		fd = syscall(__NR_perf_event_open, &p, 0, -1, -1,
			     PERF_FLAG_FD_CLOEXEC);
		if (fd == -1) {
			fprintf(stderr,
				"Failed to set counter : %s , probably your "
				"system does "
				"not support it! \n",
				items[i].event.name);
			abort();
		}

		items[i].fd = fd;
	}
}

static void sc_read(struct sc_perf_item *items, size_t size)
{
	struct read_format {
		uint64_t value;
		uint64_t time_enabled;
		uint64_t time_running;
	} fmt;

	for (size_t i = 0; i < size; i++) {
		double n = 1.0;

		sc_perf_assert(read(items[i].fd, &fmt, sizeof(fmt)) ==
			       sizeof(fmt));

		if (fmt.time_enabled > 0 && fmt.time_running > 0) {
			n = (double) fmt.time_running /
			    (double) fmt.time_enabled;
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
	init = 0;

	for (size_t i = 0; i < ITEMS_SIZE; i++) {
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

	return ((uint64_t) (ts.tv_nsec + (ts.tv_sec * 1000 * 1000 * 1000)));
}

void sc_perf_start(void)
{
	if (!init) {
		sc_perf_clear();
		sc_perf_set(sc_perf_items, ITEMS_SIZE);
		init = 1;
	}

	sc_perf_assert(prctl(PR_TASK_PERF_EVENTS_ENABLE) == 0);

	start = sy_time_nano();
	running = 1;
}

void sc_perf_pause(void)
{
	sc_perf_assert(init);

	if (!running) {
		return;
	}

	sc_perf_assert(prctl(PR_TASK_PERF_EVENTS_DISABLE) == 0);

	total += sy_time_nano() - start;
	running = 0;
}

void sc_perf_end(void)
{
	sc_perf_assert(init);

	sc_perf_pause();
	sc_read(sc_perf_items, ITEMS_SIZE);

	for (size_t i = 0; i < ITEMS_SIZE; i++) {
		close(sc_perf_items[i].fd);
	}

	printf("\n| %-25s | %-18s | %s  \n", "Event", "Value",
	       "Measurement time");
	printf("---------------------------------------------------------------"
	       "\n");
	printf("| %-25s | %-18.2f | %s  \n", "time (seconds)",
	       ((double) total / 1e9), "(100,00%)");

	for (size_t i = 0; i < ITEMS_SIZE; i++) {
		printf("| %-25s | %-18.2f | (%.2f%%)  \n",
		       sc_perf_items[i].event.name, sc_perf_items[i].value,
		       sc_perf_items[i].active * 100);
	}

	sc_perf_clear();
}
