#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "NPC/NPCSlice.h"
#include "NPC/NPCSliceActions.h"

using namespace rtk;
using namespace NPCSlice;

/**
 * Test: setNPCInfo dispatches and updates state
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNPCSliceSetInfoTest,
                                 "ForbocAI.Slices.NPC.setNPCInfo",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FNPCSliceSetInfoTest::RunTest(const FString &Parameters) {
  Slice<FNPCSliceState> NpcSlice = createNPCSlice();

  FNPCSliceState State;
  FNPCInternalState Info;
  Info.Id = TEXT("npc_001");
  Info.Persona = TEXT("A brave knight");

  State = NpcSlice.Reducer(State, NPCSlice::Actions::setNPCInfo(Info));

  TestEqual("ActiveNpcId set", State.ActiveNpcId, FString(TEXT("npc_001")));

  func::Maybe<FNPCInternalState> Found = selectNPCById(State, TEXT("npc_001"));
  TestTrue("NPC found in entities", Found.hasValue);
  if (Found.hasValue) {
    TestEqual("NPC Id matches", Found.value.Id, FString(TEXT("npc_001")));
    TestEqual("NPC Persona matches", Found.value.Persona,
              FString(TEXT("A brave knight")));
    TestTrue("StateLog has initial entry", Found.value.StateLog.Num() > 0);
  }

  /**
   * Add a second NPC
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FNPCInternalState Info2;
  Info2.Id = TEXT("npc_002");
  Info2.Persona = TEXT("A sly rogue");
  State = NpcSlice.Reducer(State, NPCSlice::Actions::setNPCInfo(Info2));

  TestEqual("ActiveNpcId updated to second", State.ActiveNpcId,
            FString(TEXT("npc_002")));

  TArray<FNPCInternalState> AllNpcs = selectAllNPCs(State);
  TestEqual("Two NPCs in state", AllNpcs.Num(), 2);

  return true;
}

/**
 * Test: removeNPC dispatches and clears state
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNPCSliceRemoveTest,
                                 "ForbocAI.Slices.NPC.removeNPC",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FNPCSliceRemoveTest::RunTest(const FString &Parameters) {
  Slice<FNPCSliceState> NpcSlice = createNPCSlice();
  FNPCSliceState State;

  FNPCInternalState Info;
  Info.Id = TEXT("npc_rm");
  Info.Persona = TEXT("Doomed NPC");
  State = NpcSlice.Reducer(State, NPCSlice::Actions::setNPCInfo(Info));

  TestTrue("NPC exists before removal",
           selectNPCById(State, TEXT("npc_rm")).hasValue);
  TestEqual("ActiveNpcId is npc_rm", State.ActiveNpcId,
            FString(TEXT("npc_rm")));

  State = NpcSlice.Reducer(State, NPCSlice::Actions::removeNPC(TEXT("npc_rm")));

  TestFalse("NPC removed from entities",
            selectNPCById(State, TEXT("npc_rm")).hasValue);
  TestTrue("ActiveNpcId cleared", State.ActiveNpcId.IsEmpty());

  TArray<FNPCInternalState> AllNpcs = selectAllNPCs(State);
  TestEqual("No NPCs remain", AllNpcs.Num(), 0);

  return true;
}

/**
 * Test: Selectors — selectActiveNPC, selectAllNPCs, selectNPCById
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNPCSliceSelectorsTest,
                                 "ForbocAI.Slices.NPC.Selectors",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FNPCSliceSelectorsTest::RunTest(const FString &Parameters) {
  Slice<FNPCSliceState> NpcSlice = createNPCSlice();
  FNPCSliceState State;

  /**
   * Empty state selectors
   * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
   */
  func::Maybe<FNPCInternalState> EmptyActive = selectActiveNPC(State);
  TestFalse("No active NPC on empty state", EmptyActive.hasValue);
  TestEqual("selectAllNPCs empty", selectAllNPCs(State).Num(), 0);
  TestEqual("selectNPCIds empty", selectNPCIds(State).Num(), 0);
  TestEqual("selectNPCEntities empty", selectNPCEntities(State).Num(), 0);
  TestEqual("selectTotalNPCs empty", selectTotalNPCs(State), 0);
  TestFalse("selectNPCById returns nothing on empty",
            selectNPCById(State, TEXT("ghost")).hasValue);

  /**
   * Add NPCs
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FNPCInternalState A;
  A.Id = TEXT("sel_a");
  A.Persona = TEXT("Alpha");
  State = NpcSlice.Reducer(State, NPCSlice::Actions::setNPCInfo(A));

  FNPCInternalState B;
  B.Id = TEXT("sel_b");
  B.Persona = TEXT("Beta");
  State = NpcSlice.Reducer(State, NPCSlice::Actions::setNPCInfo(B));

  /**
   * Active should be last set
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  func::Maybe<FNPCInternalState> Active = selectActiveNPC(State);
  TestTrue("Active NPC exists", Active.hasValue);
  if (Active.hasValue) {
    TestEqual("Active is sel_b", Active.value.Id, FString(TEXT("sel_b")));
  }

  /**
   * selectNPCById for first NPC
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  func::Maybe<FNPCInternalState> FoundA = selectNPCById(State, TEXT("sel_a"));
  TestTrue("sel_a found", FoundA.hasValue);
  if (FoundA.hasValue) {
    TestEqual("sel_a persona", FoundA.value.Persona, FString(TEXT("Alpha")));
  }

  /**
   * selectAllNPCs
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual("Two NPCs total", selectAllNPCs(State).Num(), 2);
  TestEqual("Two NPC ids", selectNPCIds(State).Num(), 2);
  TestEqual("Total NPC selector count", selectTotalNPCs(State), 2);

  TMap<FString, FNPCInternalState> Entities = selectNPCEntities(State);
  TestEqual("Two NPC entities", Entities.Num(), 2);
  TestTrue("sel_a entity mapped", Entities.Find(TEXT("sel_a")) != nullptr);
  TestTrue("sel_b entity mapped", Entities.Find(TEXT("sel_b")) != nullptr);

  /**
   * setActiveNPC to sel_a
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  State = NpcSlice.Reducer(State, NPCSlice::Actions::setActiveNPC(TEXT("sel_a")));
  TestEqual("Active switched to sel_a", selectActiveNpcId(State),
            FString(TEXT("sel_a")));

  return true;
}
