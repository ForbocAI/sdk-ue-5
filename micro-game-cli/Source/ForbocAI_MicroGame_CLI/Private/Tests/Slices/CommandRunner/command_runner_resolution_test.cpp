#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/Configuration/ConfigurationAdapters.h"
#include "Systems/CLI/CLIThunks.h"

using namespace MicroGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMicroGameCommandRunnerResolutionTest,
    CommandRunner::CommandRunnerData().testNames.commandResolution,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a micro-game author, I need the SDK root catalog to resolve every CLI command so the harness never duplicates command grammar.
 * @fn bool FMicroGameCommandRunnerResolutionTest::RunTest( const FString &Parameters)
 */
bool FMicroGameCommandRunnerResolutionTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const CommandRunner::FCommandRunnerData &Data =
      CommandRunner::CommandRunnerData();
  const TArray<FString> Tokens{
      Data.commands.ghostDomain, Data.commands.ghostMemoryStoreAction,
      Data.aliases.ghostSession, Data.evidence.memoryObservation};
  const ForbocAI::CLI::FCommandParseResult Parsed =
      CLIOps::ResolveCommandTokens(Tokens);

  TestTrue(Data.messages.sdkCatalogMatchesHyphenatedCommand,
           Parsed.bMatched);
  TestEqual(Data.messages.sdkCatalogMatchesHyphenatedCommand,
            Parsed.CommandKey, Data.commands.ghostMemoryStore);
  TestEqual(Data.messages.sdkCatalogPreservesCommandArguments,
            Parsed.Args.Num(), Data.limits.expectedResolvedArgumentCount);
  TestEqual(Data.messages.sdkCatalogPreservesCommandArguments,
            Parsed.Args[Data.limits.firstArgumentIndex],
            Data.aliases.ghostSession);
  TestEqual(Data.messages.sdkCatalogPreservesCommandArguments,
            Parsed.Args[Data.limits.nextIndex],
            Data.evidence.memoryObservation);
  return true;
}
