import socket
import time

conn = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET)
conn.connect("/tmp/dime.sock")

conn.send(b'{"command": "register", "name": "client1"}')
print('-> {"command": "register", "name": "client1"}')
time.sleep(5)
conn.send(b'{"command": "sync"}')
print('-> {"command": "sync"}')
print('<- ' + conn.recv(4096).decode())
#print('<- ' + conn.recv(4096).decode())
#conn.send(b'{"command": "send", "name": "client2", "varname": "y", "varvalue": null}')
#print('-> {"command": "send", "name": "client2", "varname": "y", "varvalue": null}')

conn.shutdown(socket.SHUT_RDWR)
conn.close()
