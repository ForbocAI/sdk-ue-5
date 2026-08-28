from __future__ import annotations

import hashlib
import subprocess
from collections.abc import Iterable, Sequence
from pathlib import Path
from typing import Any


def repository_identity(root: Path) -> str:
    return root.resolve().name


def relative_path(path: Path, root: Path) -> str:
    return path.resolve().relative_to(root.resolve()).as_posix()


def contract_hashes(root: Path, paths: Iterable[Path]) -> dict[str, str]:
    existing = sorted(
        {path.resolve() for path in paths if path.resolve().is_file()},
        key=lambda path: relative_path(path, root),
    )
    return {
        relative_path(path, root): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in existing
    }


def contract_bundle_hash(
    root: Path,
    paths: Iterable[Path],
    label: str,
) -> dict[str, str]:
    existing = sorted(
        {path.resolve() for path in paths if path.resolve().is_file()},
        key=lambda path: relative_path(path, root),
    )
    digest = hashlib.sha256()
    for path in existing:
        digest.update(relative_path(path, root).encode("utf-8"))
        digest.update(b"\0")
        digest.update(path.read_bytes())
        digest.update(b"\0")
    return {label: digest.hexdigest()}


def latest_input_commit(root: Path, paths: Sequence[Path]) -> str:
    relative_paths = [relative_path(path, root) for path in paths if path.exists()]
    command = ["git", "-C", str(root), "log", "-1", "--format=%H", "--"]
    result = subprocess.run(
        [*command, *relative_paths],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    commit = result.stdout.strip()
    if commit:
        return commit
    fallback = subprocess.run(
        ["git", "-C", str(root), "rev-parse", "HEAD"],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return fallback.stdout.strip() or "unversioned"


def source_record(
    root: Path,
    contract_paths: Sequence[Path],
    commit_paths: Sequence[Path] | None = None,
    bundle_label: str | None = None,
) -> dict[str, Any]:
    stable_inputs = commit_paths if commit_paths is not None else contract_paths
    return {
        "repository": repository_identity(root),
        "commit": latest_input_commit(root, stable_inputs),
        "contractHashes": (
            contract_bundle_hash(root, contract_paths, bundle_label)
            if bundle_label is not None
            else contract_hashes(root, contract_paths)
        ),
    }
