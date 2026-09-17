"""Helpers for building JavaScript application packages in memory."""

from __future__ import annotations

import io
import json
import tarfile


APP_NAME = "Integration Test App"
APP_AUTHOR = "BUSY Bar QA"
APP_DESCRIPTION = "Installed by the API integration test"


def _add_tar_directory(archive: tarfile.TarFile, path: str) -> None:
    entry = tarfile.TarInfo(f"{path.rstrip('/')}/")
    entry.type = tarfile.DIRTYPE
    entry.mode = 0o755
    archive.addfile(entry)


def _add_tar_file(
    archive: tarfile.TarFile,
    path: str,
    content: bytes,
) -> None:
    entry = tarfile.TarInfo(path)
    entry.size = len(content)
    entry.mode = 0o644
    archive.addfile(entry, io.BytesIO(content))


def build_app_package(
    root_id: str,
    *,
    manifest_id: str | None = None,
    version: str = "1.0.0",
    include_main: bool = True,
    include_optional_manifest_fields: bool = True,
    extra_files: dict[str, bytes] | None = None,
    manifest_updates: dict[str, object] | None = None,
    missing_manifest_fields: tuple[str, ...] = (),
    gzip: bool = True,
    main_script: bytes | None = None,
) -> tuple[bytes, bytes]:
    """Build a valid JS application archive and return it with main.js."""
    manifest = {
        "format_version": 1,
        "id": manifest_id if manifest_id is not None else root_id,
        "name": APP_NAME,
        "version": version,
    }
    if include_optional_manifest_fields:
        manifest.update(
            {
                "description": APP_DESCRIPTION,
                "author": APP_AUTHOR,
                "heap_size_kib": 64,
                "debug": False,
            }
        )
    manifest.update(manifest_updates or {})
    for field in missing_manifest_fields:
        manifest.pop(field, None)
    manifest_bytes = json.dumps(
        manifest,
        separators=(",", ":"),
    ).encode("utf-8")
    if main_script is None:
        main_script = (
            f"globalThis.integrationVersion = {version!r};\n"
        ).encode("utf-8")

    package = io.BytesIO()
    with tarfile.open(
        fileobj=package,
        mode="w:gz" if gzip else "w",
        format=tarfile.USTAR_FORMAT,
    ) as archive:
        _add_tar_directory(archive, root_id)
        _add_tar_directory(archive, f"{root_id}/appmeta")
        _add_tar_directory(archive, f"{root_id}/scripts")
        _add_tar_file(
            archive,
            f"{root_id}/appmeta/manifest.json",
            manifest_bytes,
        )
        if include_main:
            _add_tar_file(
                archive,
                f"{root_id}/scripts/main.js",
                main_script,
            )
        for path, content in (extra_files or {}).items():
            _add_tar_file(archive, path, content)
    return package.getvalue(), main_script


def build_non_app_package() -> bytes:
    """Build a TAR archive that does not contain an application."""
    package = io.BytesIO()
    with tarfile.open(
        fileobj=package,
        mode="w:gz",
        format=tarfile.USTAR_FORMAT,
    ) as archive:
        _add_tar_file(archive, "README.txt", b"not an application")
    return package.getvalue()
