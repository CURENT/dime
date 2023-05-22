# JavaScript API Reference
## DiME Instantiation 
```
new dime.DimeClient(hostname, port)
```
Creates a new DiME instance and connects to the server.

> **Parameters:**
>> **hostname:** ***string***
>>	The hostname of the server.

>> **port:** ***number***
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

## Send_R
```
DimeClient.send_r(name, kvpairs)
```
Sends one or more variables to all clients in a specified group.

> **Parameters:**
>> **name:** ***string***
>>> The name of the group to send the variables to.

>> **kvpairs** ***associative array***
>>> Key value pairs to be sent to the server.

## Broadcast
```
DimeClient.broadcast(varargin)
```
Sends one or more variables to all other clients.

> **Parameters:**
>> **varargin:** ***string, string, ...***
>>> The names of the variables being sent.

## Broadcast_R
```
DimeClient.broadcast_r(kvpairs)
```
Sends one or more key value pairs to all other clients.

> **Parameters:**
>> **kvpairs** ***associative array***
>>> Key value pairs to be sent to the server.

## Sync
```
DimeClient.sync(n)
```
Requests all variables that have been sent to this client by other clients.

> **Parameters:**
>> **n:** ***number***
>>> The number of variables to retrieve from the server.

Sync will retrieve all variables if n is left unspecified or set to a negative value.

## Sync_R
```
DimeClient.sync_r(n)
```
Requests all variables that have been sent to this client by other clients.

> **Parameters:**
>> **n:** ***number***
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
>> **string[]**
>>> An array containing names of all the groups connected to the DiME server.


DiME also uses two custom JavaScript objects to handle interactions between the JavaScript client and the MATLAB and Python clients--Complex and NDArray.

## Complex
A custom object that represents complex numbers.

### Instantiation
```
new dime.Complex(real, imaginary)
```
Creates a new DiME Complex object.

> **Parameters:**
>> **real:** ***number***
>>	The real number part of the complex number.

>> **imaginary:** ***number***
>>	The imaginary part of the complex number

> **Returns:**
>> **Complex**
>>> The newly created Complex object.

### Add
```
Complex.add(rhs)
```
Adds two Complex objects.

> **Parameters:**
>> **rhs:** ***Complex***
>>	The Complex object on the right hand side of the equation.

> **Returns:**
>> **Complex**
>>> The sum of the two Complex objects.

### Sub
```
Complex.sub(rhs)
```
Subtracts two Complex objects.

> **Parameters:**
>> **rhs:** ***Complex***
>>	The Complex object on the right hand side of the equation (the subtractor).

> **Returns:**
>> **Complex**
>>> The difference of the two Complex objects.

### Mul
```
Complex.mul(rhs)
```
Multiplies two Complex objects.

> **Parameters:**
>> **rhs:** ***Complex***
>>	The Complex object on the right hand side of the equation.

> **Returns:**
>> **Complex**
>>> The product of the two Complex objects.

### Div
```
Complex.div(rhs)
```
Divides two Complex objects.

> **Parameters:**
>> **rhs:** ***Complex***
>>	The Complex object on the right hand side of the equation (the divisor).

> **Returns:**
>> **Complex**
>>> The quotient of the two Complex objects.

## NDArrays
A custom N-dimensional array object that acts similarly to the numpy object with the same name.

### Instantiation
```
new dime.NDArray(order, shape, array, complex)
```
Creates a new DiME NDArray object.

> **Parameters:**
>> **order:** ***{'C', 'F'}***
>>	The way the array is ordered. C-style (row-major) or Fortran-style (column-major). 

>> **shape:** ***number, ...***
>>	The length of each dimension of the array.

>> **array:** ***number[]***
>>	Optional. An already formatted array to be converted into an NDArray.

>> **complex:** ***boolean***
>>	Optional. Defaults to *false*. Whether the NDArray will consist of complex numbers or not.

> **Returns:**
>> **NDArray**
>>> The newly created NDArray object.

### Get
```
NDArray.get(index)
```
Returns the value of the NDArray at the given index.

> **Parameters:**
>> **index:** ***number, ...***
>>	The index of a value in the array. In 2D NDArray, for example, it would be *(number, number)*. 

> **Returns:**
>> **Value**
>>> The value of whatever has been stored in the NDArray at that index.

### Set
```
NDArray.set(value, index)
```
Sets the given value at the index of the NDArray.

> **Parameters:**
>> **value:** ***number***
>>	The value you are storing.

>> **index:** ***number, ...***
>>	The index that you intend to store **value** in. In a 2D NDArray, for example, it would be *(number, number)*.

### Column
```
NDArray.column(n)
```
Returns a given column from an NDArray. Only works for Fortran-style, 2D arrays. Does not support Complex matrices.

> **Parameters:**
>> **n:** ***number***
>>	The zero-indexed number of the column you would like returned. 

> **Returns:**
>> **Value[]**
>>> The specified column.

### Row
```
NDArray.row(n)
```
Returns a given row from an NDArray. Only works for C-style, 2D arrays. Does not support Complex matrices.

> **Parameters:**
>> **n:** ***number***
>>	The zero-indexed number of the row you would like returned. 

> **Returns:**
>> **Value[]**
>>> The specified row.

### Extents
```
NDArray.extents()
```
Returns the starting and ending point of an NDarray formated like a column vector. Does not support Complex matrices. NDArray must be contiguous.

> **Returns:**
>> **{begin, end}**
>>> An object containing the extents of the NDArray.

### Subarray
```
NDArray.subarray({being, end})
```
Returns the subarray of the NDArray based on the beginning and ending points provided. Does not support Complex matrices.

> **Parameters:**
>> **{begin, end}:** ***{number, number}***
>>	An object containing the specified beginning and end points. 

> **Returns:**
>> **NDArray**
>>> A new NDArray made from the NDArray that called Subarray.

### Subindex
```
NDArray.subindex(idx)
```
Creates a new NDArray based on the stored values in **idx**. Does not support Complex matrices.

> **Parameters:**
>> **idx:** ***NDArray***
>>	An NDArray where each value corresponds to an index in the NDArray that called Subindex. 

> **Returns:**
>> **NDArray**
>>> A new NDArray made from the indeces contained in **idx**.


The JavaScript client also has a few specialty functions for handling data.
## JSONloads
```
jsonloads(obj)
```
Loads a new object loaded from the JSON passed in **obj**.

> **Parameters:**
>> **obj:** ***JSON string***
>>	A string containing JSON data.

> **Returns:**
>> **obj**
>>> The Object that the JSON decoded to.


## JSONdumps
```
jsondumps(obj)
```
Returns the JSON string for an object.

> **Parameters:**
>> **obj:** ***{}***
>>	An object to be converted into a JSON string.

> **Returns:**
>> **JSON string**
>>> The JSON string of **obj**.

## DiMEbloads
```
dimebloads(bytes)
```
Loads an object from bytes.

> **Parameters:**
>> **bytes** ***bytes[]***
>>	An array of bytes.

> **Returns:**
>> **obj**
>>> The object that the bytes decoded to.

## DiMEbdumps
```
dimebdumps(obj)
```
Returns an the bytes of **obj**.

> **Parameters:**
>> **obj** ***Object***
>>	An object.

> **Returns:**
>> **bytes[]**
>>> An array of the bytes making up **obj**.

