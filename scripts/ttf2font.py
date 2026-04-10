#!/usr/bin/env python3

from flipper.app import App

from shutil import which
from subprocess import run
from os.path import relpath


class Main(App):
    def init(self):
        self.parser.add_argument("-i", "--input", required=True, help="Input .ttf file")
        self.parser.add_argument("-o", "--output", required=True, help="Output file")

        self.parser.add_argument(
            "-s", "--size", required=True, type=int, help="Output font size"
        )

        self.parser.add_argument(
            "-f",
            "--format",
            required=True,
            choices=["bin", "lvgl"],
            help="Output font format",
        )

        self.parser.set_defaults(func=self.convert)

        self.args = self.parser.parse_args()

    def convert(self):
        lv_font_conv = which("lv_font_conv")
        if not lv_font_conv:
            return 1

        run_result = run([lv_font_conv, "--version"], capture_output=True, text=True)
        if run_result.returncode != 0 or not run_result.stdout.strip().endswith(
            "flipper"
        ):
            return 1

        lv_font_conv_command = [
            lv_font_conv,
            "--font",
            relpath(self.args.input),
            "-o",
            relpath(self.args.output),
            "--bpp",
            "1",
            "--size",
            str(self.args.size),
            "--no-compress",
            "--format",
            self.args.format,
            "--range",
            "0-65535",
        ]

        return run(lv_font_conv_command).returncode


if __name__ == "__main__":
    Main()()
