#include "Misc/AutomationTest.h"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Chat/SystemsChatSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Quality/QualityAdapters.h"
#include "TestGame/Features/Systems/Terminal/Chat/TerminalChatSelectors.h"
#include "TestGame/Features/Systems/Terminal/TerminalSelectors.h"

using namespace TestGame;

namespace {

struct FChatTestSample {
  FString Id;
  FString Command;
  FString Prompt;
  FString Response;
  double DurationMs{};
};

struct FChatTestStories {
  FString CommandParsing;
  FString Selection;
  FString MissingEvidence;
  FString LogOrdering;
};

struct FChatTestData {
  FString AutomationName;
  FChatTestStories Stories;
  TArray<FChatTestSample> Samples;
  FString Summary;
};

/** User Story: As a UE chat test, I need authored transcript examples decoded independently from production selectors. @fn const FChatTestData &chatTestData() */
const FChatTestData &chatTestData() {
  static const FChatTestData Data = []() {
    const TSharedRef<FJsonObject> Root =
        DataAdapters::SettingsSource(TEXT("tests/chat.json")).Root;
    const TSharedRef<FJsonObject> Stories =
        DataAdapters::ReadObjectField(Root, TEXT("stories"));
    return FChatTestData{
        DataAdapters::ReadStringField(Root, TEXT("automationName")),
        {DataAdapters::ReadStringField(Stories, TEXT("commandParsing")),
         DataAdapters::ReadStringField(Stories, TEXT("selection")),
         DataAdapters::ReadStringField(Stories, TEXT("missingEvidence")),
         DataAdapters::ReadStringField(Stories, TEXT("logOrdering"))},
        func::map_array<TSharedPtr<FJsonValue>, FChatTestSample>(
            DataAdapters::ReadObjectArrayField(Root, TEXT("samples")),
            [](const TSharedPtr<FJsonValue> &Value) {
              const TSharedRef<FJsonObject> Object =
                  Value->AsObject().ToSharedRef();
              return FChatTestSample{
                  DataAdapters::ReadStringField(Object, TEXT("id")),
                  DataAdapters::ReadStringField(Object, TEXT("command")),
                  DataAdapters::ReadStringField(Object, TEXT("prompt")),
                  DataAdapters::ReadStringField(Object, TEXT("response")),
                  DataAdapters::ReadDoubleField(Object, TEXT("durationMs"))};
            }),
        DataAdapters::ReadStringField(Root, TEXT("summary"))};
  }();
  return Data;
}

/** User Story: As a UE chat test, I need authored examples projected into quality evidence without exercising model logic in the test harness. @fn FQualityReport createQualityReport(const FChatTestData &Data) */
FQualityReport createQualityReport(const FChatTestData &Data) {
  FQualityReport Report;
  Report.Samples = func::map_array<FChatTestSample, FQualitySample>(
      Data.Samples, [](const FChatTestSample &Input) {
        FQualitySample Sample;
        Sample.Id = Input.Id;
        Sample.Command = Input.Command;
        Sample.Response = Input.Response;
        Sample.DurationMs = Input.DurationMs;
        return Sample;
      });
  Report.Summary.Metrics =
      func::fold_array<FString, TMap<FString, FQualityMetricSummary>>(
          qualityData().MetricNames, {},
          [](const TMap<FString, FQualityMetricSummary> &Acc,
             const FString &Metric) {
            TMap<FString, FQualityMetricSummary> Next = Acc;
            FQualityMetricSummary Summary;
            Summary.Passed = qualityData().Numbers.SingularCount;
            Summary.Total = qualityData().Numbers.SingularCount;
            Summary.PassRate = qualityData().Numbers.SingularCount;
            Summary.bGatePassed = true;
            Next.Add(Metric, Summary);
            return Next;
          });
  Report.Summary.bRegressionGatePassed = true;
  Report.Summary.bQualityGatePassed = true;
  return Report;
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameChatTranscriptSelectorsTest, chatTestData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a test-game operator, I need UE chat selectors to preserve parsed prompts, authored order, missing evidence, and final-log placement.
 * @fn bool FTestGameChatTranscriptSelectorsTest::RunTest( const FString &Parameters)
 */
bool FTestGameChatTranscriptSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const FChatTestData &TestData = chatTestData();
  const FChatData &Runtime = ChatAdapters::ChatData();
  const func::Maybe<FString> ParsedPrompt =
      CommandRunner::Parsing::ReadCommandOption(
          TestData.Samples[Runtime.Numbers.InitialIndex].Command,
          Runtime.Command.TextOption);
  TestTrue(*TestData.Stories.CommandParsing, ParsedPrompt.hasValue);
  TestEqual(*TestData.Stories.CommandParsing,
            ParsedPrompt.hasValue ? ParsedPrompt.value : Runtime.Output.Empty,
            TestData.Samples[Runtime.Numbers.InitialIndex].Prompt);

  const FQualityReport Report = createQualityReport(TestData);
  const func::Maybe<FQualityReport> Evidence = func::just(Report);
  const FChatTranscript Transcript =
      ChatSelectors::SelectChatTranscript(Evidence);
  TestEqual(*TestData.Stories.Selection, Transcript.SessionId,
            Runtime.SessionId);
  TestEqual(*TestData.Stories.Selection, Transcript.Turns.Num(),
            Runtime.ProbeIds.Num());
  TestTrue(
      *TestData.Stories.Selection,
      func::map_array<FChatTranscriptTurn, FString>(
          Transcript.Turns,
          [](const FChatTranscriptTurn &Turn) { return Turn.Id; }) ==
          Runtime.ProbeIds);
  func::for_each_array<FChatTranscriptTurn>(
      Transcript.Turns, [this, &TestData](const FChatTranscriptTurn &Turn) {
        const FChatTestSample *Expected = TestData.Samples.FindByPredicate(
            [&Turn](const FChatTestSample &Sample) {
              return Sample.Id == Turn.Id;
            });
        TestNotNull(*TestData.Stories.Selection, Expected);
        Expected == nullptr
            ? void()
            : (TestEqual(*TestData.Stories.Selection, Turn.Prompt,
                         Expected->Prompt),
               TestTrue(*TestData.Stories.Selection, Turn.bAvailable), void());
      });

  const TArray<FTerminalLineViewModel> Missing =
      TerminalChatSelectors::SelectChatTranscriptViewModel(
          func::nothing<FQualityReport>());
  TestTrue(*TestData.Stories.MissingEvidence,
           Missing[Runtime.Numbers.NextIndex].bError);
  TestTrue(*TestData.Stories.MissingEvidence,
           Missing[Runtime.Numbers.NextIndex].Text.Contains(
               Runtime.ProbeIds[Runtime.Numbers.InitialIndex]));

  FGameProgress Progress;
  Progress.Type = GameAdapters::GameRuntimeData()
                      .lifecycleEvents.sessionCompleted;
  Progress.RunResult.QualityReport = Evidence;
  Progress.RunResult.Summary = TestData.Summary;
  Progress.RunResult.bComplete = true;
  const FTerminalProgressViewModel ViewModel =
      SelectTerminalProgressViewModel(Progress);
  const int32 SummaryIndex = ViewModel.Lines.IndexOfByPredicate(
      [&TestData](const FTerminalLineViewModel &Line) {
        return Line.Text == TestData.Summary;
      });
  const int32 HeadingIndex = ViewModel.Lines.IndexOfByPredicate(
      [&Runtime](const FTerminalLineViewModel &Line) {
        return Line.Text == Runtime.Output.Heading;
      });
  TestTrue(*TestData.Stories.LogOrdering, HeadingIndex > SummaryIndex);
  TestEqual(*TestData.Stories.LogOrdering, ViewModel.Lines.Last().Text,
            Runtime.Output.Footer);
  return true;
}
