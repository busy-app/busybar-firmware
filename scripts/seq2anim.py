#!/usr/bin/env python3

import os
import sys
import struct
import logging
import argparse
import tempfile
from PIL import Image
from zipfile import PyZipFile


class BusyBarAnimation:
    def __init__(self, input_folder, fps, output_file):
        if not os.path.isdir(input_folder):
            raise FileNotFoundError("Invalid path")

        self.input_folder = input_folder
        self.fps = fps
        self.output_file = output_file
        self.png_files = []
        self.width = 72  # default display width
        self.height = 16  # default display height
        self.bytes_per_pixel = 3  # RGB format
        self.logger = logging.getLogger("Seq2Anim")

    def load_images(self):
        """Load and sort PNG images from the input folder."""
        self.png_files = [
            f for f in os.listdir(self.input_folder) if f.lower().endswith(".png")
        ]
        """ Sort the files in natural order """
        self.png_files.sort(key=lambda x: int("".join(filter(str.isdigit, x))))

        if not self.png_files:
            self.logger.error("No PNG images found in the specified folder.")
            sys.exit(1)

    def process_first_image(self):
        """Process the first image to extract width, height, and color depth."""
        first_image_path = os.path.join(self.input_folder, self.png_files[0])
        with Image.open(first_image_path) as img:
            self.width, self.height = img.size
            self.bytes_per_pixel = len(
                img.getbands()
            )  # Number of color channels (e.g., 3 for RGB)

    def create_header(self, frames):
        """Create the header for the binary file."""
        magic_number = 0x69
        format_version = 0x00
        header_format = "IIIIIII"
        header = struct.pack(
            header_format,
            magic_number,
            format_version,
            self.fps,
            self.bytes_per_pixel,
            self.width,
            self.height,
            frames,
        )
        return header

    def swap_red_blue(self, image):
        """Swap the red and blue channels of the image."""
        # Split the image into individual color channels
        r, g, b = image.split()
        # Merge the channels in the order: Blue, Green, Red
        return Image.merge("RGB", (b, g, r))

    def process_images(self):
        """Process all images and write them to the output file."""
        self.load_images()
        self.process_first_image()
        frames = len(self.png_files)

        with open(self.output_file, "wb") as out_file:
            header = self.create_header(frames)
            out_file.write(header)
            self.logger.info(
                f"Header written: magic=0x69, fps={self.fps}, width={self.width}, height={self.height}, frames={frames}"
            )

            for i, png_file in enumerate(self.png_files):
                image_path = os.path.join(self.input_folder, png_file)
                with Image.open(image_path) as img:
                    img = img.convert("RGB")  # Ensure image is in RGB format
                    img = self.swap_red_blue(img)  # Swap red and blue channels
                    img_data = img.tobytes()
                    out_file.write(img_data)

            self.logger.info(f"Total frames processed: {i + 1}")


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Convert a sequence of PNG images to a binary animation file with swapped red and blue channels."
    )
    parser.add_argument(
        "-i",
        "--input_path",
        required=True,
        help="Path to the folder or .zip file containing PNG images.",
    )
    parser.add_argument(
        "-f",
        "--fps",
        type=int,
        required=True,
        help="Frames per second for the animation.",
    )
    parser.add_argument(
        "-o", "--output_file", required=True, help="Path to the output binary file."
    )
    return parser.parse_args()


def main():
    args = parse_arguments()

    if os.path.isfile(args.input_path):
        workdir = tempfile.TemporaryDirectory()

        zip_file = PyZipFile(args.input_path)
        zip_file.extractall(workdir.name)

        input_folder = os.path.join(workdir.name, os.path.splitext(os.path.basename(args.input_path))[0])

    else:
        input_folder = args.input_path

    try:
        animation = BusyBarAnimation(input_folder, args.fps, args.output_file)
        animation.process_images()

    except FileNotFoundError:
        print(f"Directory \"{input_folder}\" does not exist")
        exit(1)

    except Exception:
        print("Unknown error")
        exit(1)


if __name__ == "__main__":
    main()
