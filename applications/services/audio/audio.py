#!/usr/bin/env python3

from flipper.app import App
import subprocess
import array


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")
        self.parser_convert = self.subparsers.add_parser(
            "convert", help="Process icons and build icon registry"
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
            "12625",
            "-",  # stdout
        )
        print(" ".join(ffmpeg))
        with open(self.args.destination, "w") as file_output:
            with subprocess.Popen(ffmpeg, stdout=subprocess.PIPE) as proc:
                while data := proc.stdout.read(4096):
                    data_input = array.array("h", data)
                    data_output = array.array("H")
                    for i in data_input:
                        value = (i + 32768) >> 4
                        data_output.append(value)
                    data_block = ", ".join(str(i) for i in data_output)
                    file_output.write(data_block + ",\n")
        return 0


if __name__ == "__main__":
    Main()()
