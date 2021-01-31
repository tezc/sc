#include "sc_map.h"

#include <string.h>

#ifndef SC_SIZE_MAX
    #define SC_SIZE_MAX UINT32_MAX
#endif

#define sc_map_impl_of_strkey(name, K, V, cmp, hash_fn)                        \
    bool sc_map_cmp_##name(struct sc_map_item_##name *t, K key, uint32_t hash) \
    {                                                                          \
        return t->hash == hash && cmp(t->key, key);                            \
    }                                                                          \
                                                                               \
    void sc_map_assign_##name(struct sc_map_item_##name *t, K key, V value,    \
                              uint32_t hash)                                   \
    {                                                                          \
        t->key = key;                                                          \
        t->value = value;                                                      \
        t->hash = hash;                                                        \
    }                                                                          \
                                                                               \
    uint32_t sc_map_hashof_##name(struct sc_map_item_##name *t)                \
    {                                                                          \
        return t->hash;                                                        \
    }                                                                          \
                                                                               \
    sc_map_impl_of(name, K, V, cmp, hash_fn)

#define sc_map_impl_of_scalar(name, K, V, cmp, hash_fn)                        \
    bool sc_map_cmp_##name(struct sc_map_item_##name *t, K key, uint32_t hash) \
    {                                                                          \
        return cmp(t->key, key);                                               \
    }                                                                          \
                                                                               \
    void sc_map_assign_##name(struct sc_map_item_##name *t, K key, V value,    \
                              uint32_t hash)                                   \
    {                                                                          \
        t->key = key;                                                          \
        t->value = value;                                                      \
    }                                                                          \
                                                                               \
    uint32_t sc_map_hashof_##name(struct sc_map_item_##name *t)                \
    {                                                                          \
        return hash_fn(t->key);                                                \
    }                                                                          \
                                                                               \
    sc_map_impl_of(name, K, V, cmp, hash_fn)

#define sc_map_impl_of(name, K, V, cmp, hash_fn)                               \
                                                                               \
    static const struct sc_map_item_##name empty_items_##name[2];              \
                                                                               \
    static const struct sc_map_##name sc_map_empty_##name = {                  \
            .cap = 1,                                                          \
            .mem = (struct sc_map_item_##name *) &empty_items_##name[1]};      \
                                                                               \
    static void *sc_map_alloc_##name(uint32_t *cap, uint32_t factor)           \
    {                                                                          \
        uint32_t v = *cap;                                                     \
        struct sc_map_item_##name *t;                                          \
                                                                               \
        if (*cap > SC_SIZE_MAX / factor) {                                     \
            return NULL;                                                       \
        }                                                                      \
                                                                               \
        /* Find next power of two */                                           \
        v = v < 8 ? 8 : (v * factor);                                          \
        v--;                                                                   \
        for (uint32_t i = 1; i < sizeof(v) * 8; i *= 2) {                      \
            v |= v >> i;                                                       \
        }                                                                      \
        v++;                                                                   \
                                                                               \
        *cap = v;                                                              \
        t = sc_map_calloc(sizeof(*t), v + 1);                                  \
        return t ? &t[1] : NULL;                                               \
    }                                                                          \
                                                                               \
    bool sc_map_init_##name(struct sc_map_##name *map, uint32_t cap,           \
                            uint32_t load_factor)                              \
    {                                                                          \
        void *t;                                                               \
        uint32_t f = (load_factor == 0) ? 75 : load_factor;                    \
                                                                               \
        if (f > 95 || f < 25) {                                                \
            return false;                                                      \
        }                                                                      \
                                                                               \
        if (cap == 0) {                                                        \
            *map = sc_map_empty_##name;                                        \
            map->load_factor = f;                                              \
            return true;                                                       \
        }                                                                      \
                                                                               \
        t = sc_map_alloc_##name(&cap, 1);                                      \
        if (t == NULL) {                                                       \
            return false;                                                      \
        }                                                                      \
                                                                               \
        map->mem = t;                                                          \
        map->size = 0;                                                         \
        map->used = false;                                                     \
        map->cap = cap;                                                        \
        map->load_factor = f;                                                  \
        map->remap = (uint32_t)(map->cap * ((double) map->load_factor / 100)); \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    void sc_map_term_##name(struct sc_map_##name *map)                         \
    {                                                                          \
        if (map->mem != sc_map_empty_##name.mem) {                             \
            sc_map_free(&map->mem[-1]);                                        \
        }                                                                      \
    }                                                                          \
                                                                               \
    uint32_t sc_map_size_##name(struct sc_map_##name *map)                     \
    {                                                                          \
        return map->size;                                                      \
    }                                                                          \
                                                                               \
    void sc_map_clear_##name(struct sc_map_##name *map)                        \
    {                                                                          \
        if (map->size > 0) {                                                   \
            for (uint32_t i = 0; i < map->cap; i++) {                          \
                map->mem[i].key = 0;                                           \
            }                                                                  \
                                                                               \
            map->used = false;                                                 \
            map->size = 0;                                                     \
        }                                                                      \
    }                                                                          \
                                                                               \
    static bool sc_map_remap_##name(struct sc_map_##name *map)                 \
    {                                                                          \
        uint32_t pos, cap, mod;                                                \
        struct sc_map_item_##name *new;                                        \
                                                                               \
        if (map->size < map->remap) {                                          \
            return true;                                                       \
        }                                                                      \
                                                                               \
        cap = map->cap;                                                        \
        new = sc_map_alloc_##name(&cap, 2);                                    \
        if (new == NULL) {                                                     \
            return false;                                                      \
        }                                                                      \
                                                                               \
        mod = cap - 1;                                                         \
                                                                               \
        for (uint32_t i = 0; i < map->cap; i++) {                              \
            if (map->mem[i].key != 0) {                                        \
                pos = sc_map_hashof_##name(&map->mem[i]) & (mod);              \
                                                                               \
                while (true) {                                                 \
                    if (new[pos].key == 0) {                                   \
                        new[pos] = map->mem[i];                                \
                        break;                                                 \
                    }                                                          \
                                                                               \
                    pos = (pos + 1) & (mod);                                   \
                }                                                              \
            }                                                                  \
        }                                                                      \
                                                                               \
        if (map->mem != sc_map_empty_##name.mem) {                             \
            new[-1] = map->mem[-1];                                            \
            sc_map_free(&map->mem[-1]);                                        \
        }                                                                      \
                                                                               \
        map->mem = new;                                                        \
        map->cap = cap;                                                        \
        map->remap = (uint32_t)(map->cap * ((double) map->load_factor / 100)); \
                                                                               \
        return true;                                                           \
    }                                                                          \
                                                                               \
    bool sc_map_put_##name(struct sc_map_##name *map, K key, V value)          \
    {                                                                          \
        uint32_t pos, mod, hash;                                               \
                                                                               \
        if (!sc_map_remap_##name(map)) {                                       \
            return false;                                                      \
        }                                                                      \
                                                                               \
        if (key == 0) {                                                        \
            map->size += !map->used;                                           \
            map->used = true;                                                  \
            map->mem[-1].value = value;                                        \
                                                                               \
            return true;                                                       \
        }                                                                      \
                                                                               \
        mod = map->cap - 1;                                                    \
        hash = hash_fn(key);                                                   \
        pos = hash & (mod);                                                    \
                                                                               \
        while (true) {                                                         \
            if (map->mem[pos].key == 0) {                                      \
                map->size++;                                                   \
            } else if (sc_map_cmp_##name(&map->mem[pos], key, hash) != true) { \
                pos = (pos + 1) & (mod);                                       \
                continue;                                                      \
            }                                                                  \
                                                                               \
            sc_map_assign_##name(&map->mem[pos], key, value, hash);            \
            return true;                                                       \
        }                                                                      \
    }                                                                          \
                                                                               \
    bool sc_map_get_##name(struct sc_map_##name *map, K key, V *value)         \
    {                                                                          \
        const uint32_t mod = map->cap - 1;                                     \
        uint32_t hash, pos;                                                    \
                                                                               \
        if (key == 0) {                                                        \
            *value = map->mem[-1].value;                                       \
            return map->used;                                                  \
        }                                                                      \
                                                                               \
        hash = hash_fn(key);                                                   \
        pos = hash & mod;                                                      \
                                                                               \
        while (true) {                                                         \
            if (map->mem[pos].key == 0) {                                      \
                return false;                                                  \
            } else if (sc_map_cmp_##name(&map->mem[pos], key, hash) != true) { \
                pos = (pos + 1) & (mod);                                       \
                continue;                                                      \
            }                                                                  \
                                                                               \
            *value = map->mem[pos].value;                                      \
            return true;                                                       \
        }                                                                      \
    }                                                                          \
                                                                               \
    bool sc_map_del_##name(struct sc_map_##name *map, K key, V *value)         \
    {                                                                          \
        const uint32_t mod = map->cap - 1;                                     \
        uint32_t pos, prev_elem, curr, curr_orig, hash;                        \
                                                                               \
        if (key == 0) {                                                        \
            bool ret = map->used;                                              \
            map->size -= map->used;                                            \
            map->used = false;                                                 \
                                                                               \
            if (value != NULL) {                                               \
                *value = map->mem[-1].value;                                   \
            }                                                                  \
                                                                               \
            return ret;                                                        \
        }                                                                      \
                                                                               \
        hash = hash_fn(key);                                                   \
        pos = hash & (mod);                                                    \
                                                                               \
        while (true) {                                                         \
            if (map->mem[pos].key == 0) {                                      \
                return false;                                                  \
            } else if (sc_map_cmp_##name(&map->mem[pos], key, hash) != true) { \
                pos = (pos + 1) & (mod);                                       \
                continue;                                                      \
            }                                                                  \
                                                                               \
            if (value != NULL) {                                               \
                *value = map->mem[pos].value;                                  \
            }                                                                  \
                                                                               \
            map->size--;                                                       \
            map->mem[pos].key = 0;                                             \
            prev_elem = pos;                                                   \
            curr = pos;                                                        \
                                                                               \
            while (true) {                                                     \
                curr = (curr + 1) & (mod);                                     \
                if (map->mem[curr].key == 0) {                                 \
                    break;                                                     \
                }                                                              \
                                                                               \
                curr_orig = sc_map_hashof_##name(&map->mem[curr]) & (mod);     \
                                                                               \
                if ((curr_orig > curr &&                                       \
                     (curr_orig <= prev_elem || curr >= prev_elem)) ||         \
                    (curr_orig <= prev_elem && curr >= prev_elem)) {           \
                                                                               \
                    map->mem[prev_elem] = map->mem[curr];                      \
                    map->mem[curr].key = 0;                                    \
                    prev_elem = curr;                                          \
                }                                                              \
            }                                                                  \
                                                                               \
            return true;                                                       \
        }                                                                      \
    }


static uint32_t sc_map_hash_32(uint32_t a)
{
    return a;
}

static uint32_t sc_map_hash_64(uint64_t a)
{
    return ((uint32_t) a) ^ (uint32_t)(a >> 32u);
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
    case 7: h ^= (uint64_t) p[6] << 48ul;
    case 6: h ^= (uint64_t) p[5] << 40ul;
    case 5: h ^= (uint64_t) p[4] << 32ul;
    case 4: h ^= (uint64_t) p[3] << 24ul;
    case 3: h ^= (uint64_t) p[2] << 16ul;
    case 2: h ^= (uint64_t) p[1] << 8ul;
    case 1: h ^= (uint64_t) p[0];
        h *= m;
    default:
        break;
    };

    h ^= h >> 47u;
    h *= m;
    h ^= h >> 47u;

    return (uint32_t) h;
}

// clang-format off

#define sc_map_varcmp(a, b) ((a) == (b))
#define sc_map_strcmp(a, b) (!strcmp(a, b))

//                   name, key type,     value type,        cmp           hash
sc_map_impl_of_scalar(32,  uint32_t,     uint32_t,     sc_map_varcmp, sc_map_hash_32)
sc_map_impl_of_scalar(64,  uint64_t,     uint64_t,     sc_map_varcmp, sc_map_hash_64)
sc_map_impl_of_scalar(64v, uint64_t,     void *,       sc_map_varcmp, sc_map_hash_64)
sc_map_impl_of_scalar(64s, uint64_t,     const char *, sc_map_varcmp, sc_map_hash_64)
sc_map_impl_of_strkey(str, const char *, const char *, sc_map_strcmp, murmurhash)
sc_map_impl_of_strkey(sv,  const char *, void *,       sc_map_strcmp, murmurhash)
sc_map_impl_of_strkey(s64, const char *, uint64_t,     sc_map_strcmp, murmurhash)

// clang-format on
