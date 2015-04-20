import socket
import sys
import csv

# In case two few/many arguments
def usage():
    print """
    Usage:
        python serv.py [csv_file]

    """

# Sets up and runs the code
if __name__ == '__main__':

    if len(sys.argv)!=2:
        usage()
        sys.exit(2)

    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to the address given on the command line
    server_address = ('', 5556)
    sock.bind(server_address)
    sys.stderr.write('starting up on port {}\n'.format(sock.getsockname()))

    sock.listen(1)

    while True:
        sys.stderr.write('Waiting for a connection...\n')
        f = open(sys.argv[1], 'rU')
        try:
            connection, client_address = sock.accept()
        except:
            sys.stderr.write('\nServer halted while listening\n')            
            sys.exit(2)

        try:
            sys.stderr.write('Client [{}] has connected\n'.format(client_address))
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
