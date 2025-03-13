#!/usr/bin/env python3

import os
import sys
import struct
import argparse
from PIL import Image


class BusyBarAnimation:
    def __init__(self, input_folder, fps, output_file):
        self.input_folder = input_folder
        self.fps = fps
        self.output_file = output_file
        self.png_files = []
        self.width = 72  # default display width
        self.height = 16  # default display height
        self.bytes_per_pixel = 3  # RGB format

    def load_images(self):
        """Load and sort PNG images from the input folder."""
        self.png_files = [
            f for f in os.listdir(self.input_folder) if f.lower().endswith(".png")
        ]
        self.png_files.sort()  # Ensure files are processed in alphanumeric order

        if not self.png_files:
            print("No PNG images found in the specified folder.")
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
        header_format = "IIIIII"
        header = struct.pack(
            header_format,
            magic_number,
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
            print(
                f"Header written: magic=0x69, fps={self.fps}, width={self.width}, height={self.height}, frames={frames}"
            )

            for i, png_file in enumerate(self.png_files):
                image_path = os.path.join(self.input_folder, png_file)
                with Image.open(image_path) as img:
                    img = img.convert("RGB")  # Ensure image is in RGB format
                    img = self.swap_red_blue(img)  # Swap red and blue channels
                    img_data = img.tobytes()
                    out_file.write(img_data)
                    print(f"Processed frame {i + 1}/{frames}: {png_file}")


def parse_arguments():
    parser = argparse.ArgumentParser(
        description="Convert a sequence of PNG images to a binary animation file with swapped red and blue channels."
    )
    parser.add_argument(
        "-i",
        "--input_folder",
        required=True,
        help="Path to the folder containing PNG images.",
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
    animation = BusyBarAnimation(args.input_folder, args.fps, args.output_file)
    animation.process_images()


if __name__ == "__main__":
    main()
