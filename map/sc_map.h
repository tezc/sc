/*
 * MIT License
 *
 * Copyright (c) 2021 Ozan Tezcan
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

#define SC_MAP_VERSION "2.0.0"

#ifdef SC_HAVE_CONFIG_H
#include "config.h"
#else
#define sc_map_calloc calloc
#define sc_map_free   free
#endif

#define sc_map_dec_strkey(name, K, V)                                          \
	struct sc_map_item_##name {                                            \
		K key;                                                         \
		V value;                                                       \
		uint32_t hash;                                                 \
	};                                                                     \
                                                                               \
	sc_map_of(name, K, V)

#define sc_map_dec_scalar(name, K, V)                                          \
	struct sc_map_item_##name {                                            \
		K key;                                                         \
		V value;                                                       \
	};                                                                     \
                                                                               \
	sc_map_of(name, K, V)

#define sc_map_of(name, K, V)                                                  \
	struct sc_map_##name {                                                 \
		struct sc_map_item_##name *mem;                                \
		uint32_t cap;                                                  \
		uint32_t size;                                                 \
		uint32_t load_fac;                                             \
		uint32_t remap;                                                \
		bool used;                                                     \
	};                                                                     \
                                                                               \
	/**                                                                    \
	 * Create map                                                          \
	 *                                                                     \
	 * struct sc_map_str map;                                              \
	 * sc_map_init_str(&map, 0, 0);                                        \
	 *                                                                     \
	 * @param map map                                                      \
	 * @param cap initial capacity, zero is accepted                       \
	 * @param load_factor must be >25 and <95. Pass 0 for default value.   \
	 * @return 'true' on success,                                          \
	 *         'false' on out of memory or if 'load_factor' value is       \
	 * invalid.                                                            \
	 */                                                                    \
	bool sc_map_init_##name(struct sc_map_##name *map, uint32_t cap,       \
				uint32_t load_factor);                         \
                                                                               \
	/**                                                                    \
	 * Destroy map.                                                        \
	 *                                                                     \
	 * struct sc_map_str map;                                              \
	 * sc_map_term_str(&map);                                              \
	 *                                                                     \
	 * @param map map                                                      \
	 */                                                                    \
	void sc_map_term_##name(struct sc_map_##name *map);                    \
                                                                               \
	/**                                                                    \
	 * Get map element count                                               \
	 *                                                                     \
	 * struct sc_map_str map;                                              \
	 * uint32_t count = sc_map_size_str(&map);                             \
	 *                                                                     \
	 * @param map map                                                      \
	 * @return element count                                               \
	 */                                                                    \
	uint32_t sc_map_size_##name(struct sc_map_##name *map);                \
                                                                               \
	/**                                                                    \
	 * Clear map                                                           \
	 *                                                                     \
	 * struct sc_map_str map;                                              \
	 * sc_map_clear_str(&map);                                             \
	 *                                                                     \
	 * @param map map                                                      \
	 */                                                                    \
	void sc_map_clear_##name(struct sc_map_##name *map);                   \
                                                                               \
	/**                                                                    \
	 * Put element to the map                                              \
	 *                                                                     \
	 * struct sc_map_str map;                                              \
	 * sc_map_put_str(&map, "key", "value");                               \
	 *                                                                     \
	 * @param map map                                                      \
	 * @param K key                                                        \
	 * @param V value                                                      \
	 * @return 'true' on success, 'false' on out of memory.                \
	 */                                                                    \
	bool sc_map_put_##name(struct sc_map_##name *map, K key, V val);       \
                                                                               \
	/**                                                                    \
	 * Get element                                                         \
	 *                                                                     \
	 * bool found;                                                         \
	 * char *value;                                                        \
	 * struct sc_map_str map;                                              \
	 *                                                                     \
	 * found = sc_map_get_str(&map, "key", &value);                        \
	 *                                                                     \
	 * @param map map                                                      \
	 * @param K key                                                        \
	 * @param V pointer to put value, if key is missing, value is          \
	 * undefined                                                           \
	 * @return 'true' if key exists, 'false' otherwise                     \
	 */                                                                    \
	/** NOLINTNEXTLINE */                                                  \
	bool sc_map_get_##name(struct sc_map_##name *map, K key, V *val);      \
                                                                               \
	/**                                                                    \
	 * Delete element                                                      \
	 *                                                                     \
	 * bool found;                                                         \
	 * char *value;                                                        \
	 * struct sc_map_str map;                                              \
	 *                                                                     \
	 * found = sc_map_del_str(&map, "key", &value);                        \
	 *                                                                     \
	 * @param map map                                                      \
	 * @param K key                                                        \
	 * @param V pointer to put current value                               \
	 *          - if key does not exist, value is undefined                \
	 *          - Pass NULL if you don't want to get previous 'value'      \
	 * @return 'true' if key exists, 'false' otherwise                     \
	 */                                                                    \
	/** NOLINTNEXTLINE */                                                  \
	bool sc_map_del_##name(struct sc_map_##name *map, K key, V *val);

/**
 * Foreach loop
 *
 * char *key, *value;
 * struct sc_map_str map;
 *
 * sc_map_foreach(&map, key, value) {
 *      printf("key = %s, value = %s \n");
 * }
 */
#define sc_map_foreach(map, K, V)                                              \
	for (int64_t __i = -1, __b = 0; __i < (map)->cap; __i++)               \
		for ((V) = (map)->mem[__i].value, (K) = (map)->mem[__i].key,   \
		    __b = 1;                                                   \
		     __b && ((__i == -1 && (map)->used) || (K) != 0); __b = 0)

/**
 * Foreach loop for keys
 *
 * char *key;
 * struct sc_map_str map;
 *
 * sc_map_foreach_key(&map, key) {
 *      printf("key = %s \n");
 * }
 */
#define sc_map_foreach_key(map, K)                                             \
	for (int64_t __i = -1, __b = 0; __i < (map)->cap; __i++)               \
		for ((K) = (map)->mem[__i].key, __b = 1;                       \
		     __b && ((__i == -1 && (map)->used) || (K) != 0); __b = 0)

/**
 * Foreach loop for values
 *
 * char *value;
 * struct sc_map_str map;
 *
 * sc_map_foreach_value(&map, value) {
 *      printf("value = %s \n");
 * }
 */
#define sc_map_foreach_value(map, V)                                           \
	for (int64_t __i = -1, __b = 0; __i < (map)->cap; __i++)               \
		for ((V) = (map)->mem[__i].value, __b = 1;                     \
		     __b &&                                                    \
		     ((__i == -1 && (map)->used) || (map)->mem[__i].key != 0); \
		     __b = 0)

// clang-format off

//              name  key type      value type
sc_map_dec_scalar(32,  uint32_t,     uint32_t)
sc_map_dec_scalar(64,  uint64_t,     uint64_t)
sc_map_dec_scalar(64v, uint64_t,     void *)
sc_map_dec_scalar(64s, uint64_t,     const char *)
sc_map_dec_strkey(str, const char *, const char *)
sc_map_dec_strkey(sv,  const char *, void*)
sc_map_dec_strkey(s64, const char *, uint64_t)

// clang-format on

#endif
