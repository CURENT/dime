import collections.abc
import itertools
import json
import pickle
import socket
import struct

import dimeb

class DimeClient(collections.abc.MutableMapping):
    def __init__(self, name, proto, *args):
        self.workspace = {}

        if proto == "ipc":
            self.conn = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.conn.connect(args[0])
        elif proto == "tcp":
            self.conn = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)
            self.conn.connect(args)

        self.name = name

        self.send({
            "command": "register",
            "name": name,
            "serialization": "pickle"
        })

        jsondata, _ = self.recv()

        #if jsondata["status"] != 0:
        #    raise RuntimeError(status["error"])

        self.serialization = jsondata["serialization"]

        if jsondata["serialization"] == "pickle":
            self.loads = pickle.loads
            self.dumps = pickle.dumps
        elif jsondata["serialization"] == "dimeb":
            self.loads = dimeb.loads
            self.dumps = dimeb.dumps

    def send_var(self, name, *varnames):
        for varname in varnames:
            jsondata = {
                "command": "send",
                "name": name,
                "varname": varname,
                "serialization": self.serialization
            }

            bindata = self.dumps(self.workspace[varname])

            self.send(jsondata, bindata)

    def broadcast(self, *varnames):
        for varname in varnames:
            jsondata = {
                "command": "broadcast",
                "varname": varname,
                "serialization": self.serialization
            }

            bindata = self.dumps(self.workspace[varname])

            self.send(jsondata, bindata)

    def sync(self, n = -1):
        self.send({"command": "sync", "n": n})

        while True:
            jsondata, bindata = self.recv()

            if "varname" not in jsondata:
                break

            if jsondata["serialization"] == "pickle":
                var = pickle.loads(bindata)
            elif jsondata["serialization"] == "dimeb":
                var = dimeb.loads(bindata)
            else:
                continue

            self.workspace[jsondata["varname"]] = var

    def get_devices(self):
        self.send({"command": "devices"})
        jsondata, _ = self.recv()

        return jsondata["devices"]

    def send(self, jsondata, bindata = b""):
        jsondata = json.dumps(jsondata).encode("utf-8")

        data = b"DiME" + \
               struct.pack("!II", len(jsondata), len(bindata)) + \
               jsondata + \
               bindata

        self.conn.sendall(data)

    def recv(self):
        header = self.conn.recv(12, socket.MSG_WAITALL)

        if header[:4] != b"DiME":
            raise RuntimeError("Invalid DiME message")

        jsondata_len, bindata_len = struct.unpack("!II", header[4:])

        data = self.conn.recv(jsondata_len + bindata_len, socket.MSG_WAITALL)
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
