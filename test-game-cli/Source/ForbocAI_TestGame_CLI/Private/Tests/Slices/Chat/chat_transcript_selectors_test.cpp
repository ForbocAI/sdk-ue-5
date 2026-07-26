#include "Core/JsonInterop.h"
#include "Misc/AutomationTest.h"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Chat/SystemsChatSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
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
  FString ChatExecution;
  FString LogOrdering;
};

struct FChatConversationTestData {
  FString At;
  FString CommandGroup;
  FString Command;
  FString Status;
  FString ResponseJson;
  TArray<FString> TranscriptLines;
};

struct FChatTestData {
  FString AutomationName;
  FChatTestStories Stories;
  TArray<FChatTestSample> Samples;
  FString Summary;
  FChatConversationTestData Conversation;
};

/** User Story: As a UE chat test, I need authored transcript examples decoded independently from production selectors. @fn const FChatTestData &chatTestData() */
const FChatTestData &chatTestData() {
  static const FChatTestData Data = []() {
    const TSharedRef<FJsonObject> Root =
        TestGame::DataAdapters::SettingsSource(TEXT("tests/chat.json")).Root;
    const TSharedRef<FJsonObject> Stories =
        TestGame::DataAdapters::ReadObjectField(Root, TEXT("stories"));
    const TSharedRef<FJsonObject> Conversation =
        TestGame::DataAdapters::ReadObjectField(Root, TEXT("conversation"));
    const TSharedRef<FJsonObject> ConversationEntry =
        TestGame::DataAdapters::ReadObjectField(Conversation, TEXT("entry"));
    const TSharedRef<FJsonObject> ConversationResponse =
        TestGame::DataAdapters::ReadObjectField(Conversation,
                                                TEXT("response"));
    return FChatTestData{
        TestGame::DataAdapters::ReadStringField(Root,
                                                TEXT("automationName")),
        {TestGame::DataAdapters::ReadStringField(
             Stories, TEXT("commandParsing")),
         TestGame::DataAdapters::ReadStringField(Stories,
                                                 TEXT("selection")),
         TestGame::DataAdapters::ReadStringField(
             Stories, TEXT("missingEvidence")),
         TestGame::DataAdapters::ReadStringField(Stories,
                                                 TEXT("chatExecution")),
         TestGame::DataAdapters::ReadStringField(Stories,
                                                 TEXT("logOrdering"))},
        func::map_array<TSharedPtr<FJsonValue>, FChatTestSample>(
            TestGame::DataAdapters::ReadObjectArrayField(
                Root, TEXT("samples")),
            [](const TSharedPtr<FJsonValue> &Value) {
              const TSharedRef<FJsonObject> Object =
                  Value->AsObject().ToSharedRef();
              return FChatTestSample{
                  TestGame::DataAdapters::ReadStringField(Object,
                                                          TEXT("id")),
                  TestGame::DataAdapters::ReadStringField(
                      Object, TEXT("command")),
                  TestGame::DataAdapters::ReadStringField(
                      Object, TEXT("prompt")),
                  TestGame::DataAdapters::ReadStringField(
                      Object, TEXT("response")),
                  TestGame::DataAdapters::ReadDoubleField(
                      Object, TEXT("durationMs"))};
            }),
        TestGame::DataAdapters::ReadStringField(Root, TEXT("summary")),
        {TestGame::DataAdapters::ReadStringField(ConversationEntry,
                                                 TEXT("at")),
         TestGame::DataAdapters::ReadStringField(
             ConversationEntry, TEXT("commandGroup")),
         TestGame::DataAdapters::ReadStringField(ConversationEntry,
                                                 TEXT("command")),
         TestGame::DataAdapters::ReadStringField(ConversationEntry,
                                                 TEXT("status")),
         JsonInterop::StringifyObject(ConversationResponse),
         TestGame::DataAdapters::ReadStringArrayField(
             ConversationResponse, TEXT("transcriptLines"))}};
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

  FCommandSpec OtherCommand;
  OtherCommand.Group = GameAdapters::GameRuntimeData().commandGroups.status;
  OtherCommand.Command =
      TestData.Samples[Runtime.Numbers.InitialIndex].Command;
  FCommandSpec ConversationCommand;
  ConversationCommand.Group = TestData.Conversation.CommandGroup;
  ConversationCommand.Command = TestData.Conversation.Command;
  FScenarioStep FocusedStep;
  FocusedStep.Commands = {OtherCommand, ConversationCommand};
  FScenarioSliceState FocusedScenario;
  FocusedScenario.Steps.Add(MoveTemp(FocusedStep));
  const TArray<FScenarioStep> ConversationSteps =
      ScenarioSelectors::SelectScenarioStepsByCommandGroup(
          FocusedScenario, TestData.Conversation.CommandGroup);
  TestEqual(*TestData.Stories.ChatExecution, ConversationSteps.Num(),
            Runtime.Numbers.NextIndex);
  TestEqual(*TestData.Stories.ChatExecution,
            ConversationSteps[Runtime.Numbers.InitialIndex].Commands.Num(),
            Runtime.Numbers.NextIndex);
  TestEqual(*TestData.Stories.ChatExecution,
            ConversationSteps[Runtime.Numbers.InitialIndex]
                .Commands[Runtime.Numbers.InitialIndex]
                .Command,
            TestData.Conversation.Command);

  FGameProgress Progress;
  Progress.Type = GameAdapters::GameRuntimeData()
                      .lifecycleEvents.sessionCompleted;
  Progress.RunResult.QualityReport = Evidence;
  Progress.RunResult.Summary = TestData.Summary;
  Progress.RunResult.bComplete = true;
  FTranscriptEntry ConversationEntry;
  ConversationEntry.CommandGroup = TestData.Conversation.CommandGroup;
  ConversationEntry.Command = TestData.Conversation.Command;
  ConversationEntry.Status = TestData.Conversation.Status;
  ConversationEntry.Output = TestData.Conversation.ResponseJson;
  ConversationEntry.Timestamp = TestData.Conversation.At;
  Progress.RunResult.Transcript.Add(MoveTemp(ConversationEntry));
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
  const int32 FooterIndex = ViewModel.Lines.IndexOfByPredicate(
      [&Runtime](const FTerminalLineViewModel &Line) {
        return Line.Text == Runtime.Output.Footer;
      });
  const int32 ConversationHeadingIndex = ViewModel.Lines.IndexOfByPredicate(
      [&TestData, &Runtime](const FTerminalLineViewModel &Line) {
        return Line.Text == TestData.Conversation.TranscriptLines[
                                Runtime.Numbers.InitialIndex];
      });
  TestTrue(*TestData.Stories.LogOrdering, HeadingIndex > SummaryIndex);
  TestTrue(*TestData.Stories.LogOrdering,
           ConversationHeadingIndex > FooterIndex);
  TestEqual(*TestData.Stories.LogOrdering, ViewModel.Lines.Last().Text,
            TestData.Conversation.TranscriptLines.Last());
  return true;
}
