#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Quality/Scoring/ScoringAdapters.h"
#include "MicroGame/Features/Systems/Quality/SystemsQualitySelectors.h"

using namespace MicroGame;

namespace {

struct FQualityScoringCase {
  FString ProbeId;
  FString Output;
};

struct FQualityScoringTestData {
  FString AutomationName;
  FString Story;
  FString AcceptanceStory;
  FString WorkloadMismatchStory;
  FString WorkloadMismatchCommand;
  FString Host;
  double DurationMs{};
  TArray<FQualityScoringCase> Cases;
  TArray<FQualityScoringCase> AcceptedCases;
};

/** User Story: As a UE scorer test, I need authored quality fixtures decoded once before exercising production behavior. @fn const FQualityScoringTestData &qualityTestData() */
const FQualityScoringTestData &qualityTestData() {
  static const FQualityScoringTestData Data = []() {
    const TSharedRef<FJsonObject> Root =
        DataAdapters::SettingsSource(TEXT("tests/quality/scoring.json")).Root;
    return FQualityScoringTestData{
        DataAdapters::ReadStringField(Root, TEXT("automationName")),
        DataAdapters::ReadStringField(Root, TEXT("story")),
        DataAdapters::ReadStringField(Root, TEXT("acceptanceStory")),
        DataAdapters::ReadStringField(Root, TEXT("workloadMismatchStory")),
        DataAdapters::ReadStringField(Root, TEXT("workloadMismatchCommand")),
        DataAdapters::ReadStringField(Root, TEXT("host")),
        DataAdapters::ReadDoubleField(Root, TEXT("durationMs")),
        func::map_array<TSharedPtr<FJsonValue>, FQualityScoringCase>(
            DataAdapters::ReadObjectArrayField(Root, TEXT("cases")),
            [](const TSharedPtr<FJsonValue> &Value) {
              const TSharedRef<FJsonObject> Object =
                  Value->AsObject().ToSharedRef();
              return FQualityScoringCase{
                  DataAdapters::ReadStringField(Object, TEXT("probeId")),
                  DataAdapters::ReadStringField(Object, TEXT("output"))};
            }),
        func::map_array<TSharedPtr<FJsonValue>, FQualityScoringCase>(
            DataAdapters::ReadObjectArrayField(Root, TEXT("acceptedCases")),
            [](const TSharedPtr<FJsonValue> &Value) {
              const TSharedRef<FJsonObject> Object =
                  Value->AsObject().ToSharedRef();
              return FQualityScoringCase{
                  DataAdapters::ReadStringField(Object, TEXT("probeId")),
                  DataAdapters::ReadStringField(Object, TEXT("output"))};
            })};
  }();
  return Data;
}

/** User Story: As a UE scorer test, I need every authored case bound to its production probe without fallback behavior. @fn const FQualityProbe *qualityTestProbe(const FString &ProbeId) */
const FQualityProbe *qualityTestProbe(const FString &ProbeId) {
  return qualityData().Probes.FindByPredicate(
      [&ProbeId](const FQualityProbe &Probe) { return Probe.Id == ProbeId; });
}

/** User Story: As a UE scorer test, I need adversarial cases exposed through the same named adapter as the TS host. @fn const TArray<FQualityScoringCase> &qualityAdversarialCases() */
const TArray<FQualityScoringCase> &qualityAdversarialCases() {
  return qualityTestData().Cases;
}

/** User Story: As a UE scorer test, I need accepted semantic cases exposed through the same named adapter as the TS host. @fn const TArray<FQualityScoringCase> &qualityAcceptedCases() */
const TArray<FQualityScoringCase> &qualityAcceptedCases() {
  return qualityTestData().AcceptedCases;
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMicroGameQualityRejectsContradictionsTest,
    qualityTestData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a model evaluator, I need high-overlap contradictions rejected by the production UE quality scorer. @fn bool FMicroGameQualityRejectsContradictionsTest::RunTest(const FString &Parameters) */
bool FMicroGameQualityRejectsContradictionsTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const FQualityScoringTestData &TestData = qualityTestData();
  const FQualityData &Runtime = qualityData();
  int32 RejectedCount = Runtime.Numbers.EmptyCount;

  for (const FQualityScoringCase &Adversarial : qualityAdversarialCases()) {
    const FQualityProbe *Probe = qualityTestProbe(Adversarial.ProbeId);
    TestNotNull(*TestData.Story, Probe);
    if (Probe == nullptr) {
      continue;
    }

    CommandRunner::FCommandOutput Result;
    Result.Status = Runtime.CommandStatuses.Ok;
    Result.Output = Adversarial.Output;
    Result.DurationMs = TestData.DurationMs;
    FQualityModelMetadata Metadata;
    Metadata.InferenceLatencyBudgetMs =
        Runtime.Numbers.FallbackInferenceLatencyBudgetMs;
    const FQualitySample Sample =
        scoreQualitySample(*Probe, Result, Metadata, TestData.Host);
    const FQualityMetricEvaluation *Instruction =
        Sample.Metrics.Find(Runtime.Metrics.InstructionFollowing);
    const bool bRejected = Instruction != nullptr && !Instruction->bPassed;
    TestTrue(*TestData.Story, bRejected);
    RejectedCount += bRejected ? Runtime.Numbers.SingularCount
                               : Runtime.Numbers.EmptyCount;
  }

  TestEqual(*TestData.Story, RejectedCount, qualityAdversarialCases().Num());

  for (const FQualityScoringCase &Accepted : qualityAcceptedCases()) {
    const FQualityProbe *Probe = qualityTestProbe(Accepted.ProbeId);
    TestNotNull(*TestData.AcceptanceStory, Probe);
    if (Probe == nullptr) {
      continue;
    }

    CommandRunner::FCommandOutput Result;
    Result.Status = Runtime.CommandStatuses.Ok;
    Result.Output = Accepted.Output;
    Result.DurationMs = TestData.DurationMs;
    FQualityModelMetadata Metadata;
    Metadata.InferenceLatencyBudgetMs =
        Runtime.Numbers.FallbackInferenceLatencyBudgetMs;
    const FQualitySample Sample =
        scoreQualitySample(*Probe, Result, Metadata, TestData.Host);
    const FQualityMetricEvaluation *Instruction =
        Sample.Metrics.Find(Runtime.Metrics.InstructionFollowing);
    const FQualityMetricEvaluation *Character =
        Sample.Metrics.Find(Runtime.Metrics.CharacterConsistency);
    TestTrue(*TestData.AcceptanceStory,
             Instruction != nullptr && Instruction->bPassed);
    TestTrue(*TestData.AcceptanceStory,
             Character != nullptr && Character->bPassed);
  }

  const FQualityScoringCase &WorkloadCase = TestData.Cases.Last();
  const FQualityProbe *WorkloadProbe = qualityTestProbe(WorkloadCase.ProbeId);
  TestNotNull(*TestData.WorkloadMismatchStory, WorkloadProbe);
  if (WorkloadProbe == nullptr) {
    return false;
  }
  CommandRunner::FCommandOutput WorkloadResult;
  WorkloadResult.Status = Runtime.CommandStatuses.Ok;
  WorkloadResult.Output = WorkloadCase.Output;
  WorkloadResult.DurationMs = TestData.DurationMs;
  FQualityModelMetadata WorkloadMetadata;
  WorkloadMetadata.InferenceLatencyBudgetMs =
      Runtime.Numbers.FallbackInferenceLatencyBudgetMs;
  const FQualitySample BaselineSample = scoreQualitySample(
      *WorkloadProbe, WorkloadResult, WorkloadMetadata, TestData.Host);
  FQualitySample ChangedSample = BaselineSample;
  ChangedSample.Command = TestData.WorkloadMismatchCommand;
  FQualityState WorkloadState;
  WorkloadState.Host = TestData.Host;
  FQualityReport WorkloadBaseline;
  WorkloadBaseline.SchemaVersion = Runtime.SchemaVersion;
  WorkloadBaseline.ContractVersion = Runtime.ContractVersion;
  WorkloadBaseline.EvaluationScope = Runtime.EvaluationScope;
  WorkloadBaseline.Host = TestData.Host;
  WorkloadBaseline.Samples = {BaselineSample};
  WorkloadState.Baseline = func::just(WorkloadBaseline);
  TestEqual(*TestData.WorkloadMismatchStory,
            QualitySelectorsDetail::baselineStatus(WorkloadState,
                                                   {ChangedSample}),
            Runtime.BaselineStatuses.Incompatible);

  FQualityState LatencyState;
  LatencyState.Host = TestData.Host;
  FQualityReport Baseline;
  Baseline.Summary.MeanLatencyMs = TestData.DurationMs;
  LatencyState.Baseline = func::just(Baseline);
  const double HostBudget =
      Runtime.Hosts.FindChecked(TestData.Host).OverheadBudgetMs;
  const double JitterLatency = TestData.DurationMs +
                               HostBudget /
                                   (Runtime.Numbers.SingularCount +
                                    Runtime.Numbers.SingularCount);
  const double MaterialLatency = TestData.DurationMs + HostBudget +
                                 Runtime.Numbers.SingularCount;
  const TArray<FQualityRegression> Jitter =
      QualitySelectorsDetail::latencyRegression(
          JitterLatency, LatencyState,
          Runtime.BaselineStatuses.Compatible);
  const TArray<FQualityRegression> Material =
      QualitySelectorsDetail::latencyRegression(
          MaterialLatency, LatencyState,
          Runtime.BaselineStatuses.Compatible);

  TestFalse(*TestData.Story, Jitter.Last().bRegressed);
  TestTrue(*TestData.Story, Material.Last().bRegressed);
  return true;
}
