import argparse
from pathlib import Path
from typing import Type
from collections.abc import Iterable

def extensions(flipper_debug_extensions):
    class FwVersionExtension(flipper_debug_extensions.BaseDebugExtension):
        SCRIPTS_PATH = Path(__file__).resolve().parent.parent
        FIRMWARE_SCRIPT = "bsbversion.py"

        def append_gdb_args(self, args: argparse.Namespace) -> Iterable[flipper_debug_extensions.GdbParam]:
            yield flipper_debug_extensions.GdbParam(
                f"source {self.posix_path(self.SCRIPTS_PATH / self.FIRMWARE_SCRIPT)}"
            )
            yield flipper_debug_extensions.GdbParam("fw-version")

    yield FwVersionExtension
