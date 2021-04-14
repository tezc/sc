### INI parser

### Overview

- Ini file parser
- Based on : https://github.com/benhoyt/inih

### Features

### Comment example

```ini
#Comment
;Another comment

[Network]
#This is comment
hostname = github.com #Line comments start with space. Either " ;" or " #"  
```

### No section

Possible to use without sections

```ini
key1 = value1  ;Comment x
key2 = value2  ;Comment y
key3 = value3  #Comment z
```

```
- Item 1 : ""(Section), "key1"(Key), "value1"(Value)
- Item 2 : ""(Section), "key2"(Key), "value2"(Value)
- Item 3 : ""(Section), "key3"(Key), "value3"(Value)
```

### Multi-value

Values without keys in the next line will be reported as if belongs to
previous   
key. Those values should be indented at least with a single space character.

```ini
#Comment
;Another comment

[Network]
#This is comment
hostname = github.com
 github.io
 github.org 
```

```
- Item 1 : "Network"(Section), "hostname"(Key), "github.com"(Value)
- Item 2 : "Network"(Section), "hostname"(Key), "github.io"(Value)
- Item 3 : "Network"(Section), "hostname"(Key), "github.org"(Value)
```

### Usage

```c
#include "sc_ini.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

const char *example_ini = "# My configuration"
                            "[Network] \n"
                            "hostname = github.com \n"
                            "port = 443 \n"
                            "protocol = https \n"
                            "repo = any";

int callback(void *arg, int line, const char *section, const char *key,
             const char *value)
{
    printf("Line(%d), Section(%s), Key(%s), Value(%s) \n", line, section, key, 
           value);
    return 0;
}

void file_example(void)
{
    int rc;

    // Create example file.
    FILE *fp = fopen("my_config.ini", "w+");
    fwrite(example_ini, 1, strlen(example_ini), fp);
    fclose(fp);

    // Parse file
    rc = sc_ini_parse_file(NULL, callback, "my_config.ini");
    assert(rc == 0);
}

void string_example(void)
{
    int rc;
    
    rc = sc_ini_parse_string(NULL, callback, example_ini);
    assert(rc == 0);
}

int main(int argc, char *argv[])
{
    string_example();
    file_example();
}
```