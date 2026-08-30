#!/usr/bin/env python3
"""Regression tests for C++ function documentation discovery."""

from __future__ import annotations

import json
from pathlib import Path
import sys
import tempfile
import unittest

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

from cpp_function_docs import (
    attached_doxygen,
    collect_function_targets,
    generated_user_story,
)


STORY_CASES_PATH = SCRIPT_DIR / "data" / "tests" / "user-stories.json"


class FunctionTargetTests(unittest.TestCase):
    def targets(self, source: str) -> list[tuple[str, str]]:
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "Target.h"
            path.write_text(source, encoding="utf-8")
            return [
                (target.name, target.signature)
                for target in collect_function_targets(path)
            ]

    def test_discovers_declarations_definitions_and_operators(self) -> None:
        targets = self.targets(
            """
            int32 SelectValue(const FState& State);
            inline bool operator==(const FValue& Left, const FValue& Right) { return true; }
            struct FProject { int32 operator()(const FValue& Value) const { return 0; } };
            FThing::FThing(const FString& Name) : Name(Name) {}
            """
        )
        self.assertEqual(
            targets,
            [
                ("SelectValue", "int32 SelectValue(const FState& State)"),
                ("operator==", "inline bool operator==(const FValue& Left, const FValue& Right)"),
                ("operator()", "int32 operator()(const FValue& Value) const"),
                ("FThing", "FThing::FThing(const FString& Name)"),
            ],
        )

    def test_rejects_reflection_macros_and_callable_data(self) -> None:
        targets = self.targets(
            """
            USTRUCT()
            struct FValue {
                GENERATED_BODY()
                UPROPERTY(EditAnywhere)
                FString Name;
                TFunction<FString(const FString&)> Read;
            };
            using FReader = TFunction<FString(const FString&)>;
            typedef TFunction<void(const FString&)> FWriter;
            """
        )
        self.assertEqual(targets, [])

    def test_rejects_calls_and_constructor_initializers(self) -> None:
        targets = self.targets(
            """
            FThing::FThing(const FString& Name)
                : Name(Normalize(Name)), Value(BuildValue(Name))
            {
                ApplyValue(Value);
            }
            """
        )
        self.assertEqual(
            targets,
            [("FThing", "FThing::FThing(const FString& Name)")],
        )

    def test_macro_before_definition_does_not_hide_function_body(self) -> None:
        targets = self.targets(
            """
            DEFINE_SPEC(FValueSpec, "Value", ProductFilter)
            void FValueSpec::Define() {
                Describe("group", [this]() { It("case", []() { CheckValue(); }); });
            }
            """
        )
        self.assertEqual(targets, [("Define", "void FValueSpec::Define()")])

    def test_preprocessor_lines_do_not_hide_first_function(self) -> None:
        targets = self.targets(
            """
            #pragma once
            #include "Value.h"
            void FValue::Apply() {
                Super::Apply();
                Resolve(BuildValue());
            }
            DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChanged, FString, Value)
            """
        )
        self.assertEqual(targets, [("Apply", "void FValue::Apply()")])

    def test_template_defaults_are_part_of_function_signature(self) -> None:
        targets = self.targets(
            """
            template <typename State = FState, typename Result = FString>
            Result Select(const State& StateValue) { return Result(); }
            """
        )
        self.assertEqual(
            targets,
            [
                (
                    "Select",
                    "template <typename State = FState, typename Result = FString> Result Select(const State& StateValue)",
                )
            ],
        )

    def test_doxygen_before_ufunction_is_attached(self) -> None:
        source = """/** Existing story. */
        UFUNCTION(BlueprintCallable, meta = (DisplayName = "Apply"))
        void Apply();
        """
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "Target.h"
            path.write_text(source, encoding="utf-8")
            target = collect_function_targets(path)[0]
        self.assertEqual(attached_doxygen(source, target.start), (0, 22))

    def test_generated_stories_preserve_operator_state_intent(self) -> None:
        cases = json.loads(STORY_CASES_PATH.read_text(encoding="utf-8"))["cases"]
        actual = [
            generated_user_story(Path(case["path"]), case["name"])
            for case in cases
        ]
        self.assertEqual(actual, [case["expected"] for case in cases])


if __name__ == "__main__":
    unittest.main()
