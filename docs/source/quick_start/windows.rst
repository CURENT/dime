.. _quick_start_windows:

=========================
Windows Quick Start Guide
=========================

.. note::

    The nature of doing a manual instillation on Windows is that you will inevitably have to make 
    your system more Linux-like. Even with some affordances, the Windows implementation of DiME is 
    a lesser version than the Linux version. It lacks WebSocket compatibility, meaning that it won't 
    work with the JavaScript client. If you want to use the Linux version on Windows, then we 
    suggest using `WSL <https://docs.microsoft.com/en-us/windows/wsl/install>`_.

The Windows implementation uses `MinGW64 <https://www.mingw-w64.org/>`_ with a development 
environment like `Cygwin <https://cygwin.com/>`_ or `MSYS2 <https://www.msys2.org/>`_. You can 
try another compiler, but there are no gurarantees that it will work. For the purposes of this 
guide, we will be using MSYS2 since its built-in package manager makes getting DiME set up 
relatively painless.

Setup
-----
Once you have finished running the installer for MSYS2, find the version of it called 
``MSYS2 MinGW x64`` and open it. From the terminal you just opened, enter the command: 

.. code::

    pacman -Syuu

.. note::

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
