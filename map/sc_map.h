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

#ifndef SC_MAP_H
#define SC_MAP_H

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define sc_map_of_strkey(name, K, V)                                           \
    struct sc_map_item_##name                                                  \
    {                                                                          \
        K key;                                                                 \
        V value;                                                               \
        uint32_t hash;                                                         \
    };                                                                         \
                                                                               \
    sc_map_of(name, K, V)

#define sc_map_of_scalar(name, K, V)                                           \
    struct sc_map_item_##name                                                  \
    {                                                                          \
        K key;                                                                 \
        V value;                                                               \
    };                                                                         \
                                                                               \
    sc_map_of(name, K, V)

#define sc_map_of(name, K, V)                                                  \
    struct sc_map_##name                                                       \
    {                                                                          \
        struct sc_map_item_##name *mem;                                        \
        uint32_t cap;                                                          \
        uint32_t size;                                                         \
        uint32_t load_factor;                                                  \
        uint32_t remap;                                                        \
        V value;                                                               \
        bool used;                                                             \
    };                                                                         \
                                                                               \
    bool sc_map_init_##name(struct sc_map_##name *map, uint32_t cap,           \
                            uint32_t load_factor);                             \
    void sc_map_term_##name(struct sc_map_##name *map);                        \
    uint32_t sc_map_size_##name(struct sc_map_##name *map);                    \
    void sc_map_clear_##name(struct sc_map_##name *map);                       \
    bool sc_map_put_##name(struct sc_map_##name *map, K key, V val);           \
    bool sc_map_get_##name(struct sc_map_##name *map, K key, V *value);        \
    bool sc_map_del_##name(struct sc_map_##name *map, K key, V *value);

#define sc_map_foreach(map, K, V)                                              \
    for (uint32_t __i = 0, __b = 0; __i < (map)->cap; __i++)                   \
        for ((V) = (map)->mem[__i].value, (K) = (map)->mem[__i].key, __b = 1;  \
             __b && (V) != 0; __b = 0)

#define sc_map_foreach_key(map, K)                                             \
    for (uint32_t __i = 0, __b = 0; __i < (map)->cap; __i++)                   \
        for ((K) = (map)->mem[__i].key, __b = 1; __b && (K) != 0; __b = 0)

#define sc_map_foreach_value(map, V)                                           \
    for (uint32_t __i = 0, __b = 0; __i < (map)->cap; __i++)                   \
        for ((V) = (map)->mem[__i].value, __b = 1; __b && (V) != 0; __b = 0)

#define sc_map_calloc calloc
#define sc_map_free   free

// clang-format off

//              name  key type  value type);
sc_map_of_scalar(32,  uint32_t, uint32_t)
sc_map_of_scalar(64,  uint64_t, uint64_t)
sc_map_of_scalar(64v, uint64_t, void *)
sc_map_of_scalar(64s, uint64_t, char *)
sc_map_of_strkey(str, char *,   char *)
sc_map_of_strkey(sv,  char *,   void*)
sc_map_of_strkey(s64, char *,   uint64_t)

// clang-format on

/**
* If you want to log or abort on errors like out of memory,
* put your error function here. It will be called with printf like error msg.
*
* my_on_error(const char* fmt, ...);
*/
#define sc_map_on_error(...)

#endif
