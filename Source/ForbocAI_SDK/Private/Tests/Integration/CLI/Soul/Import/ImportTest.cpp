#include "CLI/CliHandlers.h"
#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/NPC/NPCThunks.h"
#include "Features/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Features/Testing/Memory/TestingMemoryAdapters.h"
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Store.h"

// @covers:cliOp:importNpcSoulMemories
// @covers:coreThunk:clearNodeMemoryThunk
// @covers:coreThunk:listNodeMemoryThunk
// @covers:coreThunk:recallNodeMemoryThunk
// @covers:coreThunk:storeNodeMemoryThunk

using namespace rtk;

namespace {

/**
 * User Story: As a CLI integration test, I need one authored memory selected
 * without depending on scenario order so persistence coverage remains stable
 * as fixture suites grow.
 * @fn func::Maybe<FMemoryItem> FirstAuthoredMemory()
 */
func::Maybe<FMemoryItem> FirstAuthoredMemory() {
  using namespace Testing::Memory;
  const func::Maybe<FMemoryTestScenario> Scenario =
      func::find_array<FMemoryTestScenario>(
          TestingMemoryFixtures().Scenarios,
          [](const FMemoryTestScenario &Value) {
            return func::any_array<FMemoryTestStep>(
                Value.Steps, [](const FMemoryTestStep &Step) {
                  return !Step.Action.Items.IsEmpty();
                });
          });
  return func::mbind(
      Scenario, [](const FMemoryTestScenario &Value) {
        return func::maybe_map(
            func::find_array<FMemoryTestStep>(
                Value.Steps, [](const FMemoryTestStep &Step) {
                  return !Step.Action.Items.IsEmpty();
                }),
            [](const FMemoryTestStep &Step) { return Step.Action.Items[0]; });
      });
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpsImportNpcSoulMemoriesTest,
    "ForbocAI.Integration.Ops.ImportNpcSoulMemories",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As a Soul importer, I need authenticated memories persisted by
 * the CLI through the package root store and native SQLite boundary without
 * changing their identity or chronology.
 * @fn bool FOpsImportNpcSoulMemoriesTest::RunTest(const FString &Parameters)
 */
bool FOpsImportNpcSoulMemoriesTest::RunTest(const FString &Parameters) {
  const func::Maybe<FMemoryItem> Authored = FirstAuthoredMemory();
  TestTrue("An authored memory fixture is available", Authored.hasValue);
  if (!Authored.hasValue) {
    return false;
  }

  EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  const FString DatabaseName = FGuid::NewGuid().ToString(EGuidFormats::Digits);
  const TArray<FMemoryItem> Memories{Authored.value};

  Ops::importNpcSoulMemories(Store, DatabaseName, Memories);
  const TArray<FMemoryItem> Persisted = AsyncAdapters::waitForResult(
      Store.dispatch(rtk::listNodeMemoryThunk(Memories.Num(), int32{},
                                               DatabaseName)));

  TestEqual("One imported memory is persisted", Persisted.Num(),
            Memories.Num());
  if (Persisted.Num() == Memories.Num()) {
    TestEqual("Imported identity is preserved", Persisted[0].Id,
              Authored.value.Id);
    TestEqual("Imported content is preserved", Persisted[0].Text,
              Authored.value.Text);
    TestEqual("Imported type is preserved", Persisted[0].Type,
              Authored.value.Type);
    TestEqual("Imported timestamp is preserved", Persisted[0].Timestamp,
              Authored.value.Timestamp);
    TestEqual("Imported importance is preserved", Persisted[0].Importance,
              Authored.value.Importance);
  }

  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(Store.getState().CLI);
  const CLIOps::Handlers::HandlerResult Listed =
      CLIOps::Handlers::HandleMemory(Store, Roles.MemoryList,
                                    TArray<FString>{DatabaseName});
  TestTrue("Memory list command handles the authored operation",
           Listed.hasValue);
  if (Listed.hasValue) {
    TestTrue("Memory list command reports success", Listed.value.bSuccess);
    TestTrue(
        "Memory list command result contains restored content",
        FString(UTF8_TO_TCHAR(Listed.value.message.c_str()))
            .Contains(Authored.value.Text.Left(
                ForbocAI::CLI::Memory::selectCliMemory(Store.getState().CLI)
                    .Limits.SnippetLength)));
  }

  const MemoryConfiguration::FMemoryData &MemoryData =
      MemoryConfiguration::memoryData();
  const TArray<FMemoryItem> Recalled = AsyncAdapters::waitForResult(
      Store.dispatch(rtk::recallNodeMemoryThunk(
          Authored.value.Text, Memories.Num(),
          MemoryData.Defaults.RecallThreshold, DatabaseName)));
  TestEqual("Imported memory is retrievable through sqlite-vec", Recalled.Num(),
            Memories.Num());
  if (Recalled.Num() == Memories.Num()) {
    TestEqual("Vector recall preserves imported identity", Recalled[0].Id,
              Authored.value.Id);
  }

  AsyncAdapters::waitForResult(
      Store.dispatch(rtk::clearNodeMemoryThunk(DatabaseName)));
  return true;
}
