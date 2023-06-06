.. _quick_start_linux:

=======================
Linux Quick Start Guide
=======================

Setup
-----
DiME has several dependencies that need to be install for it to compile. First, run these commands:

.. code::

    apt-get install build-essential
    apt-get install autotools-dev
    apt-get install autoconf
    apt-get install libev-dev

These contain some necessary dependencies for DiME and will make the installation of the following packages substantially smoother.

OpenSSL
^^^^^^^
Enter the following commands:

.. code::

    git clone https://github.com/openssl/openssl.git
    cd openssl
    ./config
    make
    make test

if ``make test`` returns successful, then run:

.. code::

    make install

Zlib
^^^^
Download `zlib <https://zlib.net/>`_ and extract its files. Then run:

.. code::

    cd [zlib directory]
    ./configure
    make 
    make install

Janson
^^^^^^
Download `Janson <http://digip.org/jansson/releases/>`_ and extract its files. Then run:

.. code::
    cd [jansson directory]
    ./configure
    make 
    make install

.. note::

    Jansson does have a GitHub with a slightly more up-to-date version as of writing this 
    https://github.com/akheron/jansson. Installing from this source, however, uses the 
    ``autoreconf`` command, which I have had difficulties with. If your machine has no 
    issue with ``autoreconf``, then feel free to follow these alternate instructions:

.. code::

    git clone https://github.com/akheron/jansson.git
    cd jansson
    autoreconf -i
    ./configure
    make 
    make install

DiME
----
Now that all the dependencies have been installed, DiME can now be compiled and 
installed if you so choose. If you have not already, clone the Dime repository with:

.. code::

    git clone https://github.com/CURENT/dime.git

Now run the following:

.. code::

    cd dime/server
    make
    make install

Assuming all dependencies were properly installed, DiME should compile and install without issue. 
Now you should be able to run the command:

.. code::

    dime &

.. note::

    This will start a DiMe server in the background with default settings. If you move to ``/usr/temp`` and see ``dime.sock``, then it is running. 
    
    
Now you just need to do a little setup for the clients you want to work with.

Using the MATLAB Client
^^^^^^^^^^^^^^^^^^^^^^^
First, add ``dime/client/matlab`` to your search path in MATLAB. Then run:

.. code::

    cd dime/client/matlab
    make

Using the Python Client
^^^^^^^^^^^^^^^^^^^^^^^
Run the following commands:

.. code::

    cd dime/client/python
    python3 setup.py install

The python client also uses the numpy library, so ensure that is installed before using it.

Using the JavaScript Client
^^^^^^^^^^^^^^^^^^^^^^^^^^^
Add the following to your HTML ``<head>`` element:

.. code::

    <script src="https://cdn.jsdelivr.net/gh/TheHashTableSlasher/dime2/client/javascript/dime.min.js" type="text/javascript" crossorigin=""></script>

Alternatively, you can include ``dime2/client/javascript/dime.js`` in your HTML pages.

Type Conversion Between clients
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
The following types can be transmitted between MATLAB, Python, and Javascript clients, and translate according to the following table:

+---------------------------+---------------------------+---------------------------+
| MATLAB                    | Python                    | JavaScript                |
+---------------------------+---------------------------+---------------------------+
| Empty matrix              | ``None``                  | ``null``                  |
+---------------------------+---------------------------+---------------------------+
| Logical                   | ``bool``                  | ``boolean``               |
+---------------------------+---------------------------+---------------------------+
| Integers                  | ``int``                   | ``number``                |
+---------------------------+---------------------------+---------------------------+
| Single/double             | ``float``                 | ``number``                |
+---------------------------+---------------------------+---------------------------+
| Complex                   | ``complex``               | Custom ``complex`` object |
+---------------------------+---------------------------+---------------------------+
| Matrix                    | ``numpy.ndarray``         | Custom ``NDArray`` object |
+---------------------------+---------------------------+---------------------------+
| String/Character array    | ``str``                   | ``string``                |
+---------------------------+---------------------------+---------------------------+
| Cell array                | ``list``                  | ``Array``                 |
+---------------------------+---------------------------+---------------------------+
| Struct/container.Map      | ``dict``                  | ``object``                |
+---------------------------+---------------------------+---------------------------+

Further Information
-------------------
`DiME README <https://github.com/CURENT/dime/blob/master/README.md>`_