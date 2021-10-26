# dime
A re-write of the Distributed Matlab Environment, a library enabling multiple Matlab processes to share data across an operating system or over a network. The original DiME code can be found [here](https://github.com/CURENT/dime). This re-write includes a number of enhancements with regard to simplicity and efficiency, including single-threadedness, (I/O multiplexing is done via [`poll(2)`](https://pubs.opengroup.org/onlinepubs/007908799/xsh/poll.html)) fewer dependencies, and significantly higher throughput.

## Setup

### Server
To compile the server executable, run `make` in the `server` directory. Build options can be tweaked by editing the Makefile (sane defaults are provided). The server code has the following compile-time dependencies:

* A SUS-compatible environment
  * Windows servers need to be built with [Cygwin](https://www.cygwin.com/) or [MSYS2](https://www.msys2.org/) + [Gnulib](http://www.gnu.org/software/gnulib/)
* [Jansson](https://digip.org/jansson/)
* [libev](http://software.schmorp.de/pkg/libev.html)
* [OpenSSL](https://www.openssl.org/)
* [zlib](http://zlib.net/)

### Matlab Client
To use the Matlab client, add `client/matlab` to your [Matlab search path](https://www.mathworks.com/help/matlab/matlab_env/what-is-the-matlab-search-path.html).

The Matlab client supports TCP and Unix domain socket connections. However, compiling some code is necessary for Matlab on Unix-like OSes if you wish to connect to Unix domain sockets. To do so, run `make` in the `client/matlab` directory. Build options can be tweaked by editing the Makefile (sane defaults are provided).

### Python Client
To use the Python client, either add `client/python` to your [PYTHONPATH](https://docs.python.org/3/using/cmdline.html#envvar-PYTHONPATH) environment variable, or run `python3 setup.py install` in that directory.

The Python client supports TCP and Unix domain socket connections.

### Javascript Client
To use the Javascript client, add the following to your `<head>` element:
```html
<script src="https://cdn.jsdelivr.net/gh/TheHashTableSlasher/dime2/client/javascript/dime.min.js" type="text/javascript" crossorigin=""></script>
```
Or include `client/javascript/dime.js` in your HTML pages in some other way.

The Javascript client supports WebSocket connections.

## Usage

### Server
To run the server with default settings (assuming `server/dime` has been installed somewhere in your path):
```
$ dime
```

"Default settings" in this context means listening on the Unix domain socket `/tmp/dime.sock` on Unix-like systems and on the TCP port 5000 on Windows systems. To change this behavior, use the `-l` flag:
```
$ dime -l tcp:8888
```

More than one connection, including connections of different types, can be hosted on using this flag:
```
$ dime -l unix:./dime.sock -l unix:/var/run/dime.sock -l tcp:8888 -l ws:8889
```

By default, nothing is printed to standard output either. The `-v` flag outputs some debug information:
```
$ dime -v
```

Extra `-v` flags increase the level of verbosity:
```
$ dime -vvv
```

For more information, including other, less useful options, run:
```
$ dime -h
```

### Matlab Client
```matlab
% Suppose the DiME server is running on a Unix domain socket at /tmp/dime.sock
d = dime('ipc', '/tmp/dime.sock');
d.join('matlab');

a = [1, 2, 3; 4, 5, 6; 7, 8, 9];
d.send('matlab', 'a');
clear a;

d.sync();
disp(a);

% a =
%      1     2     3
%      4     5     6
%      7     8     9
```

### Python Client
```python
# Suppose the DiME server is running on a Unix domain socket at /tmp/dime.sock
import numpy as np
from dime import DimeClient

d = DimeClient("ipc", "/tmp/dime.sock")
d.join("python")

d["a"] = np.array([[1, 2, 3], [4, 5, 6], [7, 8, 9]])
d.send("python", "a")
del d["a"]

d.sync()
print(d["a"])

# array([[1, 2, 3],
#        [4, 5, 6],
#        [7, 8, 9]])
```

### Javascript Client
The Javascript client code relies heavily on promises, so it's recommended to be used in an async function:
```javascript
// Suppose the DiME server is running on a WebSocket on the current computer, on port 8888
let promise = (async function() {
    let d = new dime.DimeClient("localhost", 8888);
    await d.join("javascript");

    d.workspace.a = "Hello world!";
    await d.send("javascript", "a");
    delete d.workspace.a;

    await d.sync();
    console.log(d.workspace.a);

    // Hello world!
})();

// Can do something with the promise if that is desired
```

## Caveats

### Type conversions between languages
The following types can be transmitted between Matlab, Python, and Javascript clients, and translate according to the following table:

| Matlab                  | Python                  | Javascript              |
| ----------------------- | ----------------------- | ----------------------- |
| Empty matrix            | `None`                  | `null`                  |
| Logical                 | `bool`                  | `boolean`               |
| Integers                | `int`                   | `number`                |
| Single/double           | `float`                 | `number`                |
| Complex                 | `complex`               | Custom `Complex` object |
| Matrix                  | `numpy.ndarray`         | Custom `NDArray` object |
| String/Character array  | `str`                   | `string`                |
| Cell array              | `list`                  | `Array`                 |
| Struct/container.Map    | `dict`                  | `object`                |

## FAQs/Justifications

### Why a rewrite?

The old DiME code works fine for small-to-medium workloads, but introducing greater scalability into the old code would require several radical changes in the way it handles I/O. This is an issue, as the server's performance is starting to become a bottleneck for the large simulation projects CURENT intends to run on the platform. So the code needs to be improved with respect to performance, but room for improvement without an almost-total reimplementation is rather limited.

I was originally tasked with optimizing the old DiME code, and upon analyzing it I tried to see if I could reimplement its core functionality in a smaller package. I had done so [in ~200 lines of Python](https://github.com/TheHashTableSlasher/dime2/blob/554e99e12db343757445c87d46f9caac20b71d35/server/prototype.py). From that point, I determined that a rewrite would involve less effort than trying to further optimize the old code.

### Why are you avoiding multi-threading?

It's been said that single-threaded programs have _x_ bugs, whereas multi-threaded programs running _y_ threads have _x^y_ bugs. Even when disregarding the class of problems introduced by concurrency, it is usually a nicety and most programs don't need it. An I/O-bound program like the DiME server will be simpler, less error-prone, use less memory, and have comparable performance if it uses `poll` rather than spawning a thread for each incoming connection.

That having been said, multi-threading may not be avoided in the future. A technique used by several HTTP servers to tackle the C10k problem (e.g. Nginx, lighttpd) is to have a fixed-size pool of worker threads that each `poll` a set of client connections dispatched to them by the main thread. (An excellent article on the I/O performance of threads vs `poll`/`select` can be found [here](https://thetechsolo.wordpress.com/2016/02/29/scalable-io-events-vs-multithreading-based/).) This approach may be used by this code in the future, if such scalability is desired.

### Why are you writing this in C instead of < insert my favorite language here >?

~~Because your favorite language is bad.~~

We wanted a language that was small, simple, efficient, allowed for hacks to improve performance, and compiled to machine code without depending on several shared libraries. This narrowed our options to C and Go. Go might've been a better option with its features oriented to building servers. However, C was chosen on the basis that it would be more familiar to undergraduate and guaduate computer scientists at UTK.
