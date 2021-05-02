### Generic queue

### Overview

- Queue implementation which grows when you add elements.
- Add/remove from head/tail is possible so it can be used as list, stack,  
  queue, dequeue etc.
- It comes with predefined types, check out at the end of sc_queue.h, you can
  add there (sc_queue_def) if you need more.


### Usage


```c
#include "sc_queue.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
	int elem;
	struct sc_queue_int queue;

	sc_queue_init(&queue);
	
	sc_queue_add_last(&queue, 2);
	sc_queue_add_last(&queue, 3);
	sc_queue_add_last(&queue, 4);
	sc_queue_add_first(&queue, 1);

	sc_queue_foreach (&queue, elem) {
		printf("elem = [%d] \n", elem);
	}

	elem = sc_queue_del_last(&queue);
	printf("Last element was : [%d] \n", elem);

	elem = sc_queue_del_first(&queue);
	printf("First element was : [%d] \n", elem);

	sc_queue_term(&queue);
	return 0;
}
```
