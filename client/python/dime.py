import collections.abc
import itertools
import json
import pickle
import socket
import struct

import dimeb

class DimeClient(collections.abc.MutableMapping):
    """DiME client

    Allows a Python script to send/receive variables to other clients connected
    to a shared DiME server. Since there is no concept of a "base workspace" in
    Python, this instance acts as a mapping, where key-value pairs are
    variables in the workspace.
    """

    def __init__(self, name, proto, *args):
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

        args
            Additional arguments, as described above.
        """

        self.workspace = {}

        if proto == "ipc":
            self.conn = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.conn.connect(args[0])
        elif proto == "tcp":
            self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
            self.conn.connect(args)

        self.__send({"command": "register", "serialization": "pickle"})

        jsondata, _ = self.__recv()

        #if jsondata["status"] != 0:
        #    raise RuntimeError(status["error"])

        self.serialization = jsondata["serialization"]

        if jsondata["serialization"] == "pickle":
            self.loads = pickle.loads
            self.dumps = pickle.dumps
        elif jsondata["serialization"] == "dimeb":
            self.loads = dimeb.loads
            self.dumps = dimeb.dumps

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

        for name in names:
            self.__send({"command": "join", "name": name})

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

        for name in names:
            self.__send({"command": "leave", "name": name})

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

        for varname in varnames:
            jsondata = {
                "command": "send",
                "name": name,
                "varname": varname,
                "serialization": self.serialization
            }

            bindata = self.dumps(self.workspace[varname])

            self.__send(jsondata, bindata)

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

        for varname in varnames:
            jsondata = {
                "command": "broadcast",
                "varname": varname,
                "serialization": self.serialization
            }

            bindata = self.dumps(self.workspace[varname])

            self.__send(jsondata, bindata)

    def sync(self, n = -1):
        """Send a "sync" command to the server

        Sends one or more variables from the mapping of this instance to all
        other clients.

        Parameters
        ----------
        self : DimeClient
            The dime instance.

        n : int
           The maximum number of variables to receive, or a negative value to
           receive all variables sent.
        """

        self.__send({"command": "sync", "n": n})

        while True:
            jsondata, bindata = self.__recv()

            if "varname" not in jsondata:
                break

            if jsondata["serialization"] == "pickle":
                var = pickle.loads(bindata)
            elif jsondata["serialization"] == "dimeb":
                var = dimeb.loads(bindata)
            else:
                continue

            self.workspace[jsondata["varname"]] = var

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

        return jsondata["devices"]

    def __send(self, jsondata, bindata = b""):
        jsondata = json.dumps(jsondata).encode("utf-8")

        data = b"DiME" + \
               struct.pack("!II", len(jsondata), len(bindata)) + \
               jsondata + \
               bindata

        self.conn.__sendall(data)

    def __recv(self):
        header = self.conn.__recv(12, socket.MSG_WAITALL)

        if header[:4] != b"DiME":
            raise RuntimeError("Invalid DiME message")

        jsondata_len, bindata_len = struct.unpack("!II", header[4:])

        data = self.conn.__recv(jsondata_len + bindata_len, socket.MSG_WAITALL)
        jsondata = json.loads(data[:jsondata_len].decode("utf-8"))
        bindata = data[jsondata_len:]

        return jsondata, bindata

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
