#!/usr/bin/env python3
"""Verify that each git submodule is on its expected remote branch."""

import argparse
import subprocess
import sys
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


def containing_branches(submodule_path: Path) -> list[str]:
    commit = subprocess.check_output(
        ["git", "rev-parse", "HEAD"], cwd=submodule_path, text=True
    ).strip()
    out = subprocess.check_output(
        ["git", "branch", "-r", "--contains", commit],
        cwd=submodule_path,
        text=True,
    )
    return [b.strip() for b in out.splitlines() if b.strip()]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "data_file",
        nargs="?",
        type=Path,
        default=DEFAULT_DATA_FILE,
        metavar="BRANCHES_FILE",
        help="path to expected-submodule-branches.txt (default: .github/expected-submodule-branches.txt)",
    )
    args = parser.parse_args()

    expected = load_expected(args.data_file)
    failures: list[str] = []

    for sm_path, expected_branch in expected.items():
        full_path = REPO_ROOT / sm_path
        prefix = f"  {sm_path}:"

        if not full_path.is_dir():
            print(f"{prefix} directory not found, skipping")
            continue

        try:
            branches = containing_branches(full_path)
        except subprocess.CalledProcessError as exc:
            print(f"{prefix} git error — {exc}")
            failures.append(sm_path)
            continue

        if expected_branch in branches:
            print(f"{prefix} correct branch ({expected_branch})")
        else:
            actual = ", ".join(branches) or "(none)"
            print(
                f"{prefix} incorrect branch (expected {expected_branch}, got {actual})"
            )
            failures.append(sm_path)

    if failures:
        print(f"\nNot all modules are on the correct branch: {', '.join(failures)}")
        return 1

    print("\nAll submodules are on the correct branch.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
