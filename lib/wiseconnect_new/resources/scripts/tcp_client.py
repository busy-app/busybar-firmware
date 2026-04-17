import socket
import sys
import time

def main():
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print(f"Usage: python {sys.argv[0]} <server_ip> <server_port> [timeout_seconds]")
        sys.exit(1)

    server_ip = sys.argv[1]
    server_port = int(sys.argv[2])
    timeout = float(sys.argv[3]) if len(sys.argv) == 4 else None

    buffer_size = 3072
    total_bytes = 0

    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(5)  # Timeout for connect/recv operations
            s.connect((server_ip, server_port))
            print(f"Connected to {server_ip}:{server_port}")

            start_time = time.time()
            while True:
                now = time.time()
                if timeout is not None and (now - start_time) > timeout:
                    print(f"Timeout of {timeout} seconds reached. Closing connection.")
                    break
                try:
                    data = s.recv(buffer_size)
                    if not data:
                        print("Server closed the connection.")
                        break
                    total_bytes += len(data)
                except socket.timeout:
                    # Continue trying until overall timeout
                    continue
            end_time = time.time()

            duration = end_time - start_time
            print(f"Total data received: {total_bytes} bytes in {duration:.2f} seconds.")
            if duration > 0:
                print(f"Throughput: {total_bytes/duration:.2f} bytes/s")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    main()