from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
import sys


SCRIPTS_ROOT = Path(__file__).resolve().parents[1]
if str(SCRIPTS_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_ROOT))

from parity.report.contracts import ReportDefinition
from parity.report.model import capability, endpoint_evidence
from parity.report.writer import write_report_set


class ReportWriterTests(unittest.TestCase):
    def test_writes_schema_valid_bounded_byte_stable_parts(self) -> None:
        repository = Path(__file__).resolve().parents[2]
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "generated"
            definition = ReportDefinition(
                schema_version=1,
                max_capabilities_per_part=2,
                report_id="parity-test",
                generated_by="test_report_writer.py",
                directory=output,
                index_schema=repository / "reports/parity/report-index.schema.json",
                part_schema=repository / "reports/parity/report-part.schema.json",
            )
            evidence = endpoint_evidence(True, ("contract.json",))
            capabilities = [
                capability(
                    f"test.capability.{index}",
                    "test",
                    "behavioral",
                    "same",
                    evidence,
                    evidence,
                    None,
                )
                for index in range(3)
            ]
            sources = {
                "typescript": {"repository": "sdk", "commit": "ts", "contractHashes": {}},
                "unreal": {"repository": "sdk-ue-5", "commit": "ue", "contractHashes": {}},
            }

            write_report_set(definition, sources, {"commands": capabilities})
            first = {
                path.relative_to(output): path.read_bytes()
                for path in sorted(output.rglob("*.json"))
            }
            write_report_set(definition, sources, {"commands": capabilities})
            second = {
                path.relative_to(output): path.read_bytes()
                for path in sorted(output.rglob("*.json"))
            }

            self.assertEqual(first, second)
            self.assertEqual(len(first), 4)
            self.assertTrue(all(data.endswith(b"\n") for data in first.values()))


if __name__ == "__main__":
    unittest.main()
