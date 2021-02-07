### Overview

Common data structures and utilities for C.  

Each folder is independent and contains a header and a source file.  
There is no build for libraries, just copy *.h *.c files into your project.  
Each folder also contains a readme and an example.  

CI runs on Linux, MacOS, FreeBSD and Windows with gcc, clang and msvc.


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
| **[signal](signal)**           | Signal handler & signal safe snprintf (handling CTRL+C, printing backtrace on crash etc)   |
| **[socket](socket)**           | Pipe / tcp sockets(also unix domain sockets) /Epoll/Kqueue/WSAPoll for Posix and Windows   |
| **[string](string)**           | Length prefixed, null terminated C strings.                                                |
| **[thread](thread)**           | Thread wrapper for Posix and Windows.                                                      |
| **[time](time)**               | Time and sleep functions for Posix and Windows                                             |
| **[timer](timer)**             | Hashed timing wheel implementation with fast poll / cancel ops                             |
| **[uri](uri)**                 | A basic uri parser                                                                         |

### Test

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

#coverage
mkdir build; cd build;
cmake .. -DCMAKE_BUILD_TYPE=Coverage; make; make coverage

</pre>

