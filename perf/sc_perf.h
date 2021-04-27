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

#ifndef SC_PERF_H
#define SC_PERF_H

#include <linux/perf_event.h>
#include <stdint.h>

#define SC_PERF_VERSION "2.0.0"

#define SC_PERF_HW_CACHE(CACHE, OP, RESULT)                                    \
	((PERF_COUNT_HW_CACHE_##CACHE) | (PERF_COUNT_HW_CACHE_OP_##OP << 8u) | \
	 (PERF_COUNT_HW_CACHE_RESULT_##RESULT << 16u))

struct sc_perf_event {
	char *name;
	uint64_t type;
	uint64_t config;
};

// clang-format off
static const struct sc_perf_event sc_perf_hw[] = {
        {"cpu-clock",               PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK                  },
        {"task-clock",              PERF_TYPE_SOFTWARE, PERF_COUNT_SW_TASK_CLOCK                 },
        {"page-faults",             PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS                },
        {"context-switches",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES           },
        {"cpu-migrations",          PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_MIGRATIONS             },
        {"page-fault-minor",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MIN            },
     // {"page-fault-major",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS_MAJ            },
     // {"alignment-faults",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ALIGNMENT_FAULTS           },
     // {"emulation-faults",        PERF_TYPE_SOFTWARE, PERF_COUNT_SW_EMULATION_FAULTS           },

        {"cpu-cycles",              PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES                 },
        {"instructions",            PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS               },
     // {"cache-references",        PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES           },
        {"cache-misses",            PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES               },
     // {"branch-instructions",     PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS        },
     // {"branch-misses",           PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES              },
     // {"bus-cycles",              PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES                 },
     // {"stalled-cycles-frontend", PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND    },
     // {"stalled-cycles-backend",  PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND     },
     // {"ref-cpu-cycles",          PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES             },

     // {"L1D-read-access",         PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1D, READ, ACCESS)      },
        {"L1D-read-miss",           PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1D, READ, MISS)        },
     // {"L1D-write-access",        PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1D, WRITE, ACCESS)     },
     // {"L1D-write-miss",          PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1D, WRITE, MISS)       },
     // {"L1D-prefetch-access",     PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1D, PREFETCH, ACCESS)  },
     // {"L1D-prefetch-miss",       PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1D, PREFETCH, MISS)    },
     // {"L1I-read-access",         PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1I, READ, ACCESS)      },
        {"L1I-read-miss",           PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1I, READ, MISS)        },
     // {"L1I-write-access",        PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1I, WRITE, ACCESS)     },
     // {"L1I-write-miss",          PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1I, WRITE, MISS)       },
     // {"L1I-prefetch-access",     PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1I, PREFETCH, ACCESS)  },
     // {"L1I-prefetch-miss",       PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(L1I, PREFETCH, MISS)    },
     // {"LL-read-access",          PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(LL, READ, ACCESS)       },
     // {"LL-read-miss",            PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(LL, READ, MISS)         },
     // {"LL-write-access",         PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(LL, WRITE, ACCESS)      },
     // {"LL-write-miss",           PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(LL, WRITE, MISS)        },
     // {"LL-prefetch-access",      PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(LL, PREFETCH, ACCESS)   },
     // {"LL-prefetch-miss",        PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(LL, PREFETCH, MISS)     },
     // {"DTLB-read-access",        PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(DTLB, READ, ACCESS)     },
     // {"DTLB-read-miss",          PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(DTLB, READ, MISS)       },
     // {"DTLB-write-access",       PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(DTLB, WRITE, ACCESS)    },
     // {"DTLB-write-miss",         PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(DTLB, WRITE, MISS)      },
     // {"DTLB-prefetch-access",    PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(DTLB, PREFETCH, ACCESS) },
     // {"DTLB-prefetch-miss",      PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(DTLB, PREFETCH, MISS)   },
     // {"ITLB-read-access",        PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(ITLB, READ, ACCESS)     },
     // {"ITLB-read-miss",          PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(ITLB, READ, MISS)       },
     // {"ITLB-write-access",       PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(ITLB, WRITE, ACCESS)    },
     // {"ITLB-write-miss",         PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(ITLB, WRITE, MISS)      },
     // {"ITLB-prefetch-access",    PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(ITLB, PREFETCH, ACCESS) },
     // {"ITLB-prefetch-miss",      PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(ITLB, PREFETCH, MISS)   },
     // {"BPU-read-access",         PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(BPU, READ, ACCESS)      },
     // {"BPU-read-miss",           PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(BPU, READ, MISS)        },
     // {"BPU-write-access",        PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(BPU, WRITE, ACCESS)     },
     // {"BPU-write-miss",          PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(BPU, WRITE, MISS)       },
     // {"BPU-prefetch-access",     PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(BPU, PREFETCH, ACCESS)  },
     // {"BPU-prefetch-miss",       PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(BPU, PREFETCH, MISS)    },
     // {"NODE-read-access",        PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(NODE, READ, ACCESS)     },
     // {"NODE-read-miss",          PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(NODE, READ, MISS)       },
     // {"NODE-write-access",       PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(NODE, WRITE, ACCESS)    },
     // {"NODE-write-miss",         PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(NODE, WRITE, MISS)      },
     // {"NODE-prefetch-access",    PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(NODE, PREFETCH, ACCESS) },
     // {"NODE-prefetch-miss",      PERF_TYPE_HW_CACHE, SC_PERF_HW_CACHE(NODE, PREFETCH, MISS)   },
};

// clang-format on

void sc_perf_start();
void sc_perf_pause();
void sc_perf_end();

#endif
