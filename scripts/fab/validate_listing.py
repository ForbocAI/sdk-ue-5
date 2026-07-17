#!/usr/bin/env python3
"""Validate canonical Fab listing data and generated upload media."""

from __future__ import annotations

import json
from pathlib import Path
import struct
from urllib.parse import urlparse

from render_listing import render_listing


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parents[1]
LISTING_PATH = PROJECT_ROOT / "fab" / "listing.json"
RENDERED_LISTING_PATH = PROJECT_ROOT / "fab" / "listing.md"
PACKAGE_PATH = SCRIPT_DIR / "data" / "package.json"
GENERATED_MEDIA_PATH = PROJECT_ROOT / "fab" / "media" / "generated"
MAX_MEDIA_BYTES = 3 * 1024 * 1024
MEDIA_DIMENSIONS = (1920, 1080)
PREFERRED_TITLE_CHARACTERS = 30
UNSUBSTANTIATED_CLAIMS = ("#1", "best seller", "number one", "market leader")


def require_non_empty(value: object, field: str) -> None:
    """User Story: As a Fab reviewer, I need required listing fields to contain publishable values. Signature: require_non_empty(value: object, field: str) -> None."""
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"Listing field must be a non-empty string: {field}")


def require_unique_strings(value: object, field: str) -> list[str]:
    """User Story: As a buyer, I need concise listing collections without blank or duplicate entries. Signature: require_unique_strings(value: object, field: str) -> list[str]."""
    if not isinstance(value, list) or not value:
        raise ValueError(f"Listing field must be a non-empty array: {field}")
    values = [item.strip() for item in value if isinstance(item, str) and item.strip()]
    if len(values) != len(value):
        raise ValueError(f"Listing field contains a blank or non-string value: {field}")
    if len({item.casefold() for item in values}) != len(values):
        raise ValueError(f"Listing field contains duplicate values: {field}")
    return values


def require_https_url(value: object, field: str) -> None:
    """User Story: As a buyer, I need every support destination to use a complete secure URL. Signature: require_https_url(value: object, field: str) -> None."""
    require_non_empty(value, field)
    parsed = urlparse(str(value))
    if parsed.scheme != "https" or not parsed.netloc:
        raise ValueError(f"Listing field must be a complete HTTPS URL: {field}")


def png_dimensions(path: Path) -> tuple[int, int]:
    """User Story: As release automation, I need listing image dimensions checked without optional dependencies. Signature: png_dimensions(path: Path) -> tuple[int, int]."""
    with path.open("rb") as image:
        header = image.read(24)
    if header[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Fab media must be PNG: {path}")
    return struct.unpack(">II", header[16:24])


def validate_media(media: object) -> None:
    """User Story: As a publisher, I need every declared listing image present and upload-ready. Signature: validate_media(media: object) -> None."""
    if not isinstance(media, list) or len(media) < 3:
        raise ValueError("Fab listing requires at least three declared media images")
    declared: set[str] = set()
    for index, item in enumerate(media):
        if not isinstance(item, dict):
            raise ValueError(f"Fab media entry must be an object: media[{index}]")
        require_non_empty(item.get("file"), f"media[{index}].file")
        require_non_empty(item.get("purpose"), f"media[{index}].purpose")
        relative = str(item["file"])
        if relative in declared:
            raise ValueError(f"Fab media path is declared more than once: {relative}")
        declared.add(relative)
        path = (PROJECT_ROOT / "fab" / relative).resolve()
        if PROJECT_ROOT / "fab" not in path.parents:
            raise ValueError(f"Fab media path escapes the listing directory: {relative}")
        if path.suffix.casefold() != ".png":
            raise ValueError(f"Fab media must use PNG files: {relative}")
        if not path.is_file():
            raise FileNotFoundError(f"Declared Fab media does not exist: {path}")
        if png_dimensions(path) != MEDIA_DIMENSIONS:
            raise ValueError(f"Fab media must be {MEDIA_DIMENSIONS[0]}x{MEDIA_DIMENSIONS[1]}: {path}")
        if path.stat().st_size > MAX_MEDIA_BYTES:
            raise ValueError(f"Fab media exceeds 3 MiB: {path}")
    generated = {
        path.relative_to(PROJECT_ROOT / "fab").as_posix()
        for path in GENERATED_MEDIA_PATH.glob("*.png")
    }
    if generated != declared:
        raise ValueError(
            "Generated Fab media and canonical listing declarations must match exactly",
        )


def validate_pricing(pricing: object) -> None:
    """User Story: As a publisher, I need coherent license tiers before submitting the listing. Signature: validate_pricing(pricing: object) -> None."""
    if not isinstance(pricing, dict):
        raise ValueError("Listing pricing must be an object")
    require_non_empty(pricing.get("license"), "pricing.license")
    personal = pricing.get("personalUsd")
    professional = pricing.get("professionalUsd")
    if not isinstance(personal, (int, float)) or personal <= 0:
        raise ValueError("pricing.personalUsd must be positive")
    if not isinstance(professional, (int, float)) or professional < personal:
        raise ValueError("pricing.professionalUsd must be at least pricing.personalUsd")


def validate_claims(data: dict[str, object]) -> None:
    """User Story: As a publisher, I need marketplace copy free of unverifiable ranking claims. Signature: validate_claims(data: dict[str, object]) -> None."""
    copy = json.dumps(data, ensure_ascii=True).casefold()
    match = next((claim for claim in UNSUBSTANTIATED_CLAIMS if claim in copy), None)
    if match is not None:
        raise ValueError(f"Fab listing contains an unsubstantiated ranking claim: {match}")


def validate_faq(faq: object) -> None:
    """User Story: As a buyer, I need installation and ownership questions answered before purchase. Signature: validate_faq(faq: object) -> None."""
    if not isinstance(faq, list) or len(faq) < 4:
        raise ValueError("Fab listing requires at least four FAQ entries")
    questions: set[str] = set()
    for index, item in enumerate(faq):
        if not isinstance(item, dict):
            raise ValueError(f"FAQ entry must be an object: faq[{index}]")
        require_non_empty(item.get("question"), f"faq[{index}].question")
        require_non_empty(item.get("answer"), f"faq[{index}].answer")
        question = str(item["question"]).casefold()
        if question in questions:
            raise ValueError(f"FAQ question is duplicated: {item['question']}")
        questions.add(question)


def validate_rendered_listing(data: dict[str, object]) -> None:
    """User Story: As a publisher, I need reviewer copy proven current with its canonical JSON source. Signature: validate_rendered_listing(data: dict[str, object]) -> None."""
    if not RENDERED_LISTING_PATH.is_file():
        raise FileNotFoundError(f"Rendered Fab listing does not exist: {RENDERED_LISTING_PATH}")
    if RENDERED_LISTING_PATH.read_text(encoding="utf-8") != render_listing(data):
        raise ValueError("Rendered Fab listing is stale; run render_listing.py")


def validate_product_identity(data: dict[str, object], package: dict[str, object]) -> None:
    """User Story: As a buyer, I need the Fab listing and installed Unreal plugin to use one product name. Signature: validate_product_identity(data: dict[str, object], package: dict[str, object]) -> None."""
    if str(data.get("title")) != str(package.get("productName")):
        raise ValueError("Fab listing title must match the package productName")


def validate_listing(data: dict[str, object], package: dict[str, object]) -> None:
    """User Story: As release automation, I need product copy and package metadata validated as one release contract. Signature: validate_listing(data: dict[str, object], package: dict[str, object]) -> None."""
    for field in ("title", "subtitle"):
        require_non_empty(data.get(field), field)
    if len(str(data["title"])) > PREFERRED_TITLE_CHARACTERS:
        raise ValueError(
            f"Fab listing title exceeds {PREFERRED_TITLE_CHARACTERS} characters",
        )
    require_unique_strings(data.get("category"), "category")
    require_unique_strings(data.get("description"), "description")
    require_unique_strings(data.get("features"), "features")
    require_unique_strings(data.get("tags"), "tags")
    validate_media(data.get("media"))
    validate_pricing(data.get("pricing"))
    technical = data.get("technicalDetails")
    if not isinstance(technical, dict):
        raise ValueError("Listing technicalDetails must be an object")
    for field in (
        "engineVersion",
        "moduleType",
        "network",
        "account",
        "localStorage",
        "dependencies",
    ):
        require_non_empty(technical.get(field), f"technicalDetails.{field}")
    require_unique_strings(
        technical.get("testedPlatforms"),
        "technicalDetails.testedPlatforms",
    )
    if not isinstance(technical.get("codeModules"), int) or technical["codeModules"] <= 0:
        raise ValueError("technicalDetails.codeModules must be a positive integer")
    if str(technical.get("engineVersion")) != str(package.get("engineVersion")):
        raise ValueError("Listing and package engine versions must match")
    disclosures = data.get("contentDisclosures")
    required_disclosures = {
        "mature",
        "allowsUsageWithGenerativeAI",
        "generatedWithGenerativeAI",
    }
    if not isinstance(disclosures, dict) or set(disclosures) != required_disclosures or any(
        not isinstance(value, bool) for value in disclosures.values()
    ):
        raise ValueError("Listing contentDisclosures must contain the exact boolean contract")
    support = data.get("support")
    if not isinstance(support, dict) or not support:
        raise ValueError("Listing support must be a non-empty object")
    for field, value in support.items():
        require_https_url(value, f"support.{field}")
    validate_faq(data.get("faq"))
    validate_rendered_listing(data)
    validate_product_identity(data, package)
    validate_claims(data)


def main() -> int:
    """User Story: As a publisher, I need one command that rejects incomplete Fab release assets. Signature: main() -> int."""
    data = json.loads(LISTING_PATH.read_text(encoding="utf-8"))
    package = json.loads(PACKAGE_PATH.read_text(encoding="utf-8"))
    validate_listing(data, package)
    print(f"Fab listing valid: {LISTING_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
