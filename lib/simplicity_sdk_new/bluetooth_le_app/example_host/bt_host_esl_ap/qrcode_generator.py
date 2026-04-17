#!/usr/bin/env python3
"""
QR code generator for ESL demo
"""

# Copyright 2023 Silicon Laboratories Inc. www.silabs.com
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

from image_converter import XbmConverter, Image
from itertools import zip_longest
from intelhex import IntelHex
import multiprocessing
import subprocess
import ap_logger
import argparse
import bincopy
import qrcode
import uuid
import sys
import os
import re
import io

QR_CODE_VERSION = 3  # 29x29 matrix
QR_CODE_BORDER = 4  # default is 4, which is the minimum in the specification
WSTK_DISPLAY_H = 128
WSTK_DISPLAY_W = 128
MAGIC_CONST = b".qrc_ph_pic_data"
FALLBACK_HEX = "readout_data.hex"

log = ap_logger.getLogger("QRG")
# Add commander path to system environment variables
try:
    os.environ["PATH"] += os.pathsep + os.environ["PATH_SCMD"]
except KeyError:
    # Commander path has to be set up manually
    pass


class ProcessAbortError(Exception):
    """Custom exception for aborting process in multiprocessing worker"""
    def __init__(self, exitcode):
        self.exitcode = exitcode
        super().__init__(f"Process aborted with exit code {exitcode}")


def abort(exitcode):
    ap_logger.log(f"Aborting with error code {exitcode}!", _half_indent_log=True)
    raise ProcessAbortError(exitcode)

class Commander:
    """Simplicity Commander helper"""

    def __init__(self, serialno=None, ip=None):
        self.conn_args = []
        if serialno is not None:
            self.conn_args += ["--serialno", serialno]
        if ip is not None:
            self.conn_args += ["--ip", ip]

    def call(self, args):
        try:
            ap_logger.log(
                "Excuting Simplicity Commander with arguments:",
                args[:3],
                _half_indent_log=True,
            )
            return subprocess.run(
                ["commander"] + args + self.conn_args,
                stdout=subprocess.PIPE,
                shell=False,
                check=True,
            ).stdout.decode()
        except FileNotFoundError:
            log.critical("Simplicity Commander not found.")
            log.info(
                "Make sure that the Simplicity Commander is installed and the PATH environment variable is set up correctly."
            )
            abort(-1)
        except subprocess.CalledProcessError as error:
            log.error("Simplicity Commander failed.")
            ap_logger.log(error.stdout.decode())
            abort(-2)

    def get_board_uid(self):
        uid = ""
        # Executing the Commander call and store the console output
        commander_response = self.call(["device", "info"])
        ap_logger.log(commander_response)
        # Look for the line "Unique ID      : <MAC address>"
        uid_info = re.search(r"Unique ID\s*:\s*(.*)\s*.*", commander_response)
        if uid_info:
            # Keep the MAC address, chopping the unnecessary values from the center
            uid = (
                uid_info.group(1)[0:2]
                + ":"
                + uid_info.group(1)[2:4]
                + ":"
                + uid_info.group(1)[4:6]
                + ":"
                + uid_info.group(1)[10:12]
                + ":"
                + uid_info.group(1)[12:14]
                + ":"
                + uid_info.group(1)[14:16]
            )
        return uid

    def flash_board(self, filename):
        commander_response = self.call(["flash", filename])
        ap_logger.log(commander_response)

    def read_mem(self, filename):
        commander_response = self.call(
            ["readmem", "--region", "@mainflash", "--outfile", filename]
        )
        ap_logger.log(commander_response)
        log.info(f"Created temporary hex file: {filename}")


def generate_qrcode(data, height, width):
    # QR code is a square we take the smaller
    pixel = min(height, width)
    # Creating box_size: This parameter controls how many pixels each "box" of the QR code is
    box_size = int(pixel / 37)
    qr = qrcode.QRCode(
        QR_CODE_VERSION, qrcode.constants.ERROR_CORRECT_M, box_size, QR_CODE_BORDER
    )
    # Adding data to the instance 'qr'
    qr.add_data(data)
    # Handle overflow exception
    try:
        qr.make(fit=False)
    except qrcode.exceptions.DataOverflowError as err:
        log.critical(err)
        abort(-3)
    # Create black and white QR code image
    img = qr.make_image(fill_color="black", back_color="white")
    img = img.convert("RGBA") # BG-18013: fix regression caused by Pillow module update v11, keeps backward compatibiliy with older Pillow versions
    qr_width, qr_height = img.size
    log.info(f"Generated QR code size: {qr_width}x{qr_height}")
    log.info(f"Scaling it to: {width}x{height}")
    # Adding border
    top_border = round((height - qr_height) / 2)
    left_border = round((width - qr_width) / 2)
    result = Image.new("RGBA", (width, height), XbmConverter.xbm_white)
    result.paste(img, (left_border, top_border))
    # Create bytes from image
    img_xbm = XbmConverter(result)
    xbm = img_xbm.convert()
    return xbm, result


def merge_qr_hex(qrcode, ihex, sa, hex_file_out="merged.hex"):
    qrhex = IntelHex()
    qrhex.puts(sa, qrcode)
    ihex.merge(qrhex, overlap="replace")
    ihex.write_hex_file(hex_file_out)
    log.info(f"Created new hex file: {hex_file_out}")
    return hex_file_out


def find_magic_in_hex(ihex):
    found = ihex.find(MAGIC_CONST)
    if found == -1:
        raise EOFError
    offset = found + len(MAGIC_CONST)
    size = ihex[offset] + ihex[offset + 1] * 256
    addr = (
        ihex[(offset + 2)]
        + ihex[(offset + 3)] * 256
        + ihex[(offset + 4)] * (256**2)
        + ihex[(offset + 5)] * (256**3)
    )
    log.info(f"Size of original picture data: {size}")
    log.info(f"Start address: {hex(addr)}")
    return addr, size


def process_device(params):
    board, ip, args = params
    commander = Commander(board, ip)
    # Helper for generating unique filenames during parallel processing
    device_id = board or ip or str(uuid.uuid4())
    # Make sure that the resulting filename is valid
    safe_id = re.sub(r'[^a-zA-Z0-9_-]', '_', device_id)

    # 1) Check if file is in a known format: Intel HEX (.hex), or Motorola S-Records (.s37)
    ihex = None
    if args.hex == FALLBACK_HEX:
        log.warning(
            "No input file specified, try reading it from the device. This may take a little longer than if we had an input file."
        )
        args.hex = f"readout_{safe_id}.hex"
        commander.read_mem(args.hex)
        log.info("Temporary file processing in progress...")
    try:
        f = bincopy.BinFile(args.hex)
    except IOError as e:
        log.critical(e)
        abort(e.errno)
    except bincopy.UnsupportedFileFormatError:
        log.error(f"Can't open file: {args.hex} due to unknown format.")
        abort(-4)
    else:
        try:
            ihex = IntelHex(io.StringIO(f.as_ihex()))
        except:
            log.error("IntelHex import error.")
            abort(-5)
    finally:
        if args.hex.startswith("readout_") and args.hex.endswith(".hex"):
            try:
                os.remove(args.hex)
                log.info(f"Temporary file removed: {args.hex}")
            except PermissionError:
                log.warning(f"Could not remove temporary file: {args.hex}")

    # 2) Read MAC address and create QR code data
    uid = commander.get_board_uid()
    log.info(f"UID: {uid}")
    data = "connect " + uid

    # 3) Generate QR code: the generated data should be a binary which can be flashed to NVM
    bin_image, _ = generate_qrcode(data, args.height, args.width)

    # 4) Find the magic constant with commander in the given hex file
    try:
        start_addr, size = find_magic_in_hex(ihex)
    except EOFError:
        log.error(
            "The QR Code region could not be found. Please check if you have the correct firmware and specified the right target!"
        )
        abort(-6)

    # 5) Check if the space is enough for the QR code
    if size < len(bin_image):
        log.error("There is not enough memory to flash the QR code")
        abort(-7)

    # 6) Generate unique filename per device
    merged_hex_filename = f"merged_{safe_id}.hex"

    merged_hex = merge_qr_hex(bin_image, ihex, start_addr, hex_file_out=merged_hex_filename)
    commander.flash_board(merged_hex)
    log.info(f"Done. Cleaning up merged_{safe_id}.hex")
    os.remove(merged_hex)


def validate_serialno(value):
    """Validate J-Link serial number: must be exactly 9 digits"""
    if not re.match(r'^\d{9}$', value):
        raise argparse.ArgumentTypeError(f"Serial number must be exactly 9 digits, got: {value}")
    return value


def validate_ip(value):
    """Validate IP address: must be valid IPv4 or IPv6 format"""
    # Simple validation for IPv4
    ipv4_pattern = r'^(\d{1,3}\.){3}\d{1,3}$'
    # Simple validation for IPv6
    ipv6_pattern = r'^([0-9a-fA-F]{0,4}:){2,7}[0-9a-fA-F]{0,4}$'
    
    if re.match(ipv4_pattern, value):
        # Additional check for valid IPv4 octets (0-255)
        octets = value.split('.')
        if all(0 <= int(octet) <= 255 for octet in octets):
            return value
    elif re.match(ipv6_pattern, value):
        return value
    
    raise argparse.ArgumentTypeError(f"IP address must be valid IPv4 or IPv6 format, got: {value}")


def main():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=lambda prog: argparse.RawTextHelpFormatter(
            prog, max_help_position=8
        ),
    )
    parser.add_argument(
        "hex",
        help="""The output file of the ESL Tag project or pre-built demo in Intel HEX (.hex) or Motorola S-Records (.s37) format."
Can be omitted if the device is already flashed with the correct file and is unlocked.""",
        nargs="?",
        default=FALLBACK_HEX,
    )
    parser.add_argument(
        "-s", "--serialno", 
        action='append',
        type=validate_serialno,
        help="J-Link serial number of target WSTK (exactly 9 digits). Use multiple times for multiple devices: -s 440128129 -s 440128130"
    )
    parser.add_argument(
        "-i", "--ip", 
        action='append',
        type=validate_ip,
        help="IP address of target WSTK (IPv4 or IPv6). Use multiple times for multiple devices: -i 192.168.1.10 -i 192.168.1.11"
    )
    parser.add_argument(
        "-w",
        "--width",
        type=int,
        help="width of the desired image in pixel",
        default=WSTK_DISPLAY_W,
    )
    parser.add_argument(
        "-he",
        "--height",
        type=int,
        help="height of the desired image in pixel",
        default=WSTK_DISPLAY_H,
    )
    args = parser.parse_args()

    if args.serialno is None:
        args.serialno = [None]
    if args.ip is None:
        args.ip = [None]

    device_list = list(zip_longest(args.serialno, args.ip))
    task_list = [(board, ip, args) for board, ip in device_list]

    cpu_count = os.cpu_count() or 1
    chunk_size = max(1, len(task_list) // cpu_count)

    def cleanup_pool(pool):
        """Helper to terminate and join pool"""
        if pool:
            pool.terminate()
            pool.join()
    
    pool = None
    try: 
        pool = multiprocessing.Pool(processes=min(cpu_count, len(task_list)))
        async_result = pool.map_async(process_device, task_list, chunksize=chunk_size)
        
        # Poll with short timeout to make CTRL+C responsive
        while not async_result.ready():
            try:
                async_result.get(timeout=0.5)  # Short timeout for quick CTRL+C response
                break
            except multiprocessing.TimeoutError:
                continue  # Not ready yet, continue polling
        
        # If we got here, tasks completed successfully
        async_result.get()  # Verify there are no errors
            
    except KeyboardInterrupt:
        log.warning("Interrupted by user")
        cleanup_pool(pool)
        sys.exit(130)  # Standard exit code for CTRL+C
    except ProcessAbortError as e:
        log.critical(f"Process failed with exit code {e.exitcode}")
        cleanup_pool(pool)
        sys.exit(e.exitcode)
    except Exception as e:
        log.critical(f"Unexpected error: {e}")
        cleanup_pool(pool)
        sys.exit(1)
    finally:
        if pool:
            pool.close()
            pool.join()

if __name__ == "__main__":
    main()
