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

#include "sc_map.h"

#include <string.h>

#ifndef SC_MAP_MAX
#define SC_MAP_MAX UINT32_MAX
#endif

#define sc_map_def_strkey(name, K, V, cmp, hash_fn)                            \
	bool sc_map_cmp_##name(struct sc_map_item_##name *t, K key,            \
			       uint32_t hash)                                  \
	{                                                                      \
		return t->hash == hash && cmp(t->key, key);                    \
	}                                                                      \
                                                                               \
	void sc_map_assign_##name(struct sc_map_item_##name *t, K key,         \
				  V value, uint32_t hash)                      \
	{                                                                      \
		t->key = key;                                                  \
		t->value = value;                                              \
		t->hash = hash;                                                \
	}                                                                      \
                                                                               \
	uint32_t sc_map_hashof_##name(struct sc_map_item_##name *t)            \
	{                                                                      \
		return t->hash;                                                \
	}                                                                      \
                                                                               \
	sc_map_def(name, K, V, cmp, hash_fn)

#define sc_map_def_scalar(name, K, V, cmp, hash_fn)                            \
	bool sc_map_cmp_##name(struct sc_map_item_##name *t, K key,            \
			       uint32_t hash)                                  \
	{                                                                      \
		(void) hash;                                                   \
		return cmp(t->key, key);                                       \
	}                                                                      \
                                                                               \
	void sc_map_assign_##name(struct sc_map_item_##name *t, K key,         \
				  V value, uint32_t hash)                      \
	{                                                                      \
		(void) hash;                                                   \
		t->key = key;                                                  \
		t->value = value;                                              \
	}                                                                      \
                                                                               \
	uint32_t sc_map_hashof_##name(struct sc_map_item_##name *t)            \
	{                                                                      \
		return hash_fn(t->key);                                        \
	}                                                                      \
                                                                               \
	sc_map_def(name, K, V, cmp, hash_fn)

#define sc_map_def(name, K, V, cmp, hash_fn)                                   \
                                                                               \
	static const struct sc_map_item_##name empty_items_##name[2];          \
                                                                               \
	static const struct sc_map_##name sc_map_empty_##name = {              \
		.cap = 1,                                                      \
		.mem = (struct sc_map_item_##name *) &empty_items_##name[1]};  \
                                                                               \
	static void *sc_map_alloc_##name(uint32_t *cap, uint32_t factor)       \
	{                                                                      \
		uint32_t v = *cap;                                             \
		struct sc_map_item_##name *t;                                  \
                                                                               \
		if (*cap > SC_MAP_MAX / factor) {                              \
			return NULL;                                           \
		}                                                              \
                                                                               \
		/* Find next power of two */                                   \
		v = v < 8 ? 8 : (v * factor);                                  \
		v--;                                                           \
		for (uint32_t i = 1; i < sizeof(v) * 8; i *= 2) {              \
			v |= v >> i;                                           \
		}                                                              \
		v++;                                                           \
                                                                               \
		*cap = v;                                                      \
		t = sc_map_calloc(sizeof(*t), v + 1);                          \
		return t ? &t[1] : NULL;                                       \
	}                                                                      \
                                                                               \
	bool sc_map_init_##name(struct sc_map_##name *m, uint32_t cap,         \
				uint32_t load_fac)                             \
	{                                                                      \
		void *t;                                                       \
		uint32_t f = (load_fac == 0) ? 75 : load_fac;                  \
                                                                               \
		if (f > 95 || f < 25) {                                        \
			return false;                                          \
		}                                                              \
                                                                               \
		if (cap == 0) {                                                \
			*m = sc_map_empty_##name;                              \
			m->load_fac = f;                                       \
			return true;                                           \
		}                                                              \
                                                                               \
		t = sc_map_alloc_##name(&cap, 1);                              \
		if (t == NULL) {                                               \
			return false;                                          \
		}                                                              \
                                                                               \
		m->mem = t;                                                    \
		m->size = 0;                                                   \
		m->used = false;                                               \
		m->cap = cap;                                                  \
		m->load_fac = f;                                               \
		m->remap = (uint32_t) (m->cap * ((double) m->load_fac / 100)); \
                                                                               \
		return true;                                                   \
	}                                                                      \
                                                                               \
	void sc_map_term_##name(struct sc_map_##name *m)                       \
	{                                                                      \
		if (m->mem != sc_map_empty_##name.mem) {                       \
			sc_map_free(&m->mem[-1]);                              \
			*m = sc_map_empty_##name;                              \
		}                                                              \
	}                                                                      \
                                                                               \
	uint32_t sc_map_size_##name(struct sc_map_##name *m)                   \
	{                                                                      \
		return m->size;                                                \
	}                                                                      \
                                                                               \
	void sc_map_clear_##name(struct sc_map_##name *m)                      \
	{                                                                      \
		if (m->size > 0) {                                             \
			for (uint32_t i = 0; i < m->cap; i++) {                \
				m->mem[i].key = 0;                             \
			}                                                      \
                                                                               \
			m->used = false;                                       \
			m->size = 0;                                           \
		}                                                              \
	}                                                                      \
                                                                               \
	static bool sc_map_remap_##name(struct sc_map_##name *m)               \
	{                                                                      \
		uint32_t pos, cap, mod;                                        \
		struct sc_map_item_##name *new;                                \
                                                                               \
		if (m->size < m->remap) {                                      \
			return true;                                           \
		}                                                              \
                                                                               \
		cap = m->cap;                                                  \
		new = sc_map_alloc_##name(&cap, 2);                            \
		if (new == NULL) {                                             \
			return false;                                          \
		}                                                              \
                                                                               \
		mod = cap - 1;                                                 \
                                                                               \
		for (uint32_t i = 0; i < m->cap; i++) {                        \
			if (m->mem[i].key != 0) {                              \
				pos = sc_map_hashof_##name(&m->mem[i]) & mod;  \
                                                                               \
				while (true) {                                 \
					if (new[pos].key == 0) {               \
						new[pos] = m->mem[i];          \
						break;                         \
					}                                      \
                                                                               \
					pos = (pos + 1) & (mod);               \
				}                                              \
			}                                                      \
		}                                                              \
                                                                               \
		if (m->mem != sc_map_empty_##name.mem) {                       \
			new[-1] = m->mem[-1];                                  \
			sc_map_free(&m->mem[-1]);                              \
		}                                                              \
                                                                               \
		m->mem = new;                                                  \
		m->cap = cap;                                                  \
		m->remap = (uint32_t) (m->cap * ((double) m->load_fac / 100)); \
                                                                               \
		return true;                                                   \
	}                                                                      \
                                                                               \
	V sc_map_put_##name(struct sc_map_##name *m, K key, V value)           \
	{                                                                      \
		V ret;                                                         \
		uint32_t pos, mod, h;                                          \
                                                                               \
		m->oom = false;                                                \
                                                                               \
		if (!sc_map_remap_##name(m)) {                                 \
			m->oom = true;                                         \
			return 0;                                              \
		}                                                              \
                                                                               \
		if (key == 0) {                                                \
			ret = (m->used) ? m->mem[-1].value : 0;                \
			m->found = m->used;                                    \
			m->size += !m->used;                                   \
			m->used = true;                                        \
			m->mem[-1].value = value;                              \
                                                                               \
			return ret;                                            \
		}                                                              \
                                                                               \
		mod = m->cap - 1;                                              \
		h = hash_fn(key);                                              \
		pos = h & (mod);                                               \
                                                                               \
		while (true) {                                                 \
			if (m->mem[pos].key == 0) {                            \
				m->size++;                                     \
			} else if (!sc_map_cmp_##name(&m->mem[pos], key, h)) { \
				pos = (pos + 1) & (mod);                       \
				continue;                                      \
			}                                                      \
                                                                               \
			m->found = m->mem[pos].key != 0;                       \
			ret = m->found ? m->mem[pos].value : 0;                \
			sc_map_assign_##name(&m->mem[pos], key, value, h);     \
                                                                               \
			return ret;                                            \
		}                                                              \
	}                                                                      \
                                                                               \
	/** NOLINTNEXTLINE */                                                  \
	V sc_map_get_##name(struct sc_map_##name *m, K key)                    \
	{                                                                      \
		const uint32_t mod = m->cap - 1;                               \
		uint32_t h, pos;                                               \
                                                                               \
		if (key == 0) {                                                \
			m->found = m->used;                                    \
			return m->used ? m->mem[-1].value : 0;                 \
		}                                                              \
                                                                               \
		h = hash_fn(key);                                              \
		pos = h & mod;                                                 \
                                                                               \
		while (true) {                                                 \
			if (m->mem[pos].key == 0) {                            \
				m->found = false;                              \
				return 0;                                      \
			} else if (!sc_map_cmp_##name(&m->mem[pos], key, h)) { \
				pos = (pos + 1) & (mod);                       \
				continue;                                      \
			}                                                      \
                                                                               \
			m->found = true;                                       \
			return m->mem[pos].value;                              \
		}                                                              \
	}                                                                      \
                                                                               \
	/** NOLINTNEXTLINE */                                                  \
	V sc_map_del_##name(struct sc_map_##name *m, K key)                    \
	{                                                                      \
		const uint32_t mod = m->cap - 1;                               \
		uint32_t pos, prev, it, p, h;                                  \
		V ret;                                                         \
                                                                               \
		if (key == 0) {                                                \
			m->found = m->used;                                    \
			m->size -= m->used;                                    \
			m->used = false;                                       \
                                                                               \
			return m->found ? m->mem[-1].value : 0;                \
		}                                                              \
                                                                               \
		h = hash_fn(key);                                              \
		pos = h & (mod);                                               \
                                                                               \
		while (true) {                                                 \
			if (m->mem[pos].key == 0) {                            \
				m->found = false;                              \
				return 0;                                      \
			} else if (!sc_map_cmp_##name(&m->mem[pos], key, h)) { \
				pos = (pos + 1) & (mod);                       \
				continue;                                      \
			}                                                      \
                                                                               \
			m->found = true;                                       \
			ret = m->mem[pos].value;                               \
                                                                               \
			m->size--;                                             \
			m->mem[pos].key = 0;                                   \
			prev = pos;                                            \
			it = pos;                                              \
                                                                               \
			while (true) {                                         \
				it = (it + 1) & (mod);                         \
				if (m->mem[it].key == 0) {                     \
					break;                                 \
				}                                              \
                                                                               \
				p = sc_map_hashof_##name(&m->mem[it]) & (mod); \
                                                                               \
				if ((p > it && (p <= prev || it >= prev)) ||   \
				    (p <= prev && it >= prev)) {               \
                                                                               \
					m->mem[prev] = m->mem[it];             \
					m->mem[it].key = 0;                    \
					prev = it;                             \
				}                                              \
			}                                                      \
                                                                               \
			return ret;                                            \
		}                                                              \
	}

static uint32_t sc_map_hash_32(uint32_t a)
{
	return a;
}

static uint32_t sc_map_hash_64(uint64_t a)
{
	return ((uint32_t) a) ^ (uint32_t) (a >> 32u);
}

// clang-format off
uint32_t murmurhash(const char *key)
{
	const uint64_t m = UINT64_C(0xc6a4a7935bd1e995);
	const size_t len = strlen(key);
	const unsigned char* p = (const unsigned char*) key;
	const unsigned char *end = p + (len & ~(uint64_t) 0x7);
	uint64_t h = (len * m);

	while (p != end) {
		uint64_t k;
		memcpy(&k, p, sizeof(k));

		k *= m;
		k ^= k >> 47u;
		k *= m;

		h ^= k;
		h *= m;
		p += 8;
	}

	switch (len & 7u) {
	case 7: h ^= (uint64_t) p[6] << 48ul; // fall through
	case 6: h ^= (uint64_t) p[5] << 40ul; // fall through
	case 5: h ^= (uint64_t) p[4] << 32ul; // fall through
	case 4: h ^= (uint64_t) p[3] << 24ul; // fall through
	case 3: h ^= (uint64_t) p[2] << 16ul; // fall through
	case 2: h ^= (uint64_t) p[1] << 8ul;  // fall through
	case 1: h ^= (uint64_t) p[0];         // fall through
		h *= m;
	default:
		break;
	};

	h ^= h >> 47u;
	h *= m;
	h ^= h >> 47u;

	return (uint32_t) h;
}

#define sc_map_eq(a, b) ((a) == (b))
#define sc_map_streq(a, b) (!strcmp(a, b))

//              name,  key type,   value type,     cmp           hash
sc_map_def_scalar(32,  uint32_t,     uint32_t,     sc_map_eq,    sc_map_hash_32)
sc_map_def_scalar(64,  uint64_t,     uint64_t,     sc_map_eq,    sc_map_hash_64)
sc_map_def_scalar(64v, uint64_t,     void *,       sc_map_eq,    sc_map_hash_64)
sc_map_def_scalar(64s, uint64_t,     const char *, sc_map_eq,    sc_map_hash_64)
sc_map_def_strkey(str, const char *, const char *, sc_map_streq, murmurhash)
sc_map_def_strkey(sv,  const char *, void *,       sc_map_streq, murmurhash)
sc_map_def_strkey(s64, const char *, uint64_t,     sc_map_streq, murmurhash)

// clang-format on
