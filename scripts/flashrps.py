#!/usr/bin/env python3

import argparse
import io
import os
import time

import serial

"""
Purpose:
    Flashing the firmware to a SiWx917 chip via the serial bootloader.

How to use:
    1. Connect a USB-UART adapter: TX -> GPIO_9, RX -> GPIO_8 (data directions as denoted on chip)
    2. Connect JTAG_TDO_SWO to ground
    3. Reset the chip
    4. Start this program by specifying the serial port and the firmware file
    5. If firmware does not start after a successful flashing, disconnect JTAG_TDO_SWO and reset the chip

Resoures used:

    1. Kermit protocol manual: https://www.kermitproject.org/kproto.pdf
    2. Kermit packet reference: https://www.kermitproject.org/kpackets.html
    3. Embedded C implementation: https://github.com/KermitProject/ekermit

    Caution: above sources occasionally contradict each other, consider cross-checking

"""


class Kermit:
    """
    Partial implementation of the Kermit protocol needed for flashing of
    the SiWx917 series devices.
    """

    def __init__(self):
        # Sequence number
        self.sequence = 0
        # Maximum regular packet length
        self.max_packet_length = 94
        # Maximum extended packet length
        self.max_ext_packet_length = 9024
        # Block check length
        self.check_length = 1
        # Timetout in seconds
        self.timeout = 5
        # Control prefix character (see below)
        self.control_prefix = b"#"

    def init_packet(self) -> bytes:
        """
        Create a Kermit initial connection packet.
        It is sent only once when initiating a connection with the target.

        Returns:
            Buffer containing a fully formed init packet
        """

        data = bytes()
        # Maximum regular packet length
        data += Kermit.__tochar(self.max_packet_length)
        # Timeout in seconds
        data += Kermit.__tochar(self.timeout)
        # Number of padding characters (none)
        data += Kermit.__tochar(0)
        # Padding character (not using padding)
        data += Kermit.__ctl(0)
        # End of packet character (terminator)
        data += Kermit.__tochar(ord("\r"))
        # Control prefix - used to escape special characthers
        data += self.control_prefix
        # No 8th bit quoting (using a proper 8-bit line)
        data += b"N"
        # Block check type
        data += b"1"
        # Run-length prefix (not using run-length compression)
        data += b" "
        # Capabilities: extended packets (bit 1)
        data += Kermit.__tochar(0x02)
        # Sliding window size (not using sliding window)
        data += Kermit.__tochar(0)
        # Maximum extended packet length (2 bytes, mod 95)
        data += Kermit.__tochar(self.max_ext_packet_length // 95)
        data += Kermit.__tochar(self.max_ext_packet_length % 95)

        return self.__encode_packet(b"S", data)

    def file_header_packet(self) -> bytes:
        """
        Create a file header packet.
        It is sent before each file to be transmitted.

        Returns:
            Buffer containing a fully formed file header packet
        """

        # Use a hardcoded name for the file for simplicity
        return self.__encode_packet(b"F", b"FIRMWA.RPS")

    def file_data_packet(self, stream: io.BufferedReader) -> bytes:
        """
        Create an extended file data packet.
        It is sent one or more times per file, depending on the file size.

        This funciton will read up to self.max_ext_packet_length - self.check_length
        bytes from the stream, then encode it into an extended packet.

        If no more data is available to read from the stream, an empty buffer will be returned.

        Args:
            stream: the stream to read the data from

        Returns:
            Buffer containing a fully formed file data packet, or an empty buffer if no data left
        """

        data = bytes()

        def packet_length():
            return len(data) + self.check_length

        while packet_length() < self.max_ext_packet_length:
            c = stream.read(1)

            if len(c) == 0:
                break

            # len(ec) can be either 1 or 2
            ec = self.__encode_character(c)

            if packet_length() + len(ec) > self.max_ext_packet_length:
                # adding the next encoded character would exceed the maximum packet length
                # go back 1 position to process it later
                stream.seek(-1, os.SEEK_CUR)
                break

            data += ec

        return self.__encode_extended_packet(b"D", data) if len(data) != 0 else data

    def eof_packet(self) -> bytes:
        """
        Create an end of file packet.
        It is sent after all the file data has been transmitted.

        Returns:
            Buffer containing a fully formed EOF packet
        """
        return self.__encode_packet(b"Z", bytes())

    def break_packet(self) -> bytes:
        """
        Create a break (end of communication) packet.
        It is sent to signal the end of communication (which
        is needed to start the firmware update procedure)

        Returns:
            Buffer containing a fully formed break packet
        """
        return self.__encode_packet(b"B", bytes())

    def process_response_packet(self, data: bytes):
        """
        Process the response packet received from the target and increase
        the sequence number for the next request packet.

        Args:
            data: buffer containing the response packet
            init: if True, treat the data as a response to the init packet
        """

        # TODO: More checks, better exceptions
        if chr(data[3]) != "Y":
            raise ConnectionError

        # Advance sequence number
        self.sequence = (self.sequence + 1) % 64

    def process_init_response_packet(self, data: bytes):
        # Adjust regular packet length
        self.max_packet_length = Kermit.__unchar(data[0])
        # Adjust extended packet length
        self.max_ext_packet_length = Kermit.__unchar(data[11]) * 95 + Kermit.__unchar(
            data[12]
        )

    def __encode_character(self, c: bytes) -> bytes:
        """
        Encode a single character according to the Kermit scheme:
        - Control characters (0x0...0x20, 0x7F) are prefixed and encoded with ctl()
        - The prefix character is prefixed with itself
        - All other characters are transmitted verbatim

        Args:
            c: buffer of size 1 containing the character to be encoded

        Returns:
            Buffer of size 1 or 2 containing the encoded data
        """

        ret = bytes()
        cval = ord(c)

        if cval & 0x7F < 0x20 or cval & 0x7F == 0x7F:
            # Control character, prefix and encode ctl()
            ret += self.control_prefix
            ret += Kermit.__ctl(cval)
        elif c == self.control_prefix:
            # Control prefix character, prefix and transmit as is
            ret += self.control_prefix
            ret += self.control_prefix
        else:
            # Not a special character, transmit as is
            ret = c

        return ret

    def __encode_packet(self, packet_type: bytes, data: bytes) -> bytes:
        """
        Encode a payload packet to a top-level Kermit packet.

        Packet types used in this implementation:
        - "S": init (send-init)
        - "F": file header
        - "D": data
        - "Z": end of file
        - "B": break

        Args:
            packet_type: a single character denoting the packet type (see above)
            data: buffer containing the payload to be encoded

        Returns:
            Buffer containing a fully formed Kermit packet
        """

        # Counted packet length: data + seq + type + check
        packet_len = len(data) + self.check_length + 2

        if packet_len > self.max_packet_length:
            raise ValueError("Maximum packet length is exceeded")

        ret = bytes()
        # Mark character: ASCII 0x01
        ret += 0x01.to_bytes()
        # Packet length counting from (but not including) this field
        ret += Kermit.__tochar(packet_len)
        # Sequence number
        ret += Kermit.__tochar(self.sequence)
        # Packet type
        ret += packet_type
        # Data payload
        ret += data
        # Block check (Mark character not included)
        ret += Kermit.__check(ret[1:])
        # Terminator character (end of packet)
        ret += b"\r"

        return ret

    def __encode_extended_packet(self, packet_type: bytes, data: bytes) -> bytes:
        """
        Encode a payload packet to an extended top-level Kermit packet.

        Exteneded packets are a protocol extension allowing for faster communication speed.
        They can only be used after the connection has been initialised via the init packet.

        See __encode_packet() for packet types.

        Args:
            packet_type: a single character denoting the packet type
            data: buffer containing the payload to be encoded

        Returns:
            Buffer containing a fully formed extended Kermit packet
        """

        # Counted packet length: data + check
        packet_len = len(data) + self.check_length

        if packet_len > self.max_ext_packet_length:
            raise ValueError("Maximum extended packet length is exceeded")

        ret = bytes()
        # Mark character: ASCII 0x01
        ret += 0x01.to_bytes()
        # Blank field: indication that this is an extended packet
        ret += Kermit.__tochar(0)
        # Sequence number
        ret += Kermit.__tochar(self.sequence)
        # Packet type
        ret += packet_type
        # Packet length (2 bytes, mod 95), counted from (including) the first data byte.
        ret += Kermit.__tochar(packet_len // 95)
        ret += Kermit.__tochar(packet_len % 95)
        # Extended header block check, calculated from pos 1 to 5 (inclusive)
        ret += Kermit.__check(ret[1:6])
        # Data payload
        ret += data
        # Block check (Mark character not included)
        ret += Kermit.__check(ret[1:])
        # Terminator character (end of packet)
        ret += b"\r"

        return ret

    @staticmethod
    def __tochar(c: int) -> bytes:
        """
        Convert an integer value to a printable character, according to the Kermit protocol scheme.

        Args:
            c: the value to be encoded (must be less than or equal to 95)

        Returns:
            Buffer of length 1 containing the printable representation of the value
        """
        return (c + 32).to_bytes()

    @staticmethod
    def __unchar(c: int) -> int:
        """
        Convert a printable character back to its value, according to the Kermit protocol scheme.

        Args:
            c: ordinal value of the printable representation

        Returns:
            The original numerical value that was encoded with __tochar()
        """
        return c - 32

    @staticmethod
    def __ctl(c: int) -> bytes:
        """
        Convert a control character to a printable character, according to the Kermit protocol scheme.

        Args:
            c: the character value to be encoded (0..31, 127)

        Returns:
            Buffer of length 1 containing the printable representation of the value
        """
        return (c ^ 0x40).to_bytes()

    @staticmethod
    def __check(data: bytes) -> bytes:
        """
        Calculate a simple 1-byte checksum for the given buffer.
        This is the standard Kermit checksum, used both in top-level packets
        and in extended packet headers.

        Args:
            data: buffer containing the data to calculate the checksum for

        Returns:
            Buffer containing the printable representation of the checksum
        """
        s = sum(data)
        return Kermit.__tochar((s + ((s & 0xC0) >> 6)) & 0x3F)


class RpsFlasher:
    RESET_DELAY_SEC = 0.8

    def __init__(self, port_name: str, filename: str, image_type: str, debug=0):
        """
        Args:
            port_name: the path to the port to use (/dev/ttyXXX or COMX)
            filename: the path to the *.rps file to be flashed
            image_type: the type of image to flash (m4 or ta)
            debug: debug logging level (0 - no logging, 3 - max verbosity)
        """

        self.serial = None
        self.serial = serial.Serial(port_name, 115200)
        self.rps_file = open(filename, "rb")
        self.file_size = os.path.getsize(filename)
        self.image_type = image_type
        self.kermit = Kermit()
        self.debug = debug

    def __del__(self):
        if self.serial:
            self.serial.close()

    def init_device(self):
        """
        Initialise the target device to accept the firmware file by selecting
        the appropriate textual menu options.
        """

        print("Waiting for target...")
        self.__wait_for_device()
        print("Target connected!")
        print("Setting up, please wait...")
        self.__increase_baudrate()
        self.__wait_for_baudrate()
        self.__initiate_transfer()
        print("Target initialised!")

    def flash_firmware(self):
        """
        Flash the firmware using the Kermit protocol.
        """

        self.__kermit_init_transaction()

        if self.debug > 1:
            print(f"Regular packet length: {self.kermit.max_packet_length}")
            print(f"Extended packet length: {self.kermit.max_ext_packet_length}")

        self.__kermit_transaction(self.kermit.file_header_packet())

        while True:
            if self.debug == 0:
                progress = int((self.rps_file.tell() / self.file_size) * 100)
                print(f"Uploading firmware: {progress}%", end="\r")

            packet = self.kermit.file_data_packet(self.rps_file)

            if len(packet) == 0:
                break

            self.__kermit_transaction(packet)

        self.__kermit_transaction(self.kermit.eof_packet())
        self.__kermit_transaction(self.kermit.break_packet())

        print("\nFirmware upload complete!")

    def check_firmware(self):
        """
        Wait until the bootloader reports success.
        """
        print("Checking firmware, please wait...")

        self.__raw_transaction(
            bytes(),
            b"Upgradation Successful\r\n\r\nEnter Next Command\r\n",
            30,
        )

        print("Firmware flashed successfully!")

    # dtr: 15 pin on Flipper -> goes to reset pin
    def __signal_reset(self, state):
        """
        Set the DTR signal connected to RESET pin to the specified state.
        """
        self.serial.dtr = state

    # rts: 16 pin on Flipper -> goes to boot pin (SWO)
    def __signal_boot(self, state):
        """
        Set the RTS signal connected to BOOT pin to the specified state.
        """
        self.serial.rts = state

    def reset_device(self):
        """
        Reset the target device.
        """
        self.__signal_reset(1)
        time.sleep(self.RESET_DELAY_SEC)
        self.__signal_reset(0)

    def __put_device_to_bootloader(self):
        """
        Put the target device into the bootloader mode.
        """
        self.__signal_boot(1)
        self.__signal_reset(1)
        time.sleep(self.RESET_DELAY_SEC)
        self.__signal_reset(0)
        time.sleep(self.RESET_DELAY_SEC)
        self.__signal_boot(0)

    def __wait_for_device(self):
        """
        Wait until the bootloader reports readiness.
        """

        while True:
            try:
                # self.__put_device_to_bootloader()
                self.__wait_for_menu()
                break

            except TimeoutError:
                continue

    def __wait_for_menu(self):
        """
        Wait until the bootloader prints the textual menu.
        """

        self.__raw_transaction(b"\xff", b"Enter 'U'")
        self.__raw_transaction(b"U", b"Change UART Baud Rate\r\n")

    def __wait_for_baudrate(self):
        """
        Wait until the bootloader completes the baud rate change.
        """

        self.__raw_transaction(
            b"\xff", b"Waiting for Correct Option...\r\n", timeout=5.0
        )

    def __initiate_transfer(self):
        """
        Initiate the firmware update procedure (before starting a Kermit connection).
        """

        if self.image_type == "m4":
            self.__raw_transaction(b"4", b"Enter M4 Image No(1-f)\r\n")
            self.__raw_transaction(b"1", b"Send MCU firmware(*.rps)         \r\n")
        elif self.image_type == "ta":
            self.__raw_transaction(b"B", b"Enter Wireless Image No(0-f)\r\n")
            self.__raw_transaction(b"0", b"Send NWP firmware(*.rps)         \r\n")

    def __increase_baudrate(self):
        """
        Configure the bootloader to use 921600 baud instead of 115200.
        """

        self.__raw_transaction(b"b", b"5 115200\r\n")
        self.__raw_transaction(b"4", b"4")
        # Switch baud rate
        self.serial.baudrate = 921600
        # Confirm new baud rate
        self.__raw_transaction(b"U", b"Baud Rate was updated successfully!")

    def __raw_transaction(self, command: bytes, expected: bytes, timeout=0.1) -> bytes:
        """
        Perform a request-response bootloader transaction (text mode) and
        wait for the expected response.

        Args:
            command: the command to send to the bootloader
            expected: the expected sequence to be seen last in the output
            timeout: maximum time to wait for expected output, in seconds
        """

        self.serial.timeout = timeout

        if len(command):
            self.serial.write(command)

        rx_data = self.serial.read_until(expected)

        if self.debug > 2:
            print(f"TX: {command}")
            print(f"RX: {rx_data}")
        if self.debug > 3:
            print(
                f"TX: {len(command)}b: " + " ".join(f"{byte:02x}" for byte in command)
            )
            print(
                f"RX: {len(rx_data)}b: " + " ".join(f"{byte:02x}" for byte in rx_data)
            )
            print("-" * 40)

        if not rx_data.endswith(expected):
            raise TimeoutError

        return rx_data

    def __kermit_init_transaction(self):
        """
        Initialise the Kermit-based connection.
        During the protocol initialisation, the sides negotiate several parameters,
        including capabilities and buffer sizes.
        """

        tx_data = self.kermit.init_packet()
        rx_data = self.__raw_transaction(tx_data, b"\r", self.kermit.timeout)

        self.kermit.process_response_packet(rx_data)
        self.kermit.process_init_response_packet(rx_data[4:])

    def __kermit_transaction(self, packet: bytes):
        """
        Perform a Kermit transaction.

        Args:
            packet: a valid Kermit packet to send to the target
        """
        rx_data = self.__raw_transaction(packet, b"\r", self.kermit.timeout)
        self.kermit.process_response_packet(rx_data)


def main():
    parser = argparse.ArgumentParser()

    parser.add_argument("-p", required=True, metavar="PORT", help="serial port to use")
    parser.add_argument(
        "-d",
        type=int,
        default=0,
        metavar="LEVEL",
        help="debug level (0 - none, 4 - full)",
    )
    parser.add_argument(
        "-t",
        default="m4",
        metavar="TYPE",
        help='image type to flash ("m4" or "ta")',
    )
    parser.add_argument("filename", metavar="FILE", help="full path to *.rps file")

    args = parser.parse_args()

    if args.t != "m4" and args.t != "ta":
        print(f"Unknown image type: {args.t}\n")
        parser.print_help()
        exit(1)

    if args.p == "auto":
        print("Please specify the serial port to use\n")
        parser.print_help()
        exit(1)

    flasher = RpsFlasher(args.p, args.filename, args.t, args.d)

    flasher.init_device()
    flasher.flash_firmware()
    flasher.check_firmware()
    flasher.reset_device()


if __name__ == "__main__":
    main()
