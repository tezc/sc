### Hashmap

### Overview

- Open addressing hashmap with linear probing.
- Requires postfix naming, e.g. sc_map_str, sc_map_int. It's ugly but necessary  
  for good performance & type safety. 

- Comes with predefined key value pairs :

```
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
```

If you need more types, you can add at the end of sc_map.h and sc_map.c

- This is a very fast hashmap.
    - Single array allocation for all data.
    - Linear probing over an array.
    - Deletion without tombstones.
    - Macros generate functions in sc_map.c. So, inlining is upto the compiler.

### Note

Key and value types can be integers(32bit/64bit) or pointers only.  
Other types can be added but must be scalar types, not structs. This is a   
design decision, I don't remember when was the last time I wanted to store  
struct as a key or value. I use hashmap for fast look-ups and small key-value  
pairs with linear probing play well with cache lines and hardware-prefetcher.  
If you want to use structs anyway, you need to change the code a little.

### Usage

```c
#include "sc_map.h"

#include <stdio.h>

void example_str(void)
{
	const char *key, *value;
	struct sc_map_str map;

	sc_map_init_str(&map, 0, 0);

	sc_map_put_str(&map, "jack", "chicago");
	sc_map_put_str(&map, "jane", "new york");
	sc_map_put_str(&map, "janie", "atlanta");

	sc_map_foreach (&map, key, value) {
		printf("Key:[%s], Value:[%s] \n", key, value);
	}

	sc_map_term_str(&map);
}

void example_int_to_str(void)
{
	uint32_t key;
	const char *value;
	struct sc_map_ints map;

	sc_map_init_ints(&map, 0, 0);

	sc_map_put_ints(&map, 100, "chicago");
	sc_map_put_ints(&map, 200, "new york");
	sc_map_put_ints(&map, 300, "atlanta");

	value = sc_map_get_ints(&map, 200);
	if (sc_map_found(&map)) {
		printf("Found Value:[%s] \n", value);
	}

	value = sc_map_del_ints(&map, 100);
	if (sc_map_found(&map)) {
		printf("Deleted : %s \n", value);
	}

	sc_map_foreach (&map, key, value) {
		printf("Key:[%d], Value:[%s] \n", key, value);
	}

	value = sc_map_del_ints(&map, 200);
	if (sc_map_found(&map)) {
		printf("Found : %s \n", value);
	}

	value = sc_map_put_ints(&map, 300, "los angeles");
	if (sc_map_found(&map)) {
		printf("overridden : %s \n", value);
	}

	sc_map_term_ints(&map);
}

int main(void)
{
	example_str();
	example_int_to_str();

	return 0;
}
```
