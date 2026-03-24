#!/usr/bin/env python3
# Dummy script that simply copies pre-converted fonts

from flipper.app import App
from shutil import copy


class Main(App):
    def init(self):
        self.parser.add_argument("source", help="Source file")
        self.parser.add_argument("destination", help="Destination file")
        self.parser.set_defaults(func=self.convert)

        self.args = self.parser.parse_args()

    def convert(self):
        copy(self.args.source, self.args.destination)
        return 0


if __name__ == "__main__":
    Main()()
