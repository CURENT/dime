.. _quick_start_windows:

=========================
Windows Quick Start Guide
=========================

.. note::

    The nature of doing a manual instillation on Windows is that you will inevitably have to make your system more Linux-like. 
    Even with some affordances, the Windows implementation of DiME is a lesser version than the Linux version. It lacks WebSocket 
    compatibility, meaning that it won't work with the JavaScript client. If you want to use the Linux version on Windows, then 
    we suggest using `WSL <https://docs.microsoft.com/en-us/windows/wsl/install>`_.

The Windows implementation uses `MinGW64 <https://www.mingw-w64.org/>`_ with a development environment like 
`Cygwin <https://cygwin.com/>`_ or `MSYS2 <https://www.msys2.org/>`_. You can try another compiler, but there are no gurarantees 
that it will work. For the purposes of this guide, we will be using MSYS2 since its built-in package manager makes getting DiME 
set up relatively painless.

Setup
-----
Once you have finished running the installer for MSYS2, find the version of it called ``MSYS2 MinGW x64`` and open it. 
From the terminal you just opened, enter the command: 

.. code::

    pacman -Syuu


Keep running the command until there is nothing left to be updated or installed. 

Now run the following commands:

.. code::

    pacman -S vim
    pacman -S git
    pacman -S --needed base-devel mingw-w64-x86_64-toolchain

.. note::

    This will install vim, git, and all the necessary MinGW64 dependencies and compilers. 


Now download `libev <http://software.schmorp.de/pkg/libev.html>`_. 
Finish up the setup by running:

.. code::

    git clone https://github.com/CURENT/dime

OpenSSL and zlib
^^^^^^^^^^^^^^^^
By running the commands in Setup, OpenSSL and zlib will have automatically been installed. 
If you decide you want to install OpenSSL manually, make sure to install the 1.1.1q version.

Jansson
^^^^^^^

Enter the following command to automatically install Jansson:

.. code::

    pacman -S mingw-w64-jansson

Libev
^^^^^
Unzip the downloaded package and move to its directory. Now simply running:

.. code::

    ./configure
    make install

should be enough to install libev.

DiME
----

.. note::

    With all of the Setup done, making and installing DiME is quite similar to how its done in the Linux version. 
    There are just a few more changes that need to be made. Assuming you are in the dime folder, move to the 
    server folder. There are two files that are going to need changes --- Makefile and config.mk. 

Open config.mk. There are three lines at the bottom of the file:

.. code::

    # Uncomment the lines below to compile for Windows
    #CC := x86_64-w64-mingw32-gcc
    #LDFLAGS += -lws2_32

Change the last two so that they are uncommented:

.. code::

    # Uncomment the lines below to compile for Windows
    CC := x86_64-w64-mingw32-gcc
    LDFLAGS += -lws2_32

Now you need to make one final change to Makefile. You need to make sure libssp gets linked. You can do this by changing

.. code:: javascript
    :

    dime: ${OBJS}
	    ${CC} ${OBJS} -o $@ -ljansson -lev -lssl -lcrypto -lz ${LDFLAGS}

to

.. code:: javascript

    dime: ${OBJS}
        ${CC} ${OBJS} -o $@ -ljansson -lev -lssl -lssp -lcrypto -lz ${LDFLAGS}

Now to finally make and install DiME, exit the Makefile and run:

.. code::

    make
    install -s dime /c/msys64/mingw64/bin

Assuming you used the default installation location for MSYS2, then you should now be able to run DiME from your command line. 
Simply running ```dime``` in your command line will create a DiME server running on TCP port 5000.

DiME Clients
^^^^^^^^^^^^
Instructions on running creating and running the DiME clients are essentially the same as they are in the 
:ref:`Linux Quick Start Guide <quick_start_linux>`. This section will point out a few key differences.

If you are connecting the MATLAB client to a Windows-run DiME server, you do not have to run the ``make`` 
command in the MATLAB client directory. You only need to link it.

Python is basically the exact same, just use ``pacman -S mingw-w64-python-numpy`` to install numpy. 
You can try to use pip to do it, but there is no guarantee that it will work.

There is no way to use the JavaScript client with a Windows-run DiME server. Since the JavaScript client only connects to 
WebSockets, and the Windows version of DiME can only do TCP, they are incompatible.

Further Information
-------------------
`DiME README <https://github.com/CURENT/dime/blob/master/README.md>`_