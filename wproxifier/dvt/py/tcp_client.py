import socket

HOST = 'localhost'
PORT = int(input("Enter server port: "))

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    while True:
        msg = input('>>')
        s.sendall(msg.encode())
        reply = s.recv(1024)
        print(f'<<{reply.decode()}')