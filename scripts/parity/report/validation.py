from __future__ import annotations

import json
from pathlib import Path
from typing import Any


def _type_matches(expected: str, value: Any) -> bool:
    matches = {
        "array": lambda item: isinstance(item, list),
        "boolean": lambda item: isinstance(item, bool),
        "integer": lambda item: isinstance(item, int) and not isinstance(item, bool),
        "null": lambda item: item is None,
        "object": lambda item: isinstance(item, dict),
        "string": lambda item: isinstance(item, str),
    }
    predicate = matches.get(expected)
    return bool(predicate and predicate(value))


def _resolve_pointer(document: dict[str, Any], pointer: str) -> dict[str, Any]:
    value: Any = document
    for token in pointer.removeprefix("#/").split("/"):
        value = value[token.replace("~1", "/").replace("~0", "~")]
    return value


def _resolve_reference(
    reference: str,
    schema: dict[str, Any],
    schema_path: Path,
) -> tuple[dict[str, Any], dict[str, Any], Path]:
    file_name, separator, pointer = reference.partition("#")
    target_path = schema_path if not file_name else schema_path.parent / file_name
    target = schema if not file_name else json.loads(target_path.read_text(encoding="utf-8"))
    resolved = _resolve_pointer(target, f"#/{pointer.removeprefix('/')}") if separator else target
    return resolved, target, target_path


def _validate_object(
    rule: dict[str, Any],
    value: dict[str, Any],
    path: str,
    schema: dict[str, Any],
    schema_path: Path,
) -> list[str]:
    required = set(rule.get("required", []))
    properties = rule.get("properties", {})
    errors = [f"{path}: missing required property {key}" for key in sorted(required - value.keys())]
    errors.extend(
        error
        for key, child in value.items()
        if key in properties
        for error in _validate_rule(
            properties[key], child, f"{path}.{key}", schema, schema_path
        )
    )
    additional = rule.get("additionalProperties", True)
    unknown = sorted(value.keys() - properties.keys())
    if additional is False:
        errors.extend(f"{path}: unexpected property {key}" for key in unknown)
    elif isinstance(additional, dict):
        errors.extend(
            error
            for key in unknown
            for error in _validate_rule(
                additional, value[key], f"{path}.{key}", schema, schema_path
            )
        )
    return errors


def _validate_rule(
    rule: dict[str, Any],
    value: Any,
    path: str,
    schema: dict[str, Any],
    schema_path: Path,
) -> list[str]:
    if "$ref" in rule:
        resolved, target, target_path = _resolve_reference(
            str(rule["$ref"]), schema, schema_path
        )
        return _validate_rule(resolved, value, path, target, target_path)
    if "anyOf" in rule:
        alternatives = [
            _validate_rule(option, value, path, schema, schema_path)
            for option in rule["anyOf"]
        ]
        return [] if any(not errors for errors in alternatives) else alternatives[0]
    if "enum" in rule and value not in rule["enum"]:
        return [f"{path}: {value!r} is outside the declared enum"]
    expected = rule.get("type")
    if expected and not _type_matches(str(expected), value):
        return [f"{path}: expected {expected}, received {type(value).__name__}"]
    errors: list[str] = []
    if expected == "object":
        errors.extend(_validate_object(rule, value, path, schema, schema_path))
    if expected == "array" and "items" in rule:
        errors.extend(
            error
            for index, item in enumerate(value)
            for error in _validate_rule(
                rule["items"], item, f"{path}[{index}]", schema, schema_path
            )
        )
    if expected == "integer" and "minimum" in rule and value < rule["minimum"]:
        errors.append(f"{path}: {value} is below minimum {rule['minimum']}")
    return errors


def validate_document(schema_path: Path, document: dict[str, Any]) -> list[str]:
    schema = json.loads(schema_path.read_text(encoding="utf-8"))
    return _validate_rule(schema, document, "$", schema, schema_path)
