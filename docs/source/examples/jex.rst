===================
JavaScript Examples
===================
This page will walk you through some simple JavaScript functions that show off DiME's API calls. 
All examples will operate under the assumption that ``dime -l ws:8000`` was used to run the DiME server 
and that the JavaScript client was included somewhere in the HTML.

----------------
Join and Devices
----------------
The simplest way to check if connections are properly being made is with the Join and Devices calls, 
so we'll start there.

First, you'll need to create a few DimeClients, and add them to groups:

.. code:: javascript

    async function fubar() {
        
        let d1 = new DimeClient("localhost", 8000);
        let d2 = new DimeClient("localhost", 8000);
        let d3 = new DimeClient("localhost", 8000);

        await d1.join("turtle", "lizard");
        await d2.join("crocodile");
        await d3.join("dragon", "wyvern");

        console.log(d1.devices())
    }

If you start a DiME server (``dime -l ws:8000 &``) and run this code, it should output 
``['crocodile', 'wyvern', 'lizard', 'dragon', 'turtle']``. There's no guarantee that the list will be in 
this same order, so feel free to sort it so that it's consistent. If your output is correct, then move on 
to the next sections. We'll expand this code block to showcase more functions.

-----
Leave
-----
Leave is essentially the opposite of Join. It works almost certainly how you expect it to. 

We'll expand the previous code to show what happens when DimeClients leave groups they are a part of:

.. code:: javascript

    async function fubar() {
	
        let d1 = new DimeClient("localhost", 8000);
        let d2 = new DimeClient("localhost", 8000);
        let d3 = new DimeClient("localhost", 8000);

        await d1.join("turtle", "lizard");
        await d2.join("crocodile", "wyvern");
        await d3.join("dragon", "wyvern");

        await d1.leave("lizard");
        await d2.leave("crocodile");
        await d3.leave("wyvern");

        await d1.join("lizard");
        
        const groups = d1.devices();
        console.log(groups.sort());
    }

.. note::

    Note that we're now sorting the list of groups so that the output is consistent. The output will be 
    ``['dragon', 'lizard', 'turtle', 'wyvern']``. Leaving and rejoining works as expected: lizard is still 
    listed because d1 rejoined before requesting the group list. Crocodile is not listed because it is empty 
    after d2 left it. Wyvern is still listed, despite d3 leaving it, because d2 is still in it.

-------------------------
Broadcast, Send, and Sync
-------------------------
Broadcast, Send, and Sync are the ways clients can communicate with each other. Broadcast and Send are for 
sending data to other clients. Sync is for receiving data from clients. These commands have a bit more 
nuance compared to the others, so the examples will be more extensive in this section.

Broadcasting vs. Sending
^^^^^^^^^^^^^^^^^^^^^^^^
First, we'll show off the difference between Broadcast and Sync:

.. code:: javascript

    async function fubar() {
        
        let d1 = new DimeClient("localhost", 8000);
        let d2 = new DimeClient("localhost", 8000);
        let d3 = new DimeClient("localhost", 8000);

        await d1.join("turtle", "lizard");
        await d2.join("crocodile", "wyvern");
        await d3.join("dragon", "wyvern");

        d1.workspace.a = 1;
        d1.workspace.b = 20;
        d1.workspace.c = 300;

        await d1.send("crocodile", "a");
        await d1.broadcast("b");

        d2.workspace.a = 2;
        d2.workspace.b = 40;
        d2.workspace.c = 600;

        d3.workspace.a = 3;
        d3.workspace.b = 60;
        d3.workspace.c = 900;

        await d2.sync();
        await d3.sync();

        console.log(d2.workspace.a);
        console.log(d2.workspace.b);
        console.log(d2.workspace.c);
        console.log();
        console.log(d3.workspace.a);
        console.log(d3.workspace.b);
        console.log(d3.workspace.c);
    }

Your output should look like this:

.. code:: javascript

    1
    20
    600

.. note::

    Note how d2 and d3's *b* variables have both been changed, meanwhile only d2's *a* variable has been 
    changed. This is because Broadcast sends the provided variables to every client that isn't the sender. 
    Send, on the other hand, only sends the given variables to the specified groups.

Changing Variables After Sending Them
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
As you might expect, the values sent with Broadcast and Send are not dynamic. If a client changes a 
variable after it was sent, the variable the receiving client gets will reflect the state of the variable 
at the time of sending. This snippet and its associated output will demonstrate this:

.. code:: javascript

    async function fubar() {
	
        let d1 = new DimeClient("localhost", 8000);
        let d2 = new DimeClient("localhost", 8000);
        let d3 = new DimeClient("localhost", 8000);

        await d1.join("turtle", "lizard");
        await d2.join("crocodile", "wyvern");
        await d3.join("dragon", "wyvern");

        d1.workspace.a = 1;
        d1.workspace.b = 20;
        d1.workspace.c = 300;

        d2.workspace.a = 2;
        d2.workspace.b = 40;
        d2.workspace.c = 600;

        d3.workspace.a = 3;
        d3.workspace.b = 60;
        d3.workspace.c = 900;

        await d1.send("wyvern", "c");

        d1.workspace.c = 0;

        await d2.sync();
        await d3.sync();

        console.log(d1.workspace.c);
        console.log(d2.workspace.c);
        console.log(d3.workspace.c);
    }

The output is this:

.. code:: javascript

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

.. code:: javascript

    async function fubar() {
	
        let d1 = new DimeClient("localhost", 8000);
        let d2 = new DimeClient("localhost", 8000);
        let d3 = new DimeClient("localhost", 8000);

        await d1.join("turtle", "lizard");
        await d2.join("crocodile", "wyvern");
        await d3.join("dragon", "wyvern");

        d1.workspace.a = 1;
        d1.workspace.b = 20;
        d1.workspace.c = 300;

        d2.workspace.a = 2;
        d2.workspace.b = 40;
        d2.workspace.c = 600;

        d3.workspace.a = 3;
        d3.workspace.b = 60;
        d3.workspace.c = 900;

        await d1.broadcast("b");
        d1.workspace.b = 0;

        await d1.sync();
        console.log(d1.workspace.b);

        d1.workspace.b = 25;
        await d1.send("lizard", "b");
        d1.workspace.b = 10;
        await d1.sync();
        console.log()
        console.log(d1.workspace.b);
    }

The output for this snippet is:

.. code:: javascript

    0

    25

.. note::

    Syncing after Broadcasting the value of 20 does not change d1's *b* variable back from 0 to 20. 
    Syncing after Sending 25 to the lizard group does change d1's *b* variable from 10 to 25, however.

----
Wait
----
The Wait command forces a client to wait until another client Sends or Broadcasts a variable to it. 
Once the Wait is done, the client can Sync the variables that were sent to it. Note that Wait does not 
automatically Sync variables. This example will use three snippets--testa, testb, and testc.

testa:

.. code:: javascript

    //testa
    async function fubara() {
        
        let d1 = new DimeClient("localhost", 8000);

        await d1.join("turtle");
        d1.workspace.a = 39;

        console.log("d1: " + d1.workspace.a);

        await d1.send("crocodile", "a");

        await d1.wait();
        await d1.sync();

        d1.workspace.a = d1.workspace.a + 1;

        console.log("d1: " + d1.workspace.a);	
    }

testb:

.. code:: javascript

    //testb
    async function fubarb() {
        
        let d2 = new DimeClient("localhost", 8000);
        await d2.join("crocodile");
        
        d2.workspace.a = 0;

        await d2.wait();
        await d2.sync();
        d2.workspace.a = d2.workspace.a + 1;

        console.log("d2: " + d2.workspace.a);
        
        await d2.send("dragon", "a");
    }

testc:

.. code:: javascript

    //testc
    async function fubarc() {
        
        let d3 = new DimeClient("localhost", 8000);
        await d3.join("dragon");

        d3.workspace.a = 0;

        await d3.wait();
        await d3.sync();
        d3.workspace.a = d3.workspace.a + 1;

        console.log("d3: " + d3.workspace.a);

        await d1.send("turtle", "a");
    }

If you run these snippets in the order of testc, testb, and then testa, the output will always be:

.. code:: javascript

    d1: 39
    d2: 40
    d3: 41
    d1: 42

.. note::
    
    Without the Wait command, there would be no guarantee as to when each client will output.
