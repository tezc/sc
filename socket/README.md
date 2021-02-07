### Socket and networking for Linux, BSDs, MacOS and Windows

### Overview

- Includes three implementations :

| Implementation    | Description                                              |
|-------------------|----------------------------------------------------------|
| sc_sock_xxx       | TCP socket wrapper for blocking and nonblocking sockets  |
| sc_sock_poll_xxx  | Epoll / Kqueue / WSAPoll wrapper                         |
| sc_sock_pipe_xxx  | Unix pipe() and an equivalent implementation for Windows.|
  

- Works for IPv4, IPv6 and Unix domain sockets. (~ Windows 10 2018 added Unix   
  domain sockets support.)
- No UDP support.
- Works for blocking and nonblocking sockets.


### Usage

This is not an "easy to use" implementation. It just provides wrappers to   
provide portability between operating systems. So, you're expected to know what  
you're doing. (familiar with sockets API and know how to use it). Please take  
a look at the code and grab pieces you want. Hopefully, I will add an example  
soon.