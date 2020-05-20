# dime2
A re-write of the Distributed Matlab Environment, a library enabling multiple Matlab processes to share data across an operating system or over a network. The original DiME code can be found [here](https://github.com/CURENT/dime). This re-write includes a number of enhancements with regard to simplicity and efficiency, including single-threadedness, (I/O multiplexing is done via [`poll(2)`](https://pubs.opengroup.org/onlinepubs/007908799/xsh/poll.html)) fewer dependencies, and significantly higher throughput.

## Compiling

### Client
Compiling client code is only necessary on Unix-like OSes if you wish to connect to Unix domain sockets. To do so, run `make` in the `client` directory. Build options can be tweaked by editing the Makefile (sane defaults are provided).

### Server
To compile the server executable, run `make` in the `server` directory. Build options can be tweaked by editing the Makefile (sane defaults are provided). The server code has the following compile-time dependencies:

* A SUSv2-compatible environment
  * Windows servers need to be built with [Cygwin](https://www.cygwin.com/) or [MSYS2](https://www.msys2.org/)
* [Jansson](https://digip.org/jansson/)

## Running
To use the software, simply add `client` to your Matlab path. The following code demonstrates the library in use:
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
