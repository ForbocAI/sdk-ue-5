from __future__ import annotations

import re
from pathlib import Path

from parity.sdk.discovery import relative
from parity.sdk.types import Symbol
from parity.source.comments import strip_comments


def strip_leading_template_declaration(line: str) -> str:
    previous = None
    stripped = line.strip()
    while previous != stripped:
        previous = stripped
        stripped = re.sub(r"^template\s*<[^<>]*>\s*", "", stripped).strip()
    return stripped


def split_names(body: str) -> list[str]:
    values = (
        re.sub(r"=.*$", "", re.sub(r"\s+as\s+\w+$", "", raw.strip()))
        .split(":")[-1]
        .strip()
        for raw in body.split(",")
    )
    return [value for value in values if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", value)]


def extract_ts_symbols(path: Path, ts_root: Path) -> list[Symbol]:
    if path.name == "index.ts":
        return []
    rel = relative(path, ts_root)
    text = strip_comments(path.read_text(encoding="utf-8"))
    patterns = (
        (r"\bexport\s+(?:async\s+)?function\s+([A-Za-z_][A-Za-z0-9_]*)", "function"),
        (r"\bexport\s+const\s+([A-Za-z_][A-Za-z0-9_]*)\b", "const"),
        (r"\bexport\s+class\s+([A-Za-z_][A-Za-z0-9_]*)", "class"),
        (r"\bexport\s+interface\s+([A-Za-z_][A-Za-z0-9_]*)", "interface"),
        (r"\bexport\s+type\s+([A-Za-z_][A-Za-z0-9_]*)", "type"),
        (r"\bexport\s+enum\s+([A-Za-z_][A-Za-z0-9_]*)", "enum"),
    )
    symbols = [
        Symbol(name, kind, rel)
        for pattern, kind in patterns
        for name in re.findall(pattern, text)
    ]
    symbols.extend(
        Symbol(name, "const", rel)
        for body in re.findall(r"\bexport\s+const\s+\{(?P<body>.*?)\}\s*=", text, re.S)
        for name in split_names(body)
    )
    symbols.extend(
        Symbol(name, "re-export", rel)
        for body in re.findall(r"\bexport\s+\{(?P<body>.*?)\}", text, re.S)
        for name in split_names(body)
    )
    unique = {f"{item.name}:{item.kind}:{item.file}": item for item in symbols}
    return sorted(unique.values(), key=lambda item: (item.file, item.name))


def _type_symbol(stripped: str, rel: str) -> Symbol | None:
    patterns = (
        (r"(?:USTRUCT\([^)]*\)\s*)?(?:struct|class)\s+(?:FORBOCAI_SDK_API\s+)?([A-Za-z_][A-Za-z0-9_]*)", "type"),
        (r"enum\s+class\s+([A-Za-z_][A-Za-z0-9_]*)", "enum"),
        (r"using\s+([A-Za-z_][A-Za-z0-9_]*)\s*=", "type"),
        (r"typedef\s+.+?\s+([A-Za-z_][A-Za-z0-9_]*)\s*;", "type"),
    )
    match = next(
        ((re.match(pattern, stripped), kind) for pattern, kind in patterns if re.match(pattern, stripped)),
        None,
    )
    return Symbol(match[0].group(1), match[1], rel) if match else None


def _function_symbol(declaration: str, rel: str) -> Symbol | None:
    match = re.match(
        r"(?:FORBOCAI_SDK_API\s+)?(?:virtual\s+)?(?:static\s+)?(?:inline\s+)?"
        r"(?:constexpr\s+)?[A-Za-z_][A-Za-z0-9_:<>*& ,]*?"
        r"(?:\s|[*&])([A-Za-z_][A-Za-z0-9_]*)\s*\(",
        declaration,
    )
    excluded = {"if", "for", "while", "switch", "return"}
    return Symbol(match.group(1), "function", rel) if match and match.group(1) not in excluded else None


def _value_symbol(declaration: str, indent: int, rel: str) -> Symbol | None:
    match = re.match(
        r"(?:FORBOCAI_SDK_API\s+)?(?:static\s+)?(?:constexpr\s+)?(?:const\s+)?(?:inline\s+)?"
        r"(?:[A-Za-z_][A-Za-z0-9_:<>*& ,]*\s+)+([A-Za-z_][A-Za-z0-9_]*)\s*(?:=|;|\{)",
        declaration,
    )
    return Symbol(match.group(1), "const", rel) if indent == 0 and match else None


def extract_ue_symbols(path: Path, root: Path) -> list[Symbol]:
    if path.suffix not in {".h", ".hpp", ".cpp"}:
        return []
    rel = relative(path, root)
    lines = strip_comments(path.read_text(encoding="utf-8")).splitlines()
    symbols: list[Symbol] = []
    for index, line in enumerate(lines):
        indent = len(line) - len(line.lstrip(" "))
        stripped = strip_leading_template_declaration(line)
        ignored = indent > 2 or not stripped or stripped.startswith(("#", "template"))
        ignored = ignored or bool(re.match(r"namespace(?:\s+\w+)?\s*\{", stripped))
        if ignored:
            continue
        type_symbol = _type_symbol(stripped, rel)
        if type_symbol is not None:
            symbols.append(type_symbol)
            continue
        next_line = lines[index + 1] if index + 1 < len(lines) else ""
        next_indent = len(next_line) - len(next_line.lstrip(" "))
        continuation = (
            strip_leading_template_declaration(next_line)
            if "(" not in stripped and next_indent <= 2
            else ""
        )
        declaration = f"{stripped} {continuation}".strip()
        symbol = _function_symbol(declaration, rel) or _value_symbol(declaration, indent, rel)
        if symbol is not None:
            symbols.append(symbol)
    unique = {f"{item.name}:{item.kind}:{item.file}": item for item in symbols}
    return sorted(unique.values(), key=lambda item: (item.file, item.name))
