#!/usr/bin/env python3

import os
import tempfile
from pathlib import Path
from typing import Callable

from flipper.app import App
from seq2anim import BSBAnimConverter, ConversionError, ConversionInfo
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
            "anim", help="Play a BSB formatted animation on the device, from a folder with images"
        )
        self.parser_anim.add_argument("-s", "--display", help="Display to play animation on")
        self.parser_anim.add_argument("source_dir", type=Path, help="Source directory")
        self.parser_anim.set_defaults(func=self.anim)

        self.parser_anim_zip = self.subparsers.add_parser(
            "anim_zip", help="Play a BSB formatted animation on the device, from a ZIP file with images"
        )
        self.parser_anim_zip.add_argument("-s", "--display", help="Display to play animation on")
        self.parser_anim_zip.add_argument("source_zip", type=Path, help="Source ZIP file")
        self.parser_anim_zip.set_defaults(func=self.anim_zip)

        self.parser_lottie = self.subparsers.add_parser(
            "lottie", help="Start a lottie animation on the device"
        )
        self.parser_lottie.add_argument("lottie_file", help="Lottie JSON file")
        self.parser_lottie.set_defaults(func=self.lottie)

    def _play_converted(self, converted: Path, display_id: str|None):
        with FlipperStorage(self._get_address()) as storage:
            # TODO: Common CLI library for everything that uses it

            self.logger.info("Uploading animation")
            converted_on_device = str(Path(self.ANIM_DEVICE_PATH) / self.ANIM_FILE_NAME)
            FlipperStorageOperations(storage).recursive_send(
                converted_on_device,
                str(converted),
                True
            )

            self.logger.info("Starting player app")
            separator = ":" if display_id else ""
            storage.send_and_wait_prompt(
                f"loader open animation_player {display_id or ''}{separator}{converted_on_device}\r"
            )

    def _generic_play_anim(self, convert_fn: Callable[[BSBAnimConverter, Path], ConversionInfo], display_id: str|None) -> int:
        converter = BSBAnimConverter()
        with tempfile.TemporaryDirectory(prefix="bsb-anim-viewer-") as tmp_dir:
            converted = Path(tmp_dir) / self.ANIM_FILE_NAME
            try:
                self.logger.info("Converting animation")
                info = convert_fn(converter, converted)
                self.logger.info(info)
            except ConversionError as ex:
                self.logger.error(ex)
                return 1
            
            self._play_converted(converted, display_id)

        return 0

    def anim_zip(self):
        args = self.args
        return self._generic_play_anim(
            lambda converter, converted:
                converter.convert_zip(args.source_zip, converted),
            args.display
        )

    def anim(self):
        args = self.args
        return self._generic_play_anim(
            lambda converter, converted:
                converter.convert_dir(args.source_dir, converted),
            args.display
        )

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
