import socket
import threading
import time

# Define the server address and port
SERVER_HOST = '0.0.0.0'  # Listen on all interfaces
SERVER_PORT = 5005       # Port to listen on

# Define the data to send - exactly 1460 bytes
data_to_send = b"A" * 1460  # 1460 bytes of the character 'A'

def handle_client(client_socket, client_address):
    print(f"Connection from {client_address}")
    
    data_sent = 0
    start_time = time.time()

    try:
        while True:
            # Send data to the client
            client_socket.sendall(data_to_send)
            data_sent += len(data_to_send)
    except socket.error as e:
        print(f"Exception occurred: {e}")
    finally:
        # Calculate throughput after connection is closed
        end_time = time.time()
        duration = end_time - start_time
        if duration > 0:
            throughput = data_sent / duration
            print(f"Connection with {client_address} closed.")
            print(f"Total data sent: {data_sent} bytes in {duration:.2f} seconds.")
            print(f"Throughput: {throughput:.2f} bytes/s")
        else:
            print(f"Connection with {client_address} closed. No data sent.")
        client_socket.close()

def start_server():
    # Create a socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Reuse the address
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # Bind the socket to the server address and port
    server_socket.bind((SERVER_HOST, SERVER_PORT))
    # Listen for incoming connections
    server_socket.listen(5)
    print(f"Server listening on {SERVER_HOST}:{SERVER_PORT}")

    while True:
        # Accept a new client connection
        client_socket, client_address = server_socket.accept()
        # Start a new thread to handle the client connection
        client_thread = threading.Thread(target=handle_client, args=(client_socket, client_address))
        client_thread.start()

if __name__ == "__main__":
    start_server()
