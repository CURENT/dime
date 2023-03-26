# MATLAB API Reference
## DiME Instantiation 
```
dime(protocol, varargin)
```
Creates a new DiME instance using the specified protocol.

> **Parameters:**
>> **protocol:** ***string ('ipc' or 'tcp')***
>>	The chosen protocol. Must either be 'ipc' or 'tpc'.

>> **varargin:** ***string or string, integer***
>>	Additional parameters. Varies based on the protocol chosen.

>>>	**IPC:** The function expects one additional argument--the pathname of the Unix socket to connect to.

>>>	**TCP:** The function expects two additional arguments--the hostname (string) and port (integer) of the socket, in that order.

If no arguments are given, the function will default to using **'ipc'** and **'tmp/dime.sock'**.

> **Returns:**
>> **dime**
>>> The newly created DiME object.

## DiME Deletion
```
delete(obj)
```
Deletes a DiME instance and performs cleanup on the connection.

> **Parameters:**
>> **obj:** dime
>>> The DiME object to be deleted.

## Join
```
dime.join(varargin)
```
Instructs the DiME server to add the client to the specified groups.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> A cell array of strings containing the group names for the client to join.

## Leave
```
dime.leave(varargin)
```
Instructs the DiME server to remove the client from the specified groups.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> A cell array of strings containing the group names for the client to leave.

## Send
```
dime.send(name, varargin)
```
Sends one or more variables to the specified group.

> **Parameters:**
>> **name:** ***string***
>>> The name of the group to send the variables to.

>> **varargin:** ***string, string, ...***
>>> The names of the variables being sent.

## Broadcast
```
dime.broadcast(varargin)
```
Sends one or more variables to all other clients.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> The names of the variables being sent.

## Sync
```
dime.sync(n)
```
Requests all variables that have been sent to this client by other clients.

> **Parameters:**
>> **n:** ***integer***
>>> The number of variables to retrieve from the server.

Sync will retrieve all variables if n is left unspecified or set to a negative value.

## Wait
```
dime.wait()
```
Requests that the server sends a message to the client once a message has been received for said client. This call will block the current thread until the message is received.

## Devices
```
dime.devices()
```
Requests a list of all named, nonempty groups from the server.

> **Returns:**
>> **{string, string, ...}**
>>> A cell array containing names of all the groups connected to the DiME server.
