#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Components/Testing/API/Codec/Ghost/CodecGhostTypes.h"
#include "Systems/Testing/API/Codec/Ghost/Attribution/AttributionAdapters.h"

namespace Testing::API::Codec::Ghost {

/** User Story: As Ghost evidence tests, I need every expected counter independently read from authored JSON. @fn inline FGhostEvidenceSummary ReadGhostEvidenceExpected(const TSharedRef<FJsonObject> &Object) */
inline FGhostEvidenceSummary
ReadGhostEvidenceExpected(const TSharedRef<FJsonObject> &Object) {
  FGhostEvidenceSummary Expected;
  Expected.CompletedProcessTurns = DataAdapters::ReadNumberField(
      Object, TEXT("completedProcessTurns"));
  Expected.ValidCognitionOutcomes = DataAdapters::ReadNumberField(
      Object, TEXT("validCognitionOutcomes"));
  Expected.InvalidCognitionOutcomes = DataAdapters::ReadNumberField(
      Object, TEXT("invalidCognitionOutcomes"));
  Expected.SignedCognitionOutcomes = DataAdapters::ReadNumberField(
      Object, TEXT("signedCognitionOutcomes"));
  Expected.ThoughtResults =
      DataAdapters::ReadNumberField(Object, TEXT("thoughtResults"));
  Expected.ReasoningResults =
      DataAdapters::ReadNumberField(Object, TEXT("reasoningResults"));
  Expected.ConsistentReasoningResults = DataAdapters::ReadNumberField(
      Object, TEXT("consistentReasoningResults"));
  Expected.DiagnosticResults =
      DataAdapters::ReadNumberField(Object, TEXT("diagnosticResults"));
  Expected.GroundedDiagnosticResults = DataAdapters::ReadNumberField(
      Object, TEXT("groundedDiagnosticResults"));
  Expected.Contradictions =
      DataAdapters::ReadNumberField(Object, TEXT("contradictions"));
  Expected.UnsupportedClaims =
      DataAdapters::ReadNumberField(Object, TEXT("unsupportedClaims"));
  Expected.Failures =
      DataAdapters::ReadNumberField(Object, TEXT("failures"));
  Expected.Timeouts =
      DataAdapters::ReadNumberField(Object, TEXT("timeouts"));
  Expected.UniqueObservations =
      DataAdapters::ReadNumberField(Object, TEXT("uniqueObservations"));
  Expected.ChoicePoints =
      DataAdapters::ReadNumberField(Object, TEXT("choicePoints"));
  return Expected;
}

/** User Story: As Ghost result tests, I need each expected check read without invoking the production wire decoder. @fn inline FGhostResultRecord ReadGhostResultRecordExpected(const TSharedPtr<FJsonObject> &Object) */
inline FGhostResultRecord
ReadGhostResultRecordExpected(const TSharedPtr<FJsonObject> &Object) {
  check(Object.IsValid());
  const TSharedRef<FJsonObject> Value = Object.ToSharedRef();
  FGhostResultRecord Expected;
  Expected.TestName =
      DataAdapters::ReadStringField(Value, TEXT("name"));
  Expected.bTestPassed =
      DataAdapters::ReadBooleanField(Value, TEXT("passed"));
  Expected.TestDuration =
      DataAdapters::ReadInt64Field(Value, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2EBBBD98C82A));
  Expected.TestError = func::match(
      DataAdapters::ReadOptionalStringField(Value, TEXT("error")),
      [](const FString &Error) { return Error; },
      []() { return FString(); });
  Expected.TestScreenshot = func::match(
      DataAdapters::ReadOptionalStringField(Value, TEXT("screenshot")),
      [](const FString &Screenshot) { return Screenshot; },
      []() { return FString(); });
  return Expected;
}

/** User Story: As Ghost result tests, I need one complete expected API diagnostic object composed independently from authored JSON. @fn inline FGhostResults ReadGhostResultsExpected(const TSharedRef<FJsonObject> &Object) */
inline FGhostResults
ReadGhostResultsExpected(const TSharedRef<FJsonObject> &Object) {
  FGhostResults Expected;
  Expected.SessionId =
      DataAdapters::ReadStringField(Object, TEXT("sessionId"));
  Expected.GhostName =
      DataAdapters::ReadStringField(Object, TEXT("ghostName"));
  Expected.RuntimeIdentity = ReadGhostRuntimeIdentity(
      DataAdapters::ReadObjectField(Object, TEXT("runtimeIdentity")));
  Expected.Evidence = ReadGhostEvidenceExpected(
      DataAdapters::ReadObjectField(Object, TEXT("evidence")));
  Expected.EvidenceDimensions =
      DataAdapters::ReadStringArrayField(Object, TEXT("evidenceDimensions"));
  Expected.EvaluationDimensions = DataAdapters::ReadStringArrayField(
      Object, TEXT("evaluationDimensions"));
  Expected.TotalTests =
      DataAdapters::ReadNumberField(Object, TEXT("totalTests"));
  Expected.Passed =
      DataAdapters::ReadNumberField(Object, TEXT("passed"));
  Expected.Failed =
      DataAdapters::ReadNumberField(Object, TEXT("failed"));
  Expected.Skipped =
      DataAdapters::ReadNumberField(Object, TEXT("skipped"));
  Expected.Duration =
      DataAdapters::ReadInt64Field(Object, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2EBBBD98C82A));
  Expected.Tests = func::map_array<TSharedPtr<FJsonObject>,
                                   FGhostResultRecord>(
      DataAdapters::ReadObjectArrayField(Object, TEXT("tests")),
      ReadGhostResultRecordExpected);
  Expected.Coverage =
      DataAdapters::ReadFloatField(Object, TEXT("coverage"));
  Expected.Verdict =
      DataAdapters::ReadStringField(Object, TEXT("verdict"));
  Expected.Summary =
      DataAdapters::ReadStringField(Object, TEXT("summary"));
  return Expected;
}

} // namespace Testing::API::Codec::Ghost
