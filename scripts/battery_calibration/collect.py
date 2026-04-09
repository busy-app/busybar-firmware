#!/usr/bin/env python3

import serial
import sys
import struct

port = serial.Serial("/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0", 921600)
SAVE_RATE = 10
PRINT_RATE = 1_000

print("Waiting for 'begin'...")
port.read_until(b"begin\r\n")
print("Synced")

last_saved = 0
last_printed = 0

with open(sys.argv[1], "wb") as dump:
    try:
        while True:
            line = str(port.read_until(b"\r\n")[:-2], "utf8")
            tokens = line.split(" ")
            tokens = (tok.split("=") for tok in tokens)
            tokens = {t[0]: int(t[1]) for t in tokens}

            if tokens["t"] - last_saved >= SAVE_RATE:
                last_saved = tokens["t"]
                line = struct.pack("<iiii", tokens["t"], tokens["bv"], tokens["lv"], tokens["i"])
                dump.write(line)

            if tokens["t"] - last_printed >= PRINT_RATE:
                last_printed = tokens["t"]
                print(tokens)
    except KeyboardInterrupt:
        pass
