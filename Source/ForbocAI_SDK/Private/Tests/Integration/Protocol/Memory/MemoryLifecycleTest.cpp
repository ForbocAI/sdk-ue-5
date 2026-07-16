#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Memory/MemorySelectors.h"
#include "Features/Memory/MemorySlice.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProtocolMemoryLifecycleTest,
    "ForbocAI.Integration.Protocol.MemoryLifecycle",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a integration protocol memory consumer, I need to invoke run test through a stable signature so the integration protocol memory workflow remains explicit and composable. @fn bool FProtocolMemoryLifecycleTest::RunTest(const FString &Parameters) */
bool FProtocolMemoryLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  TestStore.dispatch(MemorySlice::Actions::memoryStoreStart());
  FRuntimeState State = TestStore.getState();
  TestEqual("Store status storing", State.Memory.StorageStatus,
            FString(TEXT("storing")));

  FMemoryItem Item;
  Item.Id = TEXT("mem_1");
  Item.Text = TEXT("The player found a key");
  Item.Type = TEXT("observation");
  Item.Importance = 0.8f;
  Item.Timestamp = 1000;
  TestStore.dispatch(MemorySlice::Actions::memoryStoreSuccess(Item));
  State = TestStore.getState();
  TestEqual("Store status idle", State.Memory.StorageStatus,
            FString(TEXT("idle")));

  TestStore.dispatch(MemorySlice::Actions::memoryRecallStart());
  State = TestStore.getState();
  TestEqual("Recall status recalling", State.Memory.RecallStatus,
            FString(TEXT("recalling")));

  TArray<FMemoryItem> Recalled;
  FMemoryItem Recalled1;
  Recalled1.Id = TEXT("mem_1");
  Recalled1.Text = TEXT("The player found a key");
  Recalled1.Similarity = 0.9f;
  Recalled.Add(Recalled1);
  TestStore.dispatch(MemorySlice::Actions::memoryRecallSuccess(Recalled));

  State = TestStore.getState();
  TestEqual("Recall status idle", State.Memory.RecallStatus,
            FString(TEXT("idle")));
  const TArray<FMemoryItem> RecalledState =
      MemorySelectors::selectRecalledMemories(State.Memory);
  TestEqual("One recalled memory", RecalledState.Num(), 1);
  TestEqual("Recalled text", RecalledState[0].Text,
            FString(TEXT("The player found a key")));

  TestStore.dispatch(MemorySlice::Actions::memoryStoreStart());
  TestStore.dispatch(
      MemorySlice::Actions::memoryStoreFailed(TEXT("DB connection lost")));
  State = TestStore.getState();
  TestEqual("Store status error", State.Memory.StorageStatus,
            FString(TEXT("error")));

  TestStore.dispatch(MemorySlice::Actions::memoryRecallStart());
  TestStore.dispatch(
      MemorySlice::Actions::memoryRecallFailed(TEXT("Query timeout")));
  State = TestStore.getState();
  TestEqual("Recall status error", State.Memory.RecallStatus,
            FString(TEXT("error")));

  TestStore.dispatch(MemorySlice::Actions::memoryClear());
  State = TestStore.getState();
  TestEqual("Memories cleared",
            MemorySelectors::selectAllMemories(State.Memory).Num(), 0);
  return true;
}
