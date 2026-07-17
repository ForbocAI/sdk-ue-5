#!/usr/bin/env python3
"""Render and validate deterministic Fab listing media with a local browser."""

from __future__ import annotations

import os
from pathlib import Path
import platform
import shutil
import struct
import subprocess
from urllib.parse import quote


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parents[1]
SOURCE_DIR = PROJECT_ROOT / "fab" / "media" / "source"
OUTPUT_DIR = PROJECT_ROOT / "fab" / "media" / "generated"
WIDTH = 1920
HEIGHT = 1080
MAX_BYTES = 3 * 1024 * 1024


def is_wsl() -> bool:
    """User Story: As portable media automation, I need WSL detected explicitly. Signature: is_wsl() -> bool."""
    version = Path("/proc/version")
    return version.exists() and "microsoft" in version.read_text(
        encoding="utf-8",
        errors="ignore",
    ).lower()


def chrome_path() -> Path:
    """User Story: As portable media automation, I need an installed Chromium browser resolved across supported hosts. Signature: chrome_path() -> Path."""
    candidates = [
        os.environ.get("CHROME_PATH", ""),
        shutil.which("google-chrome") or "",
        shutil.which("chromium") or "",
        shutil.which("chromium-browser") or "",
        "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome",
        "/mnt/c/Program Files/Google/Chrome/Application/chrome.exe",
        "C:/Program Files/Google/Chrome/Application/chrome.exe",
    ]
    match = next((Path(value) for value in candidates if value and Path(value).is_file()), None)
    if match is None:
        raise FileNotFoundError("Google Chrome or Chromium was not found; set CHROME_PATH")
    return match


def windows_path(path: Path) -> str:
    """User Story: As WSL media automation, I need Linux paths converted before invoking Windows Chrome. Signature: windows_path(path: Path) -> str."""
    return subprocess.run(
        ["wslpath", "-w", str(path)],
        check=True,
        capture_output=True,
        text=True,
    ).stdout.strip()


def browser_paths(source: Path, output: Path) -> tuple[str, str]:
    """User Story: As portable media automation, I need browser input and output paths normalized for the active host. Signature: browser_paths(source: Path, output: Path) -> tuple[str, str]."""
    if is_wsl():
        source_value = windows_path(source).replace("\\", "/")
        return "file:///" + quote(source_value, safe="/:"), windows_path(output)
    return source.as_uri(), str(output)


def render(source: Path, output: Path, browser: Path) -> None:
    """User Story: As a Fab publisher, I need each authored view rendered at the required listing resolution. Signature: render(source: Path, output: Path, browser: Path) -> None."""
    source_uri, output_path = browser_paths(source, output)
    subprocess.run(
        [
            str(browser),
            "--headless=new",
            "--disable-gpu",
            "--hide-scrollbars",
            "--allow-file-access-from-files",
            "--force-device-scale-factor=1",
            f"--window-size={WIDTH},{HEIGHT}",
            f"--screenshot={output_path}",
            source_uri,
        ],
        check=True,
    )


def png_dimensions(path: Path) -> tuple[int, int]:
    """User Story: As a Fab validator, I need PNG dimensions read without optional image dependencies. Signature: png_dimensions(path: Path) -> tuple[int, int]."""
    with path.open("rb") as image:
        header = image.read(24)
    if header[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Generated media is not a PNG: {path}")
    return struct.unpack(">II", header[16:24])


def validate(path: Path) -> None:
    """User Story: As a Fab publisher, I need generated media rejected when dimensions or size violate upload policy. Signature: validate(path: Path) -> None."""
    dimensions = png_dimensions(path)
    if dimensions != (WIDTH, HEIGHT):
        raise ValueError(f"Expected {WIDTH}x{HEIGHT}, received {dimensions}: {path}")
    if path.stat().st_size > MAX_BYTES:
        raise ValueError(f"Generated media exceeds 3 MiB: {path}")


def main() -> int:
    """User Story: As a publisher, I need all Fab media regenerated and validated through one command. Signature: main() -> int."""
    browser = chrome_path()
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    sources = sorted(SOURCE_DIR.glob("*.html"))
    if not sources:
        raise FileNotFoundError(f"No Fab media sources found in {SOURCE_DIR}")
    expected = {source.with_suffix(".png").name for source in sources}
    for stale in OUTPUT_DIR.glob("*.png"):
        if stale.name not in expected:
            stale.unlink()
    for source in sources:
        output = OUTPUT_DIR / source.with_suffix(".png").name
        render(source, output, browser)
        validate(output)
        print(f"Generated {output.relative_to(PROJECT_ROOT)} ({output.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
