#include "CLI/CliHandlers.h"
#include "CoreMinimal.h"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Testing/Dependencies/Command/CommandAdapters.h"
#include "Misc/AutomationTest.h"
#include "CLI/RuntimeCommandlet.h"
#include "Store.h"

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

FCommandOutcome RunCommand(UForbocAICommandlet &Commandlet,
                           const FString &Command,
                           const TArray<FString> &Arguments) {
  FCommandOutcome Outcome;
  Commandlet.createCommandPipeline(Command, Arguments)
      .then([&Outcome]() { Outcome.bCompleted = true; })
      .catch_([&Outcome](std::string Message) {
        Outcome.Error = UTF8_TO_TCHAR(Message.c_str());
      })
      .execute();
  return Outcome;
}

void TestSuccessfulOutcome(FAutomationTestBase &Test,
                           const FString &Command,
                           const FCommandOutcome &Outcome) {
  Test.TestTrue(Command, Outcome.bCompleted);
  Test.TestTrue(Command, Outcome.Error.IsEmpty());
}

} // namespace

/**
 * Test: setup_check passes commandlet validation and executes.
 * User Story: As CLI automation, I need setup commands accepted by the
 * commandlet validation layer so the supported entrypoint can run them.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSetupCommandletValidationTest,
    "ForbocAI.Integration.Setup.CommandletValidation",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSetupCommandletValidationTest::RunTest(const FString &Parameters) {
  UForbocAICommandlet *Commandlet = NewObject<UForbocAICommandlet>();
  func::for_each_array<Testing::Dependencies::Command::FSetupTestCommand>(
      Testing::Dependencies::Command::SetupTestFixtures().Commands,
      [this, Commandlet](
          const Testing::Dependencies::Command::FSetupTestCommand &Command) {
        TestSuccessfulOutcome(
            *this, Command.Label,
            RunCommand(*Commandlet, Command.Key, Command.Arguments));
      });
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FDependenciesStoreLifecycleTest,
    "ForbocAI.Integration.Dependencies.StoreLifecycle",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FDependenciesStoreLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  Store.dispatch(rtk::checkNativeDependenciesThunk().pending(rtk::FEmptyPayload{}));

  TestTrue("Dependencies selector reports active dependency check",
           DependenciesSelectors::selectDependenciesBusy(Store.getState()));

  FNativeDependenciesReport Report;
  Report.Vectorizer.bAvailable = true;
  Report.VectorDb.bAvailable = true;
  Store.dispatch(rtk::checkNativeDependenciesThunk().fulfilled(Report));

  TestTrue("Dependencies selector derives native dependency readiness",
           DependenciesSelectors::selectNativeDependenciesReady(
               Store.getState().Dependencies));
  TestFalse("Dependencies selector clears busy state after fulfillment",
            DependenciesSelectors::selectDependenciesBusy(Store.getState()));
  TestEqual("Vector state remains independently idle",
            Store.getState().Vector.Status, FString(TEXT("idle")));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FVectorStoreLifecycleTest,
    "ForbocAI.Integration.Vector.StoreLifecycle",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FVectorStoreLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  Store.dispatch(rtk::initVectorThunk().pending(rtk::FEmptyPayload{}));

  TestTrue("Vector selector reports initialization in progress",
           VectorSelectors::selectVectorBusy(Store.getState()));
  Store.dispatch(rtk::initVectorThunk().fulfilled(rtk::FEmptyPayload{}));

  TestTrue("Vector selector derives readiness after fulfillment",
           VectorSelectors::selectVectorReady(Store.getState().Vector));
  TestFalse("Vector selector clears busy state after fulfillment",
            VectorSelectors::selectVectorBusy(Store.getState()));
  TestTrue("Vector root state is mounted in the canonical store",
           Store.getState().Vector.bIsReady);

  const TArray<float> Embedding = AsyncAdapters::waitForResult(
      Store.dispatch(rtk::generateEmbeddingThunk()(TEXT("vector parity"))));
  TestEqual("Embedding thunk returns the canonical vector dimension",
            Embedding.Num(), 384);
  return true;
}
