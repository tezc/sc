## Overview

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![codecov](https://codecov.io/gh/tezc/sc/branch/master/graph/badge.svg?token=O8ZHQ0XZ30)](https://codecov.io/gh/tezc/sc)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/tezc/sc.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/tezc/sc/alerts/)

Portable, stand-alone C libraries and data structures. (C99)

Each folder is stand-alone with a single header/source pair in it.  
There is no build for libraries, just copy files you want.  
e.g If you want logger, copy sc_log.h and sc_log.c to your project.

I use on Linux mostly but libraries are portable, CI runs on

<pre>
OS         : Linux, MacOS, FreeBSD and Windows  
Compilers  : GCC, Clang, MSVC  
Arch       : x64, aarch64, armv6(32 bit), armv7(32 bit), ppc64le, s390x(big endian)  
Sanitizers : valgrind and clang/gcc sanitizers(address, undefined, thread)
</pre>

## List

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
