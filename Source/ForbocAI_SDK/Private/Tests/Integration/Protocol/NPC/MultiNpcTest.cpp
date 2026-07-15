#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProtocolMultiNpcTest,
                                 "ForbocAI.Integration.Protocol.MultiNpc",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)

bool FProtocolMultiNpcTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc1;
  Npc1.Id = TEXT("ag_m1");
  Npc1.Persona = TEXT("Guard");
  FNPCInternalState Npc2;
  Npc2.Id = TEXT("ag_m2");
  Npc2.Persona = TEXT("Merchant");

  TestStore.dispatch(NPCActions::setNPCInfo(Npc1));
  TestStore.dispatch(NPCActions::setNPCInfo(Npc2));
  TestStore.dispatch(NPCActions::setActiveNPC(TEXT("ag_m1")));

  FRuntimeState State = TestStore.getState();
  TestEqual("Two NPCs", NPCSelectors::selectAllNPCs(State.NPCs).Num(), 2);
  TestEqual("Active is m1", NPCSelectors::selectActiveNpcId(State.NPCs),
            FString(TEXT("ag_m1")));

  FAgentState M2State;
  M2State.JsonData = TEXT("{") TEXT("\n\"goods\":[\"sword\",\"potion\"]\n}");
  TestStore.dispatch(NPCActions::updateNPCState(TEXT("ag_m2"), M2State));

  State = TestStore.getState();
  auto M1 = NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_m1"));
  auto M2 = NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_m2"));
  TestTrue("M1 exists", M1.hasValue);
  TestTrue("M2 exists", M2.hasValue);
  if (M1.hasValue && M2.hasValue) {
    TestEqual("M1 state unchanged", M1.value.State.JsonData,
              FString(FString(TEXT("{") TEXT("}"))));
    TestTrue("M2 state updated", M2.value.State.JsonData.Contains(TEXT("sword")));
  }

  TestStore.dispatch(NPCActions::setActiveNPC(TEXT("ag_m2")));
  State = TestStore.getState();
  TestEqual("Active is m2", NPCSelectors::selectActiveNpcId(State.NPCs),
            FString(TEXT("ag_m2")));

  auto Active = NPCSelectors::selectActiveNPC(State.NPCs);
  TestTrue("Active NPC found", Active.hasValue);
  if (Active.hasValue) {
    TestEqual("Active persona", Active.value.Persona,
              FString(TEXT("Merchant")));
  }
  return true;
}
