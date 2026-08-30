#!/usr/bin/env python3
"""Remove Unreal build residue from the compiled Fab package."""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil

from validate_package import load_config, resolve_package_root


def forbidden_path_roots(
    root: Path,
    forbidden_parts: set[str],
) -> set[Path]:
    """User Story: As package finalization, I need generated subtrees selected from the authored release contract. Signature: forbidden_path_roots(root: Path, forbidden_parts: set[str]) -> set[Path]."""
    relative_paths = (path.relative_to(root) for path in root.rglob("*"))
    return {
        root.joinpath(*relative.parts[: index + 1])
        for relative in relative_paths
        for index, part in enumerate(relative.parts)
        if part.casefold() in forbidden_parts
    }


def forbidden_files(root: Path, forbidden_extensions: set[str]) -> set[Path]:
    """User Story: As package finalization, I need generated files selected from authored extension rules. Signature: forbidden_files(root: Path, forbidden_extensions: set[str]) -> set[Path]."""
    return {
        path
        for path in root.rglob("*")
        if path.is_file() and path.suffix.casefold() in forbidden_extensions
    }


def outermost_paths(paths: set[Path]) -> tuple[Path, ...]:
    """User Story: As package finalization, I need nested residue collapsed to one deletion per owned subtree. Signature: outermost_paths(paths: set[Path]) -> tuple[Path, ...]."""
    ordered = sorted(paths, key=lambda path: (len(path.parts), path.as_posix()))
    return tuple(
        path
        for path in ordered
        if not any(parent in paths for parent in path.parents)
    )


def remove_path(path: Path) -> None:
    """User Story: As package finalization, I need one effect boundary for removing a selected file or subtree. Signature: remove_path(path: Path) -> None."""
    path.unlink() if path.is_file() or path.is_symlink() else shutil.rmtree(path)


def finalize(root: Path, config: dict[str, object]) -> tuple[Path, ...]:
    """User Story: As a Fab customer, I need the compiled plugin stripped of development-only residue. Signature: finalize(root: Path, config: dict[str, object]) -> tuple[Path, ...]."""
    forbidden_parts = {
        str(value).casefold() for value in config["forbiddenPathParts"]
    }
    forbidden_extensions = {
        str(value).casefold() for value in config["forbiddenExtensions"]
    }
    residue = outermost_paths(
        forbidden_path_roots(root, forbidden_parts)
        | forbidden_files(root, forbidden_extensions),
    )
    tuple(remove_path(path) for path in residue)
    return residue


def main() -> int:
    """User Story: As release automation, I need one explicit compiled-package finalization command. Signature: main() -> int."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-root", help="Compiled plugin package root")
    args = parser.parse_args()
    config = load_config()
    root = resolve_package_root(config, args.package_root)
    if not root.is_dir():
        raise FileNotFoundError(f"Compiled package root does not exist: {root}")
    removed = finalize(root, config)
    print(f"Finalized Fab package: removed {len(removed)} residue path(s)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
