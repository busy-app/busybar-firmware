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
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        self.parser_anim = self.subparsers.add_parser(
            "anim", help="Start an animation on the device"
        )
        self.parser_anim.add_argument("source_dir", help="Source directory")
        self.parser_anim.add_argument("-f", "--fps", help="Animation FPS", type=int, default=60)
        self.parser_anim.set_defaults(func=self.anim)

    def anim(self):
        with tempfile.TemporaryDirectory(prefix="bsb_anim") as temp_dir_path:
            local_anim_path = os.path.join(temp_dir_path, self.ANIM_FILE_NAME)
            device_anim_path = os.path.join(self.ANIM_DEVICE_PATH, self.ANIM_FILE_NAME)

            self.logger.info("Processing source files")
            animation = BusyBarAnimation(self.args.source_dir, self.args.fps, local_anim_path)
            animation.process_images()

            with FlipperStorage(self._get_port()) as storage:
                self.logger.info("Closing currently running app")
                storage.send_and_wait_prompt("loader kill\r")

                self.logger.info("Uploading animation")
                FlipperStorageOperations(storage).recursive_send(
                    device_anim_path, local_anim_path, True
                )

                self.logger.info("Starting player app")
                # TODO: Common CLI library for everything that uses it
                storage.send_and_wait_prompt(
                    f"loader open animation_player {device_anim_path}\r"
                )

        return 0

    def _get_port(self):
        return ("10.0.4.20", 23)


if __name__ == "__main__":
    Main()()
