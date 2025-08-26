#!/usr/bin/env python3

import os
import tempfile

from flipper.app import App
from seq2anim import BusyBarAnimation
from flipper.storage_socket import FlipperStorage, FlipperStorageOperations


class Main(App):
    ANIM_FILE_NAME = "temp.anim"
    ANIM_DEVICE_PATH = "/ext/animations"

    def init(self):
        self.parser.add_argument(
            "-a", "--address", help="IP address or hostname", default="auto"
        )

        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        self.parser_anim = self.subparsers.add_parser(
            "anim", help="Start an animation on the device"
        )
        self.parser_anim.add_argument("source_dir", help="Source directory")
        self.parser_anim.add_argument(
            "-f", "--fps", help="Animation FPS", type=int, default=60
        )
        self.parser_anim.set_defaults(func=self.anim)

        self.parser_lottie = self.subparsers.add_parser(
            "lottie", help="Start a lottie animation on the device"
        )
        self.parser_lottie.add_argument("lottie_file", help="Lottie JSON file")
        self.parser_lottie.set_defaults(func=self.lottie)

    def anim(self):
        with tempfile.TemporaryDirectory(prefix="bsb_anim") as temp_dir_path:
            local_anim_path = os.path.join(temp_dir_path, self.ANIM_FILE_NAME)
            device_anim_path = os.path.join(self.ANIM_DEVICE_PATH, self.ANIM_FILE_NAME)

            self.logger.info("Processing source files")
            animation = BusyBarAnimation(
                self.args.source_dir, self.args.fps, local_anim_path
            )
            animation.process_images()

            with FlipperStorage(self._get_address()) as storage:
                # TODO: Common CLI library for everything that uses it

                self.logger.info("Uploading animation")
                FlipperStorageOperations(storage).recursive_send(
                    device_anim_path, local_anim_path, True
                )

                self.logger.info("Starting player app")
                storage.send_and_wait_prompt(
                    f"loader open animation_player {device_anim_path}\r"
                )

        return 0

    def lottie(self):
        with FlipperStorage(self._get_address()) as storage:
            lottie_file_path = self.args.lottie_file
            lottie_file_name = os.path.basename(lottie_file_path)
            device_anim_path = os.path.join(self.ANIM_DEVICE_PATH, lottie_file_name)

            # TODO: Common CLI library for everything that uses it

            self.logger.info("Uploading animation")
            FlipperStorageOperations(storage).recursive_send(
                device_anim_path, lottie_file_path, True
            )

            self.logger.info("Starting player app")
            storage.send_and_wait_prompt(
                f"loader open lottie_player {device_anim_path}\r"
            )

        return 0

    def _get_address(self):
        if self.args.address != "auto":
            return (self.args.address, 23)
        else:
            return ("10.0.4.20", 23)


if __name__ == "__main__":
    Main()()
