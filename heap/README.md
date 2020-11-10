# Heap

### Overview 

- Min-heap implementation, it can be used as Max-heap/priority queue as well.
- Just copy <b>sc_heap.h</b> and <b>sc_heap.c</b> to your project.

#### Usage


```c

#include "sc_heap.h"

#include <stdio.h>


int main(int argc, char *argv[])
{
    struct data
    {
        int priority;
        char *data;
    };

    struct data n[] = {{1, "first"},
                       {4, "fourth"},
                       {5, "fifth"},
                       {3, "third"},
                       {2, "second"}};

    int64_t key;
    void *data;
    struct sc_heap heap;

    sc_heap_init(&heap, 0);

    // Min-heap usage
    for (int i = 0; i < 5; i++) {
        sc_heap_add(&heap, n[i].priority, n[i].data);
    }

    while (sc_heap_pop(&heap, &key, &data)) {
        printf("key = %ld, data = %s \n", key, (char *) data);
    }
    printf("---------------- \n");

    /**
     * Max-heap usage, negate when adding into heap
     * and negate back after pop :
     */

    for (int i = 0; i < 5; i++) {
        sc_heap_add(&heap, -(n[i].priority), n[i].data);
    }

    while (sc_heap_pop(&heap, &key, &data)) {
        printf("key = %ld, data = %s \n", -key, (char *) data);
    }

    return 0;
}
```

####Internals
##### Memory

- All items are on a single contiguous memory.
- Lazy allocation is possible. (No memory allocation until the first item)

##### Performance

- Heaps are not the fastest data structures as they require unpredictable  
  branches on add/extract operations. Intentionally, only 'void*' values are  
  allowed, so this not a 'generic' data structure. Placing larger values only  
  will worsen this implementation's performance. An item is 16 bytes in the  
  heap and operating on a contiguous memory gives us acceptable performance  
  for some use cases.
