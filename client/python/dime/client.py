import base64
import collections.abc
import itertools
import json
import pickle
import re
import socket
import struct

from dime import dimeb
from dime import json as dimejson

__all__ = ["DimeClient"]

ADDRESS_REGEX = re.compile(r"(?P<proto>[a-z]+)://(?P<hostname>([^:]|((?<=\\)(?:\\\\)*:))+)(:(?P<port>[0-9]+))?")

class DimeClient(collections.abc.MutableMapping):
    """DiME client

    Allows a Python script to send/receive variables to other clients connected
    to a shared DiME server. Since there is no concept of a "base workspace" in
    Python, this instance acts as a mapping, where key-value pairs are
    variables in the workspace.
    """

    def __init__(self, proto = "ipc", *args):
        """Construct a dime instance

        Create a dime client via the specified protocol. The exact arguments
        depend on the protocol:

        * If the protocol is 'ipc', then the function expects one additional
          argument: the pathname of the Unix domain socket to connect to.
        * If the protocol is 'tcp', then the function expects two additional
          arguments: the hostname and port of the TCP socket to connect to, in
          that order.

        Parameters
        ----------
        proto : {'ipc', 'tcp'}
            Transport protocol to use.

        args : tuple
            Additional arguments, as described above.
        """

        self.proto = proto
        self.args = args

        self.workspace = {}

        self.open()

    def open(self, proto = None, *args, use_json = False):
        global __ADDRESS_REGEX

        if proto is None:
            proto = self.proto
            args = self.args

        if proto == "ipc" or proto == "unix":
            if len(args) == 0:
                args = ("/tmp/dime.sock",)

            self.conn = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.conn.connect(args[0])
        elif proto == "tcp":
            self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
            self.conn.connect(args)
        else:
            match = ADDRESS_REGEX.fullmatch(proto)

            if match is None:
                raise TypeError()

            proto = match.group("proto")

            if match.group("port") is not None:
                args = (match.group("hostname"), int(match.group("port")))
            else:
                args = (match.group("hostname"),)

            self.open(proto, *args)
            return

        self.__send({"command": "handshake", "serialization": "json" if use_json else "pickle", "tls": False})

        jsondata, _ = self.__recv()

        if jsondata["status"] < 0:
            raise RuntimeError(status["error"])

        self.serialization = jsondata["serialization"]

        if jsondata["serialization"] == "pickle":
            self.loads = pickle.loads
            self.dumps = pickle.dumps
        elif jsondata["serialization"] == "dimeb":
            self.loads = dimeb.loads
            self.dumps = dimeb.dumps
        elif jsondata["serialization"] == "json":
            self.loads = dimejson.loads
            self.dumps = dimejson.dumps

    def close(self):
        self.conn.close()

    def join(self, *names):
        """Send a "join" command to the server

        Instructs the DiME server to add the client to one or more groups by
        name.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        names : tuple of str
           The group name(s).
        """

        self.__send({"command": "join", "name": list(names)})

        jsondata, _ = self.__recv()

        if jsondata["status"] < 0:
            raise RuntimeError(status["error"])

    def leave(self, *names):
        """Send a "leave" command to the server

        Instructs the DiME server to remove the client from one or more groups
        by name.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        names : tuple of str
           The group name(s).
        """

        self.__send({"command": "leave", "name": list(names)})

        jsondata, _ = self.__recv()

        if jsondata["status"] < 0:
            raise RuntimeError(status["error"])

    def send(self, name, *varnames):
        """Send a "send" command to the server

        Sends one or more variables from the mapping of this instance to all
        clients in a specified group.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        name : str
           the group name.

        varnames : tuple of str
           The variable name(s) in the mapping.
        """

        self.send_r(name, **{varname: self.workspace[varname] for varname in varnames})

    def send_r(self, name, **kvpairs):
        """Send key value pairs to the server

        Sends one or more variables to all clients in a specified group.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        name : str
           the group name.
           
        **kvpairs : dict
            Keyword arguments representing the variable name(s) and their corresponding values.
        """
        
        kviter = iter(kvpairs.items())
        serialization = self.serialization

        for _ in range(0, len(kvpairs), 16):
            n = 0

            for i in range(16):
                try:
                    varname, var = next(kviter)
                except StopIteration:
                    break

                jsondata = {
                    "command": "send",
                    "name": name,
                    "varname": varname,
                    "serialization": self.serialization
                }
                bindata = self.dumps(var)

                self.__send(jsondata, bindata)

                n += 1

            for _ in range(n):
                jsondata, _ = self.__recv()

                if jsondata["status"] < 0:
                    raise RuntimeError(status["error"])

            if serialization != self.serialization:
                self.send_r(name, **kvpairs)
                return

    def broadcast(self, *varnames):
        """Send a "broadcast" command to the server

        Sends one or more variables from the mapping of this instance to all
        other clients.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        varnames : tuple of str
           The variable name(s) in the mapping.
        """

        self.broadcast_r(**{varname: self.workspace[varname] for varname in varnames})

    def broadcast_r(self, **kvpairs):
        """Send a "broadcast" command to the server

        Sends one or more key value pairs to all other clients.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        **kvpairs : dict
            Keyword arguments representing the variable name(s) and their corresponding values.
        """
        kviter = iter(kvpairs.items())
        serialization = self.serialization

        for _ in range(0, len(kvpairs), 16):
            n = 0

            for i in range(16):
                try:
                    varname, var = next(kviter)
                except StopIteration:
                    break

                jsondata = {
                    "command": "broadcast",
                    "varname": varname,
                    "serialization": self.serialization
                }
                bindata = self.dumps(var)

                self.__send(jsondata, bindata)

                n += 1

            for _ in range(n):
                jsondata, _ = self.__recv()

                if jsondata["status"] < 0:
                    raise RuntimeError(status["error"])

            if serialization != self.serialization:
                self.send_r(name, **kvpairs)
                return

    def sync(self, n = -1):
        """Send a "sync" command to the server

        Receives all variables sent to this client by all other clients.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        n : int
           The maximum number of variables to receive, or a negative value to
           receive all variables sent.

        Returns
        ----------
        dict
            Key value pairs sent to the client
        """
        updates = self.sync_r(n)

        self.workspace.update(updates)

        return set(updates.keys())

    def sync_r(self, n = -1):
        """
        Send a "sync" command to the server

        Receives all variables sent to this client by all other clients.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        n : int (optional)
           The maximum number of variables to receive, or a negative value to
           receive all variables sent.

        Returns
        ----------
        dict
            Key value pairs sent to the client

        Raises
        ----------
        RuntimeError
            If the received status is less than 0
        """

        self.__send({"command": "sync", "n": n})

        ret = {}
        m = n

        while True:
            jsondata, bindata = self.__recv()

            if "status" in jsondata:
                if jsondata["status"] < 0:
                    raise RuntimeError(status["error"])

                break

            if jsondata["serialization"] == "pickle":
                var = pickle.loads(bindata)
            elif jsondata["serialization"] == "dimeb":
                var = dimeb.loads(bindata)
            elif jsondata["serialization"] == "json":
                var = dimejson.loads(bindata)
            else:
                m -= 1
                continue

            ret[jsondata["varname"]] = var

        if n > 0 and m < n:
            ret.update(self.sync_r(n - m))

        return ret

    def wait(self):
        """Send a "wait" command to the server

        Tell the server to send a message once at least one message has been
        received for the client. This method blocks the current thread of
        execution until the message is received.

        Parameters
        ----------
        obj : DimeClient
            The dime instance.
        """

        self.__send({"command": "wait"})

        jsondata, _ = self.__recv()

        if jsondata["status"] < 0:
            raise RuntimeError(status["error"])

        return jsondata["n"]

    def devices(self):
        """send Send a "devices" command to the server

        Tell the server to send this client a list of all the named,
        nonempty groups connected to the server.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        Returns
        -------
        list of str
            A list of all groups connected to the DiME server.
        """

        self.__send({"command": "devices"})
        jsondata, _ = self.__recv()

        if jsondata["status"] < 0:
            raise RuntimeError(status["error"])

        return jsondata["devices"]

    def __send(self, jsondata, bindata = b""):
        #print("->", jsondata)

        jsondata = json.dumps(jsondata).encode("utf-8")

        data = b"DiME" + \
               struct.pack("!II", len(jsondata), len(bindata)) + \
               jsondata + \
               bindata

        self.conn.sendall(data)

    def __recv(self):
        header = self.conn.recv(12, socket.MSG_WAITALL)

        if header[:4] != b"DiME":
            raise RuntimeError("Invalid DiME message")

        jsondata_len, bindata_len = struct.unpack("!II", header[4:])

        data = self.conn.recv(jsondata_len + bindata_len, socket.MSG_WAITALL)
        jsondata = json.loads(data[:jsondata_len].decode("utf-8"))
        bindata = data[jsondata_len:]

        if "status" in jsondata and jsondata["status"] > 0 and "meta" in jsondata and jsondata["meta"]:
            self.__meta(jsondata)
            return self.__recv()

        #print("<-", jsondata)

        return jsondata, bindata

    def __meta(self, jsondata):
        if jsondata["command"] == "reregister":
            self.serialization = jsondata["serialization"]

            if jsondata["serialization"] == "pickle":
                self.loads = pickle.loads
                self.dumps = pickle.dumps
            elif jsondata["serialization"] == "dimeb":
                self.loads = dimeb.loads
                self.dumps = dimeb.dumps
            elif jsondata["serialization"] == "json":
                self.loads = dimejson.loads
                self.dumps = dimejson.dumps
        else: # No other commands supported yet
            raise RuntimeError("Received unknown meta-status from server")

    def __enter__(self):
        self.conn.__enter__()
        return self

    def __exit__(self, etype, value, traceback):
        return self.conn.__exit__(etype, value, traceback)

    def __getitem__(self, varname):
        return self.workspace.__getitem__(varname)

    def __setitem__(self, varname, var):
        self.workspace.__setitem__(varname, var)

    def __delitem__(self, varname):
        self.workspace.__delitem__(varname)

    def __iter__(self):
        return self.workspace.__iter__()

    def __len__(self):
        return self.workspace.__len__()
