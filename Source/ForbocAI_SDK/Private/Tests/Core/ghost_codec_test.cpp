#include "Systems/API/Endpoints/Ghost/EndpointsGhostAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Systems/Testing/API/Codec/Ghost/CodecGhostAdapters.h"
#include "Misc/AutomationTest.h"

namespace {

/**
 * User Story: As Ghost attribution tests, I need complete runtime identities compared without presentation fallbacks.
 * @fn inline bool GhostRuntimeIdentityEquals( const FGhostRuntimeIdentity &Left, const FGhostRuntimeIdentity &Right)
 */
inline bool GhostRuntimeIdentityEquals(
    const FGhostRuntimeIdentity &Left,
    const FGhostRuntimeIdentity &Right) {
  return Left.ApiVersion == Right.ApiVersion &&
         Left.SlmStatus == Right.SlmStatus &&
         Left.SlmVersion == Right.SlmVersion &&
         Left.SlotContractVersion == Right.SlotContractVersion;
}

/** User Story: As Ghost evidence tests, I need every API counter compared as one diagnostic contract. @fn inline bool GhostEvidenceEquals(const FGhostEvidenceSummary &Left, const FGhostEvidenceSummary &Right) */
inline bool GhostEvidenceEquals(const FGhostEvidenceSummary &Left,
                                const FGhostEvidenceSummary &Right) {
  return Left.CompletedProcessTurns == Right.CompletedProcessTurns &&
         Left.ValidCognitionOutcomes == Right.ValidCognitionOutcomes &&
         Left.InvalidCognitionOutcomes == Right.InvalidCognitionOutcomes &&
         Left.SignedCognitionOutcomes == Right.SignedCognitionOutcomes &&
         Left.ThoughtResults == Right.ThoughtResults &&
         Left.ReasoningResults == Right.ReasoningResults &&
         Left.ConsistentReasoningResults ==
             Right.ConsistentReasoningResults &&
         Left.DiagnosticResults == Right.DiagnosticResults &&
         Left.GroundedDiagnosticResults ==
             Right.GroundedDiagnosticResults &&
         Left.Contradictions == Right.Contradictions &&
         Left.UnsupportedClaims == Right.UnsupportedClaims &&
         Left.Failures == Right.Failures &&
         Left.Timeouts == Right.Timeouts &&
         Left.UniqueObservations == Right.UniqueObservations &&
         Left.ChoicePoints == Right.ChoicePoints;
}

/** User Story: As strict nested-codec tests, I need one authored array entry replaced while retaining every surrounding wire field. @fn inline FString ReplaceGhostArrayEntry(const FString &Json, const FString &Field, int32 Index, const FString &Replacement) */
inline FString ReplaceGhostArrayEntry(const FString &Json,
                                      const FString &Field,
                                      int32 Index,
                                      const FString &Replacement) {
  TSharedPtr<FJsonObject> Root;
  const bool bParsed = JsonInterop::ParseJsonObject(Json, Root) &&
                       Root.IsValid();
  TArray<TSharedPtr<FJsonValue>> Values =
      bParsed ? Root->GetArrayField(Field)
              : TArray<TSharedPtr<FJsonValue>>();
  return bParsed && Values.IsValidIndex(Index)
             ? [Root, Values, Field, Index, Replacement]() mutable {
                 const TSharedPtr<FJsonValue> ReplacementValue =
                     MakeShared<FJsonValueString>(Replacement);
                 Values[Index] = ReplacementValue;
                 Root->SetArrayField(Field, Values);
                 return DataAdapters::SerializeObject(Root.ToSharedRef());
               }()
             : FString();
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGhostCodecContractTest, FORBOCAI_SDK_AUTHORED_STRINGV7497B5CFCB62,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As UE and TS parity verification, I need strict Ghost lifecycle and evidence decoding proven against the same authored wire examples. @fn bool FGhostCodecContractTest::RunTest(const FString &Parameters) */
bool FGhostCodecContractTest::RunTest(const FString &Parameters) {
  using namespace Testing::API::Codec::Ghost;
  const FGhostCodecFixture &Fixture = GhostCodecFixtures();

  FGhostRunResponse Run;
  TestTrue(*Fixture.Labels.Run,
           APISlice::Detail::DecodeGhostRunResponse(
               Fixture.RunResponseJson, Run));
  TestEqual(*Fixture.Labels.SessionId, Run.SessionId,
            Fixture.RunExpected.SessionId);
  TestEqual(*Fixture.Labels.GhostName, Run.GhostName,
            Fixture.RunExpected.GhostName);
  TestTrue(*Fixture.Labels.RuntimeIdentity,
           GhostRuntimeIdentityEquals(Run.RuntimeIdentity,
                                      Fixture.RunExpected.RuntimeIdentity));

  FGhostStatus Status;
  TestTrue(*Fixture.Labels.Status,
           APISlice::Detail::DecodeGhostStatusResponse(
               Fixture.StatusResponseJson, Status));
  TestEqual(*Fixture.Labels.TestSuite, Status.TestSuite,
            Fixture.StatusExpected.TestSuite);
  TestEqual(*Fixture.Labels.Progress, Status.Progress,
            Fixture.StatusExpected.Progress);
  TestTrue(*Fixture.Labels.RuntimeIdentity,
           GhostRuntimeIdentityEquals(
               Status.RuntimeIdentity,
               Fixture.StatusExpected.RuntimeIdentity));

  FGhostResults Results;
  TestTrue(*Fixture.Labels.Results,
           APISlice::Detail::DecodeGhostResultsResponse(
               Fixture.ResultsResponseJson, Results));
  TestTrue(*Fixture.Labels.Evidence,
           GhostEvidenceEquals(Results.Evidence,
                               Fixture.ResultsExpected.Evidence));
  TestEqual(*Fixture.Labels.EvidenceDimensions,
            Results.EvidenceDimensions,
            Fixture.ResultsExpected.EvidenceDimensions);
  const FString ActualEvidenceDimension =
      Results.EvidenceDimensions.IsValidIndex(
          Fixture.Probes.EvidenceDimensionIndex)
          ? Results.EvidenceDimensions[
                Fixture.Probes.EvidenceDimensionIndex]
          : FString();
  const FString ExpectedEvidenceDimension =
      Fixture.ResultsExpected.EvidenceDimensions.IsValidIndex(
          Fixture.Probes.EvidenceDimensionIndex)
          ? Fixture.ResultsExpected.EvidenceDimensions[
                Fixture.Probes.EvidenceDimensionIndex]
          : FString();
  TestEqual(
      *Fixture.Labels.EvidenceDimensions,
      ActualEvidenceDimension, ExpectedEvidenceDimension);
  TestEqual(*Fixture.Labels.EvaluationDimensions,
            Results.EvaluationDimensions,
            Fixture.ResultsExpected.EvaluationDimensions);
  const FString ActualEvaluationDimension =
      Results.EvaluationDimensions.IsValidIndex(
          Fixture.Probes.EvaluationDimensionIndex)
          ? Results.EvaluationDimensions[
                Fixture.Probes.EvaluationDimensionIndex]
          : FString();
  const FString ExpectedEvaluationDimension =
      Fixture.ResultsExpected.EvaluationDimensions.IsValidIndex(
          Fixture.Probes.EvaluationDimensionIndex)
          ? Fixture.ResultsExpected.EvaluationDimensions[
                Fixture.Probes.EvaluationDimensionIndex]
          : FString();
  TestEqual(
      *Fixture.Labels.EvaluationDimensions,
      ActualEvaluationDimension, ExpectedEvaluationDimension);
  TestEqual(*Fixture.Labels.TestCount, Results.Tests.Num(),
            Fixture.ResultsExpected.Tests.Num());
  const FString ActualTestName =
      Results.Tests.IsValidIndex(Fixture.Probes.TestIndex)
          ? Results.Tests[Fixture.Probes.TestIndex].TestName
          : FString();
  const FString ExpectedTestName =
      Fixture.ResultsExpected.Tests.IsValidIndex(Fixture.Probes.TestIndex)
          ? Fixture.ResultsExpected.Tests[Fixture.Probes.TestIndex].TestName
          : FString();
  TestEqual(*Fixture.Labels.Results,
            ActualTestName, ExpectedTestName);
  TestEqual(*Fixture.Labels.Coverage, Results.Coverage,
            Fixture.ResultsExpected.Coverage);
  const float ActualMetric = func::or_else(
      func::find_map_value<FString, float>(
          Results.Metrics, Fixture.Probes.MetricName),
      Fixture.Probes.MetricValue - Fixture.Probes.MetricValue);
  TestEqual(*Fixture.Labels.Metric, ActualMetric,
            Fixture.Probes.MetricValue);
  TestEqual(*Fixture.Labels.Verdict, Results.Verdict,
            Fixture.ResultsExpected.Verdict);
  TestEqual(*Fixture.Labels.Summary, Results.Summary,
            Fixture.ResultsExpected.Summary);

  FGhostStopResponse Stop;
  TestTrue(*Fixture.Labels.Stop,
           APISlice::Detail::DecodeGhostStopResponse(
               Fixture.StopResponseJson, Stop));
  TestEqual(*Fixture.Labels.SessionId, Stop.StopSessionId,
            Fixture.StopExpected.StopSessionId);
  TestEqual(*Fixture.Labels.Stop, Stop.bStopped,
            Fixture.StopExpected.bStopped);
  TestEqual(*Fixture.Labels.Stop, Stop.StopStatus,
            Fixture.StopExpected.StopStatus);

  FGhostHistoryResponse History;
  TestTrue(*Fixture.Labels.History,
           APISlice::Detail::DecodeGhostHistoryResponse(
               Fixture.HistoryResponseJson, History));
  TestEqual(*Fixture.Labels.TestCount, History.Sessions.Num(),
            Fixture.HistoryExpected.Sessions.Num());
  const FGhostHistoryEntry ActualHistory =
      History.Sessions.IsValidIndex(Fixture.Probes.HistoryIndex)
          ? History.Sessions[Fixture.Probes.HistoryIndex]
          : FGhostHistoryEntry{};
  const FGhostHistoryEntry ExpectedHistory =
      Fixture.HistoryExpected.Sessions.IsValidIndex(
          Fixture.Probes.HistoryIndex)
          ? Fixture.HistoryExpected.Sessions[
                Fixture.Probes.HistoryIndex]
          : FGhostHistoryEntry{};
  TestEqual(*Fixture.Labels.GhostName, ActualHistory.GhostName,
            ExpectedHistory.GhostName);
  TestTrue(*Fixture.Labels.RuntimeIdentity,
           GhostRuntimeIdentityEquals(ActualHistory.RuntimeIdentity,
                                      ExpectedHistory.RuntimeIdentity));

  FGhostRunResponse InvalidRun;
  TestFalse(*Fixture.Labels.RejectsMissingIdentity,
            APISlice::Detail::DecodeGhostRunResponse(
                Fixture.Malformed.RunMissingIdentityJson, InvalidRun));
  TestFalse(*Fixture.Labels.RejectsEmptyIdentity,
            APISlice::Detail::DecodeGhostRunResponse(
                Fixture.Malformed.RunEmptyIdentityJson, InvalidRun));

  const auto &Fields = APISlice::Endpoints::GhostConfiguration::
      ghostApiConfiguration().Fields;
  FGhostResults InvalidResults;
  TestFalse(*Fixture.Labels.RejectsMalformedTest,
            APISlice::Detail::DecodeGhostResultsResponse(
                ReplaceGhostArrayEntry(
                    Fixture.ResultsResponseJson, Fields.Results.Tests,
                    Fixture.Malformed.TestIndex,
                    Fixture.Malformed.TestReplacement),
                InvalidResults));
  TestFalse(*Fixture.Labels.RejectsMalformedMetric,
            APISlice::Detail::DecodeGhostResultsResponse(
                ReplaceGhostArrayEntry(
                    Fixture.ResultsResponseJson, Fields.Results.Metrics,
                    Fixture.Malformed.MetricIndex,
                    Fixture.Malformed.MetricReplacement),
                InvalidResults));
  FGhostHistoryResponse InvalidHistory;
  TestFalse(*Fixture.Labels.RejectsMalformedHistory,
            APISlice::Detail::DecodeGhostHistoryResponse(
                ReplaceGhostArrayEntry(
                    Fixture.HistoryResponseJson, Fields.History.Sessions,
                    Fixture.Malformed.HistoryIndex,
                    Fixture.Malformed.HistoryReplacement),
                InvalidHistory));
  return true;
}
