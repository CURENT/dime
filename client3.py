import socket

conn = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
conn.connect("/tmp/dime.sock")

conn.send(b'{"command": "register", "name": "client3"}')
print('-> {"command": "register", "name": "client3"}')
conn.send(b'{"command": "send", "name": "client1", "varname": "x", "varvalue": 3}')
print('-> {"command": "send", "name": "client1", "varname": "x", "varvalue": 3}')
#conn.send(b'{"command": "broadcast", "varname": "z", "varvalue": ["hello world!"]}')

conn.shutdown(socket.SHUT_RDWR)
conn.close()
