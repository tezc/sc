
## Standalone C libraries
- Each sub-folder is independent, consist one *.c *.h pair, just copy files to your project.

##### OS
- Linux, Windows, macOS : ![.github/workflows/.actions.yml](https://github.com/tezc/simple-c/workflows/.github/workflows/.actions.yml/badge.svg)
- FreeBSD               : [![Build Status](https://api.cirrus-ci.com/github/tezc/simple-c.svg)](https://cirrus-ci.com/github/tezc/simple-c)
[![codecov](https://codecov.io/gh/tezc/simple-c/branch/master/graph/badge.svg)](https://codecov.io/gh/tezc/simple-c)


## Vector

|                 |                                                                            | |
|-----------------|-----------------------------------------------------------------------------|
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


#### OS
|                     |          |
|---------------------|----------|
| Linux |
| Windows
| MacOS
| FreeBSD