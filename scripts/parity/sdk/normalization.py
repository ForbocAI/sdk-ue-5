from __future__ import annotations

import difflib
import re
from collections.abc import Callable, Sequence
from pathlib import Path
from typing import TypeVar


T = TypeVar("T")
CANDIDATE_LIMIT = 3
CANDIDATE_THRESHOLD = 0.62


def normalize_name(name: str) -> str:
    without_unreal_prefix = re.sub(r"^(F|E|U|I)(?=[A-Z])", "", name)
    return re.sub(r"[^a-z0-9]", "", without_unreal_prefix.lower())


def normalize_path_name(name: str) -> str:
    return re.sub(r"[^a-z0-9]", "", name.lower())


def package_for_ts_path(path: str) -> str:
    parts = Path(path).parts
    if len(parts) >= 2 and parts[0] == "packages":
        return parts[1]
    return parts[0] if parts else "other"


def normalized_path_segments(path: str) -> tuple[str, ...]:
    source = Path(path)
    raw_parts = (*source.parts[:-1], source.stem) if source.suffix else source.parts
    return tuple(
        segment
        for segment in (normalize_path_name(part) for part in raw_parts)
        if segment
    )


def normalize_path_signature(path: str) -> str:
    return "/".join(normalized_path_segments(path))


def ts_path_signature(path: str, source_roots: Sequence[Path]) -> str:
    from parity.sdk.discovery import source_root_for_ts_path

    source_root = source_root_for_ts_path(path, source_roots)
    if source_root is None:
        return normalize_path_signature(path)
    relative_path = Path(path).relative_to(source_root).as_posix()
    return (
        normalize_path_name(package_for_ts_path(path))
        if relative_path == "."
        else normalize_path_signature(relative_path)
    )


def normalized_path_segment_variants(path: str) -> tuple[tuple[str, ...], ...]:
    segments = normalized_path_segments(path)
    if not Path(path).suffix or len(segments) < 3:
        return (segments,)
    leaf = segments[-1]
    variants = [segments]
    variants.extend(
        (*segments[:-1], leaf[len(ancestor) :])
        for ancestor in segments[:-2]
        if leaf.startswith(ancestor) and len(leaf) > len(ancestor)
    )
    return tuple(dict.fromkeys(variants))


def ue_suffix_index(paths: Sequence[str]) -> dict[str, list[str]]:
    index: dict[str, list[str]] = {}
    for path in paths:
        for segments in normalized_path_segment_variants(path):
            for start in range(len(segments)):
                matches = index.setdefault("/".join(segments[start:]), [])
                if path not in matches:
                    matches.append(path)
    return index


def name_similarity(left: str, right: str) -> float:
    return difflib.SequenceMatcher(
        None, normalize_name(left), normalize_name(right)
    ).ratio()


def index_by_normalized_name(
    values: Sequence[T], name_of: Callable[[T], str]
) -> dict[str, list[T]]:
    index: dict[str, list[T]] = {}
    for value in values:
        index.setdefault(normalize_name(name_of(value)), []).append(value)
    return index


def closest_candidates(
    ts_name: str,
    ue_values: Sequence[T],
    name_of: Callable[[T], str],
    sort_of: Callable[[T], str],
) -> tuple[T, ...]:
    scored = [
        (name_similarity(ts_name, name_of(value)), sort_of(value), value)
        for value in ue_values
        if normalize_name(ts_name) != normalize_name(name_of(value))
    ]
    filtered = [item for item in scored if item[0] >= CANDIDATE_THRESHOLD]
    ordered = sorted(filtered, key=lambda item: (-item[0], item[1]))
    return tuple(value for _score, _sort, value in ordered[:CANDIDATE_LIMIT])


def mirror_status(exact: bool) -> str:
    return "Same" if exact else "Different"


def mirror_notes(candidates: Sequence[object], exact: bool) -> str:
    if exact:
        return "Same normalized path or name signature."
    if candidates:
        return "No exact signature; evidence contains nearest runtime-language candidates."
    return "No exact signature or close runtime-language candidate was found."
