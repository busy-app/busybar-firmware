#!/usr/bin/env python3
# How to use:
# audio.py input.mp3 output.snd

from flipper.app import App

import json
import os
import re
import subprocess
import tempfile


class Main(App):
    def init(self):
        self.parser.add_argument("source", help="Source file")
        self.parser.add_argument("destination", help="Destination file")
        self.parser.set_defaults(func=self.convert)

        self.args = self.parser.parse_args()

        self.workdir = tempfile.TemporaryDirectory()
        self.source_name = os.path.splitext(os.path.basename(self.args.source))[0]
        self.temp_wav_path = os.path.join(self.workdir.name, self.source_name) + ".wav"

    # TODO: characterize the speaker frequency response and adjust the EQ accordingly
    def apply_eq(self):
        cmd = (
            "ffmpeg",
            "-y",
            "-i",
            self.args.source,
            "-ac",
            "1",
            "-af",
            "superequalizer="
            "1b=0:2b=0:3b=0:4b=0:"
            "5b=0:6b=0:7b=0:8b=1:"
            "9b=0.8:10b=0.3:11b=0.4:12b=0.5:"
            "13b=0.5:14b=0.5:15b=0.5:16b=0.5:"
            "17b=0:18b=0",
            self.temp_wav_path,
        )

        subprocess.run(cmd, capture_output=True, check=True)

    def analyze_loudness(self):
        cmd = (
            "ffmpeg",
            "-y",
            "-i",
            self.temp_wav_path,
            "-af",
            "loudnorm=print_format=json",
            "-f",
            "null",
            "-"
        )

        proc = subprocess.run(cmd, capture_output=True, check=True)
        # Find the JSON output inside the plaintext output in stderr
        json_str = re.search(r"\{.+\}", proc.stderr.decode(), re.DOTALL)[0]

        self.loudness_data = json.loads(json_str)

    def apply_loudness(self):
        cmd = (
            "ffmpeg",
            "-y",
            "-i",
            self.temp_wav_path,
            "-af",
            "loudnorm=i=-6:tp=-2:"
            f"measured_i={self.loudness_data['input_i']}:"
            f"measured_lra={self.loudness_data['input_lra']}:"
            f"measured_tp={self.loudness_data['input_tp']}:"
            f"measured_thresh={self.loudness_data['input_thresh']}",
            "-acodec",
            "pcm_s16le",
            "-f",
            "s16le",
            "-ar",
            "44100",
            self.args.destination,
        )

        subprocess.run(cmd, capture_output=True, check=True)

    def convert(self):
        self.apply_eq()
        self.analyze_loudness()
        self.apply_loudness()

        return 0


if __name__ == "__main__":
    Main()()
