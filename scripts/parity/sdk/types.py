from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any


@dataclass(frozen=True)
class Symbol:
    name: str
    kind: str
    file: str


@dataclass(frozen=True)
class FolderMirror:
    ts_folder: str
    ts_signature: str
    ue_folders: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class FileMirror:
    ts_file: str
    ts_signature: str
    ue_files: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class SymbolMirror:
    ts_symbol: Symbol
    ue_symbols: tuple[Symbol, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class ParityProgram:
    label: str
    ts_source_roots: tuple[Path, ...]
    ue_source_roots: tuple[Path, ...]


@dataclass(frozen=True)
class ProgramInventory:
    program: ParityProgram
    ts_folders: list[str]
    ue_folders: list[str]
    folder_mirrors: list[FolderMirror]
    ts_files: list[Path]
    ue_files: list[Path]
    file_mirrors: list[FileMirror]
    ts_symbols: list[Symbol]
    ue_symbols: list[Symbol]
    symbol_mirrors: list[SymbolMirror]


@dataclass(frozen=True)
class CliContract:
    root: Path
    directory: Path
    vocabulary: dict[str, Any]
    roles: dict[str, Any]
    domains: dict[str, dict[str, dict[str, Any]]]
    domain_order: tuple[str, ...]
    files: tuple[Path, ...]
