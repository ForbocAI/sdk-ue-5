#include "Misc/AutomationTest.h"
#include "TestGame/Features/Systems/Memory/MemorySelectors.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameStoreDomainSelectorsTest,
    "ForbocAI.Slices.TestGame.StoreDomainSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices store consumer, I need to invoke run test through a stable signature so the tests slices store workflow remains explicit and composable. @fn bool FTestGameStoreDomainSelectorsTest::RunTest(const FString &Parameters) */
bool FTestGameStoreDomainSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  const rtk::Slice<FGameMemorySliceState> MemorySlice = CreateGameMemorySlice();
  FMemoryRecord FirstMemory;
  FirstMemory.Id = TEXT("mem-1");
  FirstMemory.NpcId = TEXT("miller");
  FirstMemory.Text = TEXT("Keeps a medkit hidden.");
  FMemoryRecord SecondMemory;
  SecondMemory.Id = TEXT("mem-2");
  SecondMemory.NpcId = TEXT("scout");
  SecondMemory.Text = TEXT("Saw the exit route.");
  FGameMemorySliceState MemoryState = MemorySlice.Reducer(
      FGameMemorySliceState(), GameMemoryActions::storeMemory(FirstMemory));
  MemoryState = MemorySlice.Reducer(
      MemoryState, GameMemoryActions::storeMemory(SecondMemory));

  TestEqual("Memory total selector reads adapter state",
            GameMemorySelectors::SelectMemoryTotal(MemoryState), 2);
  TestTrue("Memory by id selector finds entity",
           GameMemorySelectors::SelectMemoryById(MemoryState, TEXT("mem-1"))
               .hasValue);
  TestEqual("Memory by NPC selector filters records",
            GameMemorySelectors::SelectMemoriesByNpcId(MemoryState,
                                                       TEXT("miller"))
                .Num(),
            1);

  FSetOwnerInventoryPayload InventoryPayload;
  InventoryPayload.OwnerId = TEXT("scout");
  InventoryPayload.Items.Add(TEXT("coin-pouch"));
  InventoryPayload.Items.Add(TEXT("signal-key"));
  const FInventoryState InventoryState = CreateInventorySlice().Reducer(
      FInventoryState(), InventoryActions::setOwnerInventory(InventoryPayload));
  TestEqual("Owner inventory selector reads keyed state",
            InventorySelectors::SelectOwnerInventory(InventoryState,
                                                     TEXT("scout"))
                .Num(),
            2);

  FMarkSoulExportedPayload ExportPayload;
  ExportPayload.NpcId = TEXT("miller");
  ExportPayload.TxId = TEXT("tx-001");
  const rtk::Slice<FSoulTrackingState> SoulSlice = CreateGameSoulSlice();
  FSoulTrackingState SoulState = SoulSlice.Reducer(
      FSoulTrackingState(), GameSoulActions::markSoulExported(ExportPayload));
  SoulState = SoulSlice.Reducer(
      SoulState, GameSoulActions::markSoulImported(TEXT("tx-001")));

  const func::Maybe<FString> TxId =
      GameSoulSelectors::SelectSoulExportTxId(SoulState, TEXT("miller"));
  TestTrue("Soul export selector finds tx id", TxId.hasValue);
  TestEqual("Soul import selector reads tx ids",
            GameSoulSelectors::SelectImportedSoulTxIds(SoulState).Num(), 1);

  return true;
}
