#!/usr/bin/env python3

import re
from dataclasses import dataclass
from typing import List, Optional, Tuple

from flipper.app import App, CatchExceptions
from flipper.cli import Cli


def _auto_int(x):
    return int(x, 0)


class CryptoStorage(Cli):
    CRYPTO_CMD = "crypto"

    def __init__(self, portname: tuple[str, int]):
        Cli.__init__(self, portname)

    def __enter__(self):
        Cli.__enter__(self)
        self.send_and_wait_prompt("sl_cli\r")
        # Initialize NWP crypto subsystem (required on older firmware)
        self.send_and_wait_prompt(f"{self.CRYPTO_CMD} init\r")
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.send_and_wait_prompt("exit\r")
        Cli.__exit__(self, exc_type, exc_value, traceback)

    @dataclass(frozen=True)
    class KeyEntry:
        partition: int
        key_type: int
        key_id: int

    def _fetch_partition_listing(self, partition: int) -> Tuple[str, int]:
        data = self.send_and_wait_prompt(f"{self.CRYPTO_CMD} list {partition}\r")
        return self._parse_response(data)

    def list_partition(self, partition: int):
        listing, ret = self._fetch_partition_listing(partition)
        print(listing)
        return listing, ret

    def enumerate_keys(
        self, partition: int, *, echo: bool = True
    ) -> Tuple[List["CryptoStorage.KeyEntry"], str, int]:
        listing, ret = self._fetch_partition_listing(partition)
        if echo:
            print(listing)
        return self._parse_key_listing(listing), listing, ret

    def ensure_key_absent(
        self,
        partition: int,
        key_type: int,
        key_id: int,
        *,
        echo: bool = True,
        error_message: Optional[str] = None,
    ) -> str:
        keys, listing, ret = self.enumerate_keys(partition, echo=echo)
        if ret != 0:
            raise Exception(f"list_partition failed with error {ret}")

        for entry in keys:
            if (
                entry.partition == partition
                and entry.key_type == key_type
                and entry.key_id == key_id
            ):
                raise RuntimeError(error_message or "Key slot already provisioned")

        return listing

    def wipe_partition(self, partition: int, *, echo: bool = True):
        data = self.send_and_wait_prompt(f"{self.CRYPTO_CMD} wipe {partition}\r")
        parsed_data, ret = self._parse_response(data)

        if echo:
            print(parsed_data)
        return ret

    def read_key(
        self, partition: int, key_type: int, key_id: int, *, echo: bool = True
    ):
        data = self.send_and_wait_prompt(
            f"{self.CRYPTO_CMD} read {partition} {key_type} {key_id:x}\r"
        )
        parsed_data, ret = self._parse_response(data)

        if echo:
            print(parsed_data)
        return ret

    def write_key(
        self,
        partition: int,
        key_type: int,
        key_id: int,
        flags: int,
        size: int,
        payload: str,
        *,
        echo: bool = True,
    ):
        data = self.send_and_wait_prompt(
            f"{self.CRYPTO_CMD} write {partition} {key_type} {key_id:x} {flags:x} {size} {payload}\r"
        )
        parsed_data, ret = self._parse_response(data)

        if echo:
            print(parsed_data)
        return ret

    def gen_key(
        self,
        partition: int,
        key_type: int,
        key_id: int,
        flags: int,
        *,
        echo: bool = True,
    ):
        data = self.send_and_wait_prompt(
            f"{self.CRYPTO_CMD} gen {partition} {key_type} {key_id:x} {flags:x}\r"
        )
        parsed_data, ret = self._parse_response(data)

        if echo:
            print(parsed_data)
        return ret

    def gen_csr(
        self,
        partition: int,
        key_id: int,
        flags: int,
        subject_name: str,
        *,
        echo: bool = True,
    ):
        data = self.send_and_wait_prompt(
            f"{self.CRYPTO_CMD} gen_csr {partition} {key_id:x} {flags:x} {subject_name}\r"
        )
        parsed_data, ret = self._parse_response(data)

        if echo:
            print(parsed_data)
        return ret

    def read_key_data(
        self, partition: int, key_type: int, key_id: int
    ) -> Optional[bytes]:
        data = self.send_and_wait_prompt(
            f"{self.CRYPTO_CMD} read {partition} {key_type} {key_id:x}\r"
        )
        parsed_data, ret = self._parse_response(data)

        if ret != 0:
            print(parsed_data)
            return None

        return self._parse_key_data(parsed_data)

    @staticmethod
    def _parse_key_data(text: str) -> bytes:
        result = bytearray()
        in_data = False
        for line in text.splitlines():
            line = line.strip()
            if line.startswith("key_data:"):
                in_data = True
                continue
            if not in_data:
                continue
            # Lines look like: "00000000: aa bb cc dd ..."
            parts = line.split(":", 1)
            if len(parts) != 2:
                continue
            addr, hex_part = parts[0].strip(), parts[1].strip()
            # Address must be an 8-digit hex value
            if len(addr) != 8 or not all(c in "0123456789abcdefABCDEF" for c in addr):
                break
            if not hex_part:
                continue
            for token in hex_part.split():
                result.append(int(token, 16))
        return bytes(result)

    @staticmethod
    def _parse_key_listing(listing: str) -> List["CryptoStorage.KeyEntry"]:
        entries: List[CryptoStorage.KeyEntry] = []

        for raw_line in listing.splitlines():
            line = raw_line.strip()
            if not line.lower().startswith("key:"):
                continue

            columns = line[4:].strip().split()
            if len(columns) < 3:
                continue

            try:
                partition = int(columns[0], 0)
                key_type = int(columns[1], 0)
                key_id = int(columns[2], 0)
            except ValueError:
                continue

            entries.append(CryptoStorage.KeyEntry(partition, key_type, key_id))

        return entries

    _ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")

    def _parse_response(self, data: bytes) -> tuple[str, int]:
        # Strip ANSI escape codes and normalize line endings
        text = self._ANSI_RE.sub("", data.decode("ascii", errors="replace"))
        text = text.replace("\r\n", "\n").replace("\r", "\n")

        # Find the return code line
        ret_match = re.search(r"^RET: (\d+)", text, re.MULTILINE)
        if not ret_match:
            print(f"Unexpected response format:\n{text}")
            print(f"Raw bytes: {data!r}")
            raise Exception("Response format error")

        # Everything between the echo (first line) and the RET line is the body
        first_nl = text.find("\n")
        body = text[first_nl + 1 : ret_match.start()].strip("\n")

        return (body, int(ret_match.group(1)))


class Main(App):
    def init(self):  # type: ignore[override]
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # List command
        self.list_parser = self.subparsers.add_parser("list", help="List all keys")
        self.list_parser.add_argument(
            "-P", "--partition", required=True, type=int, help="Partition number"
        )
        self.list_parser.set_defaults(func=self._list)

        # Wipe command
        self.wipe_parser = self.subparsers.add_parser(
            "wipe", help="Erase all keys on a partition"
        )
        self.wipe_parser.add_argument(
            "-P", "--partition", required=True, type=int, help="Partition number"
        )
        self.wipe_parser.set_defaults(func=self._wipe)

        # Read command
        self.read_parser = self.subparsers.add_parser("read", help="Read a key")
        self.read_parser.add_argument(
            "-P", "--partition", required=True, type=int, help="Partition number"
        )
        self.read_parser.add_argument(
            "-t", "--type", dest="key_type", required=True, type=int, help="Key type"
        )
        self.read_parser.add_argument(
            "-i", "--id", dest="key_id", required=True, type=_auto_int, help="Key ID"
        )
        self.read_parser.set_defaults(func=self._read)

        # Write command
        self.write_parser = self.subparsers.add_parser("write", help="Write a key")
        self.write_parser.add_argument("key", help="Key data", type=str)
        self.write_parser.add_argument(
            "-P", "--partition", required=True, type=int, help="Partition number"
        )
        self.write_parser.add_argument(
            "-t", "--type", dest="key_type", required=True, type=int, help="Key type"
        )
        self.write_parser.add_argument(
            "-i", "--id", dest="key_id", required=True, type=_auto_int, help="Key ID"
        )
        self.write_parser.add_argument(
            "-F", "--flags", required=True, type=_auto_int, help="Key flags"
        )
        self.write_parser.add_argument(
            "-z", "--size", required=True, type=int, help="Key size"
        )
        self.write_parser.set_defaults(func=self._write)

    def _get_portname(self):
        return ("10.0.4.20", 23)

    @CatchExceptions
    def _list(self):
        with CryptoStorage(self._get_portname()) as storage:
            storage.list_partition(self.args.partition)

    @CatchExceptions
    def _wipe(self):
        with CryptoStorage(self._get_portname()) as storage:
            storage.wipe_partition(self.args.partition)

    @CatchExceptions
    def _read(self):
        with CryptoStorage(self._get_portname()) as storage:
            storage.read_key(self.args.partition, self.args.key_type, self.args.key_id)

    @CatchExceptions
    def _write(self):
        with CryptoStorage(self._get_portname()) as storage:
            storage.write_key(
                self.args.partition,
                self.args.key_type,
                self.args.key_id,
                self.args.flags,
                self.args.size,
                self.args.key,
            )


if __name__ == "__main__":
    Main()()
