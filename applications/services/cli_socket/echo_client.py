#!/usr/bin/env python3

import socket
import random

def wait_for_string(client_socket, data, message):
    print("Waiting for message: ", message)
    while True:
        part = client_socket.recv(1024)
        if part:
            data += part.decode()
            # print("Received data: ", data)

            # check if the data is containing the message
            if message in data:
                print("Message received successfully")
                data = data.replace(message, "")
                break
        else:
            print("Error: No data received")
            break

def start_client(host='10.12.34.1', port=23):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
        client_socket.connect((host, port))
        data = ""

        while True:


            # generate random string
            # length = random.randint(1, 800)
            message = ''.join(random.choices('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ', k=832))

            print("Sending message: ", message)
            client_socket.sendall("echo ".encode() + message.encode() + "\r\n".encode())
            wait_for_string(client_socket, data, "echo " + message)
            wait_for_string(client_socket, data, message)
            wait_for_string(client_socket, data, ">:")

if __name__ == "__main__":
    start_client()