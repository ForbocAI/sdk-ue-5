#!/usr/bin/env python3
"""Create a deterministic Fab upload archive and integrity manifest."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path
import stat
import zipfile

from validate_package import load_config, resolve_package_root, validate


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parents[1]
ZIP_TIMESTAMP = (2026, 1, 1, 0, 0, 0)


def archive_name(config: dict[str, object], version: str) -> str:
    """User Story: As release consumers, I need archives named from plugin and engine versions. Signature: archive_name(config: dict[str, object], version: str) -> str."""
    return str(config["archiveTemplate"]).format(version=version, engineVersion=config["engineVersion"])


def write_archive(root: Path, destination: Path, plugin_name: str) -> int:
    """User Story: As reproducible releases, I need sorted files with stable zip metadata. Signature: write_archive(root: Path, destination: Path, plugin_name: str) -> int."""
    files = sorted(path for path in root.rglob("*") if path.is_file())
    destination.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(destination, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for path in files:
            relative = Path(plugin_name) / path.relative_to(root)
            info = zipfile.ZipInfo(relative.as_posix(), ZIP_TIMESTAMP)
            mode = stat.S_IMODE(path.stat().st_mode)
            info.external_attr = (mode & 0xFFFF) << 16
            info.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(info, path.read_bytes(), compresslevel=9)
    return len(files)


def sha256(path: Path) -> str:
    """User Story: As artifact verification, I need a stable digest for each upload archive. Signature: sha256(path: Path) -> str."""
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for block in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def verify_archive(path: Path, expected_file_count: int) -> None:
    """User Story: As a Fab customer, I need the completed upload archive reopened and integrity-checked before release. Signature: verify_archive(path: Path, expected_file_count: int) -> None."""
    with zipfile.ZipFile(path, "r") as archive:
        names = archive.namelist()
        if len(names) != expected_file_count:
            raise ValueError(
                f"Archive contains {len(names)} files; expected {expected_file_count}",
            )
        if len(set(names)) != len(names):
            raise ValueError("Archive contains duplicate paths")
        corrupted = archive.testzip()
        if corrupted is not None:
            raise ValueError(f"Archive CRC validation failed: {corrupted}")


def main() -> int:
    """User Story: As a publisher, I need one validated archive plus machine-readable evidence. Signature: main() -> int."""
    config = load_config()
    root = resolve_package_root(config, None)
    findings = validate(root, config)
    if findings:
        raise RuntimeError("Refusing to archive an invalid package:\n" + "\n".join(f"- {value}" for value in findings))

    descriptor = json.loads((root / str(config["descriptor"])).read_text(encoding="utf-8"))
    version = str(descriptor["VersionName"])
    release_root = PROJECT_ROOT / str(config["paths"]["releaseDirectory"])
    archive = release_root / archive_name(config, version)
    file_count = write_archive(root, archive, str(config["pluginName"]))
    verify_archive(archive, file_count)
    digest = sha256(archive)
    manifest = {
        "schemaVersion": 1,
        "artifact": archive.name,
        "sha256": digest,
        "bytes": archive.stat().st_size,
        "fileCount": file_count,
        "pluginVersion": version,
        "engineVersion": config["engineVersion"],
    }
    manifest_path = archive.with_suffix(".manifest.json")
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    archive.with_suffix(".sha256").write_text(f"{digest}  {archive.name}\n", encoding="utf-8")
    print(json.dumps(manifest, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
