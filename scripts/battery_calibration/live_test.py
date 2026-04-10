#!/usr/bin/env python3

import serial
import sys
import struct

V_TABLE = []
with open(sys.argv[1], "rb") as f:
    HEADER_FMT = "<8sHHHH"
    header = f.read(struct.calcsize(HEADER_FMT))
    signature, tolerance, i_range, percent_points, current_points = struct.unpack(HEADER_FMT, header)
    assert signature == b"bsbbcal0"
    print(f"Calibration tolerance: +/-{tolerance}%")
    for i in range(current_points):
        curve = []
        last_mv = 0
        for p in range(percent_points):
            if p < 2 or p >= percent_points - 2:
                mv = struct.unpack("<H", f.read(2))[0]
            else:
                mv = last_mv + struct.unpack("<b", f.read(1))[0]
            curve.append(mv)
            last_mv = mv
        V_TABLE.append(curve)

print(V_TABLE)

def determine_soc(i, v):
    i_bucket = round(((i / i_range) * (current_points / 2)) + (current_points / 2))
    i_bucket = max(0, min(i_bucket, current_points - 1))
    # print(f"{i} => {i_bucket}")
    best_p = -1
    best_v_delta = 1000000
    for p_cand, v_cand in enumerate(V_TABLE[i_bucket]):
        if abs(v - v_cand) < best_v_delta:
            best_p = round(p_cand * (100 / (percent_points - 1)))
            best_v_delta = abs(v - v_cand)
    return best_p

port = serial.Serial("/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0", 921600)

print("Waiting for 'begin'...")
port.read_until(b"begin\r\n")
print("Synced")

PRINT_RATE = 500
avg_sum = (0, 0)
avg_cnt = 0
last_printed = 0

while True:
    line = str(port.read_until(b"\r\n")[:-2], "utf8")
    tokens = line.split(" ")
    tokens = (tok.split("=") for tok in tokens)
    tokens = {t[0]: int(t[1]) for t in tokens}

    avg_sum = (avg_sum[0] + (tokens["i"] // 1000), avg_sum[1] + (tokens["bv"] // 1000))
    avg_cnt += 1

    if tokens["t"] - last_printed >= PRINT_RATE:
        last_printed = tokens["t"]

        i = avg_sum[0] // avg_cnt
        v = avg_sum[1] // avg_cnt
        soc = determine_soc(i, v)
        avg_sum = (0, 0)
        avg_cnt = 0

        print(f"{i}mA, {v}mV => {soc}%")
