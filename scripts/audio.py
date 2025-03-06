#!/usr/bin/env python3
# How to use:
# audio.py convert input.mp3 output.wav

from flipper.app import App
import subprocess


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        self.parser_convert = self.subparsers.add_parser(
            "convert", help="Convert audio file using FFmpeg"
        )
        self.parser_convert.add_argument("source", help="Source file")
        self.parser_convert.add_argument("destination", help="Destination file")

        self.parser_convert.set_defaults(func=self.convert)

    def convert(self):
        self.args = self.parser.parse_args()
        ffmpeg = (
            "ffmpeg",
            "-i",
            self.args.source,
            "-acodec",
            "pcm_s16le",
            "-f",
            "s16le",
            "-ac",
            "1",
            "-ar",
            "44100",
            self.args.destination,
        )
        print("Running command:", " ".join(ffmpeg))
        subprocess.run(ffmpeg, check=True)
        return 0


if __name__ == "__main__":
    Main()()
