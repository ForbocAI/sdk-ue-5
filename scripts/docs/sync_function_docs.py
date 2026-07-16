#!/usr/bin/env python3
"""Synchronize Doxygen user stories and exact function signatures in every UE target."""

from __future__ import annotations

from pathlib import Path
import re
import sys


SCRIPT_DIR = Path(__file__).resolve().parent
SCRIPTS_DIR = SCRIPT_DIR.parent
sys.path.insert(0, str(SCRIPT_DIR))
sys.path.insert(0, str(SCRIPTS_DIR))

from cpp_function_docs import (  # noqa: E402
    FunctionDocTarget,
    attached_doxygen,
    collect_function_targets,
    documented_signature,
    first_party_cpp_files,
    generated_user_story,
    read_cpp_source,
)
from ue_targets import ue_source_roots  # noqa: E402


SIGNATURE_TAG = re.compile(r"(?:^|\s)@(fn|signature)\b")


def _comment_content(documentation: str) -> list[str]:
    body = documentation[3:-2]
    lines = body.splitlines() or [body]
    content: list[str] = []
    for line in lines:
        value = re.sub(r"^[ \t]*\*[ \t]?", "", line).strip()
        content.append(value)
    while content and not content[0]:
        content.pop(0)
    while content and not content[-1]:
        content.pop()
    return content


def _without_signature_tags(content: list[str]) -> list[str]:
    output: list[str] = []
    index = 0
    while index < len(content):
        value = content[index].strip()
        tag = SIGNATURE_TAG.search(value)
        if tag is None:
            output.append(content[index])
            index += 1
            continue
        prefix = value[: tag.start()].rstrip()
        if prefix:
            output.append(prefix)
        declaration = value[tag.end() :].strip()
        balance = declaration.count("(") - declaration.count(")")
        index += 1
        while balance > 0 and index < len(content):
            balance += content[index].count("(") - content[index].count(")")
            index += 1
    return output


def _line_indent(source: str, position: int) -> str:
    line_start = source.rfind("\n", 0, position) + 1
    return re.match(r"[ \t]*", source[line_start:position]).group(0)


def _newline(source: str) -> str:
    return "\r\n" if source.count("\r\n") > source.count("\n") // 2 else "\n"


def _render_existing_documentation(
    source: str,
    start: int,
    documentation: str,
    target: FunctionDocTarget,
) -> str:
    content = _without_signature_tags(_comment_content(documentation))
    if not any("User Story:" in line for line in content):
        content.append(generated_user_story(target.path, target.name))
    insertion = next(
        (index for index, line in enumerate(content) if line.lstrip().startswith("@")),
        len(content),
    )
    content.insert(insertion, f"@fn {target.signature}")
    indent = _line_indent(source, start)
    newline = _newline(source)
    body = newline.join(f"{indent} *{(' ' + line) if line else ''}" for line in content)
    return f"/**{newline}{body}{newline}{indent} */"


def _render_new_documentation(source: str, target: FunctionDocTarget) -> str:
    story = generated_user_story(target.path, target.name)
    return f"/** {story} @fn {target.signature} */{_newline(source)}{_line_indent(source, target.start)}"


def synchronize_file(path: Path) -> int:
    source = read_cpp_source(path)
    changes = 0
    for target in reversed(collect_function_targets(path)):
        attached = attached_doxygen(source, target.start)
        if attached is None:
            replacement = _render_new_documentation(source, target)
            source = source[: target.start] + replacement + source[target.start :]
            changes += 1
            continue
        start, end = attached
        documentation = source[start:end]
        if (
            "User Story:" in documentation
            and documented_signature(documentation) == target.signature
            and "@signature" not in documentation
        ):
            continue
        replacement = _render_existing_documentation(
            source,
            start,
            documentation,
            target,
        )
        if replacement != documentation:
            source = source[:start] + replacement + source[end:]
            changes += 1
    if changes:
        path.write_bytes(source.encode("utf-8"))
    return changes


def main() -> int:
    files = first_party_cpp_files(ue_source_roots())
    changed_files = 0
    changed_functions = 0
    for path in files:
        changes = synchronize_file(path)
        if changes:
            changed_files += 1
            changed_functions += changes
            print(f"updated {path}: {changes} function documentation block(s)")
    print(
        "Function documentation synchronization complete: "
        f"{changed_functions} block(s) across {changed_files} of {len(files)} C++ files."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
