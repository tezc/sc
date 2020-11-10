# Length prefixed string

Length prefixed C strings, length is at the start of the allocated memory  

    e.g :
    -----------------------------------------------
    | 0 | 0 | 0 | 4 | 'T' | 'E' | 'S' | 'T' | '\0'|
    -----------------------------------------------
                    ^
                  return
    User can keep pointer to first character, so it's like C style strings with
    additional functionality when it's used with these functions here.

##### Pros
- User gets a null terminated `char*`, so it still works with c style string  
  functions, e.g printf, strcmp.
- Faster length access and copy.
- Provides a few more functions to make easier create/append/trim/substring  
  operations.

##### Cons
- 4 bytes fixed overhead per string.

##### Memory
- 4 bytes fixed overhead per string.

##### Performance
- Faster length access and copy.
- When you create/set/append a string new memory is allocated. If you are  
  modifying strings a lot, consider using buffer-like implementation for that if  
  performance is critical for your use-case. I modify strings rarely but access  
  a lot (copy/move etc.), so ease of use and read/copy/move performance was  
  primary goal for this implementation.
  
  
```c


```