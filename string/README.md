### Length prefixed string

Length prefixed C strings, length is at the start of the allocated memory

    e.g :
    -----------------------------------------------
    | 0 | 0 | 0 | 4 | 'T' | 'E' | 'S' | 'T' | '\0'|
    -----------------------------------------------
                    ^
                  return
    User can keep pointer to first character, so it's like C style strings with
    additional functionality.

### Pros
- User gets a null terminated `char*`, so it still works with c style string  
  functions, e.g printf, strcmp.
- This implementation is mostly about avoiding strlen() cost.  
  Provides a few more functions to make easier create/append/trim/substring  
  operations.

### Cons
- 4 bytes fixed overhead per string and max string size is ~4gb.
- When you create/set/append a string, new memory is allocated. If you are  
  modifying strings a lot, consider using buffer-like implementation for that if  
  performance is critical for your use-case. I modify strings rarely but read  
  a lot (copy/move etc.).
  
```c
#include "sc_str.h"

#include <stdio.h>

int main(int argc, char *argv[])
{
    char* s1;

    s1 = sc_str_create("*-hello-*");
    printf("%s \n", s1); // prints *-hello-*

    sc_str_trim(&s1, "*-");
    printf("%s \n", s1); // prints hello

    sc_str_append_fmt(&s1, "%d", 2);
    printf("%s \n", s1); // prints hello2

    sc_str_replace(&s1, "2", " world!");
    printf("%s \n", s1); // prints hello world!

    sc_str_substring(&s1, 0, 5);
    printf("%s \n", s1); // prints hello

    sc_str_destroy(&s1);

    return 0;
}

```