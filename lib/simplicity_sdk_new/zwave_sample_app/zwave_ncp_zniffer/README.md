# Zniffer

The Zniffer is a development tool for capturing Z-Wave RF communication and sending the frames to a host application.

Z-Wave Zniffer is a PC side host application which communicates with the Zniffer via a USB serial connection and presents the frames in a graphical user interface.
This enables Z-Wave developers to analyze all RF communication between Z-Wave nodes.

The host application can connect to the Zniffer through either:

- a virtual COM port over a USB serial connection (or through port 4901 when an Ethernet connection is available)
- or via the Packet Trace Interface (PTI) over TCP/IP on port 4905.