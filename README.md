
## Standalone C libraries
- Each sub-folder is independent, consists one .c and .h pair, just copy these files to your project.


## Libraries
 
|                 |                                                                             | 
|:---------------:|-----------------------------------------------------------------------------|
| **array**       | Generic growable array                                                      |
| **crc32**       | Crc32 implementation contains hardware & software versions                  |
| **heap**        | Heap implementation which can be used as minheap/max heap/priority queue    |
| **ini**         | Ini file parser                                                             | 
| **linked-list** | Intrusive linked-list which can be used as queue/dequeue/stack etc.         |
| **logger**      | Logger which is configurable to deliver logs to file/stdout/callback.       |
| **map**         | Generic hashmap                                                             |
| **mutex**       | Mutex wrapper for POSIX and Windows                                         |
| **perf**        | Simple benchmarking tool for Linux                                          |
| **queue**       | Generic queue implementation which can be used as queue/dequeue/stack etc.  |
| **string**      | Length prefixed string with a few utility functions                         |
| **time**        | Time functions for POSIX and Windows                                        |
| **timer**       | Hashed timer wheel implementation                                           |


#### Tests

CI runs on

|              |                                              |
|--------------|----------------------------------------------|
| **OS**       |Linux, Windows, macOs, FreeBSD                |
| **Arch**     | x64, x86, s390x, arm6, arm7, aarch64, ppc64le|
| **Compiler** | gcc, clang, msvc                             |
 
[![codecov](https://codecov.io/gh/tezc/simple-c/branch/master/graph/badge.svg)](https://codecov.io/gh/tezc/simple-c)
[![Build Status](https://api.cirrus-ci.com/github/tezc/simple-c.svg)](https://cirrus-ci.com/github/tezc/simple-c)
![.github/workflows/.actions.yml](https://github.com/tezc/simple-c/workflows/.github/workflows/.actions.yml/badge.svg)
![.github/workflows/.actions.yml](https://github.com/tezc/simple-c/workflows/.github/workflows/windows/badge.svg)