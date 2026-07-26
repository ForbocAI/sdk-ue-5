#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Components/Inventory/InventorySelectors.h"
#include "MicroGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/CrossSdkConformanceAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationSelectors.h"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioActions.h"
#include "MicroGame/Features/Systems/Memory/MemorySelectors.h"
#include "MicroGame/Features/Systems/Terminal/UI/UIActions.h"
#include "MicroGame/MicroGameStore.h"

using namespace MicroGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMicroGameStoreDomainSelectorsTest,
    CrossSdkConformanceAdapters::CrossSdkConformanceData()
        .stateDomains.automationNames.store,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices store consumer, I need to invoke run test through a stable signature so the tests slices store workflow remains explicit and composable. @fn bool FMicroGameStoreDomainSelectorsTest::RunTest(const FString &Parameters) */
bool FMicroGameStoreDomainSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const CrossSdkConformance::FStateDomainsVerificationData &Data =
      CrossSdkConformanceAdapters::CrossSdkConformanceData().stateDomains;
  FMicroGameStore Store = createMicroGameStore();

  func::for_each_array<FMemoryRecord>(
      Data.store.memory.records, [&Store](const FMemoryRecord &Memory) {
        Store.dispatch(GameMemoryActions::storeMemory(Memory));
      });
  const FGameMemorySliceState &MemoryState = Store.getState().Memory;

  TestEqual(Data.stories.store,
            GameMemorySelectors::SelectMemoryTotal(MemoryState),
            Data.store.memory.expectedTotal);
  TestTrue(Data.stories.store,
           GameMemorySelectors::SelectMemoryById(
               MemoryState, Data.store.memory.lookupId)
               .hasValue);
  TestEqual(Data.stories.store,
            GameMemorySelectors::SelectMemoriesByNpcId(MemoryState,
                                                       Data.store.memory.npcId)
                .Num(),
            Data.store.memory.expectedNpcCount);

  FSetOwnerInventoryPayload InventoryPayload;
  InventoryPayload.OwnerId = Data.store.inventory.ownerId;
  InventoryPayload.Items = Data.store.inventory.items;
  Store.dispatch(InventoryActions::setOwnerInventory(InventoryPayload));
  TestEqual(Data.stories.store,
            InventorySelectors::SelectOwnerInventory(
                Store.getState().Inventory, Data.store.inventory.ownerId)
                .Num(),
            Data.store.inventory.items.Num());

  FMarkSoulExportedPayload ExportPayload;
  ExportPayload.NpcId = Data.store.soul.npcId;
  ExportPayload.TxId = Data.store.soul.transactionId;
  Store.dispatch(GameSoulActions::markSoulExported(ExportPayload));
  Store.dispatch(
      GameSoulActions::markSoulImported(Data.store.soul.transactionId));

  const func::Maybe<FString> TxId =
      GameSoulSelectors::SelectSoulExportTxId(Store.getState().Soul,
                                               Data.store.soul.npcId);
  TestTrue(Data.stories.store, TxId.hasValue);
  TestEqual(Data.stories.store,
            GameSoulSelectors::SelectImportedSoulTxIds(Store.getState().Soul)
                .Num(),
            Data.store.soul.expectedImportedCount);

  FScenarioContractPayload ChatContract;
  ChatContract.RequiredCommandGroups = {
      VerificationVocabularyAdapters::GameRuntimeData().commandGroups.status};
  Store.dispatch(ScenarioActions::setContract(MoveTemp(ChatContract)));
  Store.dispatch(UIActions::setMode(
      VerificationVocabularyAdapters::GameRuntimeData().modes.chat));
  const FGameRunResult MissingConversationResult =
      VerificationSelectors::SelectGameRunResult(Store.getState());
  TestEqual(Data.stories.store,
            MissingConversationResult.MissingGroups.Num(),
            VerificationVocabularyAdapters::GameRuntimeData().numbers.nextIndex);
  TestEqual(Data.stories.store,
            MissingConversationResult.MissingGroups[
                VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount],
            VerificationVocabularyAdapters::GameRuntimeData().commandGroups.npc_conversation);
  Store.dispatch(CoverageActions::markCovered(
      VerificationVocabularyAdapters::GameRuntimeData().commandGroups.npc_conversation));
  const FGameRunResult ChatResult =
      VerificationSelectors::SelectGameRunResult(Store.getState());
  TestEqual(Data.stories.store, ChatResult.MissingGroups.Num(),
            VerificationVocabularyAdapters::GameRuntimeData().numbers.emptyCount);

  return true;
}
