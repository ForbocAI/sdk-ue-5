#include "Entities/CLI/Ghost/CLIGhostSelectors.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Systems/CLI/Ghost/CLIGhostAdapters.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"
#include "Systems/Testing/API/Codec/Ghost/CodecGhostAdapters.h"
#include "Misc/AutomationTest.h"

namespace CliGhost = ForbocAI::CLI::Ghost;
namespace CodecGhost = Testing::API::Codec::Ghost;

namespace {

/** User Story: As Ghost CLI projection tests, I need exact authored lines located without relying on output order. @fn inline bool ContainsGhostLine(const TArray<FString> &Lines, const FString &Expected) */
inline bool ContainsGhostLine(const TArray<FString> &Lines,
                              const FString &Expected) {
  return func::any_array<FString>(
      Lines, [&Expected](const FString &Line) { return Line == Expected; });
}

/** User Story: As Ghost CLI projection tests, I need all expected lines proven against one selected output. @fn inline bool ContainsEveryGhostLine(const TArray<FString> &Lines, const TArray<FString> &Expected) */
inline bool ContainsEveryGhostLine(const TArray<FString> &Lines,
                                   const TArray<FString> &Expected) {
  return func::all_array<FString>(
      Expected, [&Lines](const FString &Line) {
        return ContainsGhostLine(Lines, Line);
      });
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGhostCliProjectionTest,
    TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7C8537C8626B),
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As UE and TS CLI parity verification, I need API-owned Ghost identity and diagnostics projected without client inference. @fn bool FGhostCliProjectionTest::RunTest(const FString &Parameters) */
bool FGhostCliProjectionTest::RunTest(const FString &Parameters) {
  using namespace ForbocAI::CLI;
  const CodecGhost::FGhostCodecFixture &Fixture =
      CodecGhost::GhostCodecFixtures();
  const CliGhost::FCLIGhostState State = CliGhost::readCliGhostState();
  const FGhostResults &Results = Fixture.ResultsExpected;

  const TArray<FString> FixedResultLines = {
      Presentation::formatCliMessage(State.Identity, Results.GhostName),
      CliGhost::selectCliGhostRuntimeLine(State, Results.RuntimeIdentity),
      Presentation::formatCliMessage(State.Results, Results.Passed,
                                     Results.TotalTests),
      Presentation::formatCliMessage(State.Verdict, Results.Verdict),
      Presentation::formatCliMessage(
          State.Coverage, FMath::RoundToInt(Results.Coverage)),
      Presentation::formatCliMessage(
          State.Evidence, Results.Evidence.CompletedProcessTurns,
          Results.Evidence.ValidCognitionOutcomes,
          Results.Evidence.InvalidCognitionOutcomes,
          Results.Evidence.ThoughtResults, Results.Evidence.ReasoningResults,
          Results.Evidence.ConsistentReasoningResults,
          Results.Evidence.DiagnosticResults,
          Results.Evidence.GroundedDiagnosticResults),
      Presentation::formatCliMessage(
          State.Integrity, Results.Evidence.SignedCognitionOutcomes,
          Results.Evidence.Contradictions,
          Results.Evidence.UnsupportedClaims, Results.Evidence.Failures,
          Results.Evidence.Timeouts),
      Presentation::formatCliMessage(
          State.Exploration, Results.Evidence.UniqueObservations,
          Results.Evidence.ChoicePoints),
      Presentation::formatCliMessage(
          State.EvidenceDimensions,
          FString::Join(Results.EvidenceDimensions,
                        *State.DimensionSeparator)),
      Presentation::formatCliMessage(
          State.EvaluationDimensions,
          FString::Join(Results.EvaluationDimensions,
                        *State.DimensionSeparator)),
      Presentation::formatCliMessage(State.Summary, Results.Summary),
  };
  const TArray<FString> CheckLines =
      func::map_array<FGhostResultRecord, FString>(
          Results.Tests, [&State](const FGhostResultRecord &Result) {
            return Presentation::formatCliMessage(
                State.Result,
                Result.bTestPassed ? State.Passed : State.FailedTest,
                Result.TestName);
          });
  const TArray<FString> MetricLines =
      CliGhost::selectCliGhostMetricLines(State, Results.Metrics);
  const TArray<FString> ResultLines =
      CliGhost::selectCliGhostResultsLines(State, Results);

  TestEqual(*Fixture.Labels.TestCount, ResultLines.Num(),
            FixedResultLines.Num() + CheckLines.Num() + MetricLines.Num());
  TestTrue(*Fixture.Labels.Results,
           ContainsEveryGhostLine(ResultLines, FixedResultLines));
  TestTrue(*Fixture.Labels.Results,
           ContainsEveryGhostLine(ResultLines, CheckLines));
  TestTrue(*Fixture.Labels.Metric,
           ContainsEveryGhostLine(ResultLines, MetricLines));

  const FGhostStatus &Status = Fixture.StatusExpected;
  const TArray<FString> ExpectedStatusLines = {
      Presentation::formatCliMessage(State.Identity, Status.GhostName),
      CliGhost::selectCliGhostRuntimeLine(State, Status.RuntimeIdentity),
      Presentation::formatCliMessage(State.Suite, Status.TestSuite),
      Presentation::formatCliMessage(State.Session, Status.SessionId),
      Presentation::formatCliMessage(
          State.Status,
          Status.Status == State.CompletedStatus ? State.CompletedColor
                                                  : State.ActiveColor,
          Status.Status.ToUpper()),
      Presentation::formatCliMessage(State.Progress, Status.Progress),
      Presentation::formatCliMessage(State.Errors, Status.Errors),
      Presentation::formatCliMessage(State.Duration, Status.Duration),
  };
  const TArray<FString> StatusLines =
      CliGhost::selectCliGhostStatusLines(State, Status);
  TestEqual(*Fixture.Labels.Status, StatusLines.Num(),
            ExpectedStatusLines.Num());
  TestTrue(*Fixture.Labels.Status,
           ContainsEveryGhostLine(StatusLines, ExpectedStatusLines));

  const TArray<FString> HistoryLines =
      CliGhost::selectCliGhostHistoryLines(
          State, Fixture.HistoryExpected.Sessions);
  const TArray<FString> ExpectedHistoryLines =
      func::fold_array<FGhostHistoryEntry, TArray<FString>>(
          Fixture.HistoryExpected.Sessions, TArray<FString>(),
          [&State](const TArray<FString> &Lines,
                   const FGhostHistoryEntry &Session) {
            return func::concat_arrays<FString>(
                {Lines,
                 CliGhost::selectCliGhostHistoryEntryLines(State, Session)});
          });
  TestEqual(*Fixture.Labels.History, HistoryLines.Num(),
            ExpectedHistoryLines.Num());
  TestTrue(*Fixture.Labels.History,
           ContainsEveryGhostLine(HistoryLines, ExpectedHistoryLines));
  return true;
}
