#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Entities/NPC/NPCSlice.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

// @covers:coreThunk:checkApiStatusThunk
// @covers:coreThunk:clearNodeMemoryThunk
// @covers:coreThunk:doctorThunk
// @covers:coreThunk:exportSoulThunk
// @covers:coreThunk:generateEmbeddingThunk
// @covers:coreThunk:getBridgeRulesThunk
// @covers:coreThunk:getGhostHistoryThunk
// @covers:coreThunk:getGhostResultsThunk
// @covers:coreThunk:getGhostStatusThunk
// @covers:coreThunk:listSoulsThunk
// @covers:coreThunk:importSoulThunk
// @covers:coreThunk:importSoulThunk
// @covers:coreThunk:initNodeMemoryThunk
// @covers:coreThunk:initVectorThunk
// @covers:coreThunk:listRulePresetsThunk
// @covers:coreThunk:listRulesetsThunk
// @covers:coreThunk:loadBridgePresetThunk
// @covers:coreThunk:localExportSoulThunk
// @covers:coreThunk:localImportSoulThunk
// @covers:coreThunk:validateBridgeThunk
// @covers:coreThunk:MakeDelete
// @covers:coreThunk:MakeEndpoint
// @covers:coreThunk:MakeGet
// @covers:coreThunk:MakeGetWithCodec
// @covers:coreThunk:MakePost
// @covers:coreThunk:MakePostRawWithCodec
// @covers:coreThunk:MakePostWithCodec
// @covers:coreThunk:nodeMemoryRecallThunk
// @covers:coreThunk:nodeMemoryStoreThunk
// @covers:coreThunk:processNPC
// @covers:coreThunk:recallNodeMemoryThunk
// @covers:coreThunk:exportSoulThunk
// @covers:coreThunk:verifySoulThunk
// @covers:coreThunk:startGhostThunk
// @covers:coreThunk:stopGhostThunk
// @covers:coreThunk:storeNodeMemoryThunk
// @covers:coreThunk:validateBridgeThunk
// @covers:coreThunk:verifySoulThunk

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProtocolStoreWiringTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGV5DE872FB1DD2,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)

/** User Story: As a integration protocol npc consumer, I need to invoke run test through a stable signature so the integration protocol npc workflow remains explicit and composable. @fn bool FProtocolStoreWiringTest::RunTest(const FString &Parameters) */
bool FProtocolStoreWiringTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc;
  Npc.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1);
  Npc.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF4A147CAA98E);
  TestStore.dispatch(NPCActions::setNPCInfo(Npc));
  TestStore.dispatch(NPCActions::setActiveNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1)));

  FRuntimeState State = TestStore.getState();
  func::Maybe<FNPCInternalState> Found =
      NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV6C712DA7EDD2, Found.hasValue);
  if (Found.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVBFAD8D9A2DC6, Found.value.Persona,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF4A147CAA98E)));
  }
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVCC9C3960FFC3, NPCSelectors::selectActiveNpcId(State.NPCs),
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1)));

  FAgentState NewState;
  NewState.JsonData = FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF0320EEDEC6F) TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1BDCBA3FA2CB));
  TestStore.dispatch(NPCActions::updateNPCState(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1), NewState));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV05D2C81007CB, Found.hasValue);
  if (Found.hasValue) {
    TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV6F4BC07F0E77,
             Found.value.State.JsonData.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5E9D79A19FC5)));
  }

  TestStore.dispatch(NPCActions::addToHistory(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC326CBA6C049), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0670E86F8BEC)));
  TestStore.dispatch(NPCActions::addToHistory(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV62BC7243C9C2), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV65D099044655)));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV3A32D9186272, Found.hasValue);
  if (Found.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV3CDA55432FB2, Found.value.History.Num(), FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVAA04986FD1D3, Found.value.History[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Role,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC326CBA6C049)));
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVD4393345B952, Found.value.History[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Content,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0670E86F8BEC)));
  }

  FAgentAction Attack;
  Attack.Type = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5EF270DC93FB);
  Attack.Target = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVCC3E8E496F2C);
  Attack.Reason = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV33D6A3AC0736);
  const rtk::AnyAction ActionReceived =
      NPCActions::actionReceived(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1), Attack);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV1BBED6B02745,
           NPCActions::actionReceivedActionCreator().match(ActionReceived));
  const auto ActionPayload =
      NPCActions::actionReceivedActionCreator().extract(ActionReceived);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV8A4630130879, ActionPayload.hasValue);
  if (ActionPayload.hasValue) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV7F2A2B26AFD5, ActionPayload.value.Action.Type,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5EF270DC93FB)));
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVAF3A82394FEC, ActionPayload.value.Action.Target,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVCC3E8E496F2C)));
  }
  TestStore.dispatch(ActionReceived);

  TestStore.dispatch(NPCActions::blockAction(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFF8480B053CB)));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVABDCADF912CF, Found.hasValue);
  if (Found.hasValue) {
    TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV8E9E63EAA083, Found.value.bIsBlocked);
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV60B7DE8A653F, Found.value.BlockReason,
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFF8480B053CB)));
  }

  TestStore.dispatch(NPCActions::clearBlock(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1)));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV76610D91B2F1));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV305EAC4982DB, Found.hasValue);
  if (Found.hasValue) {
    TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV57187E3F02EA, Found.value.bIsBlocked);
  }
  return true;
}
