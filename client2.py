import socket

conn = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
conn.connect("/tmp/dime.sock")

conn.send(b'{"command": "register", "name": "client2"}')
print('-> {"command": "register", "name": "client2"}')
print('<- ' + conn.recv(4096).decode())
print('<- ' + conn.recv(4096).decode())

conn.close()
