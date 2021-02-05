# Overview

- Stand-alone, portable C libraries. 
- Each folder includes a single .h .c pair which is independent of other files.
- There is no build, just copy paste *.h *.c file into your project. 
- Tested on Linux, MacOS, FreeBSD and Windows with GCC, Clang and MSVC.

# Details
d

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
| **[perf](perf)**               | Benchmark utility to get performance counters info                                         | 
| **[queue](queue)**             | Generic queue which can be used as dequeue/stack/list as well                              |
| **[rc4](rc4)**                 | Random number generator                                                                    |
| **[signal](signal)**           | Signal handler & signal safe snprintf (handling CTRL+C, printing backtrace on crash etc)   |
| **[socket](socket)**           | Pipe, TCP sockets, Epoll/Kqueue/WSAPoll for Posix and Windows                              |
| **[string](string)**           | Length prefixed, null terminated C strings.                                                |
| **[thread](thread)**           | Thread wrapper for Posix and Windows.                                                      |
| **[time](time)**               | Time and sleep functions for Posix and Windows                                             |
| **[timer](timer)**             | Hashed timer wheel implementation for fast poll / cancel ops                               |
| **[uri](uri)**                 | A basic uri parser                                                                         |

