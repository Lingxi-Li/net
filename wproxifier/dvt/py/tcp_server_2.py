import socket

HOST = 'localhost'
PORT = 8001

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    print(f'Server listening on {HOST}:{PORT}')
    while True:
        conn, addr = s.accept()
        with conn:
            print(f'Connected by {addr}')
            while data := conn.recv(1024):
                msg = data.decode().title()
                conn.sendall(msg.encode())
