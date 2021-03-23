### Linked List

### Overview

- Intrusive doubly linked list.
- Basically, same as adding next and prev pointers to your structs.
- Add/remove from head/tail is possible so it can be used as list, stack,  
  queue, dequeue etc.

### Usage


```c
#include "sc_list.h"

#include <stdio.h>


int main()
{
    struct user
    {
        char *name;
        struct sc_list next;
    };

    struct user users[] = {{"first", {0}},
                           {"second", {0}},
                           {"third", {0}},
                           {"fourth", {0}},
                           {"fifth", {0}}};

    struct sc_list list;

    sc_list_init(&list);
    

    for (int i = 0; i < 5; i++) {
        sc_list_init(&users[i].next);
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