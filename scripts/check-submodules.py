#!/usr/bin/env python3
"""Verify that each git submodule is on its expected remote branch.

Requires all submodules to be initialized first:
    git submodule update --init --recursive
"""

import argparse
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_DATA_FILE = REPO_ROOT / ".github" / "expected-submodule-branches.txt"


def load_expected(data_file: Path) -> dict[str, str]:
    result: dict[str, str] = {}
    for line in data_file.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) != 2:
            raise ValueError(f"Bad entry in {data_file}: {line!r}")
        result[parts[0]] = parts[1]
    return result


def check_one(sm_path: str, expected_branch: str) -> tuple[bool, str]:
    prefix = f"  {sm_path}:"
    sm_dir = REPO_ROOT / sm_path
    if not sm_dir.is_dir():
        return False, f"{prefix} not initialized (run 'git submodule update --init')"
    try:
        out = subprocess.check_output(
            ["git", "-C", str(sm_dir), "branch", "-r", "--contains", "HEAD"],
            text=True,
            stderr=subprocess.DEVNULL,
        )
    except subprocess.CalledProcessError as exc:
        return False, f"{prefix} git error — {exc}"
    branches = [b.strip() for b in out.splitlines() if b.strip()]
    if expected_branch in branches:
        return True, f"{prefix} correct branch ({expected_branch})"
    return False, f"{prefix} incorrect branch (expected {expected_branch})"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "data_file",
        nargs="?",
        type=Path,
        default=DEFAULT_DATA_FILE,
        metavar="BRANCHES_FILE",
        help="path to expected-submodule-branches.txt"
        " (default: .github/expected-submodule-branches.txt)",
    )
    args = parser.parse_args()

    expected = load_expected(args.data_file)
    failures: list[str] = []

    with ThreadPoolExecutor(max_workers=8) as pool:
        futures = {
            pool.submit(check_one, sm_path, branch): sm_path
            for sm_path, branch in expected.items()
        }
        results: dict[str, tuple[bool, str]] = {}
        for future in as_completed(futures):
            results[futures[future]] = future.result()

    for sm_path in expected:  # stable order
        ok, msg = results[sm_path]
        print(msg)
        if not ok:
            failures.append(sm_path)

    if failures:
        print(f"\nNot all modules are on the correct branch: {', '.join(failures)}")
        return 1

    print("\nAll submodules are on the correct branch.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
