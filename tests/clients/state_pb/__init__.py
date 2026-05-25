"""
On-demand Python bindings for the BSB protobuf schema.

The proto sources live in the repo-root ``assets/proto/`` submodule. On first
import this module shells out to ``grpc_tools.protoc`` to generate the
``*_pb2.py`` files into ``_generated/`` (gitignored). Subsequent imports are
no-ops as long as the hash of all ``.proto`` mtimes still matches the value
stored in ``_generated/.stamp``.

After generation, ``_generated/`` is added to ``sys.path`` and the most
commonly needed modules are re-exported at the package top level:

    from clients.state_pb import state_pb2          # BSB_State.State
    from clients.state_pb import state              # subpackage: device_name_pb2 etc.

The re-exports below cover what the WebSocket state-publisher regression
tests need today; if you need more, just add them.
"""

from __future__ import annotations

import hashlib
import importlib
import os
import subprocess
import sys
from pathlib import Path
from typing import List

_HERE = Path(__file__).resolve().parent
_GENERATED = _HERE / "_generated"
_STAMP = _GENERATED / ".stamp"


def _locate_proto_dir() -> Path:
    """Walk up from this file looking for ``assets/proto/`` with .proto files."""
    cur = _HERE
    for _ in range(8):
        candidate = cur / "assets" / "proto"
        if candidate.is_dir() and any(candidate.rglob("*.proto")):
            return candidate
        if cur.parent == cur:
            break
        cur = cur.parent
    raise RuntimeError(
        "Could not locate assets/proto/ relative to "
        f"{_HERE} — is the submodule initialised?"
    )


def _list_protos(proto_dir: Path) -> List[Path]:
    return sorted(proto_dir.rglob("*.proto"))


def _hash_protos(proto_dir: Path, proto_files: List[Path]) -> str:
    h = hashlib.sha256()
    for p in proto_files:
        st = p.stat()
        # rel path + size + mtime_ns
        h.update(str(p.relative_to(proto_dir)).encode())
        h.update(b"|")
        h.update(str(st.st_size).encode())
        h.update(b"|")
        h.update(str(st.st_mtime_ns).encode())
        h.update(b"\n")
    return h.hexdigest()


def _clear_generated() -> None:
    if not _GENERATED.exists():
        return
    # Remove tree but keep dir
    for child in _GENERATED.iterdir():
        if child.is_dir():
            import shutil

            shutil.rmtree(child)
        else:
            try:
                child.unlink()
            except FileNotFoundError:
                pass


def _ensure_init_files(root: Path) -> None:
    """Drop an empty ``__init__.py`` into every subdir of ``root``.

    grpc_tools.protoc does not synthesise ``__init__.py`` files, so generated
    modules that live in subdirectories (e.g. ``state/wifi_pb2.py``) are not
    importable as ``state.wifi_pb2`` without these shims.
    """
    for dirpath, _dirnames, _filenames in os.walk(root):
        d = Path(dirpath)
        init = d / "__init__.py"
        if not init.exists():
            init.write_text("")


def _run_protoc(proto_dir: Path, proto_files: List[Path]) -> None:
    _GENERATED.mkdir(parents=True, exist_ok=True)
    cmd = [
        sys.executable,
        "-m",
        "grpc_tools.protoc",
        f"-I{proto_dir}",
        f"--python_out={_GENERATED}",
        *[str(p) for p in proto_files],
    ]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        raise RuntimeError(
            "grpc_tools.protoc failed:\n"
            f"cmd: {' '.join(cmd)}\n"
            f"stdout: {res.stdout}\n"
            f"stderr: {res.stderr}"
        )


def _generate_if_needed() -> Path:
    proto_dir = _locate_proto_dir()
    proto_files = _list_protos(proto_dir)
    if not proto_files:
        raise RuntimeError(f"No .proto files under {proto_dir}")

    current_hash = _hash_protos(proto_dir, proto_files)
    if _STAMP.exists() and _STAMP.read_text().strip() == current_hash and _GENERATED.exists():
        # Stamp matches; make sure subdir __init__.py shims are there in case
        # someone hand-cleaned them.
        _ensure_init_files(_GENERATED)
        return _GENERATED

    _clear_generated()
    _GENERATED.mkdir(parents=True, exist_ok=True)
    _run_protoc(proto_dir, proto_files)
    _ensure_init_files(_GENERATED)
    _STAMP.write_text(current_hash)
    return _GENERATED


_generated_root = _generate_if_needed()

# Expose generated modules on sys.path so they can be imported by their
# original (proto-relative) names.
_gen_str = str(_generated_root)
if _gen_str not in sys.path:
    sys.path.insert(0, _gen_str)


def _safe_import(name: str):
    try:
        return importlib.import_module(name)
    except Exception:  # pragma: no cover - surfaces upstream on first use
        return None


# Re-export the modules the test suite touches directly. Anything missing
# (e.g. when the proto schema evolves) becomes ``None`` rather than blowing
# up the whole import — callers asserting on these will fail loudly.
state_pb2 = _safe_import("state_pb2")
timer_pb2 = _safe_import("timer_pb2")
error_pb2 = _safe_import("error_pb2")
frame_pb2 = _safe_import("frame_pb2")
input_pb2 = _safe_import("input_pb2")

# Subpackage modules under ``state/``
state = _safe_import("state")  # package
device_name_pb2 = _safe_import("state.device_name_pb2")
audio_pb2 = _safe_import("state.audio_pb2")
brightness_pb2 = _safe_import("state.brightness_pb2")
timezone_pb2 = _safe_import("state.timezone_pb2")
wifi_pb2 = _safe_import("state.wifi_pb2")
ble_pb2 = _safe_import("state.ble_pb2")

__all__ = [
    "state_pb2",
    "timer_pb2",
    "error_pb2",
    "frame_pb2",
    "input_pb2",
    "state",
    "device_name_pb2",
    "audio_pb2",
    "brightness_pb2",
    "timezone_pb2",
    "wifi_pb2",
    "ble_pb2",
]
