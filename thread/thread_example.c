/*
 *  Copyright (C) 2020 Syncstack Authors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

uint64_t sc_time_mono_ns()
{
#if defined(_WIN32) || defined(_WIN64)
    static int64_t frequency = 0;
    if (frequency == 0) {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        assert(freq.QuadPart != 0);
        frequency = freq.QuadPart;
    }
    LARGE_INTEGER count;
    QueryPerformanceCounter(&count);
    return (uint64_t)(count.QuadPart * 1000000000) / frequency;
#else
    int rc;
    struct timespec ts;

    rc = clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(rc == 0);

    return ((uint64_t) ts.tv_sec * 1000000000 + (uint64_t) ts.tv_nsec);
#endif
}


int main(int argc, char *argv[])
{
    char buf[32];
    pthread_key_t key = 0;
    pthread_key_t key2 = 0;

    pthread_setname_np(pthread_self(), "fsadsa");
    pthread_setspecific(key, "hoooop");

    pthread_getspecific(key);

    for (int i =0 ; i < 1000; i++) {
        uint64_t t = sc_time_mono_ns();
        pthread_getname_np(pthread_self(), buf, 32);
        printf("%zu \n", sc_time_mono_ns() - t);
    }

    printf("%s \n", buf);
    return 0;
}
