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

    elem = sc_queue_remove_last(queue);
    printf("Last element was : [%d] \n", elem);

    elem = sc_queue_remove_first(queue);
    printf("First element was : [%d] \n", elem);

    sc_queue_destroy(queue);

    return 0;
}
