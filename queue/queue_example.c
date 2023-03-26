#include "sc_queue.h"

#include <stdio.h>

void example_str(void)
{
	const char *elem;
	struct sc_queue_str queue;

	sc_queue_init(&queue);

	sc_queue_add_last(&queue, "one");
	sc_queue_add_last(&queue, "two");
	sc_queue_add_last(&queue, "three");

	sc_queue_foreach (&queue, elem) {
		printf("elem = [%s] \n", elem);
	}

	sc_queue_term(&queue);
}

void example_int(void)
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

}

int main(int argc, char *argv[])
{
	example_str();
	example_int();
	return 0;
}


