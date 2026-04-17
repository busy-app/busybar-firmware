#!/usr/bin/env python3
"""
Dynamic Hardware Configuration host application
"""

# Copyright 2025 Silicon Laboratories Inc. www.silabs.com
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

import argparse
import time
import datetime
import math
import json
import os
import socket
import subprocess
from dhc_classes import *
import importlib

# Configuration
PATH_CONFIG = os.path.abspath(os.path.join((__file__), '../configuration_sample.json'))
PATH_XAPI   = os.path.abspath(os.path.join((__file__), '../../../../../../protocol/bluetooth/api/sl_bt.xapi'))
JSON_GROUP  = "silabs_dhc"
MSG_LEN_MAX = 16

def load_package(name):
    module = importlib.import_module(name)
    globals()[name] = module

def main(port,
         protocol,
         config_file=PATH_CONFIG,
         read_config=False,
         verify_only=False,
         xapi=PATH_XAPI):
    print(__doc__)
    print(f"Port: {port}")
    print(f"Transport protocol: {protocol}")
    if not read_config:
        print(f"Configuration data file: {config_file}")

    if protocol == "NCP":
        load_package("bgapi")
        print(f"XAPI file: {xapi}")
        dhc_transport = DHCTransportNcp(port, xapi)
    elif protocol == "RCP":
        dhc_transport = DHCTransportRcp(port)
    else:
        raise ValueError("Invalid transport protocol")

    msg_handler = DHCMessageHandler(dhc_transport)
    print()

    if read_config:
        read_all(msg_handler)
        exit(0)

    config = load_config(config_file)

    # Check configuration version on device
    version_device = Version.from_bytes(msg_handler.read(Version.id()))
    if not version_device.validate():
        raise ValueError(f"Version '{version_device}' is not supported. Supported version: '{Version._SUPPORTED_VERSION}'")
    if version_device != config.version:
        raise ValueError(f"Configuration data version mismatch. Device='{version_device.to_json()}', Host='{config.version.to_json()}'")
    print(f"Version ok. Version: '{version_device.to_json()}'")

    # Write all attributes from the JSON
    if not verify_only:
        write_all(msg_handler, config)
        # Reset device for the new configuration to take effect
        dhc_transport.reset()

    # Verify written attributes
    status = verify_all(msg_handler, config)
    if not status:
        raise Exception("Configuration data verification failed.")
    print("Configuration data verification successful")

    if not verify_only:
        print("Configuration finished successfully")
    exit(0)

def write_all(msg_handler, config):
    """Write every configuration to device from the JSON file"""
    print(f"{os.linesep}Writing configuration data to the device{os.linesep}")

    for _, item in config.__dict__.items():
        if not item:
            continue # empty field
        if isinstance(item, Version):
            continue # read only

        print(f"Writing '{item.key()}'.{os.linesep}Value:\t{item.to_json()}")

        if isinstance(item, PaCurve):
            _write_pa_curve_fragments(msg_handler, item)
        else:
            status = msg_handler.write(item.id(), item.to_bytes())
            if status is None:
                print("error")
            else:
                print("ok")
        print()

def verify_all(msg_handler, config):
    """Verify every configuration from the JSON file"""
    print(f"{os.linesep}Verifying configuration{os.linesep}")
    all_match = True

    for _, item in config.__dict__.items():
        if not item:
            continue # empty field
        print(f"Verifying '{item.key()}'.{os.linesep}Value:\t{item.to_json()}")

        if isinstance(item, PaCurve):
            msg = _read_pa_curve_fragments(msg_handler)
        else:
            msg = msg_handler.read(item.id())
        if msg is None:
            print(f"Error reading '{item.key()}'!")
            all_match = False
            continue

        device = item.__class__.from_bytes(msg)
        print(f"Device:\t{device.to_json()}")

        if item == device:
            print("ok")
        else:
            print("mismatch!")
            all_match = False
        print()
    return all_match

def read_all(msg_handler):
    """Read all configuration attributes from device and write them to a JSON file"""
    json_data = {JSON_GROUP: {}}

    # Version
    version = Version.from_bytes(msg_handler.read(Version.id()))
    if not version.validate():
        print(f"Version '{version.version}' is not supported. Supported version: '{Version._SUPPORTED_VERSION}'")
        return
    json_data[JSON_GROUP][Version.key()] = version.to_json()

    # RssiOffset
    msg = msg_handler.read(RssiOffset.id())
    if msg:
        rssi_offset = RssiOffset.from_bytes(msg)
        json_data[JSON_GROUP][RssiOffset.key()] = rssi_offset.to_json()

    # PaCurve
    pa_curves = PaCurve.from_bytes(_read_pa_curve_fragments(msg_handler))
    json_data[JSON_GROUP][PaCurve.key()] = pa_curves.to_json()

    # PaMode
    msg = msg_handler.read(PaMode.id())
    if msg:
        pa_mode = PaMode.from_bytes(msg)
        json_data[JSON_GROUP][PaMode.key()] = pa_mode.to_json()

    # Ctune
    msg = msg_handler.read(Ctune.id())
    if msg:
        ctune = Ctune.from_bytes(msg)
        json_data[JSON_GROUP][Ctune.key()] = ctune.to_json()

    # Write device configuration to a new json file
    json_object = json.dumps(json_data, indent=4)
    print(json_object)

    path_out = os.path.abspath(
        os.path.join((__file__),
                     '../',
                     'silabs-dhc-{date:%Y-%m-%d_%H-%M-%S}.json'.format(date=datetime.datetime.now()))
    )
    with open(path_out, "w") as f:
        f.write(json_object)
    print(f"Configuration data from the device written to '{path_out}'")

def load_config(config_file):
    """Load the configuration data from the provided file"""
    if not os.path.exists(config_file):
        raise FileNotFoundError(f"Configuration data file '{config_file}' does not exist.")
    config_json = json.load(open(config_file))[JSON_GROUP]

    # Parse items
    # Version (required)
    version = Version(config_json[Version.key()]) # Required
    if not version.validate():
        raise ValueError(f"Version '{version.version}' is not supported. Supported version: '{Version._SUPPORTED_VERSION}'")
    # RssiOffset
    rssi_offset = None
    if RssiOffset.key() in config_json:
        rssi_offset = RssiOffset(config_json[RssiOffset.key()])
    # PaCurve
    pa_curves = None
    if PaCurve.key() in config_json:
        pa_curves = PaCurve.from_json(config_json[PaCurve.key()])
    # PaMode
    pa_mode = None
    if PaMode.key() in config_json:
        pa_mode = PaMode(config_json[PaMode.key()])
    # Ctune
    ctune = None
    if Ctune.key() in config_json:
        ctune = Ctune(config_json[Ctune.key()])

    return Configuration(version, rssi_offset, pa_curves, pa_mode, ctune)

def _read_pa_curve_fragments(msg_handler):
    # Read fragmented PaCurve
    fragments = int.from_bytes(msg_handler.read(PaCurve.id()), 'little')
    msg = bytearray()

    for _ in range(1, fragments):
        fragment = msg_handler.read(PaCurve.id())
        if len(fragment) > MSG_LEN_MAX:
            raise BufferError(f"Fragment too long: {len(fragment)} > {MSG_LEN_MAX}")
        msg.extend(fragment)
    return msg

def _write_pa_curve_fragments(msg_handler, pa_curve):
    pa_curve_bytes = pa_curve.to_bytes()
    fragments = math.ceil(len(pa_curve_bytes) / MSG_LEN_MAX) + 1

    # Only send the number of fragments as the first fragment
    rc = msg_handler.write(pa_curve.id(), fragments.to_bytes(1, 'little'))
    if rc is None:
        print(f"Error sending the number of fragments: {fragments}")
    else:
        # Send PA curves in fragments
        for i in range(1, fragments):
            byte_index = (i - 1) * MSG_LEN_MAX
            length = min(len(pa_curve_bytes) - byte_index, MSG_LEN_MAX)
            rc = msg_handler.write(pa_curve.id(), pa_curve_bytes[byte_index:byte_index + length])

            if rc is None:
                print(f"Error sending fragment: {i}")
                break
        else:
            print("ok")

class DHCMessageHandler:
    DHC_MSG_HEADER = 0xDC
    DHC_CMD_READ = 0x00
    DHC_CMD_WRITE = 0x01

    def __init__(self, transport):
        self.transport = transport

    def read(self, setting):
        """Read a specified hardware configuration setting"""
        payload = bytearray()
        payload.append(self.DHC_MSG_HEADER)
        payload.append(self.DHC_CMD_READ)
        payload.append(setting)

        rc, data = self.transport.send_data(bytes(payload))
        if rc != 0:
            print(f"Error: {rc}")
            return None
        return data

    def write(self, setting, value: bytes):
        """Write a specified hardware configuration setting with a specified value"""
        payload = bytearray()
        payload.append(self.DHC_MSG_HEADER)
        payload.append(self.DHC_CMD_WRITE)
        payload.append(setting)
        payload.extend(value)

        rc, data = self.transport.send_data(bytes(payload))
        if rc != 0:
            print(f"Error: {rc}")
            return None
        return data

class DHCTransport:
    def __init__(self, port):
        self.port = port

    def __del__(self):
        pass

    def send_data(self, _):
        pass

    def reset(self):
        pass

class DHCTransportNcp(DHCTransport):
    def __init__(self, port, xapi_file_location):
        super().__init__(port)
        self.bgapi_context = None
        self.xapi_file_location = xapi_file_location

        # Connect to the NCP device either over IP or serial
        # Check if the provided port is an IP address
        port_is_ip = False
        ip = None
        port = None
        try:
            if not ":" in self.port:
                raise Exception
            ip, port = self.port.split(":")
            port = int(port)
            socket.inet_aton(ip)
            port_is_ip = True
            print(f"Connecting over IP to: {ip}:{port}")
        except Exception:
            # If validation as IP/port fails - treat it as a serial port
            print(f"Connecting over Serial to: {self.port}")
            pass

        # Initialize the BGAPI context
        try:
            bgapi_connector = None
            if port_is_ip:
                bgapi_connector = bgapi.SocketConnector((ip, port))
            else:
                bgapi_connector = bgapi.SerialConnector(self.port)
            self.bgapi_context = bgapi.BGLib(bgapi_connector, os.path.join(self.xapi_file_location))
            self.bgapi_context.open()
            print("NCP connection successful")
        except Exception as e:
            raise Exception("NCP connection error: " + str(e))

    def __del__(self):
        if self.bgapi_context is not None:
            self.bgapi_context.close()

    def send_data(self, payload: bytes):
        """Send the provided payload to the NCP device"""
        try:
            rc, response = self.bgapi_context.bt.user.message_to_target(payload)
            return rc, bytes(response)
        except Exception as e:
            print("Error during data transmit: " + str(e))
        return -1, None

    def reset(self):
        print("Resetting device...")
        self.bgapi_context.bt.system.reboot()
        time.sleep(1) # Wait for the device to reset

class DHCTransportRcp(DHCTransport):
    # About HCI messages:
    # https://docs.silabs.com/wifi91xrcp/latest/wifi91xrcp-developers-guide-ble-tools/hciconfig-and-hcitool
    def __init__(self, port):
        super().__init__(port)

    def __del__(self):
        pass

    def send_data(self, payload):
        """Send the provided payload to the RCP/HCI device using 'hcitool'"""
        return self._send_hci_command(["sudo", "hcitool", "-i", self.port, "cmd", "0x003f", "0x0309"] + [str(hex(x)) for x in payload])

    def reset(self):
        return self._send_hci_command(["sudo", "hcitool", "-i", self.port, "cmd", "0x0003", "0x0003"])

    def _send_hci_command(self, args):
        hcitool_proc = subprocess.Popen(args,
                                        stdout=subprocess.PIPE,
                                        stderr=subprocess.PIPE)
        std_out, std_err = hcitool_proc.communicate()
        if hcitool_proc.returncode != 0:
            raise Exception("Error executing hcitool", hcitool_proc.returncode, std_err)
        return self._parse_hcitool_output(std_out.decode())

    def _parse_hcitool_output(self, output):
        """Parse a 'hcitool' output to obtain the return code and incoming payload"""
        try:
            # Find the event data in the hcitool process output
            lines = output.splitlines()
            dataline_idx = 0
            for line in lines:
                if "> HCI Event: " in line:
                    data_line = lines[dataline_idx + 1]
                    break
                dataline_idx += 1
            else:
                raise NameError("HCI Event not found in HCI output:", output)

            result = []
            for value in data_line.split():
                result.append(int(value, 16))

            # Get the return code from the event header
            if result[3] != 0:
                return result[3], None
            # Remove the first 4 bytes (event header)
            return 0, bytes(result[4:])
        except Exception as e:
            raise Exception("Error parsing HCI response:", str(e))

if __name__ == "__main__":
    # Parse command line arguments
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('-p', '--port',
                           help='Serial/IP port/HCI device on which the target board is connected. ' \
                           'For NCP connection, define serial port ("COM#" on Windows, "/dev/tty#" on POSIX), ' \
                           'or the IP address. ' \
                           'For RCP connection, define the HCI device (E.g. hci0). ' \
                           'You can access the available hci devices using the "hcitool dev" command.',
                           required=True)
    argparser.add_argument('-c', '--config_file',
                           default=PATH_CONFIG,
                           help='Config JSON file containing the configuration data. (default: %(default)s)')
    argparser.add_argument('-r', '--read_config',
                           help='Reads the configuration from a device and writes it to a JSON file',
                           action='store_true')
    argparser.add_argument('-v', '--verify_only',
                           help='Check if the provided JSON file matches the values on the device - without writing',
                           action='store_true')
    argparser.add_argument('-x', '--xapi',
                           default=PATH_XAPI,
                           help='Specifies the location for the XAPI file needed for NCP mode. (default: %(default)s)')
    group = argparser.add_mutually_exclusive_group(required=True)
    group.add_argument('--ncp',
                       help="Use NCP for transport.",
                       action='store_true')
    group.add_argument('--rcp',
                       help='Use RCP/HCI for transport. ' \
                        'Note that the EFR device should be attached using the btattach command beforehand.',
                       action='store_true')
    args = argparser.parse_args()

    main(args.port,
         "NCP" if args.ncp else "RCP",
         args.config_file,
         args.read_config,
         args.verify_only,
         args.xapi)
