# Hashmap

### Overview 

- Open addressing hashmap with linear probing. 
- Just copy <b>sc_map.h</b> and <b>sc_map.c</b> to your project.

#### Usage


```c
#include "sc_map.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    char *key, *value;
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

####Internals
##### Memory
- Entries are kept in a single array without additional bookkeeping data. 

- Single allocation for all the data. Underlying array size is always power   
  of two, so there are some overhead for allocated but yet not used entries.  
  e.g (If you have 9 entries, underlying array has capacity for 16 entries)

##### Performance
- Hashmap is basically a map of 'uint64_t' keys to 'void*' values.  

- An entry is 16 bytes (64 bit systems), as this is an open addressing hashmap,  
linear probing with 16 bytes entries plays nicely with cache lines and   
hardware prefetcher. 

- It is intentionally not a 'generic' structure. Storing small key value pairs  
 provides really good performance.