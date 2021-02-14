### Overview

Common C libraries and data structures. (C99)  

Each folder is stand-alone and contains a single .h .c pair.   
There is no build, copy .h .c files you want.

Libraries are portable, see [test](#test) section for details.  

As a general rule, all libraries report errors back to users (e.g out of memory).

### List

| Library                        | Description                                                                                |
|--------------------------------|--------------------------------------------------------------------------------------------|
| **[array](array)**             | Generic array/vector                                                                       |
| **[buffer](buffer)**           | Buffer for encoding/decoding variables, best fit for protocol/serialization implementations|
| **[condition](condition)**     | Condition wrapper for Posix and Windows                                                    |
| **[crc32](crc32)**             | Crc32c, uses crc32c CPU instruction if available                                           |
| **[heap](heap)**               | Min heap which can be used as max heap/priority queue as well                              | 
| **[ini](ini)**                 | Ini parser                                                                                 |
| **[linked list](linked-list)** | Intrusive linked list                                                                      |
| **[logger](logger)**           | Logger                                                                                     |
| **[map](map)**                 | A high performance open addressing hashmap                                                 |
| **[memory map](memory-map)**   | Mmap wrapper for Posix and Windows                                                         |
| **[mutex](mutex)**             | Mutex wrapper for Posix and Windows                                                        |
| **[option](option)**           | Cmdline argument parser. Very basic one                                                    |
| **[perf](perf)**               | Benchmark utility to get performance counters info via perf_event_open()                   | 
| **[queue](queue)**             | Generic queue which can be used as dequeue/stack/list as well                              |
| **[sc](sc)**                   | Utility functions                                                                          |
| **[signal](signal)**           | Signal safe snprintf & Signal handler (handling CTRL+C, printing backtrace on crash etc)   |
| **[socket](socket)**           | Pipe / tcp sockets(also unix domain sockets) /Epoll/Kqueue/WSAPoll for Posix and Windows   |
| **[string](string)**           | Length prefixed, null terminated C strings.                                                |
| **[thread](thread)**           | Thread wrapper for Posix and Windows.                                                      |
| **[time](time)**               | Time and sleep functions for Posix and Windows                                             |
| **[timer](timer)**             | Hashed timing wheel implementation with fast poll / cancel ops                             |
| **[uri](uri)**                 | A basic uri parser                                                                         |

### Test
[![codecov](https://codecov.io/gh/tezc/sc/branch/master/graph/badge.svg?token=O8ZHQ0XZ30)](https://codecov.io/gh/tezc/sc)

Although I use on Linux mostly, CI runs with

<pre>
OS         : Linux, MacOS, FreeBSD and Windows  
Compilers  : GCC, Clang, MSVC  
Arch       : x64, aarch64, armv6, armv7, ppc64le, s390x  
Sanitizers : valgrind and clang/gcc sanitizers(address, undefined, thread)
</pre>

To run all tests :
<pre>
#with valgrind
mkdir build; cd build;
cmake .. && make && make valgrind

#with address sanitizer
mkdir build; cd build;
cmake .. -DSANITIZER=address && make && make check

#with undefined sanitizer
mkdir build; cd build;
cmake .. -DSANITIZER=undefined && make && make check

#coverage, requires GCC and lcov
mkdir build; cd build;
cmake .. -DCMAKE_BUILD_TYPE=Coverage; make; make coverage

</pre>

### Using with cmake FetchContent
```cmake
FetchContent_Declare(
  sc_lib
  GIT_REPOSITORY https://github.com/tezc/sc.git
  GIT_TAG        master)
SET(SC_BUILD_TEST OFF CACHE BOOL "Turn off sc_lib tests" FORCE)

FetchContent_MakeAvailable(sc_lib)

add_executable(
  myapp
  myapp.c
)

target_link_libraries(
  myapp
  sc_str
)
```
