from __future__ import print_function
import datetime
import sys
from socket import *
from ssl import *

BYTES_TO_SEND = 512  # Change this to BYTES_TO_SEND present in app.c
MAX_DATA_BYTES = BYTES_TO_SEND * 1024 * 1024
TIMEOUT = 30


def ssl_rx():
    data_count = 0
    start_time = datetime.datetime.now()

    while True:
        elapsed_time = (datetime.datetime.now() - start_time).total_seconds()

        if int(elapsed_time) > TIMEOUT:
            print("Timeout.")
            break

        try:
            data_in = connection.recv(1370)

            if not data_in:
                print("Connection closed by client.")
                break

            data_count += len(data_in)

            if data_count >= MAX_DATA_BYTES:
                print("Received {}MB data. Exiting...".format(BYTES_TO_SEND))
                break
        except (OSError, IOError):
            print("Connection closed by client.")
            break

    elapsed_time = int((datetime.datetime.now() - start_time).total_seconds())
    throughput = float(data_count * 8) / (1000 * 1000 * elapsed_time)
    throughput = round(throughput, 2)
    print("data_count is {}".format(data_count))
    print("throughput is {} in {} seconds".format(throughput, elapsed_time))


server_socket = socket(AF_INET, SOCK_STREAM)
server_socket.bind(('0.0.0.0', 443))
server_socket.listen(10)

# Choose the appropriate protocol based on the Python version
if sys.version_info < (3, 0):
    ssl_context = SSLContext(PROTOCOL_TLSv1)
else:
    ssl_context = SSLContext(PROTOCOL_TLS_SERVER)

ssl_context.load_cert_chain(
    certfile='server-cert.pem', keyfile='server-key.pem')
tls_server = ssl_context.wrap_socket(server_socket, server_side=True)

print('server started')

connection, client_address = tls_server.accept()
print('connection from', client_address)

ssl_rx()

connection.close()
server_socket.close()