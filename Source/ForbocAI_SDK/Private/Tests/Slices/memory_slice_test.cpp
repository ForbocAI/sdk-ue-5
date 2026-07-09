#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Memory/MemorySlice.h"
#include "Misc/AutomationTest.h"

using namespace rtk;
using namespace MemorySlice;

/**
 * Test: memoryStoreStart / Success / Failed lifecycle
 * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemorySliceStoreTest,
                                 "ForbocAI.Slices.Memory.StoreLifecycle",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMemorySliceStoreTest::RunTest(const FString &Parameters) {
  Slice<FMemorySliceState> MemSlice = createMemorySlice();
  FMemorySliceState State;

  /**
   * Initial state
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  TestEqual("Initial StorageStatus", State.StorageStatus,
            FString(TEXT("idle")));

  /**
   * StoreStart
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  AnyAction StoreStart = MemorySlice::Actions::memoryStoreStart();
  TestEqual("memoryStoreStart action type", StoreStart.Type,
            FString(TEXT("memory/memoryStoreStart")));
  State = MemSlice.Reducer(State, StoreStart);
  TestEqual("StorageStatus storing", State.StorageStatus,
            FString(TEXT("storing")));

  /**
   * StoreSuccess
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  FMemoryItem Item;
  Item.Id = TEXT("mem_1");
  Item.Text = TEXT("The dragon appeared");
  Item.Importance = 0.9f;
  AnyAction StoreSuccess = MemorySlice::Actions::memoryStoreSuccess(Item);
  TestEqual("memoryStoreSuccess action type", StoreSuccess.Type,
            FString(TEXT("memory/memoryStoreSuccess")));
  State = MemSlice.Reducer(State, StoreSuccess);
  TestEqual("StorageStatus idle after success", State.StorageStatus,
            FString(TEXT("idle")));

  func::Maybe<FMemoryItem> Found = selectMemoryById(State, TEXT("mem_1"));
  TestTrue("Memory item stored", Found.hasValue);
  if (Found.hasValue) {
    TestEqual("Memory text", Found.value.Text,
              FString(TEXT("The dragon appeared")));
  }

  return true;
}

/**
 * Test: memoryStoreFailed sets error
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemorySliceStoreFailTest,
                                 "ForbocAI.Slices.Memory.StoreFailed",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMemorySliceStoreFailTest::RunTest(const FString &Parameters) {
  Slice<FMemorySliceState> MemSlice = createMemorySlice();
  FMemorySliceState State;

  State = MemSlice.Reducer(State, MemorySlice::Actions::memoryStoreStart());
  AnyAction StoreFailed =
      MemorySlice::Actions::memoryStoreFailed(TEXT("Network error"));
  TestEqual("memoryStoreFailed action type", StoreFailed.Type,
            FString(TEXT("memory/memoryStoreFailed")));
  State = MemSlice.Reducer(State, StoreFailed);

  TestEqual("StorageStatus error", State.StorageStatus,
            FString(TEXT("error")));
  TestEqual("Error message set", State.Error, FString(TEXT("Network error")));

  return true;
}

/**
 * Test: memoryRecallStart / Success / Failed lifecycle
 * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemorySliceRecallTest,
                                 "ForbocAI.Slices.Memory.RecallLifecycle",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMemorySliceRecallTest::RunTest(const FString &Parameters) {
  Slice<FMemorySliceState> MemSlice = createMemorySlice();
  FMemorySliceState State;

  /**
   * RecallStart
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  AnyAction RecallStart = MemorySlice::Actions::memoryRecallStart();
  TestEqual("memoryRecallStart action type", RecallStart.Type,
            FString(TEXT("memory/memoryRecallStart")));
  State = MemSlice.Reducer(State, RecallStart);
  TestEqual("RecallStatus recalling", State.RecallStatus,
            FString(TEXT("recalling")));

  /**
   * RecallSuccess with multiple items
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TArray<FMemoryItem> Items;
  FMemoryItem M1;
  M1.Id = TEXT("r1");
  M1.Text = TEXT("First memory");
  M1.Importance = 0.5f;
  Items.Add(M1);

  FMemoryItem M2;
  M2.Id = TEXT("r2");
  M2.Text = TEXT("Second memory");
  M2.Importance = 0.7f;
  Items.Add(M2);

  AnyAction RecallSuccess = MemorySlice::Actions::memoryRecallSuccess(Items);
  TestEqual("memoryRecallSuccess action type", RecallSuccess.Type,
            FString(TEXT("memory/memoryRecallSuccess")));
  State = MemSlice.Reducer(State, RecallSuccess);
  TestEqual("RecallStatus idle after success", State.RecallStatus,
            FString(TEXT("idle")));
  TestEqual("LastRecalledIds count", State.LastRecalledIds.Num(), 2);
  TestEqual("All memories count", selectAllMemories(State).Num(), 2);

  TArray<FMemoryItem> Recalled = selectLastRecalledMemories(State);
  TestEqual("LastRecalled count", Recalled.Num(), 2);

  return true;
}

/**
 * Test: memoryRecallFailed
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemorySliceRecallFailTest,
                                 "ForbocAI.Slices.Memory.RecallFailed",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMemorySliceRecallFailTest::RunTest(const FString &Parameters) {
  Slice<FMemorySliceState> MemSlice = createMemorySlice();
  FMemorySliceState State;

  State = MemSlice.Reducer(State, MemorySlice::Actions::memoryRecallStart());
  AnyAction RecallFailed =
      MemorySlice::Actions::memoryRecallFailed(TEXT("Timeout"));
  TestEqual("memoryRecallFailed action type", RecallFailed.Type,
            FString(TEXT("memory/memoryRecallFailed")));
  State = MemSlice.Reducer(State, RecallFailed);

  TestEqual("RecallStatus error", State.RecallStatus, FString(TEXT("error")));
  TestEqual("Error set", State.Error, FString(TEXT("Timeout")));

  return true;
}

/**
 * Test: memoryClear resets to initial state
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMemorySliceClearTest,
                                 "ForbocAI.Slices.Memory.Clear",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FMemorySliceClearTest::RunTest(const FString &Parameters) {
  Slice<FMemorySliceState> MemSlice = createMemorySlice();
  FMemorySliceState State;

  FMemoryItem Item;
  Item.Id = TEXT("clr_1");
  Item.Text = TEXT("Will be cleared");
  Item.Importance = 0.5f;
  State = MemSlice.Reducer(State, MemorySlice::Actions::memoryStoreSuccess(Item));
  TestEqual("One memory before clear", selectAllMemories(State).Num(), 1);

  AnyAction Clear = MemorySlice::Actions::memoryClear();
  TestEqual("memoryClear action type", Clear.Type,
            FString(TEXT("memory/memoryClear")));
  State = MemSlice.Reducer(State, Clear);
  TestEqual("No memories after clear", selectAllMemories(State).Num(), 0);
  TestEqual("StorageStatus reset", State.StorageStatus,
            FString(TEXT("idle")));
  TestEqual("RecallStatus reset", State.RecallStatus, FString(TEXT("idle")));

  return true;
}
