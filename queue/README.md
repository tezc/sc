# Generic queue

#### Overview

- Type generic queue which grows when you add elements.
- Add/remove from head/tail is possible so it can be used as list, stack,  
  queue, dequeue etc.
- Single allocation for all the data.
- Keeps separate first and last element indexes, when you remove an element,  
  it doesn't move elements to fill the space.


##### Usage


```c
#include "sc_queue.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    int *queue;
    int elem;

    sc_queue_create(queue, 0);

    sc_queue_add_last(queue, 2);
    sc_queue_add_last(queue, 3);
    sc_queue_add_last(queue, 4);
    sc_queue_add_first(queue, 1);

    sc_queue_foreach (queue, elem) {
        printf("elem = [%d] \n", elem);
    }

    elem = sc_queue_del_last(queue);
    printf("Last element was : [%d] \n", elem);

    elem = sc_queue_del_first(queue);
    printf("First element was : [%d] \n", elem);

    sc_queue_destroy(queue);

    return 0;
}
```

##### Note

Queue pointer is not stable, it may change if it expands the memory. If you  
pass the queue to another function which can add items, do it by passing  
reference of the queue pointer.

```c
void some_function_to_add_elems(long **q)
{
    sc_queue_add_last(*q, 500);
}

int main(int argc, char *argv[])
{
    long *q;

    sc_queue_create(q, 0);
    sc_queue_add_last(q, 300);
    
    some_function_to_add_elems(&q);
    sc_array_destroy(q);
}
```