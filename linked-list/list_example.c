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
