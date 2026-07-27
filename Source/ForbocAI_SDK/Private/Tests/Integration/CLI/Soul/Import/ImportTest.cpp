#include "Systems/CLI/CommandRouting/CommandRoutingThunks.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Entities/CLI/CLISelectors.h"
#include "Systems/CLI/NPC/NPCThunks.h"
#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"
#include "Systems/Testing/Memory/TestingMemoryAdapters.h"
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
            [](const FMemoryTestStep &Step) { return Step.Action.Items[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA]; });
      });
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOpsImportNpcSoulMemoriesTest,
    FORBOCAI_SDK_AUTHORED_STRINGV00A880FCD4FB,
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
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV40F4B70FDC5F, Authored.hasValue);
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

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV64213D03B5AD, Persisted.Num(),
            Memories.Num());
  if (Persisted.Num() == Memories.Num()) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV621BD5DDB684, Persisted[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Id,
              Authored.value.Id);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV822B1EE27B96, Persisted[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Text,
              Authored.value.Text);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVF074ADDFA1AD, Persisted[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Type,
              Authored.value.Type);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVFEAD1CF3CDD6, Persisted[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Timestamp,
              Authored.value.Timestamp);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVFCD3E4B045D2, Persisted[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Importance,
              Authored.value.Importance);
  }

  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(Store.getState().CLI);
  const CLIOps::CommandRouting::RouteResult Listed =
      CLIOps::CommandRouting::RouteMemoryCommand(Store, Roles.MemoryList,
                                    TArray<FString>{DatabaseName});
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVF931362EE3F8,
           Listed.hasValue);
  if (Listed.hasValue) {
    TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVC3C9320E666D, Listed.value.bSuccess);
    TestTrue(
        FORBOCAI_SDK_AUTHORED_STRINGV248AEBD18682,
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
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV6389CE9A50E3, Recalled.Num(),
            Memories.Num());
  if (Recalled.Num() == Memories.Num()) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV4B9FE78A4576, Recalled[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Id,
              Authored.value.Id);
  }

  AsyncAdapters::waitForResult(
      Store.dispatch(rtk::clearNodeMemoryThunk(DatabaseName)));
  return true;
}
