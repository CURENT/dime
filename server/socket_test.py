import socket
import struct
import json
import os

def send(sock, jsondata, bindata):
    print("->", jsondata, bindata)

    msg = bytearray()

    jsondata = json.dumps(jsondata).encode("utf-8")

    msg += struct.pack("!4sII", b"DiME", len(jsondata), len(bindata))
    msg += jsondata
    msg += bindata

    sock.sendall(msg, socket.MSG_WAITALL)

def recv(sock):
    magic, jsondata_len, bindata_len = struct.unpack("!4sII", sock.recv(12, socket.MSG_WAITALL))

    if magic != b"DiME":
        raise Exception("Invalid magic value (got " + repr(magic) + ")")

    jsondata = json.loads(sock.recv(jsondata_len, socket.MSG_WAITALL).decode("utf-8"))
    bindata = sock.recv(bindata_len, socket.MSG_WAITALL)

    print("<-", jsondata, bindata)

    return jsondata, bindata

def connect(pathname):
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    sock.connect(pathname)

    return sock

if __name__ == "__main__":
    sock = connect("/tmp/dime.sock")

    send(sock, {"command": "register", "name": "test"}, b"")
    recv(sock)
    send(sock, {"command": "send", "name": "test", "pi": 3.14}, b"Hello world!\n")
    send(sock, {"command": "sync"}, b"")
    recv(sock)
