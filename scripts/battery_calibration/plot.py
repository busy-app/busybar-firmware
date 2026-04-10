#!/usr/bin/env python3

import numpy as np
import matplotlib.pyplot as plt
import struct
import sys, os

FORMAT = "<iiii"
SIZE = struct.calcsize(FORMAT)

time = None
voltage = None
current = None
charge = None

idx = 0
last_t = None
adj_t = 0
tot_chg = 0

with open(sys.argv[1], "rb") as f:
    while point := f.read(SIZE):
        t, bv, lv, i = struct.unpack(FORMAT, point)
        t //= 1000
        bv /= 1_000_000
        lv /= 1_000_000
        i /= 1_000_000

        if time is None and abs(i) < 0.1:
            continue

        if time is None:
            here = f.tell()
            f.seek(0, os.SEEK_END)
            n_points = ((f.tell() - here) // SIZE) + 1
            f.seek(here)
            time = np.zeros(n_points)
            voltage = np.zeros(n_points)
            current = np.zeros(n_points)
            charge = np.zeros(n_points)
            last_t = t

        t += adj_t
        if t < last_t:
            adj_t += last_t
            t += last_t

        time[idx] = t
        voltage[idx] = bv
        current[idx] = i

        chg = (t - last_t) * i
        tot_chg += chg
        charge[idx] = tot_chg

        last_t = t
        idx += 1

charge /= tot_chg

cc_i = 0
calibration = []
for i in range(101):
    chg_idx = np.searchsorted(charge, i / 100, side="left")
    v = np.mean(voltage[max(chg_idx - 50, 0) : min(chg_idx + 50, len(voltage) - 1)])
    curr = np.mean(current[max(chg_idx - 50, 0) : min(chg_idx + 50, len(voltage) - 1)])
    calibration.append((curr, v))

dischg = tot_chg < 0
if dischg:
    calibration = reversed(calibration)

print(f"Total charge: {tot_chg:.1f} C")

print(f"Calibration: ", end="")
for i, v in calibration:
    print(f"{round(i * 1000)}:{round(v * 1000)}, ", end="")
print()

plt.plot(time, voltage, time, current, time, charge)
plt.show()
