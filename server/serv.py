import socket
import sys
import csv

# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the socket to the address given on the command line
server_address = ('', 5554)
sock.bind(server_address)
print (sys.stderr, 'starting up on %s port %s' % sock.getsockname())
sock.listen(1)


while True:
    print(sys.stderr, 'waiting for a connection')
    f= open(sys.argv[1], 'rt')
    connection, client_address = sock.accept()
    try:
        print(sys.stderr, 'client connected:', client_address)
        reader = csv.DictReader(f)
        for row in reader:
            data = str(row) +'\n'
            if data:
                connection.sendall(data.encode())
            else:
                break
    finally:
        f.close()
        connection.close()
