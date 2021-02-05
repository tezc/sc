### Overview

Stand-alone, portable C libraries. 

Each folder is independent and contains a header and a source file.  
There is no build, just copy paste *.h *.c file into your project.  



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
| **[map](map)**                 | A high performance hashmap                                                                 |
| **[math](math)**               | Utility functions                                                                          |
| **[memory map](memory-map)**   | Mmap wrapper for Posix and Windows                                                         |
| **[mutex](mutex)**             | Mutex wrapper for Posix and Windows                                                        |
| **[option](option)**           | Cmdline argument parser. Very basic one                                                    |
| **[perf](perf)**               | Benchmark utility to get performance counters info via perf_event_open()                   | 
| **[queue](queue)**             | Generic queue which can be used as dequeue/stack/list as well                              |
| **[rc4](rc4)**                 | Random number generator                                                                    |
| **[signal](signal)**           | Signal handler & signal safe snprintf (handling CTRL+C, printing backtrace on crash etc)   |
| **[socket](socket)**           | Pipe, TCP sockets, Epoll/Kqueue/WSAPoll for Posix and Windows                              |
| **[string](string)**           | Length prefixed, null terminated C strings.                                                |
| **[thread](thread)**           | Thread wrapper for Posix and Windows.                                                      |
| **[time](time)**               | Time and sleep functions for Posix and Windows                                             |
| **[timer](timer)**             | Hashed timer wheel implementation for fast poll / cancel ops                               |
| **[uri](uri)**                 | A basic uri parser                                                                         |

### Details
- Errors are reported to user. (e.g Out of memory, system call errors)
- Some

### Test

Although I use on Linux mostly, CI runs with some combination of 

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
cmake .. && make && make check

#with address sanitizer
mkdir build; cd build;
cmake .. -DSANITIZER=address && make && make check

#with undefined sanitizer
mkdir build; cd build;
cmake .. -DSANITIZER=undefined && make && make check
</pre>

