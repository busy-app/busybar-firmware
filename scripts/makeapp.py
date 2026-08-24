#!/usr/bin/env python3

from flipper.app import App
from shutil import copytree


class Main(App):
    def init(self):
        self.parser.add_argument("source", help="Source directory")
        self.parser.add_argument("destination", help="Destination directory")
        self.parser.set_defaults(func=self.build)

        self.args = self.parser.parse_args()

    def build(self):
        copytree(self.args.source, self.args.destination, dirs_exist_ok=True)
        return 0


if __name__ == "__main__":
    Main()()
