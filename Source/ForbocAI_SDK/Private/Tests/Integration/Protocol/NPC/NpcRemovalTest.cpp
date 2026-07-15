#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Directive/DirectiveSelectors.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProtocolNpcRemovalCascadeTest,
    "ForbocAI.Integration.Protocol.NpcRemovalCascade",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FProtocolNpcRemovalCascadeTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc;
  Npc.Id = TEXT("ag_cascade");
  Npc.Persona = TEXT("Cascade test");
  TestStore.dispatch(NPCActions::setNPCInfo(Npc));
  TestStore.dispatch(DirectiveSlice::Actions::directiveRunStarted(
      TEXT("dir_cascade"), TEXT("ag_cascade"), TEXT("obs")));

  FRuntimeState State = TestStore.getState();
  TestTrue("NPC exists",
           NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_cascade")).hasValue);
  TestTrue("Directive exists",
           DirectiveSlice::selectDirectiveById(State.Directives,
                                                TEXT("dir_cascade")).hasValue);

  TestStore.dispatch(NPCActions::removeNPC(TEXT("ag_cascade")));
  State = TestStore.getState();
  TestFalse(
      "NPC removed",
      NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_cascade")).hasValue);
  return true;
}
