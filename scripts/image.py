#!/usr/bin/env python3

import sys
import os.path
from pathlib import Path
from flipper.app import App

# FIXME
sys.path.append(
    os.path.abspath(
        os.path.join(
            os.path.dirname(__file__), os.path.pardir, "lib", "lvgl", "scripts"
        )
    )
)

from LVGLImage import LVGLImage


class Main(App):
    def init(self):
        self.parser.add_argument(
            "-o", "--output", required=True, type=Path, help="Output file"
        )
        self.parser.add_argument(
            "-i", "--input", required=True, type=Path, help="Input .png file"
        )
        self.parser.add_argument(
            "-f", "--format", default="BIN", help="Output format BIN or C"
        )
        self.parser.set_defaults(func=self.main)

    def main(self):
        args = self.args

        img = LVGLImage().from_png(args.input)
        img.adjust_stride(align=1)

        if args.format == "BIN":
            temp_bin_path = args.output.with_suffix(args.output.suffix + ".bin")
            img.to_bin(str(temp_bin_path))
            os.rename(temp_bin_path, args.output)
        elif args.format == "C":
            img.to_c_array(str(args.output))

        return 0


if __name__ == "__main__":
    Main()()
