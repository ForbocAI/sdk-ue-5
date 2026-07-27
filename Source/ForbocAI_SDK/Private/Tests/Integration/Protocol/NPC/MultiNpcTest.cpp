#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProtocolMultiNpcTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGVC85DF374DA3A,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)

/** User Story: As a integration protocol npc consumer, I need to invoke run test through a stable signature so the integration protocol npc workflow remains explicit and composable. @fn bool FProtocolMultiNpcTest::RunTest(const FString &Parameters) */
bool FProtocolMultiNpcTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc1;
  Npc1.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV81C8915D1051);
  Npc1.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV68B64CD755A8);
  FNPCInternalState Npc2;
  Npc2.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV24C7B2B7E77A);
  Npc2.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEF19BC30A5ED);

  TestStore.dispatch(NPCActions::setNPCInfo(Npc1));
  TestStore.dispatch(NPCActions::setNPCInfo(Npc2));
  TestStore.dispatch(NPCActions::setActiveNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV81C8915D1051)));

  FRuntimeState State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV1003F8EB9F42, NPCSelectors::selectAllNPCs(State.NPCs).Num(), FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV17B703EF14B8, NPCSelectors::selectActiveNpcId(State.NPCs),
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV81C8915D1051)));

  FAgentState M2State;
  M2State.JsonData = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF0320EEDEC6F) TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC092A63A601F);
  TestStore.dispatch(NPCActions::updateNPCState(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV24C7B2B7E77A), M2State));

  State = TestStore.getState();
  auto M1 = NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV81C8915D1051));
  auto M2 = NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV24C7B2B7E77A));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV195B6D36C8DD, M1.hasValue);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV03E7DD1AF75E, M2.hasValue);
  if (M1.hasValue && M2.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV1BDA4B2DFB5A, M1.value.State.JsonData,
              FString(FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF0320EEDEC6F) TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E3D7634AB68))));
    TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVB5F8F1F13876, M2.value.State.JsonData.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFAA99951D061)));
  }

  TestStore.dispatch(NPCActions::setActiveNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV24C7B2B7E77A)));
  State = TestStore.getState();
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV6E8171B60246, NPCSelectors::selectActiveNpcId(State.NPCs),
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV24C7B2B7E77A)));

  auto Active = NPCSelectors::selectActiveNPC(State.NPCs);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVA2E78A7003A7, Active.hasValue);
  if (Active.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVCC5C42496F06, Active.value.Persona,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVEF19BC30A5ED)));
  }
  return true;
}
