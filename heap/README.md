# Heap

### Overview

- Min-heap implementation, it can be used as Max-heap/priority queue as well.

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