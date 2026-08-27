#!/usr/bin/env python3

"""
Converts a sequence of bitmap images into BUSY Bar's custom animation format.
"""

import struct
import json
from io import BufferedWriter
from pathlib import Path
from tempfile import TemporaryDirectory
from PIL import Image
from zipfile import PyZipFile
from typing import Tuple
from dataclasses import dataclass
from enum import Enum
from collections import Counter

from flipper.app import App
from flipper import rle

def number_in_str(input: str) -> int:
    return int("".join(filter(str.isdigit, input))) or 0

@dataclass
class Header:
    FORMAT = "<8s BBBB HHBB II II"
    SIGNATURE = b"bicycle1" # Busybar Image Container speciallY Crafted for file Length Eradication, major ver. 1
    flags: int
    width: int
    height: int
    color_mode: int
    max_mask_len: int
    max_pixel_len: int
    fps: int
    sections_chunk_len: int
    frames_chunk_len: int
    section_count: int
    frame_count: int

    @staticmethod
    def length() -> int:
        return struct.calcsize(Header.FORMAT)
    
    def to_bytes(self) -> bytes:
        return struct.pack(
            self.FORMAT,
            self.SIGNATURE,

            self.flags,
            self.width,
            self.height,
            self.color_mode,

            self.max_mask_len,
            self.max_pixel_len,
            self.fps,
            0,

            self.sections_chunk_len,
            self.frames_chunk_len,

            self.section_count,
            self.frame_count,
        )

@dataclass
class Section:
    FORMAT = "<III"
    start: int
    end: int
    frame_offs: int
    name: str

    def length(self) -> int:
        return struct.calcsize(self.FORMAT) + len(self.name) + 1

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.FORMAT,
            self.start,
            self.end,
            self.frame_offs,
        ) + bytes(self.name, "utf8") + bytes([0])

class MaskEncoding(Enum):
    FULLY_BLACK = 0
    FULLY_WHITE = 1
    RLE_FIRST_BLACK = 2
    RLE_FIRST_WHITE = 3
    BITMAP = 4

class PixelEncoding(Enum):
    RAW = 0
    RLE = 1

run_lengths_stats = Counter()

class BitQueue:
    def __init__(self):
        self.queue = 0
        self.queue_len = 0
        self.bytes = bytearray()
        self.total_bits = 0
        self.debug = ""

    def push_bits(self, bits: int, count: int):
        bits &= (1 << count) - 1
        self.debug += f"{bits:0{count}b} "
        self.queue <<= count
        self.queue |= bits
        self.queue_len += count
        self.total_bits += count

        while self.queue_len >= 8:
            low_bit = self.queue_len - 8
            to_write = self.queue >> low_bit
            self.queue_len -= 8
            self.queue &= ((1 << self.queue_len) - 1)
            self.bytes.append(to_write)

    def serialize(self) -> Tuple[bytes, int]:
        if self.queue_len > 0:
            pad = 8 - self.queue_len
            self.bytes.append(self.queue << pad)
            self.queue_len = 0
            self.queue = 0
        return (bytes(self.bytes), self.total_bits)

@dataclass
class Frame:
    FORMAT = "<BHH"
    mask_encoding: MaskEncoding
    mask: Tuple[bytes, int]
    px_encoding: PixelEncoding
    pixels: bytes

    def length(self) -> int:
        return struct.calcsize(self.FORMAT) + len(self.mask[0]) + len(self.pixels)

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.FORMAT,
            self.px_encoding.value | (self.mask_encoding.value << 4),
            self.mask[1],
            len(self.pixels),
        ) + self.mask[0] + self.pixels

    @staticmethod
    def reduce_color(pixels: bytes, mode: str) -> bytes:
        reduced = bytearray()
        
        if mode == "rgb888":
            for i in range(0, len(pixels), 4):
                reduced.extend(pixels[i : i + 3])
                reduced.append(0xff)
        
        elif mode == "gray4":
            for i in range(0, len(pixels), 4):
                reduced.extend([pixels[i] & 0xf0] * 3)
                reduced.append(0xff)
        
        elif mode == "argb8888":
            return pixels
        
        else:
            raise NotImplemented
        
        return bytes(reduced)

    @staticmethod
    def subtract(new_pixels: bytes, old_pixels: bytes | None) -> Tuple[bytes, list[bool]]:
        px_cnt = len(new_pixels) // 4
        if not old_pixels:
            return (new_pixels, [True] * px_cnt)

        pixels = bytearray()
        mask = [False] * px_cnt

        assert len(new_pixels) == len(old_pixels)
        for i in range(0, len(new_pixels), 4):
            new_px = new_pixels[i : i + 4]
            old_px = old_pixels[i : i + 4]
            if new_px != old_px:
                pixels.extend(new_px)
                mask[i // 4] = True

        return (bytes(pixels), mask)

    @staticmethod
    def pack_pixels(pixels: bytes, mode: str) -> bytes:
        packed = bytearray()

        if mode == "rgb888":
            # actually BGR888
            for i in range(0, len(pixels), 4):
                packed.extend([pixels[i + 2], pixels[i + 1], pixels[i + 0]])
        
        elif mode == "gray4":
            for i in range(0, len(pixels), 8):
                px1 = pixels[i + 0] & 0xF0
                if i + 4 >= len(pixels):
                    px2 = 0
                else:
                    px2 = pixels[i + 4] & 0xF0
                packed.append(px1 | (px2 >> 4))
        
        elif mode == "argb8888":
            # actually BGRA8888
            for i in range(0, len(pixels), 4):
                packed.extend([pixels[i + 2], pixels[i + 1], pixels[i + 0], pixels[i + 3]])
        
        else:
            raise NotImplemented
        
        return bytes(packed)
        
    @staticmethod
    def encode_pixels(pixels: bytes, mode: str) -> Tuple[bytes, PixelEncoding]:
        packed = pixels
        blk_size = {"rgb888": 3, "gray4": 1, "argb8888": 4}[mode]
        rle_encoded = rle.compress(pixels, blk_size)

        if len(rle_encoded) < len(packed):
            return (rle_encoded, PixelEncoding.RLE)
        else:
            return (packed, PixelEncoding.RAW)

    @staticmethod
    def encode_mask(mask: list[bool]) -> Tuple[Tuple[bytes, int], MaskEncoding]:
        first_pixel = mask[0]

        run_length = 0
        run_value = first_pixel
        run_lengths = []

        for px in mask:
            if px == run_value:
                run_length += 1
            else:
                run_lengths.append(run_length)
                run_value = px
                run_length = 1
        run_lengths.append(run_length)

        run_lengths_stats.update(run_lengths)

        if len(run_lengths) == 1:
            encoding = MaskEncoding.FULLY_WHITE if first_pixel else MaskEncoding.FULLY_BLACK
            return ((bytes(), 0), encoding)

        bit_q = BitQueue()

        SHORT_RUN_BITS = 3
        LONG_RUN_BITS = 8
        LONG_RUN_THRESHOLD = (1 << SHORT_RUN_BITS) - 1
        LONG_RUN_MAX = (1 << LONG_RUN_BITS) - 1
        LONG_RUN_MARKER = LONG_RUN_THRESHOLD

        for length in run_lengths:
            if length >= LONG_RUN_THRESHOLD:
                while length >= LONG_RUN_MAX:
                    bit_q.push_bits(LONG_RUN_MARKER, SHORT_RUN_BITS)
                    bit_q.push_bits(LONG_RUN_MAX, LONG_RUN_BITS)
                    length -= LONG_RUN_MAX
                    bit_q.push_bits(0, SHORT_RUN_BITS)
                bit_q.push_bits(LONG_RUN_MARKER, SHORT_RUN_BITS)
                bit_q.push_bits(length, LONG_RUN_BITS)
            else:
                bit_q.push_bits(length, SHORT_RUN_BITS)

        compressed, compressed_bits = bit_q.serialize()

        COMPRESSION_REDUCTION_THRESHOLD = 0.75

        if compressed_bits <= len(mask) * COMPRESSION_REDUCTION_THRESHOLD:
            encoding = MaskEncoding.RLE_FIRST_WHITE if first_pixel else MaskEncoding.RLE_FIRST_BLACK
            return ((compressed, compressed_bits), encoding)

        else:
            bitmap = bytearray()
            for i in range(0, len(mask), 8):
                byte = 0
                for j in range(min(8, len(mask) - i)):
                    byte |= (128 >> j) if mask[i + j] else 0
                bitmap.append(byte)
            return ((bytes(bitmap), len(mask)), MaskEncoding.BITMAP)

@dataclass
class ConversionInfo:
    frame_cnt: int
    overall_compression_ratio: float
    percent_pixels_eliminated: float
    percent_mask_overhead: float

class ConversionError(Exception):
    pass

class BSBAnimConverter:
    def _do_convert(self, meta: dict, frames: list[Path], output: BufferedWriter) -> ConversionInfo:
        # 1. validate section metadata
        sections = [{"name": "default", "start": 0, "end": len(frames) - 1}] + meta["sections"]
        for i, section in enumerate(sections):
            if set(section.keys()) != {"name", "start", "end"}:
                raise ConversionError(f"Invalid metadata: 'sections' children must only have 'name', 'start' and 'end' fields")
            if section["start"] < 0:
                raise ConversionError(f"Invalid metadata: section '{section['name']}' has start < 0")
            if section["end"] >= len(frames):
                raise ConversionError(f"Invalid metadata: section '{section['name']}' has end past the last frame")
            if section["start"] > section["end"]:
                raise ConversionError(f"Invalid metadata: section '{section['name']}' has start > end")
            if i > 0 and section["name"] == "default":
                raise ConversionError(f"Invalid metadata: section name \"default\" is reserved")
        section_starts = set(section["start"] for section in sections)

        # 2. encode frames
        size: None | Tuple[int, int] = None
        encoded_frames: list[Frame] = []
        frames_chunk_len = 0
        max_pixels_len = 0
        max_mask_len = 0
        last_pixels = None

        input_pixel_cnt = 0
        kept_pixel_cnt = 0
        mask_size_sum = 0

        for i, frame in enumerate(frames):
            with Image.open(frame) as frame:
                frame = frame.convert("RGBA")
                if size and frame.size != size:
                    raise ConversionError(f"frame {i} has a different size than previous frames")
                size = frame.size

                raw_pixels = frame.tobytes()
                raw_pixels = Frame.reduce_color(raw_pixels, meta["color_mode"])

                must_be_keyframe = i in section_starts
                pixels, mask = Frame.subtract(raw_pixels, None if must_be_keyframe else last_pixels)

                input_pixel_cnt += len(raw_pixels) // 4
                kept_pixel_cnt += mask.count(True)

                pixels = Frame.pack_pixels(pixels, meta["color_mode"])
                pixels, pixel_encoding = Frame.encode_pixels(pixels, meta["color_mode"])
                mask, mask_encoding = Frame.encode_mask(mask)

                mask_size_sum += len(mask[0])

                frame = Frame(
                    mask_encoding,
                    mask,
                    pixel_encoding,
                    pixels
                )
                encoded_frames.append(frame)
                frames_chunk_len += frame.length()
                max_pixels_len = max(max_pixels_len, len(pixels))
                max_mask_len = max(max_mask_len, mask[1])

                last_pixels = raw_pixels

        # 3. encode sections
        encoded_sections: list[Section] = []
        sections_chunk_len = 0
        for section in sections:
            section = Section(
                start=section["start"],
                end=section["end"],
                name=section["name"],
                # precomputed start info to be filled later
                frame_offs=0,
            )
            encoded_sections.append(section)
            sections_chunk_len += section.length()

        # 4. fill section precomputed start info, now that file offsets are known
        frame_offsets: list[int] = []
        file_frame_offs = Header.length() + sections_chunk_len
        for file_frame in encoded_frames:
            frame_offsets.append(file_frame_offs)
            file_frame_offs += file_frame.length()
        
        for section in encoded_sections:
            section.frame_offs = frame_offsets[section.start]

        # 5. assemble header and write data
        assert size
        width, height = size
        color_fmt_map = {"rgb888": 0, "gray4": 1, "argb8888": 2}
        header = Header(
            flags=0,
            width=width,
            height=height,
            color_mode=color_fmt_map[meta["color_mode"]],
            fps=meta["fps"],
            max_pixel_len=max_pixels_len,
            max_mask_len=max_mask_len,
            sections_chunk_len=sections_chunk_len,
            frames_chunk_len=frames_chunk_len,
            section_count=len(encoded_sections),
            frame_count=len(encoded_frames),
        )
        output.write(header.to_bytes())
        for section in encoded_sections:
            output.write(section.to_bytes())
        for frame in encoded_frames:
            output.write(frame.to_bytes())

        # 5. assemble info about file
        file_size = output.tell()
        color_size = {"rgb888": 3, "gray4": 0.5, "argb8888": 4}[meta["color_mode"]]
        compression_ratio = (len(frames) * width * height * color_size) / file_size
        return ConversionInfo(
            len(encoded_frames),
            compression_ratio,
            100 * (1 - (kept_pixel_cnt / input_pixel_cnt)),
            100 * mask_size_sum / file_size,
        )

    def convert_dir(self, input: Path, output: Path) -> ConversionInfo:
        meta: dict = json.loads((input / "meta.json").read_text())

        frames = list(input.glob("*.png"))
        frames.sort(key=lambda x: number_in_str(x.stem))

        for i in range(len(frames)):
            if number_in_str(frames[i].stem) != i:
                raise ConversionError(f"Invalid frame numbering: missing *{i}.png")
            
        if "fps" not in meta:
            raise ConversionError(f"Invalid meta.json: must have 'fps'")
        if "color_mode" not in meta:
            raise ConversionError(f"Invalid meta.json: must have 'color_mode' ('argb8888', 'rgb888' or 'gray4')")
        if meta["color_mode"] not in ["argb8888", "rgb888", "gray4"]:
            raise ConversionError(f"Invalid meta.json: 'color_mode' must be one of: 'argb8888', 'rgb888' or 'gray4'")
        if "sections" not in meta:
            raise ConversionError(f"Invalid meta.json: must have 'sections' (even an empty array is fine)")
        
        with open(output, "wb") as output_writer:
            return self._do_convert(meta, frames, output_writer)

    def convert_zip(self, input: Path, output: Path) -> ConversionInfo:
        with TemporaryDirectory(prefix="bsb-seq2anim-") as temp_dir:
            input_zip = PyZipFile(str(input))
            input_zip.extractall(temp_dir)
            return self.convert_dir(Path(temp_dir) / input.stem, output)

class Main(App):
    def init(self):
        self.parser.add_argument("-o", "--output", required=True, type=Path, help="Output .anim file")
        self.parser.add_argument("input", type=Path, help="Input .zip file")
        self.parser.set_defaults(func=self.main)

    def main(self):
        args = self.args
        converter = BSBAnimConverter()
        try:
            info = converter.convert_zip(args.input, args.output)
            self.logger.debug(info)
            return 0
        except ConversionError as e:
            self.logger.error(f"Failed to convert {args.input}: {str(e)}")
            return 1

if __name__ == "__main__":
    Main()()
