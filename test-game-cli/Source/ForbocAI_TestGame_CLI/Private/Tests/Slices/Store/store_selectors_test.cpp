#include "Misc/AutomationTest.h"
#include "TestGame/Features/Components/Inventory/InventorySelectors.h"
#include "TestGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "TestGame/Features/Systems/Memory/MemorySelectors.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameStoreDomainSelectorsTest,
    VerificationAdapters::ArchitectureTestData()
        .stateDomains.automationNames.store,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices store consumer, I need to invoke run test through a stable signature so the tests slices store workflow remains explicit and composable. @fn bool FTestGameStoreDomainSelectorsTest::RunTest(const FString &Parameters) */
bool FTestGameStoreDomainSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;
  const Verification::FStateDomainsVerificationData &Data =
      VerificationAdapters::ArchitectureTestData().stateDomains;
  FTestGameStore Store = createTestGameStore();

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

  return true;
}
