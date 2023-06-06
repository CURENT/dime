===============
Python Examples
===============
This page will walk you through some simple Python programs that show off DiME's API calls. 
All of the examples are going to operate off the assumption that the DiME server is running on the 
default Linux settings. There are also ready to run examples on the DiME repository.

----------------
Join and Devices
----------------
The simplest way to check if connections are properly being made is with the Join and Devices calls, 
so we'll start there.

First, you'll need to import DiME, create a few DimeClients, and add them to groups:

.. code:: python

    from dime import DimeClient

    d1 = DimeClient("ipc", "/tmp/dime.sock");
    d2 = DimeClient("ipc", "/tmp/dime.sock");
    d3 = DimeClient("ipc", "/tmp/dime.sock");

    d1.join("turtle", "lizard")
    d2.join("crocodile")
    d3.join("dragon", "wyvern")

    print(d1.devices())

If you start a DiME server (``dime &``) and run this code, it should output 
``['crocodile', 'wyvern', 'lizard', 'dragon', 'turtle']``. There's no guarantee that the list will be in 
this same order, so feel free to sort it so that it's consistent. If your output is correct, then move on 
to the next sections. We'll expand this code block to showcase more functions.

-----
Leave
-----
Leave is essentially the opposite of Join. It works almost certainly how you expect it to. 

We'll expand the previous code to show what happens when DimeClients leave groups they are a part of:


.. code:: python

    from dime import DimeClient

    d1 = DimeClient("ipc", "/tmp/dime.sock")
    d2 = DimeClient("ipc", "/tmp/dime.sock")
    d3 = DimeClient("ipc", "/tmp/dime.sock")

    d1.join("turtle", "lizard")
    d2.join("crocodile", "wyvern")
    d3.join("dragon", "wyvern")

    d1.leave("lizard")
    d2.leave("crocodile")
    d3.leave("wyvern")

    d1.join("lizard")

    groups = d1.devices()
    groups.sort()
    print(groups)
    ```

.. note::
    
    Note that we're now sorting the list of groups so that the output is consistent. 
    The output will be ``['dragon', 'lizard', 'turtle', 'wyvern']``. Leaving and rejoining works as 
    expected: lizard is still listed because d1 rejoined before requesting the group list. 
    Crocodile is not listed because it is empty after d2 left it. Wyvern is still listed, despite d3 
    leaving it, because d2 is still in it.

-------------------------
Broadcast, Send, and Sync
-------------------------
Broadcast, Send, and Sync are the ways clients can communicate with each other. Broadcast and Send are 
for sending data to other clients. Sync is for receiving data from clients. These commands have a bit 
more nuance compared to the others, so the examples will be more extensive in this section.

Broadcasting vs. Sending
^^^^^^^^^^^^^^^^^^^^^^^^
First, we'll show off the difference between Broadcast and Sync:

.. code:: python

    from dime import DimeClient

    d1 = DimeClient("ipc", "/tmp/dime.sock")
    d2 = DimeClient("ipc", "/tmp/dime.sock")
    d3 = DimeClient("ipc", "/tmp/dime.sock")

    d1.join("turtle", "lizard")
    d2.join("crocodile", "wyvern")
    d3.join("dragon", "wyvern")

    d1["a"] = 1
    d1["b"] = 20
    d1["c"] = 300

    d1.send("crocodile", "a")
    d1.broadcast("b")

    d2["a"] = 2
    d2["b"] = 40
    d2["c"] = 600

    d3["a"] = 3
    d3["b"] = 60
    d3["c"] = 900

    d2.sync()
    d3.sync()

    print(d2["a"])
    print(d2["b"])
    print(d2["c"])
    print()
    print(d3["a"])
    print(d3["b"])
    print(d3["c"])

Your output should look like this:

.. code:: python

    1
    20
    600

    3
    20

.. note::

    Note how d2 and d3's *b* variables have both been changed, meanwhile only d2's *a* variable has been 
    changed. This is because Broadcast sends the provided variables to every client that isn't the sender. 
    Send, on the other hand, only sends the given variables to the specified groups.

Changing Variables After Sending Them
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: python

    from dime import DimeClient

    d1 = DimeClient("ipc", "/tmp/dime.sock")
    d2 = DimeClient("ipc", "/tmp/dime.sock")
    d3 = DimeClient("ipc", "/tmp/dime.sock")

    d1.join("turtle", "lizard")
    d2.join("crocodile", "wyvern")
    d3.join("dragon", "wyvern")

    d1["a"] = 1
    d1["b"] = 20
    d1["c"] = 300

    d2["a"] = 2
    d2["b"] = 40
    d2["c"] = 600

    d3["a"] = 3
    d3["b"] = 60
    d3["c"] = 900

    d1.send("wyvern", "c")

    d1["c"] = 0

    d2.sync()
    d3.sync()

    print(d1["c"])
    print(d2["c"])
    print(d3["c"])

The output is this:

.. code:: python

    0
    300
    300

.. note::

    The values d2 and d3's *c* variables are synced to is d1's *c* variable before it was changed to have 
    a value of 0.


Self-Sending
^^^^^^^^^^^^
Since Broadcast sends variables to every client other than the sending client, it is not possible for a 
client to send itself data using Broadcast, at least without an intermediary. This is not true for Send, 
however, which sends data to specific groups. If a client sends data to a group that it is a part of, it 
can send data to itself.

.. code:: python

    from dime import DimeClient

    d1 = DimeClient("ipc", "/tmp/dime.sock")
    d2 = DimeClient("ipc", "/tmp/dime.sock")
    d3 = DimeClient("ipc", "/tmp/dime.sock")

    d1.join("turtle", "lizard")
    d2.join("crocodile", "wyvern")
    d3.join("dragon", "wyvern")

    d1["a"] = 1
    d1["b"] = 20
    d1["c"] = 300

    d2["a"] = 2
    d2["b"] = 40
    d2["c"] = 600

    d3["a"] = 3
    d3["b"] = 60
    d3["c"] = 900

    d1.broadcast("b")
    d1["b"] = 0

    d1.sync()
    print(d1["b"])

    d1["b"] = 25
    d1.send("lizard", "b")
    d1["b"] = 10
    d1.sync()
    print()
    print(d1["b"])

The output for the snippet is:

.. code:: python

    0

    25

.. note::

    Syncing after Broadcasting the value of 20 does not change d1's *b* variable back from 0 to 20. 
    Syncing after Sending 25 to the lizard group does change d1's *b* variable from 10 to 25, however.

----
Wait
----
The Wait command forces a client to wait until another client Sends or Broadcasts a variable to it. 
Once the Wait is done, the client can Sync the variables that were sent to it. 
This example will use three snippets: ``testa.py``, ``testb.py``, and ``testc.py``.

.. note::

    Note that Wait does not automatically Sync variables.

testa.py:

.. code:: python

    #testa.py

    from dime import DimeClient

    d1 = DimeClient("ipc", "/tmp/dime.sock")
    d1.join("turtle")

    d1["a"] = 39

    print("d1: " + str(d1["a"]))

    d1.send("crocodile", "a")

    d1.wait()
    d1.sync()
    d1["a"] = d1["a"] + 1

    print("d1: " + str(d1["a"]))

testb.py:

.. code:: python

    #testb.py

    from dime import DimeClient

    d2 = DimeClient("ipc", "/tmp/dime.sock")
    d2.join("crocodile")

    d2["a"] = 0

    d2.wait()
    d2.sync()
    d2["a"] = d2["a"] + 1

    print("d2: " + str(d2["a"]))

    d2.send("dragon", "a")

testc.py:

.. code:: python

    #testc.py

    from dime import DimeClient

    d3 = DimeClient("ipc", "/tmp/dime.sock")
    d3.join("dragon")

    d3["a"] = 0

    d3.wait()
    d3.sync()
    d3["a"] = d3["a"] + 1

    print("d3: " + str(d3["a"]))

    d3.send("turtle", "a")

If you run these snippets in the order of testc.py, testb.py, and then testa.py, the output will always be:

.. code:: python

    d1: 39
    d2: 40
    d3: 41
    d1: 42

.. note::

    Without the Wait command, there would be no guarantee as to when each client will output.