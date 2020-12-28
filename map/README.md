# Hashmap

### Overview

- Open addressing hashmap with linear probing.
- Just copy <b>sc_map.h</b> and <b>sc_map.c</b> to your project.
- Key and value types can be integers(32bit/64bit) or pointers only. This is  
  required for good performance.
- Naming requires prefixes, it's ugly but macros are necessary to avoid  
  alternative solutions for copy/compare (function pointers, memcpy calls etc..)
- Other types can be added but must be scalar types, not structs.
- Comes with predefined key value pairs :
  
```
                  name  key type      value type
  sc_map_of_scalar(32,  uint32_t,     uint32_t)
  sc_map_of_scalar(64,  uint64_t,     uint64_t)
  sc_map_of_scalar(64v, uint64_t,     void *)
  sc_map_of_scalar(64s, uint64_t,     const char *)
  sc_map_of_strkey(str, const char *, const char *)
  sc_map_of_strkey(sv,  const char *, void*)
  sc_map_of_strkey(s64, const char *, uint64_t)
```

- This is a very fast hashmap. 
  - Single array allocation for all data. 
  - Linear probing over an array.
  - Deletion without tombstones
  - Macros generate functions in sc_map.h and sc_map.c, it doesn't inline  
    automatically. So, it doesn't bloat your binary. Inlining is upto compiler.

#### Usage


```c
#include "sc_map.h"

#include <stdio.h>

int main(int argc, char *argv[])
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

    return 0;
}
```
