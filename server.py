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

# Set a 200MB receiver buffer size *
BUFLEN = 200000000

class JSONSeqPacket:
    def __init__(self, conn):
        self.conn = conn
        self.conn.setblocking(False)

        self.sync = False
        self.closed = False

        self.wbuf = collections.deque()

    def sendpartial(self):
        if self.wbuf:
            try:
                self.conn.send(self.wbuf.popleft())
            except BrokenPipeError:
                self.closed = True
        else:
            self.sync = False

    def recvpartial(self):
        msg = self.conn.recv(BUFLEN).decode("utf-8")
        print(msg)

        if msg:
            return json.loads(msg)
        else:
            self.closed = True

    def push(self, msg):
        self.wbuf.append(json.dumps(msg).encode("utf-8"))

    def empty(self):
        return self.sync

    def synchronize(self):
        self.sync = (len(self.wbuf) > 0)

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

'''
class JSONStream:
    def __init__(self, conn):
        self.conn = conn
        self.conn.setblocking(False)

        self.rbuf = bytearray()
        self.wbuf = bytearray()

    def sendpartial(self):
        ct = self.conn.send(self.wbuf)
        self.wbuf = self.wbuf[ct:]

    def recvpartial(self):
        self.rbuf.extend(self.conn.recv(BUFLEN))

        try:
            msg, ct = json.JSONDecoder().raw_decode(self.rbuf.decode("utf-8"))
        except ValueError:
            return None

        self.rbuf = self.rbuf[ct:]

        return msg

    def push(self, msg):
        self.wbuf.extend(json.dumps(msg).encode("utf-8"))

    def empty(self):
        return len(self.wbuf) == 0

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
'''

def handle_message(clients, sender, msg):
    cmd = msg["command"]

    if cmd == "register":
        clients[msg["name"]] = sender

    elif cmd == "send":
        try:
            clients[msg["name"]].push(msg)
        except KeyError:
            pass

    elif cmd == "sync":
        sender.synchronize()

    elif cmd == "broadcast":
        for conn in clients.values():
            if conn is not sender:
                conn.push(msg)

def serverloop(srv):
    clients_by_fd = {}
    clients_by_name = weakref.WeakValueDictionary()

    while True:
        xs = list(clients_by_fd.values())

        rs = [srv] + xs
        ws = [conn for conn in clients_by_fd.values() if conn.sync]
        print(ws)

        rs, ws, xs = select.select(rs, ws, xs)

        for r in rs:
            if r is srv:
                conn, addr = srv.accept()
                conn = JSONSeqPacket(conn)
                print("c on %d" % conn.fileno())

                clients_by_fd[conn.fileno()] = conn

            else:
                print("r on %d" % r.fileno())
                msg = r.recvpartial()

                if msg is not None:
                    handle_message(clients_by_name, r, msg)

        for w in ws:
            w.sendpartial()

        for conn in itertools.chain(rs, ws):
            if  conn is not srv and conn.closed:
                print("x on %d" % conn.fileno())
                del clients_by_fd[conn.fileno()]
                conn.close()

        for x in xs:
            print("x on %d" % w.fileno())
            del clients_by_fd[x.fileno()]
            x.close()

if __name__ == "__main__":
    regex = re.compile(r"(?P<proto>[a-z]+)://(?P<hostname>([^:]|((?<=\\)(?:\\\\)*:))+)(:(?P<port>[0-9]+))?")
    sockopts = []

    if len(sys.argv) > 1 and regex.fullmatch(sys.argv[1]) is not None:
        match = regex.fullmatch(sys.argv[1])

        proto_tab = {
            "ipc": (socket.AF_UNIX, socket.SOCK_SEQPACKET, 0),
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
        socktype = socket.SOCK_SEQPACKET
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
