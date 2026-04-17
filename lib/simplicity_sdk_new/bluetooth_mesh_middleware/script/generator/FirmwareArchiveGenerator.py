import textwrap
from elftools.elf.elffile import ELFFile
from pathlib import Path
from typing import Tuple
import argparse
import binascii
import json
import logging
import os
import shutil
import tarfile

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")

class Firmware:
    def __init__(self, fw, metadata, id):
        self.firmware_image_file: str = fw
        self.metadata_file: str = metadata
        self.firmware_id: str = id

class Manifest:
    def __init__(self, fw):
        self.firmware: Firmware = fw

class ManifestHelper:
    def __init__(self, m):
        self.manifest: Manifest = m


# Mapping between ELF and memory:
#             ELF                            device memory
#  -----------------------               ---------------------
# |         header        |             |        FLASH        |
# |  section info, etc.   |             |         ...         |
# |          ...          |    -------->|  0x455e0: var       |
# | section FLASH: 0x4000 |   /         |         ...         |
# | offset: 0x34          |  /          |                     |
# |          ...          | /           |                     |
# | var @ 0x455e0         |-            |                     |
# |          ...          |             |                     |
# |          ...          |             |                     |
# |section RAM: 0x20000000|             |                     |
# | offset: 0x6383c       |             |                     |
# |          ...          |             |                     |
# | var2 & 0x200018c0     |-            |                     |
# |          ...          | \           |                     |
# |                       |  \          |                     |
# |                       |   \         |         ...         |
# |                       |    \        |         RAM         |
# |                       |     ------->|  0x200018c0: var2   |
# |                       |             |         ...         |
# |                       |              ---------------------
# |                       |
# |                       |
# |                       |
#  ----------------------- 
#
# In the ELF file we have a FLASH section, which starts at e.g. 0x4000 on the device.
# We also have an offset, which states the difference between ELF and memory.
# This means that a variable _in the ELF file_ has an offset of e.g. 0x4000-0x34 = 0x3fcc.
#
# So in order to extract this variable _from the ELF file_, we have to subtract this "ELF offset", or out_offset in the code.
# In the example above, he ELF file states FLASH starts @ 0x4000, offset 0x34: the ELF file offset is 0x3fcc.
#
# In short, to extract the variable known to be in flash memory at address 0x455e0, 
# we have to binary read the the ELF file @ 0x455e0-0x3fcc = 0x41614.
#
# The same works for RAM with GCC, but for some reason not with IAR. Additionally,
# IAR mangles data stored in the RAM section, probably to save some bytes if possible.
# In order to reliably extract data from an ELF file, flash memory must be used
#
# To make things worse, IAR likes padding zeros. A string in FLASH memory, e.g.
# "Hello\0" will have st_size=6 on GCC, but could be 10 in IAR: "Hello\0\0\0\0\0"
# This is not a problem with strings, i.e. Firmware ID, but we can not be sure
# that DCD Page 0 won't end in one or more zeros, so we need to extract the
# length variables too (e.g. __mesh_dcd & __mesh_dcd_len).
#
# And in an unexpected turn of events, GCC doesn't want to retain the __pragma((used)) fwid_length
# variable if its stored in flash memory, but does so in RAM. But since in our examples it
# starts with 0xFF02 and ends in a string, we can safely ignore any trailing zero bytes.

def get_data_from_elf(f: Path,
                      section_gcc: str,
                      section_iar: str,
                      symbol_name: str,
                      symbol_len_name: str ) -> bytes:
    logger.debug(f"Parsing {f.name}")
    elffile = ELFFile(f)
    # Get section data
    start = 0
    offset = 0
    for s in elffile.iter_sections():
        if s.name == section_gcc or s.name == section_iar:
            start = s.header['sh_addr']
            offset = s.header['sh_offset']
            logger.debug(f"Section {s.name}")
            break
    # The actual offset present in the .out file
    out_offset = start - offset
    logger.debug(f"Start: {hex(start)}, Offset: {hex(offset)}, Actual Offset in ELF file: {hex(out_offset)}")
    # Get symbol address
    symtab = elffile.get_section_by_name('.symtab')
    symbol_symtab = symtab.get_symbol_by_name(symbol_name)
    symbol = symbol_symtab[0]
    symbol_addr = symbol['st_value']
    logger.debug(f"Symbol: {symbol.name} @ {hex(symbol_addr)}, known size: {symbol['st_size']}")
    # Symbol binary data in the .out file
    symbol_addr = symbol_addr - out_offset
    logger.debug(f"{symbol.name} address: {hex(symbol_addr)}")

    if symbol_len_name == "__mesh_dcd_len":
        # IAR has weird padding, so we need to read the actual length of the symbol
        # Which is stored in another variable
        symbol_len_symtab = symtab.get_symbol_by_name(symbol_len_name)
        symbol_len = symbol_len_symtab[0]
        symbol_len_addr = symbol_len['st_value']
        symbol_len_len = symbol_len['st_size']
        logger.debug(f"{symbol_len.name} size: {symbol_len_len} @ {hex(symbol_len_addr)}")
        # Symbol length binary data
        symbol_len_addr = symbol_len_addr - out_offset
        f.seek(symbol_len_addr)
        len = int.from_bytes(f.read(symbol_len_len), byteorder='little')
    else:
        # With fwid, we don't care about IAR's padding,
        # as the trailing null bytes are stripped later
        len = symbol['st_size']


    logger.debug(f"Reading {len} bytes from {hex(symbol_addr)} in {f.name}")
    f.seek(symbol_addr)

    return f.read(len)

def get_file_name_stem(file_path: str) -> str:
    # Get the file name with extension from path
    file_name = os.path.basename(file_path)
    # Split the file name into name and extension
    file_name_stem, _ = os.path.splitext(file_name)
    return file_name_stem

def metadata_name_from_binary(file_path) -> str:
    return get_file_name_stem(file_path) + "_metadata.bin"

def fw_archive_name_from_binary(file_path) -> str:
    return get_file_name_stem(file_path) + "_firmware.gz"

def find_binary_by_ext(input: Path, output: Path, ext: str) -> Tuple[str, str]:
    in_path = os.path.normpath(os.path.abspath(input))
    out_path = os.path.normpath(os.path.abspath(output))

    for dirpath, _, filenames in os.walk(in_path):
        if in_path != out_path and dirpath == out_path:
            # Skip the output directory while searching for the .gbl file
            # which is essential when the output directory is proper subset
            # of the input directory.
            # If the input and output directory are the same then it shall
            # not be skipped.
            continue
        for filename in filenames:
            if filename.endswith(ext):
                return dirpath, filename
    return None, None

def generate_metadata(args, elf_path) -> str:
    logger.info("Generating DFU metadata...")
    output = args.output_dir
    metadata = None
    if not args.metadata:
        with open(elf_path, 'rb') as f:
            # Binary content of __mesh_dcd as DFU metadata file
            metadata = get_data_from_elf(f, '.text', 'application', '__mesh_dcd', '__mesh_dcd_len')
    else:
        metadata = binascii.unhexlify(args.metadata)

    if not metadata:
        logger.info("Error: could not generate metadata")
        exit(1)

    metadata_name = metadata_name_from_binary(elf_path)
    # Write the metadata to a file
    metadata_file = os.path.join(output, metadata_name)
    with open(metadata_file, "wb") as f:
        f.write(metadata)
    logger.info("Metadata generated successfully")
    logger.debug(f"{metadata}")

def generate_fwid(elf_path) -> str:
    logger.info("Parsing Firmware ID...")
    with open(elf_path, 'rb') as f:
        # firmware_id in the manifest file
        fwid_str = get_data_from_elf(f, '.text', 'application', 'fwid', 'fwid_length')

        # Strip trailing zero byte and convert to ASCII
        fwid_str = fwid_str.rstrip(b'\00')
        fwid_str = fwid_str.hex()

    if not fwid_str:
        logger.info("Error: could not retrieve firmware ID")
        exit(1)

    logger.info("Firmware ID parsed successfully")
    logger.debug(f"{fwid_str}")

    return fwid_str

def generate_manifest(args, fwid) -> str:
    # Generate the DFU manifest file as in Mesh DFU Model 1.0
    # manifest format:
    # {
    # "manifest": {
    #     "firmware": {
    #         "firmware_image_file": "firmware.bin",
    #         "metadata_file": "metadata.bin",
    #         "firmware_id": "010246573A312E332E35"
    #     }
    # }
    logger.info("Generating manifest...")
    input = args.input_dir
    output = args.output_dir
    root, filename = find_binary_by_ext(input, output, ".gbl")
    if not filename:
        logger.info("Error: No .gbl file found")
        exit(1)
    gbl_path = os.path.normpath(os.path.join(root, filename))
    target_gbl_path = os.path.normpath(os.path.join(output, filename))
    if gbl_path != target_gbl_path:
        # If the input and output directories are the same and the gbl file is
        # in the output directory directly, then it shall not be copied
        shutil.copy(gbl_path, output)
    metadata_name = metadata_name_from_binary(filename)
    manifest = ManifestHelper(Manifest(Firmware(filename, metadata_name, fwid)))
    m = json.dumps(manifest, default=lambda o: o.__dict__, indent=2)
    manifest_file = os.path.join(output, "manifest.json")
    with open(manifest_file, "w") as f:
        f.write(m)
    logger.info("Manifest generated successfully")
    return filename

def generate_archive(output, gbl):
    metadata_name = metadata_name_from_binary(gbl)
    fw_archive_name = fw_archive_name_from_binary(gbl)
    filename = os.path.join(output, fw_archive_name)
    with tarfile.open(filename, "w:gz") as f:
        f.add(os.path.join(output, "manifest.json"), arcname="manifest.json", recursive=False)
        f.add(os.path.join(output, metadata_name), arcname=metadata_name, recursive=False)
        f.add(os.path.join(output, gbl), arcname=gbl, recursive=False)
    logger.info("Archive generated successfully at " + filename)

if __name__ == "__main__":
    cwd = Path.cwd()
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent('''\
        BT Mesh DFU Metadata Generator

        Generate DFU Metadata as per Bluetooth Mesh Device Firmware Update Model Specification v1.0
        ''')
    )
    parser.add_argument("-i", "--input", dest='input_dir', nargs="?", default=cwd, type=Path, required=True, help="Path to the project root where the firmware is built.")
    parser.add_argument("-o", "--output", dest='output_dir', nargs="?", default=None, type=Path, help="Output directory for the DFU archive. Default is INPUT_DIR/dfu.")
    parser.add_argument("-m", "--metadata", dest='metadata', nargs="?", default=None, type=str, help="Metadata (hexadecimal string, converted to binary) to use in the manifest file. Default is the binary data of DCD Page 0, parsed from the firmware ELF file.")
    parser.add_argument("-f", "--fwid", dest='fwid', nargs="?", default=None, type=str, help="Firmware ID (string) to use in the manifest file. Default is parsed from the firmware ELF file.")
    parser.add_argument("-d", "--debug", dest='debug', action="store_true", help="Enable debug logging.")
    args = parser.parse_args()

    if args.debug:
        logger.info("Debug logging enabled")
        logger.setLevel(logging.DEBUG)

    logger.info(f"Firmware archive generator: {cwd}")
    # Default output dir is <input>/dfu
    if not args.output_dir:
        args.output_dir = os.path.join(args.input_dir, "dfu")

    if not os.path.exists(args.output_dir):
        os.makedirs(args.output_dir)

    # Find the .out file in the generated folder
    root, filename = find_binary_by_ext(args.input_dir, args.output_dir, ".out")
    if not filename:
        logger.info("Error: No .out file found")
        exit(1)
    elf_path = os.path.join(root, filename)

    generate_metadata(args, elf_path)
    if not args.fwid:
        fwid = generate_fwid(elf_path)
    else:
        fwid = args.fwid
    gbl = generate_manifest(args, fwid)
    generate_archive(args.output_dir, gbl)