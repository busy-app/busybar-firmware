#!/usr/bin/env python3

import re

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
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.send_and_wait_prompt("exit\r")
        Cli.__exit__(self, exc_type, exc_value, traceback)

    def list_partition(self, partition: int):
        data = self.send_and_wait_prompt(f"{self.CRYPTO_CMD} list {partition}\r")
        parsed_data, ret = self._parse_response(data)

        print(parsed_data)
        return ret

    def wipe_partition(self, partition: int):
        data = self.send_and_wait_prompt(f"{self.CRYPTO_CMD} wipe {partition}\r")
        parsed_data, ret = self._parse_response(data)

        print(parsed_data)
        return ret

    def read_key(self, partition: int, key_type: int, key_id: int):
        data = self.send_and_wait_prompt(
            f"{self.CRYPTO_CMD} read {partition} {key_type} {key_id:x}\r"
        )
        parsed_data, ret = self._parse_response(data)

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
    ):
        data = self.send_and_wait_prompt(
            f"{self.CRYPTO_CMD} write {partition} {key_type} {key_id:x} {flags:x} {size} {payload}\r"
        )
        parsed_data, ret = self._parse_response(data)

        print(parsed_data)
        return ret

    def _parse_response(self, data: bytes) -> tuple[str, int]:
        """
        Regex explanation:
            - Skip the first line (command echo): ".+\n"
            - Capture all lines before return code: "(^(?s:.)+)"
            - Skip the last newline before return code: "\n"
            - Capture the return code: "RET: (\\d+)"
        """
        match = re.search(
            ".+\n(^(?s:.)+)\nRET: (\\d+)", data.decode("ascii"), re.MULTILINE
        )

        if not match:
            raise Exception("Response format error")

        groups = match.groups()

        if len(groups) != 2:
            raise Exception("Response format error")

        return (groups[0], int(groups[1]))


class Main(App):
    def init(self):
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
