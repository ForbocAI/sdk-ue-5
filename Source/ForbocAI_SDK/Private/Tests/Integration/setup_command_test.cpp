#include "Systems/CLI/CLIThunks.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Systems/Async/AsyncAdapters.h"
#include "Systems/Testing/Dependencies/Command/CommandAdapters.h"
#include "Systems/Testing/CLI/Invocation/InvocationAdapters.h"
#include "Entities/CLI/CLISelectors.h"
#include "Systems/CLI/Invocation/InvocationAdapters.h"
#include "Systems/Store/StoreAdapters.h"
#include "Misc/AutomationTest.h"

// @covers:cli:setup
// @covers:cli:setup_check
// @covers:cliOp:checkNativeDependencies
// @covers:cliOp:setupNativeDependencies
// @covers:cliOp:refreshNativeDependencies

namespace {

struct FCommandOutcome {
  bool bCompleted = false;
  FString Error;
};

/** User Story: As a tests integration consumer, I need to invoke run command through a stable signature so the tests integration workflow remains explicit and composable. @fn FCommandOutcome RunCommand(const FString &Command, const TArray<FString> &Arguments) */
FCommandOutcome RunCommand(const FString &Command,
                           const TArray<FString> &Arguments) {
  const func::TestResult<void> Result =
      CLIOps::DispatchCommand(Command, Arguments);
  return FCommandOutcome{
      Result.isSuccessful(),
      Result.isSuccessful() || Result.message.empty()
          ? FString()
          : FString(UTF8_TO_TCHAR(Result.message.c_str()))};
}

/** User Story: As a tests integration consumer, I need to invoke test successful outcome through a stable signature so the tests integration workflow remains explicit and composable. @fn void TestSuccessfulOutcome(FAutomationTestBase &Test, const FString &Command, const FCommandOutcome &Outcome) */
void TestSuccessfulOutcome(FAutomationTestBase &Test,
                           const FString &Command,
                           const FCommandOutcome &Outcome) {
  Test.TestTrue(Command, Outcome.bCompleted);
  Test.TestTrue(Command, Outcome.Error.IsEmpty());
}

} // namespace

/**
 * Test: setup_check passes CLI validation and executes.
 * User Story: As CLI automation, I need setup commands accepted by the
 * SDK CLI layer so the supported entrypoint can run them.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSetupCliDispatchTest,
    FORBOCAI_SDK_AUTHORED_STRINGVD8297A1D4847,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FSetupCliDispatchTest::RunTest(const FString &Parameters)
 */
bool FSetupCliDispatchTest::RunTest(const FString &Parameters) {
  func::for_each_array<Testing::Dependencies::Command::FSetupTestCommand>(
      Testing::Dependencies::Command::SetupTestFixtures().Commands,
      [this](
          const Testing::Dependencies::Command::FSetupTestCommand &Command) {
        TestSuccessfulOutcome(
            *this, Command.Label,
            RunCommand(Command.Key, Command.Arguments));
      });
  const auto &Fixtures =
      Testing::CLI::Invocation::InvocationTestFixtures();
  const ForbocAI::CLI::FCLIState &CLIState =
      StoreAdapters::RootStore().getState().CLI;
  const int32 AuthoredNodeCommandCount =
      func::filter_array<ForbocAI::CLI::FCLICommandSpec>(
          ForbocAI::CLI::selectCliCommandMatrix(CLIState),
          [&CLIState](const ForbocAI::CLI::FCLICommandSpec &Command) {
            return Command.Surfaces.Contains(
                ForbocAI::CLI::selectNodeCliSurface(CLIState));
          })
          .Num();
  TestEqual(
      Fixtures.Labels.NodeCommandCount,
      ForbocAI::CLI::selectCliCommandKeys(
          CLIState, ForbocAI::CLI::selectNodeCliSurface(CLIState))
          .Num(),
      AuthoredNodeCommandCount);
  func::for_each_array<Testing::CLI::Invocation::FInvocationTestScenario>(
      Fixtures.Scenarios,
      [this, &Fixtures, &CLIState](
          const Testing::CLI::Invocation::FInvocationTestScenario &Scenario) {
        const CommandletInvocation::FInvocation Invocation =
            CommandletInvocation::ResolveInvocation(
                Scenario.CommandletParams, CLIState);
        TestEqual(Fixtures.Labels.CommandMatched,
                  !Invocation.Command.IsEmpty(),
                  Scenario.bExpectedMatched);
        TestEqual(Fixtures.Labels.CommandKey, Invocation.Command,
                  func::or_else(Scenario.ExpectedKey, FString()));
        TestTrue(Fixtures.Labels.Arguments,
                 Invocation.Args == Scenario.ExpectedArgs);
        TestEqual(Fixtures.Labels.ApiUrl, Invocation.ApiUrl,
                  func::or_else(Scenario.ExpectedApiUrl, FString()));
        TestEqual(Fixtures.Labels.ApiKey, Invocation.ApiKey,
                  func::or_else(Scenario.ExpectedApiKey, FString()));
      });
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDependenciesStoreLifecycleTest,
    FORBOCAI_SDK_AUTHORED_STRINGVC34A2D3191EC,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests integration consumer, I need to invoke run test through a stable signature so the tests integration workflow remains explicit and composable. @fn bool FDependenciesStoreLifecycleTest::RunTest(const FString &Parameters) */
bool FDependenciesStoreLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  Store.dispatch(rtk::checkNativeDependenciesThunk().pending(rtk::FEmptyPayload{}));

  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV9AEBFA414047,
           DependenciesSelectors::selectDependenciesBusy(Store.getState()));

  FNativeDependenciesReport Report;
  Report.Vectorizer.bAvailable = true;
  Report.VectorDb.bAvailable = true;
  Store.dispatch(rtk::checkNativeDependenciesThunk().fulfilled(Report));

  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV74A7AA577E1E,
           DependenciesSelectors::selectNativeDependenciesReady(
               Store.getState().Dependencies));
  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGVC6AAC7A44366,
            DependenciesSelectors::selectDependenciesBusy(Store.getState()));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV673BC90CA300,
            Store.getState().Vector.Status, FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVectorStoreLifecycleTest,
    FORBOCAI_SDK_AUTHORED_STRINGV97E593F410B5,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests integration consumer, I need to invoke run test through a stable signature so the tests integration workflow remains explicit and composable. @fn bool FVectorStoreLifecycleTest::RunTest(const FString &Parameters) */
bool FVectorStoreLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  Store.dispatch(rtk::initVectorThunk().pending(rtk::FEmptyPayload{}));

  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV64ADE3E4EF5F,
           VectorSelectors::selectVectorBusy(Store.getState()));
  Store.dispatch(rtk::initVectorThunk().fulfilled(rtk::FEmptyPayload{}));

  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV6EFD34CC6C83,
           VectorSelectors::selectVectorReady(Store.getState().Vector));
  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGVEEE151CEC773,
            VectorSelectors::selectVectorBusy(Store.getState()));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVDB8D998D0EF3,
           Store.getState().Vector.bIsReady);

  const TArray<float> Embedding = AsyncAdapters::waitForResult(
      Store.dispatch(rtk::generateEmbeddingThunk()(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV8195ED9A46FF))));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV4FA8419F8B9B,
            Embedding.Num(), FORBOCAI_SDK_AUTHORED_NUMBERVBD585E4075C4);
  return true;
}
