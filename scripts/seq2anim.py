#!/usr/bin/env python3

import struct
import json
from io import BufferedWriter
from pathlib import Path
from tempfile import TemporaryDirectory
from PIL import Image
from zipfile import PyZipFile
from typing import Tuple, Self
from dataclasses import dataclass
from logging import Logger

from flipper.app import App
from flipper import rle

def number_in_str(input: str) -> int:
    return int("".join(filter(str.isdigit, input))) or 0

@dataclass
class Header:
    FORMAT = "<8s BBBB BHB II"
    flags: int
    width: int
    height: int
    color_format: int
    fps: int
    max_encoded_len: int
    sections_chunk_len: int
    frames_chunk_len: int

    @staticmethod
    def length() -> int:
        return struct.calcsize(Header.FORMAT)
    
    def to_bytes(self) -> bytes:
        return struct.pack(
            self.FORMAT,
            b"bicycle0",

            self.flags,
            self.width,
            self.height,
            self.color_format,

            self.fps,
            self.max_encoded_len,
            0,

            self.sections_chunk_len,
            self.frames_chunk_len,
        )

@dataclass
class Section:
    FORMAT = "<IIIB"
    start: int
    end: int
    frame_offs: int
    duration_override: int
    name: str

    def length(self) -> int:
        return struct.calcsize(self.FORMAT) + len(self.name) + 1

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.FORMAT,
            self.start,
            self.end,
            self.frame_offs,
            self.duration_override,
        ) + bytes(self.name, "utf8") + bytes([0])

@dataclass
class FileFrame:
    FORMAT = "<BBH"
    encoding: int
    duration: int
    encoded: bytes

    def length(self) -> int:
        return struct.calcsize(self.FORMAT) + len(self.encoded)

    def to_bytes(self) -> bytes:
        return struct.pack(
            self.FORMAT,
            self.encoding,
            self.duration,
            len(self.encoded),
        ) + self.encoded

    @staticmethod
    def pack(frame: bytes, mode: str) -> bytes:
        packed = bytearray()

        if mode == "rgb888":
            for i in range(0, len(frame), 3):
                packed.extend([frame[i + 2], frame[i + 1], frame[i]])
        
        elif mode == "gray4":
            for i in range(0, len(frame), 6):
                px1 = frame[i] & 0xF0
                px2 = frame[i + 3] & 0xF0
                packed.append(px1 | (px2 >> 4))
        
        else:
            raise NotImplemented
        
        return bytes(packed)
        
    @staticmethod
    def encode(frame: bytes, mode: str) -> Self:
        raw = frame
        blk_size = 3 if mode == "rgb888" else 1
        rle_encoded = rle.compress(frame, blk_size)

        if len(rle_encoded) < len(raw):
            return FileFrame(encoding=1, duration=1, encoded=rle_encoded)
        else:
            return FileFrame(encoding=0, duration=1, encoded=raw)

@dataclass
class ConversionInfo:
    display_frame_cnt: int
    file_frame_cnt: int
    max_encoded_len: int
    raw_len: int
    mean_compression_ratio: float

class ConversionError(Exception):
    pass

class BSBAnimConverter:
    def _do_convert(self, meta: dict, frames: list[Path], output: BufferedWriter) -> ConversionInfo:
        # 1. encode frames
        size: None | Tuple[int, int] = None
        encoded_frames: list[FileFrame] = []
        frames_chunk_len = 0
        max_encoded_len = 0
        last_frame = None

        for i, frame in enumerate(frames):
            with Image.open(frame) as frame:
                frame = frame.convert("RGB")
                if size and frame.size != size:
                    raise ConversionError(f"frame {i} has a different size than previous frames")
                size = frame.size

                frame = frame.tobytes()
                if frame == last_frame:
                    encoded_frames[-1].duration += 1
                    continue

                last_frame = frame
                frame = FileFrame.pack(frame, meta["color"])
                frame = FileFrame.encode(frame, meta["color"])

                encoded_frames.append(frame)
                frames_chunk_len += frame.length()
                max_encoded_len = max(max_encoded_len, len(frame.encoded))

        # 2. encode sections
        encoded_sections: list[Section] = []
        sections_chunk_len = 0
        for section in meta["sections"]:
            if set(section.keys()) != {"name", "start", "end"}:
                raise ConversionError(f"Invalid metadata: 'sections' children must only have 'name', 'start' and 'end' fields")

            section = Section(
                start=section["start"],
                end=section["end"],
                name=section["name"],
                # precomputed start info to be filled later
                frame_offs=0,
                duration_override=0,
            )
            encoded_sections.append(section)
            sections_chunk_len += section.length()

        # 3. fill section precomputed start info, now that file offsets are known
        display_frame_start: list[Tuple[int, int]] = []
        file_frame_offs = Header.length() + sections_chunk_len
        disp_frame_idx = 0
        for file_frame in encoded_frames:
            for disp_offset in range(file_frame.duration, 0, -1):
                display_frame_start.append((file_frame_offs, disp_offset))
            disp_frame_idx += file_frame.duration
            file_frame_offs += file_frame.length()
        
        for section in encoded_sections:
            section.frame_offs, section.duration_override = display_frame_start[section.start]

        # 4. assemble header and write data
        assert size
        width, height = size
        color_fmt_map = {"rgb888": 0, "gray4": 1}
        header = Header(
            flags=0,
            width=width,
            height=height,
            color_format=color_fmt_map[meta["color"]],
            fps=meta["fps"],
            max_encoded_len=max_encoded_len,
            sections_chunk_len=sections_chunk_len,
            frames_chunk_len=frames_chunk_len,
        )
        output.write(header.to_bytes())
        for section in encoded_sections:
            output.write(section.to_bytes())
        for frame in encoded_frames:
            output.write(frame.to_bytes())

        # print info about file
        compression_ratio = (len(frames) * width * height * 3) / frames_chunk_len
        return ConversionInfo(
            disp_frame_idx,
            len(encoded_frames),
            max_encoded_len,
            width * height * 3,
            compression_ratio
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
        if "color" not in meta:
            raise ConversionError(f"Invalid meta.json: must have 'color'")
        if meta["color"] not in ["rgb888", "gray4"]:
            raise ConversionError(f"Invalid meta.json: 'color' must be one of: 'rgb888', 'gray4'")
        if "sections" not in meta:
            raise ConversionError(f"Invalid meta.json: must have 'sections'")
        if not meta["sections"]:
            raise ConversionError(f"Invalid meta.json: must have 'sections[0]'")
        if meta["sections"][0] != {"name": "whole", "start": 0, "end": len(frames) - 1}:
            raise ConversionError(f"Invalid meta.json: 'sections[0]' must be named 'whole' and cover entire range of frames")
        
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
            self.logger.info(info)
            return 0
        except ConversionError as e:
            self.logger.error(str(e))
            return 1

if __name__ == "__main__":
    Main()()
