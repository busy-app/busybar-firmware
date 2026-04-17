#!/usr/bin/env python3
import esl_lib_wrapper as elw


class AirCompressor:
    """A Lempel-Ziv family lossless compression class for low colour images typically used in Silabs' ESLs"""

    def save_raw_compressed(filename: str, data: bytearray):
        out = open(outname + ".air", "wb")
        out.write(data)
        out.close()

    def shrink(src: bytearray) -> bytearray:
        """
        Expect Silabs' ESL image source as bytearray, return with the compressed bytearray.
        The compressor configuration must be the same as that of the ESL Tag image decompressor, so it cannot be changed in real time.
        Typically gives 90-16% compression ratio on common ESL images consisting of text, barcodes and smaller images with few clean colors.
        In the worst case, however, the output can be up to 112.5% of the size of the input, e.g. for random data and already compressed images.
        """

        dst = bytearray()
        hash_table = [0xFF] * elw.OTF_CFG_HASH_WINDOW_SIZE
        pattern_mask = elw.OTF_CFG_PATTERN_MASK_INIT
        read_offset = 0
        match_range = range(elw.OTF_CFG_PEER_MIN, elw.OTF_CFG_PEER_MAX + 1)
        while read_offset < len(src):
            pattern_mask <<= 1
            if pattern_mask >= elw.OTF_CFG_PATTERN_MASK_LIMIT:
                pattern_mask = 1
                pattern_map_index = len(dst)
                dst.append(0)
            if read_offset > len(src) - elw.OTF_CFG_PEER_MAX:
                dst.append(src[read_offset])
                read_offset += 1
                continue
            src24 = src[read_offset : read_offset + 3]
            tbl_ix = int.from_bytes(src24, byteorder="little")
            tbl_ix += tbl_ix >> 11
            tbl_ix += tbl_ix >> 7
            tbl_ix &= elw.OTF_CFG_HASH_WINDOW_SIZE - 1
            offset = (read_offset - hash_table[tbl_ix]) & elw.OTF_CFG_OFFSET_MASK
            hash_table[tbl_ix] = read_offset
            pattern_source = read_offset - offset
            if (
                pattern_source >= 0
                and pattern_source != read_offset
                and src24 == src[pattern_source : pattern_source + 3]
            ):
                dst[pattern_map_index] |= pattern_mask
                # To prevent buffer overrun, shorten the range if we're close to the buffer's end - otherwise do it normally.
                for pattern_length in (
                    match_range
                    if read_offset + match_range[-1] < len(src)
                    else range(elw.OTF_CFG_PEER_MIN, len(src) - read_offset)
                ):
                    if (
                        src[read_offset + pattern_length]
                        != src[pattern_source + pattern_length]
                    ):
                        break
                dst.append(
                    (
                        (pattern_length - elw.OTF_CFG_PEER_MIN)
                        << elw.OTF_CFG_PATTERN_SHIFT_COUNT
                    )
                    | (offset >> elw.OTF_CFG_BYTE_WIDTH)
                )
                dst.append(offset & 0xFF)
                read_offset += pattern_length
            else:
                dst.append(src[read_offset])
                read_offset += 1
        return dst

    def swell(src: bytearray) -> bytearray:
        """
        Expect Silabs' ESL compressed image source as bytearray, return with the decompressed bytearray.

        The input stream must have the same compression configuration as the compressor used - it cannot decompress a stream from another source because it is a raw stream without configuration metadata.
        """

        dst = bytearray()
        read_offset = 0
        pattern_mask = elw.OTF_CFG_PATTERN_MASK_INIT
        while read_offset < len(src):
            pattern_mask <<= 1
            if pattern_mask >= elw.OTF_CFG_PATTERN_MASK_LIMIT:
                pattern_mask = 1
                pattern_map_index = src[read_offset]
                read_offset += 1
            if pattern_map_index & pattern_mask:
                pattern_length = (
                    src[read_offset] >> elw.OTF_CFG_PATTERN_SHIFT_COUNT
                ) + elw.OTF_CFG_PEER_MIN
                offset = (
                    (src[read_offset] << elw.OTF_CFG_BYTE_WIDTH) | src[read_offset + 1]
                ) & elw.OTF_CFG_OFFSET_MASK
                read_offset += 2
                pattern_source = len(dst) - offset
                if pattern_source < 0:
                    return None
                while pattern_length > 0:
                    dst.append(dst[pattern_source])
                    pattern_source += 1
                    pattern_length -= 1
            else:
                dst.append(src[read_offset])
                read_offset += 1
        return dst


# Below code is only for testing purposes, not needed by the compressor/decompressor routines:
def main():
    """
    Compress or extract file for testing purposes.

    Use to compress or decompress one or more input files, primarily for benchmarking and testing purposes. If you specify a single input file, you can save the result to another file.
    """
    import os
    import sys
    import glob
    import time
    import argparse

    parser = argparse.ArgumentParser(description=main.__doc__)
    parser.add_argument(
        "input",
        metavar="INPUT",
        nargs="+",
        help="Input file(s) to compress (default action) or decompress.",
    )
    parser.add_argument(
        "-o",
        "--out",
        metavar="FILE",
        type=argparse.FileType("w"),
        help="Set an output file to save to. Ignored if there are multiple input files.",
    )
    parser.add_argument(
        "-x",
        "--extract",
        action="store_true",
        help="Change the action to decompression instead of compression.",
    )
    parser.add_argument(
        "-q",
        "--quiet",
        action="store_true",
        help="Silent operation without messages. Errors are still displayed if they occur.",
    )
    args = parser.parse_args()

    def compress(data, quiet):
        if len(data) == 0:
            return None

        t0 = time.perf_counter()
        compr = AirCompressor.shrink(data)
        elapsed = time.perf_counter() - t0
        rate = len(data) / (1024 * 1024 * elapsed)
        if not quiet:
            print(
                " Compressed to %u bytes, %.2f%% in %.3f ms [%.2f MB/s]"
                % (len(compr), 100.0 * len(compr) / len(data), elapsed * 1000, rate)
            )
        decompr = AirCompressor.swell(compr)
        if decompr != data:
            print("*** Decompression failed!")
        return compr

    def decompress(data, quiet):
        if len(data) == 0:
            return None

        t0 = time.perf_counter()
        decompr = AirCompressor.swell(data)
        elapsed = time.perf_counter() - t0
        rate = len(data) / (1024 * 1024 * elapsed)
        if not quiet and decompr is not None:
            print(
                " Decompressed to %u bytes, %.2f%% in %.3f ms [%.2f MB/s]"
                % (len(decompr), 100.0 * len(decompr) / len(data), elapsed * 1000, rate)
            )
        return decompr

    def save(filename, data, quiet):
        if not quiet:
            print(f"Saving to {filename}")
        out = open(filename, "wb")
        out.write(data)
        out.close()

    files = []
    for a in args.input:
        files.extend(glob.glob(a))
    files = [f for f in files if os.path.isfile(f)]

    for file in files:
        try:
            inf = open(file, "rb")
            data = inf.read()
            inf.close()
            data_in = bytearray(data)
        except:
            print("*** Failed to open '%s'!" % file)
            exit()
        if not args.quiet:
            print("Loaded %u bytes from '%s'" % (len(data_in), file))
        if not args.extract:
            data_out = compress(data_in, args.quiet)
        else:
            data_out = decompress(data_in, args.quiet)
            if not data_out:
                print("*** Failed to decompress '%s'!" % file)
    if args.out:
        if not data_out:
            print("*** Nothing to save to '%s', aborted!" % args.out.name)
            exit()
        elif len(files) > 1:
            print(
                "*** Ignore saving because multiple input files were specified - an empty output file was created."
            )
            exit()
        try:
            save(args.out.name, data_out, args.quiet)
        except:
            print("*** Failed to save '%s'4" % args.out.name)
            exit()


if __name__ == "__main__":
    main()
