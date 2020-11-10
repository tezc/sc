# Linked List

### Overview 

- Intrusive doubly linked-list implementation.
- Basically, same as adding next and prev pointers to your structs.
- Add/remove from head/tail is possible so it can be used as list, stack,  
  queue, dequeue etc.
- Just copy <b><i>sc_list.h</i></b> and <b><i>sc_list.c</i></b> to your project.


#### Memory

- No heap memory allocation.

#### Performance

- Good fit if you already have structs allocated in memory and willing to put  
  them into a list without making extra allocations.

#### Usage


```c

#include "sc_list.h"

#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
    struct user
    {
        char *name;
        struct sc_list next;
    };

    struct user users[] = {{"first"},
                           {"second"},
                           {"third"},
                           {"fourth"},
                           {"fifth"}};

    struct sc_list list;

    sc_list_init(&list);

    for (int i = 0; i < 5; i++) {
        sc_list_add_tail(&list, &users[i].next);
    }

    struct sc_list *it;
    struct user *user;

    sc_list_foreach (&list, it) {
        user = sc_list_entry(it, struct user, next);
        printf("%s \n", user->name);
    }

    return 0;
}


```