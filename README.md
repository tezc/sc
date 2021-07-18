## Overview

[![License: BSD](https://img.shields.io/badge/License-BSD-blue.svg)](https://opensource.org/licenses/bsd-3-clause)
[![codecov](https://codecov.io/gh/tezc/sc/branch/master/graph/badge.svg?token=O8ZHQ0XZ30)](https://codecov.io/gh/tezc/sc)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/tezc/sc.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/tezc/sc/alerts/)

Portable, stand-alone C libraries and data structures. (C99)

Each folder is stand-alone with a single header/source pair in it. There is no  
build for libraries, just copy files you want. 

e.g If you want the logger, copy sc_log.h and sc_log.c to your project.

#### Features

- High performance & minimal memory usage
- Portability between many operating systems and architectures
- Tests with 100% branch coverage and multiple sanitizers
- Drag & drop source code distribution

### Test
There is 100% branch-coverage on Linux and CI runs on

<pre>
OS         : Linux, MacOS, FreeBSD and Windows  
Compilers  : GCC, Clang, MSVC  
Arch       : x64, aarch64, armv6(32 bit), armv7(32 bit), ppc64le, s390x(big endian)  
Sanitizers : valgrind and clang/gcc sanitizers(address, undefined, thread)
</pre>

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
  
-

#### Q&A

-----
* **Is it any better than library X ?**  
  I often use these libraries for high performance server-side applications. Also,  
  I care about readable and easy to debug code. In summary, these libraries show  
  my taste(trade-offs) about performance/api-design/readability. You may or may  
  not like it.


* **Why don't you change API here at X, so it will be easier to use?**  
  Send a pull request please but be sure you don't introduce an undefined  
  behavior. It's possible to provide better APIs, especially to generic libraries,  
  if you don't care about undefined behaviors. I try to avoid it.


* **What is the most efficient way to use these libraries?**  
  Just like any other code. Add to your project as source files and ideally use   
  -O3 -flto + PGO. It may not make any difference for your use case though.


* **Is library X being used in any product?**  
  Some libraries are used in the production but please always test yourself.


* **Is there any release?**   
  Please use the master branch. It's considered stable.


* **Will you keep API stable?**   
  Please don't expect a stable API. These libraries are quite  
  small (most of them are less than a few hundreds lines of code) and ideally you  
  are supposed to read the code and understand what it does and adapt it to your   
  needs. So, you should not update libraries blindly. I expect you to handle  
  any possible API differences easily. That being said, I'll do my best to keep  
  API stable.