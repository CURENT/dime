# Linux Quick Start Guide
## Setup
DiME has several dependencies that need to be install for it to compile. First, run these commands:

```
apt-get install build-essential
apt-get install autotools-dev
apt-get install autoconf
apt-get install libev-dev
```

These contain some necessary dependencies for DiME and will make the installation of the following packages substantially smoother.

### OpenSSL
Enter the following commands:

```
https://github.com/openssl/openssl.git
cd openssl
./config
make
make test
```

If ```make test``` returns a success, then run: 

```
make install
```

#### Troubleshooting
When you try to compile DiME later, you may receive an error along the lines of ```'openssl: error while loading shared libraries: libssl.so.3: cannot open shared object file: No such file or directory'```.

If you do, then run:

```
ldconfig /usr/local/lib64
```

### zlib
Download [zlib] (https://zlib.net/) and extract its files. Then run:

```
cd [zlib directory]
./configure
make 
make install
```

### Jansson
Download [Jansson] (http://digip.org/jansson/releases/) and extract its files. Then run:

```
cd [jansson directory]
./configure
make 
make install
```

Note: Jansson does have a GitHub with a slightly more up-to-date version as of writing this (https://github.com/akheron/jansson). Installing from this source, however, uses the ```autoreconf``` command, which I have had difficulties with. If your machine has no issue with ```autoreconf```, then feel free to follow these alternate instructions:

```
git clone https://github.com/akheron/jansson.git
cd jansson
autoreconf -i
./configure
make 
make install
```
## DiME
Now that all the dependencies have been installed, DiME can now be compiled and installed if you so choose. If you have not already, clone the Dime repository with:

```
git clone https://github.com/CURENT/dime2.git
```

Now run the following:

```
cd dime2/server
make
make install
```

Assuming all dependencies were properly installed, DiME should compile and install without issue. Now you should be able to run the command:

```
dime &
```

This will start a DiMe server in the background with default settings. If you move to ```/usr/temp``` and see ```dime.sock```, then it is running. Now you just need to do a little setup for the clients you want to work with.

### Using the MATLAB client
First, add ```dime2/client/matlab``` to your search path in MATLAB. Then run:

```
cd dime2/client/matlab
make
```

### Using the Python client
Run the following commands:

```
cd dime2/client/python
python3 setup.py install
```

The python client also uses the numpy library, so ensure that is installed before using it.

### Using the Javascript client
Add the following to your HTML ```<head>``` element:

```html
<script src="https://cdn.jsdelivr.net/gh/TheHashTableSlasher/dime2/client/javascript/dime.min.js" type="text/javascript" crossorigin=""></script>
```

Alternatively, you can include ```dime2/client/javascript/dime.js``` in your HTML pages.

### Type Conversion Between Clients
The following types can be transmitted between MATLAB, Python, and Javascript clients, and translate according to the following table:

| MATLAB                  | Python                  | Javascript              |
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

## Further Information
['DiME README'](https://github.com/CURENT/dime2/blob/master/README.md)
