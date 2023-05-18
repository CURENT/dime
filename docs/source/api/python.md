# Python API Reference
## DiME Instantiation 
```
DimeClient(protocol, varargin)
```
Creates a new DiME instance using the specified protocol.

> **Parameters:**
>> **protocol:** ***string ('ipc' or 'tcp')***
>>	The chosen protocol. Must either be 'ipc' or 'tpc'.

>> **varargin:** ***string or string, int***
>>	Additional parameters. Varies based on the protocol chosen.

>>>	**IPC:** The function expects one additional argument--the pathname of the Unix socket to connect to.

>>>	**TCP:** The function expects two additional arguments--the hostname (string) and port (integer) of the socket, in that order.

If no arguments are given, the function will default to using **'ipc'** and **'tmp/dime.sock'**.

> **Returns:**
>> **DimeClient**
>>> The newly created DimeClient.

## Close
```
DimeClient.close()
```
Closes the connection for the this instance of a DimeClient.

## Join
```
DimeClient.join(varargin)
```
Instructs the DiME server to add the client to the specified groups.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> A tuple of strings containing the group names for the client to join.

## Leave
```
DimeClient.leave(varargin)
```
Instructs the DiME server to remove the client from the specified groups.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> A tuple of strings containing the group names for the client to leave.

## Send
```
DimeClient.send(name, varargin)
```
Send a "send" command to the server.

Sends one or more variables from the mapping of this instance to all clients in a specified group.

> **Parameters:**
>> **name:** ***string***
>>> The name of the group to send the variables to.

>> **varargin:** ***string, string, ...***
>>> A tuple of the names of the variables being sent.

## Send_R
```
DimeClient.send_r(name, **kvpairs)
```
Sends one or more key value pairs to all clients in a specified group.

> **Parameters:**
>> **name:** ***string***
>>> The name of the group to send the variables to.

>> **\*\*kvpairs**
>>> Key value pairs to be sent to the server.

## Broadcast
```
DimeClient.broadcast(varargin)
```
Sends one or more variables to all other clients.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> A tuple of the names of the variables being sent.

## Broadcast_R
```
DimeClient.broadcast_r(**kvpairs)
```
Sends one or more key value pairs to all other clients.

> **Parameters:**
>> **\*\*kvpairs**
>>> Key value pairs to be sent to the server.

## Sync
```
DimeClient.sync(n)
```
Requests all variables that have been sent to this client by other clients.

> **Parameters:**
>> **n:** ***int***
>>> The number of variables to retrieve from the server.

Sync will retrieve all variables if n is left unspecified or set to a negative value.

## Sync_R
```
DimeClient.sync_r(n)
```
Requests all variables that have been sent to this client by other clients.

> **Parameters:**
>> **n:** ***int***
>>> The number of variables to retrieve from the server.

Sync will retrieve all variables if n is left unspecified or set to a negative value.

## Wait
```
DimeClient.wait()
```
Requests that the server sends a message to the client once a message has been received for said client. This call will block the current thread until the message is received.

## Devices
```
DimeClient.devices()
```
Requests a list of all named, nonempty groups from the server.

> **Returns:**
>> **[string, string, ...]**
>>> A list containing names of all the groups connected to the DiME server.
