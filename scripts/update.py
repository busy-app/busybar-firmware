#!/usr/bin/env python3

import shutil
import subprocess
import sys
import time
from pathlib import Path

from flipper.app import App
from flipper.storage_socket import FlipperStorage, FlipperStorageOperations


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
        self.parser_update.add_argument("dfu_path", help="Path to the DFU file")
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

    def __ensure_dfu_util(self):
        if not shutil.which("dfu-util"):
            raise RuntimeError(
                "dfu-util not found. Please install it from https://dfu-util.sourceforge.net/"
            )

    def _upload_file(self, file_path):
        remote_path = str(Path("/ext") / Path(file_path).name)
        FlipperStorageOperations(self.storage).recursive_send(
            remote_path, file_path, True
        )
        return remote_path

    def find_dfu_devices(self):
        dfu_devices = subprocess.check_output(
            ["dfu-util", "--list"], stderr=subprocess.STDOUT
        ).decode("utf-8")
        return "Found DFU" in dfu_devices

    def update_u5(self):
        try:
            self.__ensure_dfu_util()
            if self.args.to_dfu and not self.find_dfu_devices():
                self.logger.info("Trying to reset the device to DFU mode")
                self.storage = FlipperStorage(self._get_port(self.args.port))
                self.storage.start()
                print("Sending boot command to the device")
                self.storage.send_and_wait_eol("power boot u5\r\n")
                time.sleep(self.DFU_WAIT_TIME)

            self.logger.info("Uploading STM32U5 firmware")
            subprocess.check_call(
                [
                    "dfu-util",
                    "-D",
                    self.args.dfu_path,
                ],
                stdout=sys.stdout,
                stderr=subprocess.STDOUT,
            )
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
