#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Entities/Memory/MemorySelectors.h"
#include "Entities/Memory/MemorySlice.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProtocolMemoryLifecycleTest,
    FORBOCAI_SDK_AUTHORED_STRINGV6C989B163B73,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a integration protocol memory consumer, I need to invoke run test through a stable signature so the integration protocol memory workflow remains explicit and composable. @fn bool FProtocolMemoryLifecycleTest::RunTest(const FString &Parameters) */
bool FProtocolMemoryLifecycleTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  TestStore.dispatch(MemorySlice::Actions::memoryStoreStart());
  FRuntimeState State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVEAB1314FA46C, State.Memory.StorageStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3CA02E045795)));

  FMemoryItem Item;
  Item.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4DE20AE47624);
  Item.Text = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE317B018DD0A);
  Item.Type = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV90F5E365EC02);
  Item.Importance = FORBOCAI_SDK_AUTHORED_NUMBERV44BB259064FE;
  Item.Timestamp = FORBOCAI_SDK_AUTHORED_NUMBERVF62CF8F54FF5;
  TestStore.dispatch(MemorySlice::Actions::memoryStoreSuccess(Item));
  State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV99EBA705FD1F, State.Memory.StorageStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));

  TestStore.dispatch(MemorySlice::Actions::memoryRecallStart());
  State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV8026A6B6612D, State.Memory.RecallStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVBFCA76F71AEF)));

  TArray<FMemoryItem> Recalled;
  FMemoryItem Recalled1;
  Recalled1.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4DE20AE47624);
  Recalled1.Text = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE317B018DD0A);
  Recalled1.Similarity = FORBOCAI_SDK_AUTHORED_NUMBERVA03A0A41C51F;
  Recalled.Add(Recalled1);
  TestStore.dispatch(MemorySlice::Actions::memoryRecallSuccess(Recalled));

  State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVAF6841B0CA82, State.Memory.RecallStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  const TArray<FMemoryItem> RecalledState =
      MemorySelectors::selectRecalledMemories(State.Memory);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV5CB44F4F301A, RecalledState.Num(), FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV549468EC67D1, RecalledState[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Text,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE317B018DD0A)));

  TestStore.dispatch(MemorySlice::Actions::memoryStoreStart());
  TestStore.dispatch(
      MemorySlice::Actions::memoryStoreFailed(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV52695409D59F)));
  State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV6E27C3EB9A0B, State.Memory.StorageStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2851A113080D)));

  TestStore.dispatch(MemorySlice::Actions::memoryRecallStart());
  TestStore.dispatch(
      MemorySlice::Actions::memoryRecallFailed(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1F4B06C11144)));
  State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV42E497A4A640, State.Memory.RecallStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2851A113080D)));

  TestStore.dispatch(MemorySlice::Actions::memoryClear());
  State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV66D15AE79056,
            MemorySelectors::selectAllMemories(State.Memory).Num(), FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
  return true;
}
