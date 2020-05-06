import socket
import json
import select
import re
import os
import atexit
import sys
import weakref
import collections
import itertools
import struct

class MessageStream:
    def __init__(self, conn):
        self.conn = conn
        self.conn.setblocking(False)

        self.closed = False

        self.rbuf = bytearray()
        self.wbuf = bytearray()

    def sendpartial(self):
        if self.wbuf:
            try:
                n = self.conn.send(self.wbuf)
            except BrokenPipeError:
                self.closed = True
            else:
                self.wbuf = self.wbuf[n:]

    def recvpartial(self):
        buf = self.conn.recv(16384)

        if not buf:
            self.closed = True
        else:
            self.rbuf += buf
            magic = self.rbuf[:4]

            if magic != b"DiME":
                self.closed = True
            elif len(self.rbuf) >= 12:
                json_len, bindata_len = struct.unpack("!II", self.rbuf[4:12])

                if len(self.rbuf) >= 12 + json_len + bindata_len:
                    jsondata = json.loads(self.rbuf[12:12 + json_len].decode("utf-8"))
                    bindata = bytes(self.rbuf[12 + json_len:12 + json_len + bindata_len])

                    self.rbuf = self.rbuf[12 + json_len + bindata_len:]

                    return jsondata, bindata

    def push(self, jsondata, bindata):
        jsondata = json.dumps(jsondata).encode("utf-8")
        header = b"DiME" + struct.pack("!II", len(jsondata), len(bindata))

        self.wbuf += header + jsondata + bindata

    def empty(self):
        return (len(self.wbuf) == 0)

    def fileno(self):
        return self.conn.fileno()

    def close(self):
        # Attempt to shutdown before closing
        if hasattr(self.conn, "shutdown"):
            try:
                self.conn.shutdown(socket.SHUT_RDWR)
            except:
                pass

        self.conn.close()

def serverloop(srv):
    clients_by_fd = {}
    clients_by_name = weakref.WeakValueDictionary()
    queues_by_fd = {}
    queues_by_name = weakref.WeakValueDictionary()

    while True:
        xs = list(clients_by_fd.values())

        rs = [srv] + xs
        ws = [conn for conn in clients_by_fd.values() if not conn.empty()]

        rs, ws, xs = select.select(rs, ws, xs)

        for r in rs:
            if r is srv:
                conn, addr = srv.accept()
                conn = MessageStream(conn)

                clients_by_fd[conn.fileno()] = conn
                queues_by_fd[conn.fileno()] = collections.deque()

            else:
                msg = r.recvpartial()

                if msg is not None:
                    jsondata, bindata = msg
                    cmd = jsondata["command"]

                    if cmd == "register":
                        name = jsondata["name"]

                        if name not in clients_by_name:
                            clients_by_name[name] = r
                            queues_by_name[name] = queues_by_fd[r.fileno()]

                            r.push({"status": 0}, b"")

                        else:
                            r.push({"status": 1, "errmsg": name + " already in use"}, b"")

                    elif cmd == "send":
                        name = jsondata["name"]

                        try:
                            queues_by_name[name].append((jsondata, bindata))
                        except KeyError:
                            pass

                    elif cmd == "broadcast":
                        for fd, queue in queues_by_fd.items():
                            if fd != r.fileno():
                                queue.append((jsondata, bindata))

                    elif cmd == "sync":
                        queue = queues_by_fd[r.fileno()]

                        while queue:
                            r.push(*queue.popleft())

                        r.push({}, b"")

        for w in ws:
            w.sendpartial()

        for conn in itertools.chain(rs, ws):
            if conn is not srv and conn.closed:
                xs.append(conn)

        for x in xs:
            try:
                del clients_by_fd[x.fileno()]
                del queues_by_fd[x.fileno()]
            except KeyError:
                pass
            else:
                x.close()

if __name__ == "__main__":
    regex = re.compile(r"(?P<proto>[a-z]+)://(?P<hostname>([^:]|((?<=\\)(?:\\\\)*:))+)(:(?P<port>[0-9]+))?")
    sockopts = []

    if len(sys.argv) > 1 and regex.fullmatch(sys.argv[1]) is not None:
        match = regex.fullmatch(sys.argv[1])

        proto_tab = {
            "ipc": (socket.AF_UNIX, socket.SOCK_STREAM, 0),
            "tcp": (socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP),
            "sctp": (socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_SCTP)
        }

        family, socktype, proto = proto_tab[match.group("proto")]

        if family == socket.AF_UNIX:
            address = match.group("hostname")
        elif match.group("port") is not None:
            address = (match.group("hostname"), int(match.group("port")))
        else:
            address = (match.group("hostname"), 5000)

    elif os.name == "posix":
        family = socket.AF_UNIX
        socktype = socket.SOCK_STREAM
        proto = 0

        address = "/tmp/dime.sock"

    else:
        family = socket.AF_INET
        socktype = socket.SOCK_STREAM
        proto = socket.IPPROTO_TCP

        address = ("localhost", 5000)

    if family == socket.AF_UNIX:
        atexit.register(os.unlink, address)

    # Use dual-stack IPv4/IPv6 if possible
    if family == socket.AF_INET and socket.hasipv6:
        family = socket.AF_INET6
        address += (0, 0)

    with socket.socket(family, socktype, proto) as srv:
        if family == socket.AF_INET6:
            try:
                srv.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_V6ONLY, 0)
            except AttributeError:
                pass

        srv.bind(address)
        srv.listen()

        serverloop(srv)
