import hashlib
import json
import os
import re
import stat
from typing import NoReturn, TypedDict

CHECKSUM_PATTERN = re.compile(r"busybar-(?P<target>\w+)-sha256sum\.txt")
FIRMWARE_PATTERN = re.compile(
    r"busybar-(?P<target>\w+)-(?P<type>\w+)-(?P<version>.+)\.(?P<ext>\w+)"
)


class File(TypedDict):
    path: str
    sha256: str
    size: int
    target: str | None
    type: str | None


class Build(TypedDict):
    version: str
    version_token: str
    branch: str
    files: list[File]


def raise_walk_error(error: OSError) -> NoReturn:
    raise error


def build_manifest(artifacts_dir: str, branch: str, version_token: str) -> Build:
    files: list[File] = []
    versions: set[str] = set()

    for directory, _, names in os.walk(artifacts_dir, onerror=raise_walk_error):
        for name in names:
            path = os.path.join(directory, name)
            metadata = os.lstat(path)
            if not stat.S_ISREG(metadata.st_mode):
                continue

            digest = hashlib.sha256()
            with open(path, "rb") as f:
                while chunk := f.read(1024 * 1024):
                    digest.update(chunk)

            file: File = {
                "path": os.path.relpath(path, artifacts_dir).replace(os.sep, "/"),
                "sha256": digest.hexdigest(),
                "size": metadata.st_size,
                "target": None,
                "type": None,
            }
            if match := CHECKSUM_PATTERN.fullmatch(name):
                file.update(target=match["target"], type="sha256sum_txt")
            elif match := FIRMWARE_PATTERN.fullmatch(name):
                versions.add(match["version"])
                file.update(
                    target=match["target"], type=f"{match['type']}_{match['ext']}"
                )
            files.append(file)

    if not versions:
        raise ValueError(
            f"No build version found in artifact filenames in {artifacts_dir}"
        )

    if len(versions) > 1:
        raise ValueError(
            f"Conflicting build versions in artifact filenames: {sorted(versions)!r}"
        )

    return {
        "version": versions.pop(),
        "version_token": version_token,
        "branch": branch,
        "files": files,
    }


def main() -> None:
    manifest = build_manifest(
        artifacts_dir=os.environ["ARTIFACTS_DIR"],
        branch=os.environ["BRANCH_NAME"],
        version_token=os.environ["VERSION_TOKEN"],
    )
    print(json.dumps(manifest))


if __name__ == "__main__":
    main()
