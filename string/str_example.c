#include "sc_str.h"

#include <stdio.h>


int main(int argc, char *argv[])
{
    char* s1;

    s1 = sc_str_create("**hello**");
    printf("%s \n", s1); // prints **hello**

    sc_str_append_fmt(&s1, " %s", "world--");
    printf("%s \n", s1); // prints **hello**world--

    sc_str_trim(&s1, "*-");
    printf("%s \n", s1); // prints **hello**world--

    sc_str_substring(&s1, 6, 11);
    printf("%s \n", s1); // world

    sc_str_destroy(s1);

    return 0;
}
