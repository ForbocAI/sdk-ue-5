#include "ChatTranscriptTestData.h"
#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Systems/Contract/ContractSelectors.h"
#include "MicroGame/Features/Systems/Chat/SystemsChatSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "MicroGame/Features/Systems/Quality/QualityAdapters.h"
#include "MicroGame/Features/Systems/Terminal/Chat/TerminalChatSelectors.h"
#include "MicroGame/Features/Systems/Terminal/TerminalSelectors.h"

using namespace MicroGame;
using namespace MicroGame::ChatTranscriptTests;

namespace {

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
    FMicroGameChatTranscriptSelectorsTest, ChatTestData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a micro-game operator, I need UE chat selectors to preserve parsed prompts, authored order, missing evidence, and final-log placement.
 * @fn bool FMicroGameChatTranscriptSelectorsTest::RunTest( const FString &Parameters)
 */
bool FMicroGameChatTranscriptSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const FChatTestData &TestData = ChatTestData();
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
  OtherCommand.Group = VerificationVocabularyAdapters::GameRuntimeData().commandGroups.status;
  OtherCommand.Command =
      TestData.Samples[Runtime.Numbers.InitialIndex].Command;
  FCommandSpec ConversationCommand;
  ConversationCommand.Group = TestData.Conversation.Entry.CommandGroup;
  ConversationCommand.Command = TestData.Conversation.Entry.Command;
  FScenarioStep FocusedStep;
  FocusedStep.Commands = {OtherCommand, ConversationCommand};
  FScenarioSliceState FocusedScenario;
  FocusedScenario.Steps.Add(MoveTemp(FocusedStep));
  const TArray<FScenarioStep> ConversationSteps =
      ScenarioSelectors::SelectScenarioStepsByCommandGroup(
          FocusedScenario, TestData.Conversation.Entry.CommandGroup);
  TestEqual(*TestData.Stories.ChatExecution, ConversationSteps.Num(),
            Runtime.Numbers.NextIndex);
  TestEqual(*TestData.Stories.ChatExecution,
            ConversationSteps[Runtime.Numbers.InitialIndex].Commands.Num(),
            Runtime.Numbers.NextIndex);
  TestEqual(*TestData.Stories.ChatExecution,
            ConversationSteps[Runtime.Numbers.InitialIndex]
                .Commands[Runtime.Numbers.InitialIndex]
                .Command,
            TestData.Conversation.Entry.Command);

  const FGameData &Game = VerificationAdapters::GameData();
  TestEqual(*TestData.Stories.FinalCommand, Game.finalCommands.Num(),
            Runtime.Numbers.NextIndex);
  TestEqual(*TestData.Stories.FinalCommand,
            Game.finalCommands[Runtime.Numbers.InitialIndex].Command,
            TestData.Conversation.Entry.Command);
  Contract::FContractResponse StaleContract;
  const TArray<FCommandSpec> MissingFinalCommands =
      ContractSelectors::SelectMissingFinalCommands(
          StaleContract, Game.finalCommands);
  TestEqual(*TestData.Stories.FinalCommand, MissingFinalCommands.Num(),
            Runtime.Numbers.NextIndex);
  const FScenarioContractPayload RequiredContract =
      ContractSelectors::SelectContractWithFinalRequirements(
          StaleContract, Game.finalCommands);
  TestTrue(*TestData.Stories.FinalCommand,
           RequiredContract.RequiredCommandGroups.Contains(
               TestData.Conversation.Entry.CommandGroup));
  FScenarioStep CurrentStep;
  CurrentStep.Commands = Game.finalCommands;
  StaleContract.Scenarios.Add(MoveTemp(CurrentStep));
  TestEqual(*TestData.Stories.FinalCommand,
            ContractSelectors::SelectMissingFinalCommands(
                StaleContract, Game.finalCommands)
                .Num(),
            Runtime.Numbers.EmptyCount);

  FGameProgress Progress;
  Progress.Type = VerificationVocabularyAdapters::GameRuntimeData()
                      .lifecycleEvents.sessionCompleted;
  Progress.RunResult.QualityReport = Evidence;
  Progress.RunResult.Summary = TestData.Summary;
  Progress.RunResult.bComplete = true;
  FTranscriptEntry ConversationEntry;
  ConversationEntry.CommandGroup = TestData.Conversation.Entry.CommandGroup;
  ConversationEntry.Command = TestData.Conversation.Entry.Command;
  ConversationEntry.Status = TestData.Conversation.Entry.Status;
  ConversationEntry.Output = TestData.Conversation.ResponseJson;
  ConversationEntry.Timestamp = TestData.Conversation.Entry.At;
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

  FGameProgress FailedProgress;
  FailedProgress.Type = VerificationVocabularyAdapters::GameRuntimeData()
                            .lifecycleEvents.sessionCompleted;
  FailedProgress.RunResult.QualityReport = Evidence;
  FailedProgress.RunResult.Summary = TestData.Summary;
  FTranscriptEntry FailedEntry;
  FailedEntry.CommandGroup = TestData.Conversation.FailureEntry.CommandGroup;
  FailedEntry.Command = TestData.Conversation.FailureEntry.Command;
  FailedEntry.Status = TestData.Conversation.FailureEntry.Status;
  FailedEntry.Output = TestData.Conversation.FailureEntry.Output;
  FailedEntry.Timestamp = TestData.Conversation.FailureEntry.At;
  FailedProgress.RunResult.Transcript.Add(MoveTemp(FailedEntry));
  const FTerminalProgressViewModel FailedViewModel =
      SelectTerminalProgressViewModel(FailedProgress);
  const FTerminalData &Terminal = TerminalAdapters::TerminalData();
  const int32 FailedFooterIndex = FailedViewModel.Lines.IndexOfByPredicate(
      [&Runtime](const FTerminalLineViewModel &Line) {
        return Line.Text == Runtime.Output.Footer;
      });
  const int32 FailureStart =
      FailedViewModel.Lines.Num() -
      Terminal.conversationTranscript.FailureLines.Num();
  TestTrue(*TestData.Stories.FailureOrdering,
           FailureStart > FailedFooterIndex);
  TestEqual(*TestData.Stories.FailureOrdering,
            FailedViewModel.Lines[FailureStart].Text,
            Terminal.conversationTranscript.FailureLines[
                Runtime.Numbers.InitialIndex]
                .Template);
  TestEqual(*TestData.Stories.FailureOrdering,
            FailedViewModel.Lines[FailureStart + Runtime.Numbers.NextIndex]
                .Text,
            TestData.Conversation.FailureEntry.Output);
  TestEqual(*TestData.Stories.FailureOrdering,
            FailedViewModel.Lines.Last().Text,
            Terminal.conversationTranscript.FailureLines.Last().Template);
  return true;
}
