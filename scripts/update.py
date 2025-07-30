#!/usr/bin/env python3

import shutil
import subprocess
import time
from pathlib import Path

from flipper.app import App
from flipper.storage_socket import FlipperStorage, FlipperStorageOperations


class DfuProgrammerBackend:
    @classmethod
    def is_available(cls):
        print(f"Checking for {cls.UTIL_BIN_NAME}...")
        return shutil.which(cls.UTIL_BIN_NAME) is not None

    @classmethod
    def is_file_supported(cls, file_path):
        return Path(file_path).suffix.lower() == cls.SUPPORTED_FILE_EXTENSION

    def execute(self, *args):
        try:
            subprocess.check_call([self.UTIL_BIN_NAME] + list(args))
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"Error executing {self.UTIL_BIN_NAME}: {e}")

    def program_firmware(self, fw_file):
        raise NotImplementedError()

    def find_devices(self):
        raise NotImplementedError()


class DfuUtil(DfuProgrammerBackend):
    UTIL_BIN_NAME = "dfu-util"
    SUPPORTED_FILE_EXTENSION = ".dfu"

    def program_firmware(self, fw_file):
        self.execute("-a", "0", "-D", fw_file, "-R")

    def find_devices(self):
        try:
            result = subprocess.check_output(
                [self.UTIL_BIN_NAME, "--list"], stderr=subprocess.STDOUT
            )
            return b"Found DFU" in result
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"Error executing {self.UTIL_BIN_NAME}: {e}")


class CubeCLIProgrammer(DfuProgrammerBackend):
    UTIL_BIN_NAME = "STM32_Programmer_CLI"
    SUPPORTED_FILE_EXTENSION = ".hex"

    def program_firmware(self, fw_file):
        self.execute(
            "-c",
            "port=usb1",
            "-d",
            fw_file,
            "--start",
        )

    def find_devices(self):
        try:
            result = subprocess.check_output(
                [self.UTIL_BIN_NAME, "-l"], stderr=subprocess.STDOUT
            )
            return b"DFU in HS Mode" in result
        except subprocess.CalledProcessError as e:
            raise RuntimeError(f"Error executing {self.UTIL_BIN_NAME}: {e}")


_programmers = [CubeCLIProgrammer, DfuUtil]


def get_available_dfu_programmer():
    for programmer in _programmers:
        if programmer.is_available():
            print(f"Using {programmer.UTIL_BIN_NAME} as DFU programmer")
            return programmer()
    raise RuntimeError(
        f"No DFU programmer found. Please proide one of {', '.join([p.UTIL_BIN_NAME for p in _programmers])}"
    )


def get_dfu_programmer_for_file(file_path):
    for programmer in _programmers:
        if programmer.is_file_supported(file_path):
            print(f"Using {programmer.UTIL_BIN_NAME} as DFU programmer")
            return programmer()
    raise RuntimeError(
        f"No DFU programmer found for file type {Path(file_path).suffix}. Please provide one of {', '.join([p.SUPPORTED_FILE_EXTENSION for p in _programmers])}"
    )


class UpdaterMain(App):
    DFU_WAIT_TIME = 3

    def init(self):
        self.parser.add_argument(
            "-p", "--port", help="Device identifier (ip address)", default="auto"
        )
        self.parser.add_argument(
            "-a", "--address", help="Device IP address", default="auto"
        )

        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_update = self.subparsers.add_parser(
            "u5", help="Update the STM32U5 firmware"
        )
        self.parser_update.add_argument(
            "firmware_path", help="Path to the firmware file"
        )
        self.parser_update.add_argument(
            "--to-dfu",
            action="store_true",
            help="Try to reset the device to DFU mode first",
        )
        self.parser_update.set_defaults(func=self.update_u5)

        self.parser_update = self.subparsers.add_parser(
            "917", help="Update the coprocessor firmware"
        )
        self.parser_update.add_argument("rps_path", help="Path to the RPS file")
        self.parser_update.add_argument(
            "--nwp", action="store_true", help="Update file is NWP firmware"
        )
        self.parser_update.set_defaults(func=self.update_917)

        self.storage = None

    # TODO: move to common function
    def _get_port(self, port_value):
        if port_value != "auto":
            return (port_value, 23)
        return ("10.0.4.20", 23)

    def _upload_file(self, file_path):
        remote_path = str(Path("/ext") / Path(file_path).name)
        remote_path = Path(remote_path).as_posix()
        FlipperStorageOperations(self.storage).recursive_send(
            remote_path, file_path, True
        )
        return remote_path

    def update_u5(self):
        try:
            dfu_tool = get_dfu_programmer_for_file(self.args.firmware_path)
            if not dfu_tool.is_available():
                raise RuntimeError(f"{dfu_tool.UTIL_BIN_NAME} is not available")

            if self.args.to_dfu and not dfu_tool.find_devices():
                self.logger.info("Trying to reset the device to DFU mode")
                self.storage = FlipperStorage(self._get_port(self.args.port))
                self.storage.start()
                print("Sending boot command to the device")
                self.storage.send_and_wait_eol("power boot u5\r\n")
                time.sleep(self.DFU_WAIT_TIME)

            self.logger.info("Uploading STM32U5 firmware")
            dfu_tool.program_firmware(self.args.firmware_path)
            self.logger.info("Firmware installed successfully")
            return 0
        except Exception as e:
            self.logger.error(f"Failed to install firmware: {e}")
            return 1

    def update_917(self):
        try:
            self.storage = FlipperStorage(self._get_port(self.args.port))
            self.storage.start()
            self.logger.info("Uploading 917 firmware")
            remote_path = self._upload_file(self.args.rps_path)
            self.logger.info("Installing 917 firmware")
            command = f"update {'917_ta' if self.args.nwp else '917'} {remote_path}\r\n"
            result = self.storage.send_and_wait_prompt(command)
            if not b"Update succeeded" in result:
                raise RuntimeError("Update failed")
            self.logger.info("Firmware installed successfully")
            return 0
        except Exception as e:
            self.logger.error(f"Failed to install firmware: {e}")
            return 1

    def __del__(self):
        if self.storage:
            self.storage.stop()


if __name__ == "__main__":
    UpdaterMain()()
