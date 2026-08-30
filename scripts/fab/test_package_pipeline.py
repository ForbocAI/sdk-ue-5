#!/usr/bin/env python3
"""Regression coverage for compiled Fab package finalization."""

from __future__ import annotations

import json
from pathlib import Path
import tempfile
import unittest

from finalize_package import finalize
from validate_package import (
    CONFIG_PATH,
    PROJECT_ROOT,
    load_config,
    resolve_package_root,
)


FIXTURE_PATH = CONFIG_PATH.parent / "tests" / "package-pipeline.json"


def load_fixture() -> dict[str, object]:
    """Load package-pipeline examples from their authored JSON authority."""
    return json.loads(FIXTURE_PATH.read_text(encoding="utf-8"))


def write_entries(root: Path, entries: object) -> tuple[Path, ...]:
    """Materialize authored paths for one isolated package test."""
    paths = tuple(root / str(entry["path"]) for entry in entries)
    tuple(path.parent.mkdir(parents=True, exist_ok=True) for path in paths)
    tuple(
        path.write_text(str(entry["content"]), encoding="utf-8")
        for path, entry in zip(paths, entries)
    )
    return paths


class PackagePipelineTests(unittest.TestCase):
    """Framework boundary for data-driven package-pipeline assertions."""

    def test_default_root_is_compiled_package(self) -> None:
        config = load_config()
        expected = (PROJECT_ROOT / str(config["paths"]["buildDirectory"])).resolve()
        self.assertEqual(resolve_package_root(config, None), expected)

    def test_finalizer_removes_only_authored_residue(self) -> None:
        config = load_config()
        fixture = load_fixture()
        with tempfile.TemporaryDirectory() as temporary_directory:
            root = Path(temporary_directory)
            residue = write_entries(root, fixture["residue"])
            retained = write_entries(root, fixture["retained"])
            finalize(root, config)
            self.assertTrue(all(not path.exists() for path in residue))
            self.assertTrue(all(path.is_file() for path in retained))


if __name__ == "__main__":
    unittest.main()
