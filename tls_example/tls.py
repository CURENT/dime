import socket, ssl

#context = ssl.create_default_context()
context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
context.load_verify_locations('cert.pem')
context.check_hostname = False

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
    sock.connect(('localhost', 4433))

    with context.wrap_socket(sock) as ssock:
        while True:
            x = ssock.read().decode('utf-8')

            if not x:
                break

            print(x, end = "")
