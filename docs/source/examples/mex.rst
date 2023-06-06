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

    We're now sorting the list of groups so that the output is consistent. 
    
The output will be: 

.. code:: matlab

    {'dragon'} 
    {'lizard'} 
    {'turtle'}
    {'wyvern'} 

Leaving and rejoining works as expected: lizard is still listed because d1 rejoined before requesting 
the group list. Crocodile is not listed because it is empty after d2 left it. Wyvern is still listed, 
despite d3 leaving it, because d2 is still in it.

----
Wait
----
From this point onward, the examples will differ substantially from those in the JavaScript and Python 
pages. Basically every example from here on will have at least two instances of MATLAB running since, 
unlike JavaScript and Python, the variables sent over DiME are associated with each MATLAB instance as 
opposed to each DimeClient instance. Each separate instance will run a single MATLAB file. The first 
instance will run "testa.m", the second will run "testb.m", and so on. While a single MATLAB instance 
can be used to demonstrate things like Joining and Leaving, it cannot be used to show anything that 
involves sending data since all variables would already be available to all DimeClient instances. 
Because of this difference, we will talk about Wait first.

The Wait command forces a client to wait until another client Sends or Broadcasts a variable to it. 
Once the Wait is done, the client can Sync the variables that were sent to it. Note that Wait does not 
automatically Sync variables. Until going into more detail further down, just know that Send is for 
sending data to other clients and Sync is for receiving said data.

testa.m:

.. code:: matlab

    % testa.m

    d1 = dime("ipc", "/tmp/dime.sock");
    d1.join("turtle");
    a = 50;
    d1.send("crocodile", "a");
    b = 0;

    disp(b);
    d1.wait();
    d1.sync();
    disp(b);

testb.m:

.. code:: matlab

    % testb.m

    d2 = dime("ipc", "/tmp/dime.sock");
    d2.join("crocodile");
    a = 0;

    disp(a);
    d2.wait();
    d2.sync();
    disp(a);

    b = 100;

    d2.send("turtle", "b");

If you run these in the order of ``testb.m`` then ``testa.m``, the output for testa.m will always be:

.. code:: matlab

    0

    100

And the output for ``testb.m`` will always be:

.. code:: matlab

    0

    50

.. note::
    
    Without the Wait command, there would be no guarantee as to when each client will output.

------------
Sharing Data
------------
Broadcast, Send, and Sync are the ways clients can communicate with each other. Broadcast and Send are for 
sending data to other clients. Sync is for receiving data from clients. These commands have a bit more 
nuance compared to the others, so the examples will be more extensive in this section.

Broadcasting vs. Sending
^^^^^^^^^^^^^^^^^^^^^^^^
First, we'll show off the difference between Broadcast and Sync. These examples should be run so that 
``testb.m`` and ``testc.m`` start before ``testa.m``.

testa.m:

.. code:: matlab

    % testa.m

    d1 = dime("ipc", "/tmp/dime.sock");
    d1.join("turtle", "lizard");

    a = 1;
    b = 20;
    c = 300;

    d1.send("crocodile", "a");
    d1.broadcast("b");

testb.m:

.. code:: matlab

    % testb.m

    d2 = dime("ipc", "/tmp/dime.sock");
    d2.join("crocodile", "wyvern");

    a = 2;
    b = 40;
    c = 600;

    d2.wait();
    d2.sync();

    disp(a);
    disp(b);
    disp(c);

testc.m

.. code:: matlab

    % testc.m

    d3 = dime("ipc", "/tmp/dime.sock");
    d3.join("dragon", "wyvern");

    a = 3;
    b = 60;
    c = 900;

    d3.wait();
    d3.sync();

    disp(a);
    disp(b);
    disp(c);

Your output for ``testb.m`` should look like this:

.. code:: matlab

    1

    20

    600

And your output for ``testc.m`` should look like this:

.. code:: matlab

    3

    20

    900

.. note::

    Note how d2 and d3's *b* variables have both been changed, meanwhile only d2's *a* variable has been 
    changed. This is because Broadcast sends the provided variables to every client that isn't the sender. 
    Send, on the other hand, only sends the given variables to the specified groups.

Changing Variables After Sending Them
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
As you might expect, the values sent with Broadcast and Send are not dynamic. If a client changes a 
variable after it was sent, the variable the receiving client gets will reflect the state of the variable 
at the time of sending. The following snippets should be run in order of testb.m and then testa.m. The 
MATLAB pause() function is used instead of Wait to make it more apparent that d1's *a* changed before d2 
Syncs it.

testa.m:

.. code:: matlab

    % testa.m

    d1 = dime("ipc", "/tmp/dime.sock");
    d1.join("turtle");

    a = 50;
    disp(a);

    d1.send("crocodile", "a");

    a = 0;
    disp(a);

testb.m:

.. code:: matlab

    % testb.m

    d2 = dime("ipc", "/tmp/dime.sock");
    d2.join("crocodile");

    a = 10;
    disp(a);

    pause(5);
    d2.sync();
    disp(a);

The output for ``testa.m``:

.. code:: matlab

    50

    0

The output for ``testb.m``:

.. code:: matlab

    10

    50

.. note::

    The value d2's *a* variable is synced to is d1's *a* variable before it was changed to have a value of 0.

Self-Sending
^^^^^^^^^^^^
Since Broadcast sends variables to every client other than the sending client, it is not possible for a 
client to send itself data using Broadcast, at least without an intermediary. This is not true for Send, 
however, which sends data to specific groups. If a client sends data to a group that it is a part of, it 
can send data to itself.

.. code:: matlab

    d1 = dime("ipc", "/tmp/dime.sock");
    d1.join("turtle", "lizard");

    a = 1;
    b= 20;
    c = 300;


    d1.broadcast("b");
    b = 0;

    d1.sync();
    disp(b);

    b = 25;
    d1.send("lizard", "b");
    b = 10;
    d1.sync();

    disp(b);

The output for this snippet is:

.. code:: matlab

    0

    25

.. note::

    Syncing after Broadcasting the value of 20 does not change d1's *b* variable back from 0 to 20. 
    Syncing after Sending 25 to the lizard group does change d1's *b* variable from 10 to 25, however.