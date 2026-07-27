#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Entities/Directive/DirectiveSelectors.h"
#include "Entities/Directive/DirectiveSlice.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FProtocolNpcRemovalCascadeTest,
    FORBOCAI_SDK_AUTHORED_STRINGV276741B8CBD3,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a integration protocol npc consumer, I need to invoke run test through a stable signature so the integration protocol npc workflow remains explicit and composable. @fn bool FProtocolNpcRemovalCascadeTest::RunTest(const FString &Parameters) */
bool FProtocolNpcRemovalCascadeTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc;
  Npc.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV90C6CC7116F9);
  Npc.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD82F4DE3E20E);
  TestStore.dispatch(NPCActions::setNPCInfo(Npc));
  TestStore.dispatch(DirectiveSlice::Actions::directiveRunStarted(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV45F249BC72C6), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV90C6CC7116F9), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2695DC6FB088)));

  FRuntimeState State = TestStore.getState();
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV28F2157B6D54,
           NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV90C6CC7116F9)).hasValue);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV6B6F76A78959,
           DirectiveSlice::selectDirectiveById(State.Directives,
                                                TEXT(FORBOCAI_SDK_AUTHORED_STRINGV45F249BC72C6)).hasValue);

  TestStore.dispatch(NPCActions::removeNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV90C6CC7116F9)));
  State = TestStore.getState();
  TestFalse(
      FORBOCAI_SDK_AUTHORED_STRINGVD794830A5AF2,
      NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV90C6CC7116F9)).hasValue);
  return true;
}
