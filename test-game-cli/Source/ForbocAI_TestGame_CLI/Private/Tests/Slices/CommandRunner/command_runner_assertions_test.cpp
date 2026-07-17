#include "Misc/AutomationTest.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/Alias/AliasAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameCommandRunnerOutputAssertionsTest,
    CommandRunner::CommandRunnerData().testNames.outputAssertions,
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
  const FGameRuntimeData &Runtime = GameAdapters::GameRuntimeData();
  FCommandSpec Command;
  Command.Group = Runtime.commandGroups.soul_list;
  Command.Command = Data.commands.soulPrefix;
  FOutputAssertion Assertion;
  Assertion.Kind = Runtime.outputAssertionKinds.includesAlias;
  Assertion.Value = Data.aliases.soulTransaction;
  Command.OutputAssertions.Add(Assertion);

  CommandRunner::FCommandAliasState Aliases;
  Aliases.SoulTransactionAliases.Add(
      Data.aliases.soulTransaction, Data.commands.soulExport);
  const CommandRunner::FCommandOutput Observable{
      Runtime.statuses.ok, Data.commands.soulExport,
      Data.commands.soulExport, CommandRunner::FCommandAliasUpdate()};
  const CommandRunner::FCommandOutput MissingValue{
      Runtime.statuses.ok, Data.syntax.invalidCommandMessage,
      Data.commands.soulExport, CommandRunner::FCommandAliasUpdate()};

  TestEqual(
      Data.messages.capturedValuePreservesSuccess,
      CommandRunner::ValidateOutputAssertions(Command, Observable, Aliases)
          .Status,
      Runtime.statuses.ok);
  TestEqual(
      Data.messages.missingCapturedValueFails,
      CommandRunner::ValidateOutputAssertions(Command, MissingValue, Aliases)
          .Status,
      Runtime.statuses.error);
  FCommandSpec LiteralCommand;
  LiteralCommand.Group = Runtime.commandGroups.memory_list;
  LiteralCommand.Command = Data.commands.soulExport;
  FOutputAssertion LiteralAssertion;
  LiteralAssertion.Kind = Runtime.outputAssertionKinds.includesText;
  LiteralAssertion.Value = Data.commands.soulExport;
  LiteralCommand.OutputAssertions.Add(LiteralAssertion);
  TestEqual(
      Data.messages.requiredLiteralPreservesSuccess,
      CommandRunner::ValidateOutputAssertions(LiteralCommand, Observable,
                                              Aliases)
          .Status,
      Runtime.statuses.ok);
  TestEqual(
      Data.messages.missingLiteralFails,
      CommandRunner::ValidateOutputAssertions(LiteralCommand, MissingValue,
                                              Aliases)
          .Status,
      Runtime.statuses.error);
  TestEqual(
      Data.messages.missingAliasFails,
      CommandRunner::ValidateOutputAssertions(
          Command, Observable, CommandRunner::FCommandAliasState())
          .Status,
      Runtime.statuses.error);
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
