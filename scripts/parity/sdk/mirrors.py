from __future__ import annotations

from collections.abc import Sequence
from pathlib import Path

from parity.sdk.discovery import (
    iter_ts_files,
    iter_ts_folders,
    iter_ue_files,
    iter_ue_folders,
    relative,
)
from parity.sdk.normalization import (
    closest_candidates,
    index_by_normalized_name,
    mirror_notes,
    mirror_status,
    normalize_name,
    normalize_path_signature,
    ts_path_signature,
    ue_suffix_index,
)
from parity.sdk.symbols import extract_ts_symbols, extract_ue_symbols
from parity.sdk.types import (
    FileMirror,
    FolderMirror,
    ParityProgram,
    ProgramInventory,
    Symbol,
    SymbolMirror,
)


def build_folder_mirrors(
    ts_folders: Sequence[str],
    ue_folders: Sequence[str],
    ts_source_roots: Sequence[Path],
) -> list[FolderMirror]:
    ue_by_signature = ue_suffix_index(ue_folders)
    return [
        _folder_mirror(folder, ue_folders, ue_by_signature, ts_source_roots)
        for folder in ts_folders
    ]


def _folder_mirror(
    folder: str,
    ue_folders: Sequence[str],
    ue_by_signature: dict[str, list[str]],
    ts_source_roots: Sequence[Path],
) -> FolderMirror:
    signature = ts_path_signature(folder, ts_source_roots)
    same = tuple(sorted(ue_by_signature.get(signature, [])))
    candidates = same or closest_candidates(
        signature, ue_folders, normalize_path_signature, lambda value: value
    )
    return FolderMirror(
        folder,
        signature,
        candidates,
        mirror_status(bool(same)),
        mirror_notes(candidates, bool(same)),
    )


def build_file_mirrors(
    ts_files: Sequence[Path],
    ts_root: Path,
    ue_files: Sequence[Path],
    ue_root: Path,
    ts_source_roots: Sequence[Path],
) -> list[FileMirror]:
    ue_relative = [relative(path, ue_root) for path in ue_files]
    ue_by_signature = ue_suffix_index(ue_relative)
    return [
        _file_mirror(
            path,
            ts_root,
            ue_relative,
            ue_by_signature,
            ts_source_roots,
        )
        for path in ts_files
    ]


def _file_mirror(
    path: Path,
    ts_root: Path,
    ue_files: Sequence[str],
    ue_by_signature: dict[str, list[str]],
    ts_source_roots: Sequence[Path],
) -> FileMirror:
    ts_file = relative(path, ts_root)
    signature = ts_path_signature(ts_file, ts_source_roots)
    same = tuple(sorted(ue_by_signature.get(signature, [])))
    candidates = same or closest_candidates(
        signature, ue_files, normalize_path_signature, lambda value: value
    )
    return FileMirror(
        ts_file,
        signature,
        candidates,
        mirror_status(bool(same)),
        mirror_notes(candidates, bool(same)),
    )


def build_symbol_mirrors(
    ts_symbols: Sequence[Symbol], ue_symbols: Sequence[Symbol]
) -> list[SymbolMirror]:
    ue_by_name = index_by_normalized_name(ue_symbols, lambda symbol: symbol.name)
    return [
        _symbol_mirror(symbol, ue_symbols, ue_by_name)
        for symbol in ts_symbols
    ]


def _symbol_mirror(
    symbol: Symbol,
    ue_symbols: Sequence[Symbol],
    ue_by_name: dict[str, list[Symbol]],
) -> SymbolMirror:
    same = tuple(
        sorted(
            ue_by_name.get(normalize_name(symbol.name), []),
            key=lambda item: (item.file, item.kind, item.name),
        )
    )
    same_kind = [item for item in ue_symbols if item.kind == symbol.kind]
    search_values = same_kind or list(ue_symbols)
    candidates = same or closest_candidates(
        symbol.name,
        search_values,
        lambda value: value.name,
        lambda value: f"{value.file}:{value.kind}:{value.name}",
    )
    return SymbolMirror(
        symbol,
        candidates,
        mirror_status(bool(same)),
        mirror_notes(candidates, bool(same)),
    )


def build_program_inventory(
    ts_root: Path, ue_root: Path, program: ParityProgram
) -> ProgramInventory:
    ts_files = iter_ts_files(ts_root, program.ts_source_roots)
    ue_files = iter_ue_files(ue_root, program.ue_source_roots)
    ts_folders = iter_ts_folders(ts_files, ts_root, program.ts_source_roots)
    ue_folders = iter_ue_folders(ue_files, ue_root)
    ts_symbols = [
        symbol for path in ts_files for symbol in extract_ts_symbols(path, ts_root)
    ]
    ue_symbols = [
        symbol for path in ue_files for symbol in extract_ue_symbols(path, ue_root)
    ]
    return ProgramInventory(
        program,
        ts_folders,
        ue_folders,
        build_folder_mirrors(ts_folders, ue_folders, program.ts_source_roots),
        ts_files,
        ue_files,
        build_file_mirrors(
            ts_files, ts_root, ue_files, ue_root, program.ts_source_roots
        ),
        ts_symbols,
        ue_symbols,
        build_symbol_mirrors(ts_symbols, ue_symbols),
    )
