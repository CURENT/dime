# dime2
A re-write of the Distributed Matlab Environment, a library enabling multiple Matlab processes to share data across an operating system or over a network. The original DiME code can be found [here](https://github.com/CURENT/dime). This re-write includes a number of enhancements with regard to simplicity and efficiency, including single-threadedness, (I/O multiplexing is done via [`poll(2)`](https://pubs.opengroup.org/onlinepubs/007908799/xsh/poll.html)) fewer dependencies, and significantly higher throughput.

## Compiling

### Client
Compiling client code is only necessary for Matlab on Unix-like OSes if you wish to connect to Unix domain sockets. To do so, run `make` in the `client/matlab` directory. Build options can be tweaked by editing the Makefile (sane defaults are provided).

### Server
To compile the server executable, run `make` in the `server` directory. Build options can be tweaked by editing the Makefile (sane defaults are provided). The server code has the following compile-time dependencies:

* A SUSv2-compatible environment
  * Windows servers need to be built with [Cygwin](https://www.cygwin.com/) or [MSYS2](https://www.msys2.org/)
* [Jansson](https://digip.org/jansson/)

## Running
To use the software, simply add `client/matlab` to your Matlab path. The following code demonstrates the library in use:
```
% Suppose the DiME server is running on the Unix domain socket at /tmp/dime.sock
d = dime('matlab', 'ipc', '/tmp/dime.sock');

a = [1, 2, 3; 4, 5, 6; 7, 8, 9];
d.send_var('matlab', 'a');
clear a;

d.sync();
a

% a =
%      1     2     3
%      4     5     6
%      7     8     9
```

## Caveats

### Matlab/Python intercommunication
The following types can be transmitted between Matlab and Python clients, and
translate according to the following table:

| Matlab                 | Python                 |
| ---------------------- | ---------------------- |
| Empty matrix           | `None`                 |
| Logical                | `bool`                 |
| Integers               | `int`                  |
| Single/double          | `float`                |
| Complex                | `complex`              |
| Matrix                 | `numpy.ndarray`        |
| String/Character array | `str`                  |
| Cell array             | `list`                 |
| Struct/container.Map   | `dict`                 |

## FAQs/Justifications

### Why a rewrite?

The old DiME code works fine for small-to-medium workloads, but introducing greater scalability into the old code would require several radical changes in the way it handles I/O. This is an issue, as the server's performance is starting to become a bottleneck for the large simulation projects CURENT intends to run on the platform. So the code needs to be improved with respect to performance, but room for improvement without a rewrite is rather limited.

I was originally tasked with optimizing the old DiME code, and upon analyzing it I tried to see if I could reimplement its core functionality in a smaller package. I had done so [in ~200 lines of Python](https://github.com/TheHashTableSlasher/dime2/blob/554e99e12db343757445c87d46f9caac20b71d35/server/prototype.py). From that point, I determined that a rewrite would involve less effort than trying to further optimize the old code.

### Why are you avoiding multi-threading?

It's been said that single-threaded programs have _x_ bugs, whereas multi-threaded programs running _y_ threads have _x^y_ bugs. Even when disregarding the class of problems introduced by concurrency, it is usually a nicety and most programs don't need it. An I/O-bound program like the DiME server will be simpler, less error-prone, use less memory, and have comparable performance if it uses `poll` rather than spawning a thread for each incoming connection.

That having been said, multi-threading may not be avoided in the future. A technique used by several HTTP servers to tackle the C10k problem (e.g. Nginx, lighttpd) is to have a pool of worker threads that each `poll` a set of client connections dispached to them by the main thread. (An excellent article on the I/O performance of threads vs `poll`/`select` can be found [here](https://thetechsolo.wordpress.com/2016/02/29/scalable-io-events-vs-multithreading-based/).) This approach may be used by this code in the future, if such scalability is desired.

### Why are you writing this in C instead of < insert my favorite language here >?

~~Because your favorite language is bad.~~

We wanted a language that was small, simple, efficient, allowed for hacks to improve performance, and compiled to machine code without depending on several shared libraries. This narrowed our options to C and Go. Go might've been a better option with its features oriented to building servers. However, C was chosen on the basis that it would be more familiar to undergraduate and guaduate computer scientists at UTK.
