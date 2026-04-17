#!/usr/bin/env python3

# Copyright 2026 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

# Metadata
__author__ = 'Silicon Laboratories, Inc'
__copyright__ = 'Copyright 2026, Silicon Laboratories, Inc.'

import os
import time
import re
import pylink
import socket
import subprocess

_MSG_MAX_SIZE = 1200

class RTT:
    def __init__(self,
                 chip_name: str,
                 serial_no: int = None,
                 hostname: str = None,
                 msg_max_size: int = _MSG_MAX_SIZE,
                 xml_path = None):
        """Communication interface to device.

        :param chip_name: Chip name
        :param serial_no: J-Link serial number to use USB interface
        :param hostname: Hostname to use Ethernet interface
        :msg_max_size: Message chunk size limit
        :xml_path: JLinkDevices.xml file to extend/overwrite J-Link devices database with new devices
        """
        self.jlink = pylink.JLink()
        self.chip_name = self.get_device_jlink_name(chip_name)
        self.serial_no = serial_no
        self.hostname = hostname
        self.msg_max_size = msg_max_size

        if not xml_path:
            try:
                p = subprocess.run(["slt", "where", "commander", "--ignore-slconf"],
                                    text=True,
                                    capture_output=True)
                if p.returncode == 0 and p.stdout:
                    xml_path = os.path.join(p.stdout.strip(), 'resources/jlink/JLinkDevices.xml')
            except:
                # SLT and/or Simplicity Commander is not installed. J-Link devices database will not be extended.
                pass
        if xml_path:
            if os.path.exists(xml_path):
                self.jlink.exec_command(f"JLinkDevicesXMLPath = \"{xml_path}\"")
                print("J-Link Device XML extension added: " + xml_path)
            else:
                print(xml_path + " does not exist!")

    @property
    def is_connected(self) -> bool:
        """Indicate device connection is established.

        :return: True if connected, False otherwise
        """
        return self.jlink.connected()

    def connect(self):
        """Connect to device using serial_no or hostname."""
        if self.serial_no:
            self.jlink.open(serial_no=self.serial_no)
        elif self.hostname:
            ip_addr = socket.gethostbyname(self.hostname)
            self.jlink.open(ip_addr=f"{ip_addr}:19020")
        else:
            self.jlink.open()
        self.jlink.set_tif(interface=pylink.JLinkInterfaces.SWD)
        self.jlink.connect(chip_name=self.chip_name, speed="auto", verbose=True)
        self.jlink.reset(halt=True)

    def reset(self):
        """Reset the device."""
        self.jlink.reset(halt=False)

    def rtt_start(self, block_address: int = None, timeout: float = 10):
        """Start RTT processing.

        :param block_address: Optional RTT block address
        :param timeout: Time to wait for RTT processing started in seconds
        """
        self.jlink.rtt_start(block_address)
        start = now = time.time()
        while now < start + timeout:
            try:
                self.jlink.rtt_get_buf_descriptor(0, False)
            except pylink.errors.JLinkRTTException:
                pass
            else:
                return
            now = time.time()
        raise TimeoutError

    def rtt_stop(self):
        """Stops RTT."""
        self.jlink.rtt_stop()

    def rtt_send(self, data: bytes, timeout: float = 10):
        """Send data to RTT buffer.

        :param data: Bytes to write to RTT buffer.
        :param timeout: Maximum time to wait for data to be written in seconds
        """
        remaining = data

        while remaining:
            if len(remaining) <= self.msg_max_size:
                chunk = remaining
            else:
                chunk = remaining[:self.msg_max_size]

            nb_sent = 0
            start = now = time.time()

            while nb_sent == 0 and now < start + timeout:
                nb_sent = self.jlink.rtt_write(0, chunk)
                now = time.time()

            if nb_sent == 0:
                raise TimeoutError

            remaining = remaining[nb_sent:]

    def rtt_receive(self, timeout: float = 10) -> bytes:
        """Read data from RTT buffer.

        :param timeout: Maximum time to wait for data to be received in seconds
        :return: Data received
        """
        data = bytes()
        start = now = time.time()
        while len(data) == 0 and now < start + timeout:
            data = self.jlink.rtt_read(0, self.msg_max_size)
            now = time.time()
        if len(data) == 0:
            raise TimeoutError
        return bytes(data)

    def close(self):
        """Close the connection."""
        self.jlink.close()

    def run_application(self, ram_addr: int, img: bytes):
        """Flash and run the provided firmware in device's RAM.

        :param ram_addr: Address on RAM memory to flash the firmware
        :param img: Firmware image
        """
        self.jlink.memory_write8(addr=ram_addr, data=list(img))
        sp, pc = self.jlink.memory_read32(addr=ram_addr, num_words=2)
        self.jlink.register_write(reg_index="R13 (SP)", value=sp)
        self.jlink.register_write(reg_index="R15 (PC)", value=pc)
        self.jlink.restart(num_instructions=0, skip_breakpoints=False)

    def get_mac_address(self) -> str:
        """Get the device's builtin EUI-64.

        :return: EUI-64 in String format without ':'
        """
        l, h = self.jlink.memory_read32(0x0FE08000 + 0x48, 2)
        return f"{h * 0x100000000 + l:016x}"

    @staticmethod
    def get_device_jlink_name(p: str) -> str:
        _RE_EXX32_S2  = re.compile(r"^(EF[RM]32[MFBZSXP]G2[1-9][ABCEXL])[X\d]{3}(F\d{2,4}).*")
        _RE_SIXX3     = re.compile(r"^SI[MFBZXP][AGNTFPREU](3\d{2})\w[X\d]{3}([XWG-M]).*")
        _RE_XGM2xx    = re.compile(r"^([BMZF]GM2\d{2}[LPS][ABCD0]\d{2}).*")

        p = p.upper()
        if _RE_EXX32_S2.search(p):
            # EFR32MG21A020F1024IM32 -> EFR32MG21AxxxF1024 style
            p = _RE_EXX32_S2.sub(r"\1xxx\2", p)
        elif _RE_SIXX3.search(p):
            # SIMG301M114KIHA0 -> Sixx301xxxxK
            p = _RE_SIXX3.sub(r"Sixx\1xxxx\2", p)
        elif _RE_XGM2xx.search(p):
            # BGM210LA22JIF3 -> BGM210LA22
            p = _RE_XGM2xx.sub(r"\1", p)

        return p
