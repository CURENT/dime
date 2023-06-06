===============
MATLAB Examples
===============
This page will walk you through some simple MATLAB programs that show off DiME's API calls. 
All of the examples are going to operate off the assumption that the DiME server is running on the default 
Linux settings and that the MATLAB client has been linked in MATLAB's path. There are also ready to run 
examples on the DiME repository.

----------------
Join and Devices
----------------
The simplest way to check if connections are properly being made is with the Join and Devices calls, so 
we'll start there.

First, you'll need to import DiME, create a few DimeClients, and add them to grops:

.. code:: matlab

    d1 = dime("ipc", "/tmp/dime.sock");
    d2 = dime("ipc", "/tmp/dime.sock");
    d3 = dime("ipc", "/tmp/dime.sock");

    d1.join("turtle", "lizard");
    d2.join("crocodile");
    d3.join("dragon", "wyvern");

    disp(d1.devices());

If you start a DiME server (``dime &``) and run this code, it should output:

.. code:: matlab

    {'crocodile'} 
    {'wyvern'} 
    {'lizard'} 
    {'dragon'} 
    {'turtle'}

There's no guarantee that the list will be in this same order, so feel free to sort it so that it's 
consistent. If your output is correct, then move on to the next sections. We'll expand this code block 
to showcase more functions.

-----
Leave
-----
Leave is essentially the opposite of Join. It works almost certainly how you expect it to. 

We'll expand the previous code to show what happens when DimeClients leave groups they are a part of:

.. code:: matlab

    d1 = dime("ipc", "/tmp/dime.sock");
    d2 = dime("ipc", "/tmp/dime.sock");
    d3 = dime("ipc", "/tmp/dime.sock");

    d1.join("turtle", "lizard");
    d2.join("crocodile", "wyvern");
    d3.join("dragon", "wyvern");

    d1.leave("lizard");
    d2.leave("crocodile");
    d3.leave("wyvern");

    d1.join("lizard");

    groups = d1.devices();
    print(sort(groups));

.. note::

    We're now sorting the list of groups so that the output is consistent. The output will be 

    .. code:: matlab

        {'dragon'} 
        {'lizard'} 
        {'turtle'}
        {'wyvern'} 

Leaving and rejoining works as expected: lizard is still listed because d1 rejoined before requesting 
the group list. Crocodile is not listed because it is empty after d2 left it. Wyvern is still listed, 
despite d3 leaving it, because d2 is still in it.