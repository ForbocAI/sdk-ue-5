#include "Misc/AutomationTest.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/Alias/AliasAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameCommandRunnerOutputAssertionsTest,
    "ForbocAI.Slices.TestGame.CommandRunner.OutputAssertions",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As an SDK verifier, I need semantic output assertions tested through a stable signature so successful process exits cannot hide missing state continuity.
 * @fn bool FTestGameCommandRunnerOutputAssertionsTest::RunTest( const FString &Parameters)
 */
bool FTestGameCommandRunnerOutputAssertionsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const CommandRunner::FCommandRunnerData &Data =
      CommandRunner::CommandRunnerData();
  FCommandSpec Command;
  Command.Group = ECommandGroup::SoulList;
  Command.Command = Data.commands.soulPrefix;
  FOutputAssertion Assertion;
  Assertion.Kind = EOutputAssertionKind::IncludesAlias;
  Assertion.Value = Data.aliases.soulTransaction;
  Command.OutputAssertions.Add(Assertion);

  CommandRunner::FCommandAliasState Aliases;
  Aliases.SoulTransactionAliases.Add(
      Data.aliases.soulTransaction, Data.commands.soulExport);
  const CommandRunner::FCommandOutput Observable{
      ETranscriptStatus::Ok, Data.commands.soulExport,
      Data.commands.soulExport, CommandRunner::FCommandAliasUpdate()};
  const CommandRunner::FCommandOutput MissingValue{
      ETranscriptStatus::Ok, Data.syntax.invalidCommandMessage,
      Data.commands.soulExport, CommandRunner::FCommandAliasUpdate()};

  TestEqual(
      Data.messages.capturedValuePreservesSuccess,
      CommandRunner::ValidateOutputAssertions(Command, Observable, Aliases)
          .Status,
      ETranscriptStatus::Ok);
  TestEqual(
      Data.messages.missingCapturedValueFails,
      CommandRunner::ValidateOutputAssertions(Command, MissingValue, Aliases)
          .Status,
      ETranscriptStatus::Error);
  FCommandSpec LiteralCommand;
  LiteralCommand.Group = ECommandGroup::listMemory;
  LiteralCommand.Command = Data.commands.soulExport;
  FOutputAssertion LiteralAssertion;
  LiteralAssertion.Kind = EOutputAssertionKind::IncludesText;
  LiteralAssertion.Value = Data.commands.soulExport;
  LiteralCommand.OutputAssertions.Add(LiteralAssertion);
  TestEqual(
      Data.messages.requiredLiteralPreservesSuccess,
      CommandRunner::ValidateOutputAssertions(LiteralCommand, Observable,
                                              Aliases)
          .Status,
      ETranscriptStatus::Ok);
  TestEqual(
      Data.messages.missingLiteralFails,
      CommandRunner::ValidateOutputAssertions(LiteralCommand, MissingValue,
                                              Aliases)
          .Status,
      ETranscriptStatus::Error);
  TestEqual(
      Data.messages.missingAliasFails,
      CommandRunner::ValidateOutputAssertions(
          Command, Observable, CommandRunner::FCommandAliasState())
          .Status,
      ETranscriptStatus::Error);
  const TArray<FString> UnresolvedArgs{Data.aliases.soulTransaction};
  TestTrue(
      Data.messages.unresolvedIdentifierRejected,
      CommandRunner::FindUnresolvedCommandAlias(
          Data.commands.soulImport, UnresolvedArgs,
          CommandRunner::FCommandAliasState())
          .hasValue);
  TestFalse(
      Data.messages.capturedIdentifierDispatchable,
      CommandRunner::FindUnresolvedCommandAlias(
          Data.commands.soulImport, UnresolvedArgs, Aliases)
          .hasValue);
  return true;
}
