#!/usr/bin/env python3
"""Meta-test for the ECS naming/domain guards.

Each case asserts a rule fires on a bad input and stays silent on a good one, so
a rule that silently stops matching is caught. This tests the LINTER, not the
repo: it is never wired into run-tests and is not a substitute for scanning
Content/Data + Source. Run it directly:

    python3 Scripts/Checks/naming_fixtures.py
"""

from __future__ import annotations

from pathlib import Path
import sys
import tempfile

# The ECS engine (check_ecs) lives one level up in scripts/; put it on the path.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

import data_naming as dat
import domain_boundaries as dom


def rid(finding) -> str | None:
    return finding.rule_id if finding else None


def main() -> int:
    role_root = dom.SOURCE_ROLE_ROOTS[0] if dom.SOURCE_ROLE_ROOTS else dom.DEFAULT_SOURCE_ROLE_ROOT
    views = dom.SOURCE_VIEWS_ROOTS[0] if dom.SOURCE_VIEWS_ROOTS else dom.SOURCE_VIEWS_ROOT
    data = dom.CONTENT_DATA_ROOT
    location = frozenset({"french_gulch"})
    failures: list[str] = []

    cases: list[tuple[str, str | None, str | None]] = [
        ("domain-src-qualified", rid(dom.source_finding(role_root / "Demo" / "DemoActions.h")), None),
        ("domain-src-bare", rid(dom.source_finding(role_root / "Demo" / "Actions.h")), "DOMAIN-SRC-001"),
        ("domain-src-bad-prefix", rid(dom.source_finding(role_root / "Demo" / "FooActions.h")), "DOMAIN-SRC-001"),
        ("domain-src-clean", rid(dom.source_finding(role_root / "Rendering" / "Sky" / "SkyThunks.h")), None),
        ("domain-view", rid(dom.view_finding(views / "FooBarView.cpp", set())), "DOMAIN-VIEW-001"),
        ("domain-view-qualified", rid(dom.view_finding(views / "Player" / "Controller" / "ControllerView.cpp", set())), None),
        ("domain-view-bare", rid(dom.view_finding(views / "Player" / "Controller" / "View.cpp", set())), "DOMAIN-VIEW-001"),
        ("domain-data-ecs", rid(dom.data_ecs_finding(data / "components_foo.json")), "DOMAIN-DATA-002"),
        ("data-catalog-order", rid(dat.catalog_order_finding(data / "french_gulch" / "roads" / "x.json", location)), "DATA-NAME-001"),
        ("data-wide-sections", rid(dat.wide_sections_finding(data / "x.json", {"components": {}, "systems": []}, frozenset())), "DATA-NAME-004"),
        ("data-section-folder", rid(dat.section_folder_finding(data / "components" / "foo.json", {"systems": [{"a": 1}]})), "DATA-NAME-008"),
        ("data-section-folder-ok", rid(dat.section_folder_finding(data / "components" / "foo.json", {"components": [{"a": 1}]})), None),
        ("data-deep-wrap", rid(dat.deep_wrap_finding(data / "x.json", {"a": {"b": {"c": 1}}})), "DATA-NAME-009"),
        ("data-deep-wrap-ok", rid(dat.deep_wrap_finding(data / "x.json", {"a": {"b": [1]}})), None),
    ]
    for name, got, want in cases:
        if got != want:
            failures.append(f"{name}: got {got}, want {want}")

    folder = dom.folder_redundancy_findings([role_root / "Rendering" / "Rendering" / "Slice.h"])
    if not (folder and folder[0].rule_id == "DOMAIN-FOLDER-001"):
        failures.append("domain-folder-redundant: expected DOMAIN-FOLDER-001")

    with tempfile.TemporaryDirectory() as tmp:
        bad = Path(tmp) / "bad.json"
        bad.write_text("{ not: json", encoding="utf-8")
        _value, error = dat.load_json(bad)
        if error is None:
            failures.append("data-unparseable: expected a parse error to report DATA-NAME-007")

    total = len(cases) + 2
    if failures:
        print(f"Naming fixtures FAILED: {len(failures)} case(s).")
        for failure in failures:
            print(f"  - {failure}")
        return 1
    print(f"Naming fixtures passed: {total} checks.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
