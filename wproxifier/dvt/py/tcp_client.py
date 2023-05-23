import socket

HOST = 'localhost'
PORT = 8000

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    while True:
        msg = input('>>')
        s.sendall(msg.encode())