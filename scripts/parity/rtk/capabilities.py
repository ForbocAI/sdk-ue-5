from __future__ import annotations

from collections.abc import Mapping, Sequence

from parity.report.model import Capability, capability, endpoint_evidence, mismatch
from parity.rtk.types import ExportGroup


def export_capability(
    group: ExportGroup,
    name: str,
    ue_index: Mapping[str, Sequence[str]],
) -> Capability:
    ue_paths = ue_index.get(name, ())
    available = bool(ue_paths)
    return capability(
        f"rtk.{group.identifier}.{name}",
        "rtk-export",
        "behavioral",
        "same" if available else "mismatch",
        endpoint_evidence(True, (*group.evidence, name)),
        endpoint_evidence(available, ue_paths),
        None
        if available
        else mismatch(
            f"UE rtk.hpp does not expose {name} from {group.label}.",
            "Add the equivalent typed UE public primitive to the rtk.hpp include closure and verify its behavior.",
        ),
    )


def build_capability_groups(
    groups: Sequence[ExportGroup],
    ue_index: Mapping[str, Sequence[str]],
) -> dict[str, list[Capability]]:
    return {
        group.identifier: [export_capability(group, name, ue_index) for name in group.names]
        for group in groups
    }
