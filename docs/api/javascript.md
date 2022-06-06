# JavaScript API Reference
## DiME Instantiation 
```
new dime.DimeClient(hostname, port)
```
Creates a new DiME instance and connects to the server.

> **Parameters:**
>> **hostname:** ***string***
>>	The hostname of the server.

>> **port:** ***integer***
>>	The port the server is running on.

> **Returns:**
>> **DimeClient**
>>> The newly created DimeClient.

## Close
```
DimeClient.close()
```
Performs cleanup on the DimeClient connection.

## Join
```
DimeClient.join(varargin)
```
Instructs the DiME server to add the client to the specified groups.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> Strings containing the group names for the client to join.

## Leave
```
DimeClient.leave(varargin)
```
Instructs the DiME server to remove the client from the specified groups.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> Strings containing the group names for the client to leave.

## Send
```
DimeClient.send(name, varargin)
```
Sends one or more variables to the specified group.

> **Parameters:**
>> **name:** ***string***
>>> The name of the group to send the variables to.

>> **varargin:** ***string, string, ...***
>>> The names of the variables being sent.

## Braodcast
```
DimeClient.broadcast(varargin)
```
Sends one or more variables to all other clients.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> The names of the variables being sent.

## Sync
```
DimeClient.sync(n)
```
Requests all variables that have been sent to this client by other clients.

> **Parameters:**
>> **n:** ***integer***
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
>>> An array containing names of all the groups connected to the DiME server.
