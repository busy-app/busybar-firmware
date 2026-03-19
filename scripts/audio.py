#!/usr/bin/env python3
# How to use:
# audio.py input.mp3 output.snd

from flipper.app import App

import json
import os
import re
import subprocess
import tempfile
from pathlib import Path


def eq_convert_audacity_to_ffmpeg(audacity_eq):
    # audacity_eq = 'FilterCurve:f0="1000.0" f1="2000.0" <junk> v0="1.0" v1="2.0"'
    # k/v pairs are sorted alphabetically!! (e.g. f0, f1, f10, f11, ..., f19, f2, f20)
    params = audacity_eq.split(" ")
    params = list(map(lambda param: param.split(":")[-1].split("="), params))
    serialized_points = list(filter(lambda param: param[0][0] in ["f", "v"], params))

    points = [(0.0, 0.0)] * (len(serialized_points) // 2)
    for [k, v] in serialized_points:
        value = float(v[1:-1])
        index = int(k[1:])
        point = points[index]
        if k[0] == "f":
            point = (value, point[1])
        else:
            point = (point[0], value)
        points[index] = point

    ffmpeg = "; ".join(map(lambda point: f"entry{point}", points))
    return ffmpeg

with open(Path(__file__).parent / "audio_eq.txt", "r") as f:
    global FFMPEG_EQ
    FFMPEG_EQ = eq_convert_audacity_to_ffmpeg(f.read())

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
            # TODO: Find an EQ method that would work on short files
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

        if self.loudness_data["input_i"] == "-inf":
            self.loudness_data["input_i"] = "-99"
        if self.loudness_data["input_tp"] == "-inf":
            self.loudness_data["input_tp"] = "-99"

    def apply_loudness(self):
        cmd = (
            "ffmpeg",
            "-y",
            "-i",
            self.temp_wav_path,
            "-af",
            "[in]"
                "loudnorm=i=-6:tp=-2:"
                f"measured_i={self.loudness_data['input_i']}:"
                f"measured_lra={self.loudness_data['input_lra']}:"
                f"measured_tp={self.loudness_data['input_tp']}:"
                f"measured_thresh={self.loudness_data['input_thresh']}"
            " [normalized]; [normalized] "
                f"firequalizer=gain_entry='{FFMPEG_EQ}'"
            " [out]",
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
