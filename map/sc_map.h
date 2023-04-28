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
#define sc_map_free free
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
		bool oom;                                                      \
		bool found;                                                    \
	};                                                                     \
                                                                               \
	/**                                                                    \
	 * Create map                                                          \
	 *                                                                     \
	 * @param map map                                                      \
	 * @param cap initial capacity, zero is accepted                       \
	 * @param load_factor must be >25 and <95. Pass 0 for default value.   \
	 * @return 'true' on success,                                          \
	 *         'false' on out of memory or if 'load_factor' value is       \
	 *          invalid.                                                   \
	 */                                                                    \
	bool sc_map_init_##name(struct sc_map_##name *map, uint32_t cap,       \
				uint32_t load_factor);                         \
                                                                               \
	/**                                                                    \
	 * Destroy map.                                                        \
	 *                                                                     \
	 * @param map map                                                      \
	 */                                                                    \
	void sc_map_term_##name(struct sc_map_##name *map);                    \
                                                                               \
	/**                                                                    \
	 * Get map element count                                               \
	 *                                                                     \
	 * @param map map                                                      \
	 * @return element count                                               \
	 */                                                                    \
	uint32_t sc_map_size_##name(struct sc_map_##name *map);                \
                                                                               \
	/**                                                                    \
	 * Clear map                                                           \
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
	 * @return previous value if exists                                    \
	 *         call sc_map_found() to see if the returned value is valid.  \
	 */                                                                    \
	V sc_map_put_##name(struct sc_map_##name *map, K key, V val);          \
                                                                               \
	/**                                                                    \
	 * Get element                                                         \
	 *                                                                     \
	 * @param map map                                                      \
	 * @param K key                                                        \
	 * @return current value if exists.                                    \
	 *         call sc_map_found() to see if the returned value is valid.  \
	 */                                                                    \
	/** NOLINTNEXTLINE */                                                  \
	V sc_map_get_##name(struct sc_map_##name *map, K key);                 \
                                                                               \
	/**                                                                    \
	 * Delete element                                                      \
	 *                                                                     \
	 * @param map map                                                      \
	 * @param K key                                                        \
	 * @return current value if exists.                                    \
	 *         call sc_map_found() to see if the returned value is valid.  \
	 */                                                                    \
	/** NOLINTNEXTLINE */                                                  \
	V sc_map_del_##name(struct sc_map_##name *map, K key);

/**
 * @param map map
 * @return    - if put operation overrides a value, returns true
 *            - if get operation finds the key, returns true
 *            - if del operation deletes a key, returns true
 */
#define sc_map_found(map) ((map)->found)

/**
 * @param map map
 * @return    true if put operation failed with out of memory
 */
#define sc_map_oom(map) ((map)->oom)

// clang-format off

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
#define sc_map_foreach(map, K, V)                                                  \
	for (int64_t _i = -1, _b = 0; !_b && _i < (map)->cap; _i++)                \
		for ((V) = (map)->mem[_i].value, (K) = (map)->mem[_i].key, _b = 1; \
		     _b && ((_i == -1 && (map)->used) || (K) != 0) ? 1 : (_b = 0); \
		     _b = 0)

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
#define sc_map_foreach_key(map, K)                                                 \
	for (int64_t _i = -1, _b = 0; !_b && _i < (map)->cap; _i++)                \
		for ((K) = (map)->mem[_i].key, _b = 1;                             \
		     _b && ((_i == -1 && (map)->used) || (K) != 0) ? 1 : (_b = 0); \
		     _b = 0)

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
#define sc_map_foreach_value(map, V)                                                              \
	for (int64_t _i = -1, _b = 0; !_b && _i < (map)->cap; _i++)                               \
		for ((V) = (map)->mem[_i].value, _b = 1;                                          \
		     _b && ((_i == -1 && (map)->used) || (map)->mem[_i].key != 0) ? 1 : (_b = 0); \
		     _b = 0)

// integer keys: name  key type      value type
sc_map_dec_scalar(int, int,          int)
sc_map_dec_scalar(intv,int,          void*)
sc_map_dec_scalar(ints,int,          const char*)
sc_map_dec_scalar(ll,  long long,    long long)
sc_map_dec_scalar(llv, long long,    void *)
sc_map_dec_scalar(lls, long long,    const char *)
sc_map_dec_scalar(32,  uint32_t,     uint32_t)
sc_map_dec_scalar(64,  uint64_t,     uint64_t)
sc_map_dec_scalar(64v, uint64_t,     void *)
sc_map_dec_scalar(64s, uint64_t,     const char *)

// string keys:  name  key type      value type
sc_map_dec_strkey(str, const char *, const char *)
sc_map_dec_strkey(sv,  const char *, void*)
sc_map_dec_strkey(s64, const char *, uint64_t)
sc_map_dec_strkey(sll, const char *, long long)

// clang-format on

#endif
