#include "Misc/AutomationTest.h"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Quality/Scoring/ScoringAdapters.h"
#include "TestGame/Features/Systems/Quality/SystemsQualitySelectors.h"

using namespace TestGame;

namespace {

struct FAdversarialQualityCase {
  FString ProbeId;
  FString Output;
};

struct FQualityScoringTestData {
  FString AutomationName;
  FString Story;
  FString Host;
  double DurationMs{};
  TArray<FAdversarialQualityCase> Cases;
};

/** User Story: As a UE scorer test, I need adversarial examples decoded from authored data before exercising production behavior. @fn const FQualityScoringTestData &qualityScoringTestData() */
const FQualityScoringTestData &qualityScoringTestData() {
  static const FQualityScoringTestData Data = []() {
    const TSharedRef<FJsonObject> Root =
        DataAdapters::SettingsSource(TEXT("tests/quality/scoring.json")).Root;
    return FQualityScoringTestData{
        DataAdapters::ReadStringField(Root, TEXT("automationName")),
        DataAdapters::ReadStringField(Root, TEXT("story")),
        DataAdapters::ReadStringField(Root, TEXT("host")),
        DataAdapters::ReadDoubleField(Root, TEXT("durationMs")),
        func::map_array<TSharedPtr<FJsonValue>, FAdversarialQualityCase>(
            DataAdapters::ReadObjectArrayField(Root, TEXT("cases")),
            [](const TSharedPtr<FJsonValue> &Value) {
              const TSharedRef<FJsonObject> Object =
                  Value->AsObject().ToSharedRef();
              return FAdversarialQualityCase{
                  DataAdapters::ReadStringField(Object, TEXT("probeId")),
                  DataAdapters::ReadStringField(Object, TEXT("output"))};
            })};
  }();
  return Data;
}

/** User Story: As a UE scorer test, I need every authored case bound to its production probe without fallback behavior. @fn const FQualityProbe *findQualityProbe(const FString &ProbeId) */
const FQualityProbe *findQualityProbe(const FString &ProbeId) {
  return qualityData().Probes.FindByPredicate(
      [&ProbeId](const FQualityProbe &Probe) { return Probe.Id == ProbeId; });
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameQualityRejectsContradictionsTest,
    qualityScoringTestData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a model evaluator, I need high-overlap contradictions rejected by the production UE quality scorer. @fn bool FTestGameQualityRejectsContradictionsTest::RunTest(const FString &Parameters) */
bool FTestGameQualityRejectsContradictionsTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const FQualityScoringTestData &TestData = qualityScoringTestData();
  const FQualityData &Runtime = qualityData();
  int32 RejectedCount = Runtime.Numbers.EmptyCount;

  for (const FAdversarialQualityCase &Adversarial : TestData.Cases) {
    const FQualityProbe *Probe = findQualityProbe(Adversarial.ProbeId);
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

  TestEqual(*TestData.Story, RejectedCount, TestData.Cases.Num());

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
