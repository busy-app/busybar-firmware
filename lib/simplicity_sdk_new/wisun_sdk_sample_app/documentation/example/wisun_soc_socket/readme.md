# Wi-SUN - SoC Socket

## Table of Contents

* [Introduction](#introduction)
* [Getting Started](#getting-started)
* [Send TCP Packets](#send-tcp-packets)
* [TCP Echo Server](#tcp-echo-server)
* [Send UDP Packets](#send-udp-packets)
* [UDP Echo Server](#udp-echo-server)
* [Troubleshooting](#troubleshooting)
* [Resources](#resources)
* [Report Bugs & Get Support](#report-bugs--get-support)

## Introduction

The Wi-SUN Socket sample application provides:

* A simple client implementation based on the connection-oriented TCP protocol
  * The sample application operates with a command-line interface to create, close, read, or write sockets.
* A simple echo server implementation based on the connection-oriented TCP protocol
  * The sample application creates a TCP socket where all incoming packets are echoed back to the sender.
* A simple client implementation based on the UDP protocol
  * The sample application operates with a command-line interface to create, close, read, or write sockets.
* A simple echo server implementation based on the UDP protocol
  * The sample application creates a UDP socket where all incoming packets are echoed back to the sender.

The Socket sample application leverages the POSIX socket component on top of the Wi-SUN stack.

## Getting Started

To get started with Wi-SUN and Simplicity Studio, see [Getting Started with Wi-SUN Application Development](https://docs.silabs.com/wisun/latest/wisun-getting-started-development).

The Wi-SUN Socket sample application exposes a command-line interface to interact with the Wi-SUN stack. The goal of this procedure is to create the Wi-SUN network described in the following figure and have the TCP/UDP Client exchange messages with the TCP/UDP Server.

![TCP Application Wi-SUN Network](readme_img1.png)
![UDP Application Wi-SUN Network](readme_img2.png)

To get started with the example, follow the steps below:

* Flash the "Wi-SUN Border Router" demonstration to a device and start the Border Router.
* Create and build the Wi-SUN Socket project.
* Flash the Wi-SUN Socket project to a second device.
* Flash the Wi-SUN Socket project to a third device.
* Using Simplicity Studio, open consoles on both Wi-SUN Socket devices.
* Wait for the Wi-SUN Socket devices to join the Wi-SUN Border Router network.

Refer to [Wi-SUN SDK Quick Start Guide](https://docs.silabs.com/wisun/latest/wisun-getting-started-overview) for a step by step guide to create, configure and build the projects.

## Send TCP Packets

The three Wi-SUN devices (Border Router, Socket device as TCP Client, Socket device as TCP Server) are now part of the same Wi-SUN network. To check the commands exposed in the TCP Client application, enter:

```text
wisun help
```

The TCP Client application has four specific commands: `wisun tcp_client`, `wisun socket_close`, `wisun socket_write` and `wisun socket_read`. Use the first command to open a TCP socket with the TCP Server.

```text
wisun tcp_client [TCP Server Global IPv6 address] 4567
```

Port 4567 is the default port used by the TCP Server application. The TCP Client application replies with the ID of the created socket.

```text
> wisun tcp_client fd00:7283:7e00:0:20d:6fff:fe20:b6f9 4567
[Socket created: 4]
```

In the following steps, ID '4' is used to refer to the socket. The next step is to send a TCP packet with the newly created socket. Use the `socket_write` command to send the packet.

```text
wisun socket_write [Socket ID] [Message]
```

For example.

```text
> wisun socket_write 4 "hello world!"
> [Data sent on socket: 4]
```

You can then use the socket to read data echoed back by the TCP Server. Use the `socket_read` command to read the packet.

```text
wisun socket_read [Socket ID] [Number of bytes to read]
```

For example.

```text
> wisun socket_read 4 5
hello
```

Finally, you can close the TCP socket using the `socket_close` command.

```text
wisun socket_close [Socket ID]
```

## TCP Echo Server

The three Wi-SUN devices (Border Router, Socket device as TCP Client, Socket device as TCP Server) are now part of the same Wi-SUN network. After a successful connection, the TCP Server application creates a socket on port 4567 by default.

```text
[Port: 4567]
TCP server socket() [4] done.
TCP server bind() [0] done.
TCP server listen() [0] done.
[Waiting for connection request]
```

The TCP Server socket is ready to accept an incoming connection from the TCP Client. The TCP server socket can only maintain a connection with a single client at a time.

On the TCP Client connection, the TCP Server outputs the following message on the console.

```text
TCP server accept() [5] done.
```

When a packet is received, the TCP echo server sends the data back to the sender.

```text
[fd00:7283:7e00:0:20d:6fff:fe20:bd45] hello world!
[Data sent on socket: 5]
```

If the TCP Client closes the socket, the following message is output on the console.

```text
[Socket closing: 5]
[Waiting for connection request]
 ```

## Send UDP Packets

The three Wi-SUN devices (Border Router, Socket device as UDP Client, Socket device as UDP Server) are now part of the same Wi-SUN network. To check the commands exposed in the UDP Client application, enter:

```text
wisun help
```

The UDP Client application has four specific commands: `wisun udp_client`, `wisun socket_close`, `wisun socket_write` and `wisun socket_read`. Use the first command to open a UDP socket with the UDP Server.

```text
> wisun udp_client
[Socket created: 3]
```

The UDP Client application replies with the ID of the created socket. In the following steps, ID '3' is used to refer to the socket. The next step is to send a UDP packet with the newly created socket. Use the `socket_write` command to send the packet.

```text
wisun socket_write [Socket ID] [UDP Server IPv6 address] [Port] [Message]
```

For example.

```text
> wisun socket_write 3 fd00:7283:7e00:0:20d:6fff:fe20:b6f9 1234 "hello world!"
> [Data sent on socket: 3]
```

You can then use the socket to read data echoed back by the UDP Server. Use the `socket_read` command to read the packet.
> Note: For message-based sockets, such as SOCK_RAW, SOCK_DGRAM, and SOCK_SEQPACKET, the entire message shall be read in a single operation.

```text
wisun socket_read [Socket ID]
```

For example.

```text
> wisun socket_read 3
hello world!
```

Finally, you can close the UDP socket using the `socket_close` command.

```text
wisun socket_close [Socket ID]
```

## UDP Echo Server

The three Wi-SUN devices (Border Router, Socket device as UDP Client, Socket device UDP Server) are now part of the same Wi-SUN network. After a successful connection, the UDP Server application creates a socket on port 1234 by default.

```text
[Port: 1234]
UDP server socket() [3] done.
UDP server bind() [0] done.
```

The UDP Server socket is ready to receive an incoming packets from the UDP Client.

When a packet is received, the UDP echo server sends the data back to the sender.

```text
[fd00:7283:7e00:0:20d:6fff:fe20:bd45] hello world!
[Data sent on socket: 3]
```

## Troubleshooting

Before programming the radio board mounted on the WSTK, ensure the power supply switch is in the AEM position (right side), as shown.

![Radio Board Power Supply Switch](readme_img0.png)

## Resources

* [Wi-SUN Stack API documentation](https://docs.silabs.com/wisun/latest)

## Report Bugs & Get Support

You are always encouraged and welcome to ask any questions or report any issues you found to us via [Silicon Labs Community](https://community.silabs.com/s/topic/0TO1M000000qHc6WAE/wisun).
