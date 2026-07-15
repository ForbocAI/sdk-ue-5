#!/usr/bin/env python3
"""Reject hard-coded values in authored UE Source.

Numbers and strings that configure authored behavior belong in JSON under each
target's Content/Data root -- loaded through the settings adapters and read
from typed settings/profile structs -- not baked into C++ or headers. This
guard scans every UE target discovered by the SDK-owned check target registry
and rejects every numeric literal and every string literal in authored code.

Action/slice/reducer metadata strings (action type strings, slice names, thunk
type prefixes, listener action types) are discovered dynamically from the RTK
creator calls in this codebase. They are code metadata, not authored settings
data. Comments, preprocessor lines, and Unreal reflection macro arguments are
blanked before scanning because they are compiler/UHT/editor metadata. Test
registration strings and assertion labels are scanned; they are authored test
data and must live in JSON just like other authored fixtures.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

from ue_targets import ue_targets


SOURCE_SUFFIXES = {".h", ".hpp", ".cpp"}
SCRIPT_ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_ROOT.parent

STRING_LITERAL = re.compile(r'"(?:[^"\\]|\\.)*"')
NUMERIC_LITERAL = re.compile(r"(?<![A-Za-z0-9_.])\d[0-9.]*")
UNREAL_REFLECTION_MACROS = {
    "UCLASS",
    "UDELEGATE",
    "UENUM",
    "UFUNCTION",
    "UINTERFACE",
    "UMETA",
    "UPARAM",
    "UPROPERTY",
    "USTRUCT",
}
UNREAL_REFLECTION_MACRO = re.compile(
    r"\b(" + "|".join(sorted(UNREAL_REFLECTION_MACROS)) + r")\s*\("
)
JSON_SCHEMA_FIELD_CALL = re.compile(
    r"\b(?:[A-Za-z_][A-Za-z0-9_]*::)?(?:"
    r"Field|ReadBooleanField|ReadNumberField|ReadStringField|"
    r"ReadStringArrayField|ReadArrayField|ReadObjectField|ReadObjectArrayField|"
    r"ReadRequiredField|RequiredField|"
    r"RequiredFieldName|FindRequiredJsonField|ReadSettingsObjectArrayField|"
    r"SettingsSource|SettingsSourceKey|SettingsFieldName"
    r")\s*(?:<[^;{}()]*>)?\s*\("
)
SETTINGS_SOURCE_GROUPS = re.compile(
    r"\bSettingsSourceGroups\s*\("
)
ECS_SCHEMA_CALL = re.compile(
    r"\b(?:"
    r"ComponentAtom|ComponentDomain|ComponentDomains|ComponentSourceValueMap|"
    r"ComponentValueMap|MakeBotProjectionRequest|RegisteredComponentGroups"
    r")\s*(?:<[^;{}()]*>)?\s*\("
)
SCHEMA_REGISTRY_STRUCT = re.compile(
    r"\b(?:TComponentSourceValueFieldRegistry|TComponentTextRegistry|"
    r"TJsonTextValueRegistry)\s*<[^;{}]+>\s*\{"
)

# Where reducer/action metadata is defined. A string in the same statement as
# one of these (or in a `*TypePrefix` accessor) is action/slice/thunk metadata,
# not authored configuration. The values are collected dynamically from real
# usage below; this only identifies the definition sites.
METADATA_CONTEXT = re.compile(
    r"create(?:Action|Slice|Reducer|AsyncThunk)|addListener|TypePrefix"
)

GUIDANCE = (
    "Move the value into authored JSON under the target's Content/Data root, "
    "load it through the settings adapters, and read it from a typed "
    "settings/profile struct. Only action type strings, slice names, and thunk "
    "type prefixes may remain in code as reducer/action metadata "
    "(auto-detected from RTK usage)."
)
VIOLATION_PREFIX = "Authored-data violation, forward target: "


def blank_preprocessor(text: str) -> str:
    """Replace preprocessor directive lines with spaces, preserving offsets."""
    return "\n".join(
        (" " * len(line)) if line.lstrip().startswith("#") else line
        for line in text.split("\n")
    )


def blank_comments_and_chars(text: str) -> str:
    """Blank comments and char literals, keeping string literals and code.

    Every replacement preserves length and newlines so offsets still map to the
    original line/column.
    """
    result: list[str] = []
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            result.append("\n" if char == "\n" else " ")
            state = "code" if char == "\n" else state
            index += 1
            continue
        if state == "block_comment":
            end = char == "*" and nxt == "/"
            result.append("  " if end else ("\n" if char == "\n" else " "))
            index += 2 if end else 1
            state = "code" if end else state
            continue
        if state == "string":
            escaped = char == "\\"
            result.append(char)
            result.append(nxt) if escaped else None
            state = "code" if (char == quote and not escaped) else state
            index += 2 if escaped else 1
            continue
        if state == "char":
            escaped = char == "\\"
            result.append("  " if escaped else ("'" if char == "'" else " "))
            state = "code" if (char == "'" and not escaped) else state
            index += 2 if escaped else 1
            continue
        if char == "/" and nxt == "/":
            result.extend("  ")
            state = "line_comment"
            index += 2
            continue
        if char == "/" and nxt == "*":
            result.extend("  ")
            state = "block_comment"
            index += 2
            continue
        if char == '"':
            quote = char
            state = "string"
            result.append(char)
            index += 1
            continue
        if char == "'":
            state = "char"
            result.append("'")
            index += 1
            continue
        result.append(char)
        index += 1
    return "".join(result)


def blank_unreal_reflection_metadata(text: str) -> str:
    """Blank Unreal reflection macro calls, preserving offsets and newlines."""
    result = list(text)
    cursor = 0
    while True:
        match = UNREAL_REFLECTION_MACRO.search(text, cursor)
        if not match:
            break

        index = match.end() - 1
        depth = 0
        state = "code"
        quote = ""
        while index < len(text):
            char = text[index]
            nxt = text[index + 1] if index + 1 < len(text) else ""
            if state == "string":
                escaped = char == "\\"
                state = "code" if (char == quote and not escaped) else state
                index += 2 if escaped else 1
                continue
            if state == "char":
                escaped = char == "\\"
                state = "code" if (char == "'" and not escaped) else state
                index += 2 if escaped else 1
                continue
            if char == '"':
                quote = char
                state = "string"
                index += 1
                continue
            if char == "'":
                state = "char"
                index += 1
                continue
            if char == "(":
                depth += 1
            elif char == ")":
                depth -= 1
                if depth == 0:
                    index += 1
                    break
            index += 1

        for offset in range(match.start(), index):
            result[offset] = "\n" if text[offset] == "\n" else " "
        cursor = index
    return "".join(result)


def call_end(text: str, open_paren: int) -> int:
    index = open_paren
    depth = 0
    state = "code"
    quote = ""
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "string":
            escaped = char == "\\"
            state = "code" if (char == quote and not escaped) else state
            index += 2 if escaped else 1
            continue
        if state == "char":
            escaped = char == "\\"
            state = "code" if (char == "'" and not escaped) else state
            index += 2 if escaped else 1
            continue
        if char == '"':
            quote = char
            state = "string"
            index += 1
            continue
        if char == "'":
            state = "char"
            index += 1
            continue
        if char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
            if depth == 0:
                return index + 1
        index += 1
    return len(text)


def blank_string_literals_in_range(result: list[str], text: str,
                                   start: int, end: int) -> None:
    for match in STRING_LITERAL.finditer(text, start, end):
        for offset in range(match.start(), match.end()):
            result[offset] = "\n" if text[offset] == "\n" else " "


def body_end(text: str, open_brace: int) -> int:
    index = open_brace
    depth = 0
    state = "code"
    quote = ""
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "string":
            escaped = char == "\\"
            state = "code" if (char == quote and not escaped) else state
            index += 2 if escaped else 1
            continue
        if state == "char":
            escaped = char == "\\"
            state = "code" if (char == "'" and not escaped) else state
            index += 2 if escaped else 1
            continue
        if char == '"':
            quote = char
            state = "string"
            index += 1
            continue
        if char == "'":
            state = "char"
            index += 1
            continue
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index + 1
        index += 1
    return len(text)


def blank_settings_source_group_strings(result: list[str],
                                        text: str) -> None:
    cursor = 0
    while True:
        match = SETTINGS_SOURCE_GROUPS.search(text, cursor)
        if not match:
            break
        close = call_end(text, match.end() - 1)
        open_brace = text.find("{", close)
        if open_brace == -1:
            cursor = close
            continue
        close_brace = body_end(text, open_brace)
        blank_string_literals_in_range(result, text, open_brace, close_brace)
        cursor = close_brace


def blank_schema_registry_struct_strings(result: list[str], text: str) -> None:
    cursor = 0
    while True:
        match = SCHEMA_REGISTRY_STRUCT.search(text, cursor)
        if not match:
            break
        close_brace = body_end(text, match.end() - 1)
        blank_string_literals_in_range(result, text, match.start(),
                                       close_brace)
        cursor = close_brace


def blank_schema_call_strings(result: list[str], text: str) -> None:
    cursor = 0
    while True:
        match = ECS_SCHEMA_CALL.search(text, cursor)
        if not match:
            break
        close = call_end(text, match.end() - 1)
        blank_string_literals_in_range(result, text, match.start(), close)
        cursor = close


def blank_json_schema_metadata(text: str) -> str:
    """Blank schema identifiers while preserving authored JSON-backed values."""
    result = list(text)

    cursor = 0
    while True:
        match = JSON_SCHEMA_FIELD_CALL.search(text, cursor)
        if not match:
            break
        close = call_end(text, match.end() - 1)
        blank_string_literals_in_range(result, text, match.start(), close)
        cursor = close

    blank_settings_source_group_strings(result, text)
    blank_schema_registry_struct_strings(result, text)
    blank_schema_call_strings(result, text)
    return "".join(result)


def line_column(text: str, offset: int) -> tuple[int, int]:
    line_start = text.rfind("\n", 0, offset) + 1
    return text.count("\n", 0, offset) + 1, offset - line_start + 1


def collect_metadata_strings(scannables: list[str]) -> set[str]:
    """Discover action/slice/thunk metadata string values from real RTK usage."""
    metadata_strings: set[str] = set()
    for scannable in scannables:
        for match in STRING_LITERAL.finditer(scannable):
            preceding = scannable[max(0, match.start() - 200): match.start()]
            if METADATA_CONTEXT.search(preceding):
                metadata_strings.add(match.group(0)[1:-1])
    return metadata_strings


def scan_file(
    path: Path, scannable: str, metadata_strings: set[str]
) -> list[tuple[Path, int, int, str, str]]:
    violations: list[tuple[Path, int, int, str, str]] = []

    code_without_strings = list(scannable)
    for match in STRING_LITERAL.finditer(scannable):
        value = match.group(0)[1:-1]
        for index in range(match.start(), match.end()):
            code_without_strings[index] = " "
        if value == "" or value in metadata_strings:
            continue
        line, column = line_column(scannable, match.start())
        violations.append((path, line, column, "string", match.group(0)))

    code_only = "".join(code_without_strings)
    for match in NUMERIC_LITERAL.finditer(code_only):
        line, column = line_column(code_only, match.start())
        violations.append((path, line, column, "number", match.group(0)))

    return violations


def target_source_roots() -> tuple[Path, ...]:
    return tuple(target.root / "Source" for target in ue_targets())


def missing_target_data_roots() -> list[Path]:
    return [
        target.root / "Content" / "Data"
        for target in ue_targets()
        if not (target.root / "Content" / "Data").is_dir()
    ]


def iter_source_paths() -> list[Path]:
    return [
        source
        for source_root in target_source_roots()
        for source in sorted(source_root.rglob("*"))
        if source.is_file() and source.suffix in SOURCE_SUFFIXES
    ]


def scan_paths() -> list[tuple[Path, int, int, str, str]]:
    scannables = {}
    for path in iter_source_paths():
        scannable = blank_unreal_reflection_metadata(
            blank_json_schema_metadata(
                blank_comments_and_chars(
                    blank_preprocessor(
                        path.read_text(encoding="utf-8", errors="replace")
                    )
                )
            )
        )
        scannables[path] = scannable
    metadata_strings = collect_metadata_strings(list(scannables.values()))
    return [
        violation
        for path, scannable in scannables.items()
        for violation in scan_file(path, scannable, metadata_strings)
    ]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, add_help=False)
    parser.add_argument(
        "-h",
        "--help",
        action="store_true",
        help="Show this help message after the required Source scan passes.",
    )
    args = parser.parse_args()

    source_roots = target_source_roots()
    if not source_roots:
        print("Hard-coded value guard failed:")
        print(f"{PROJECT_ROOT}: no UE target Source roots were discovered")
        if args.help:
            print("")
            parser.print_help()
        return 1

    missing_data_roots = missing_target_data_roots()
    if missing_data_roots:
        print("Hard-coded value guard failed:")
        for missing_data_root in missing_data_roots:
            print(f"{missing_data_root}: data root does not exist")
        print("")
        print(GUIDANCE)
        if args.help:
            print("")
            parser.print_help()
        return 1

    violations = scan_paths()
    if not violations:
        if args.help:
            parser.print_help()
            return 0
        print("Hard-coded value guard passed.")
        return 0

    print(f"Hard-coded value guard failed: {len(violations)} value(s).")
    for path, line, column, kind, token in violations:
        print(
            f"{path}:{line}:{column}: {VIOLATION_PREFIX}"
            f"move hard-coded {kind} `{token}` into authored JSON and read it "
            "through typed settings/profile state."
        )
    print("")
    print(GUIDANCE)
    if args.help:
        print("")
        parser.print_help()
    return 1


if __name__ == "__main__":
    sys.exit(main())
