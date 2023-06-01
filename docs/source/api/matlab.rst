.. _api_matlab:

====================
MATLAB API Reference
====================


``dime(protocol, varargin)`` Creates a new DiME instance using the specified protocol.
``delete(obj)`` Deletes a DiME instance and performs cleanup on the connection.
``dime.join(varargin)`` Instructs the DiME server to add the client to the specified groups.




------------------------
dime(protocol, varargin)
------------------------
Creates a new DiME instance using the specified protocol.

-----------
delete(obj)
-----------
Deletes a DiME instance and performs cleanup on the connection.

-------------------
dime.join(varargin)
-------------------
Instructs the DiME server to add the client to the specified groups.

--------------------
dime.leave(varargin)
--------------------
Instructs the DiME server to remove the client from the specified group.

-------------------------
dime.send(name, varargin)
-------------------------
Sends one or more variables to the specified group.

---------------------------
dime.send_r(name, varargin)
---------------------------
Sends one or more variables passed as either a struct or as key-value pairs to the specified group.

------------------------
dime.broadcast(varargin)
------------------------
Sends one or more variables to all other clients.

--------------------------
dime.broadcast_r(varargin)
--------------------------
Sends one or more variables to all other clients.

------------
dime.sync(n)
------------
Requests all variables that have been sent to this client by other clients.

--------------
dime.sync_r(n)
--------------
Requests all variables that have been sent to this client by other clients. Does not access the workspace.

-----------
dime.wait()
-----------
Requests that the server sends a message to the client once a message has been received for said client.
This call will block the current thread until the message is received.

--------------
dime.devices()
--------------
Requests a list of all named, nonempty groups from the server.


