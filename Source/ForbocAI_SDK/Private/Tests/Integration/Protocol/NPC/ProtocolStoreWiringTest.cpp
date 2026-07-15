#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

// @covers:coreThunk:checkApiStatusThunk
// @covers:coreThunk:clearMemoryRemoteThunk
// @covers:coreThunk:clearNodeMemoryThunk
// @covers:coreThunk:deleteRulesetThunk
// @covers:coreThunk:doctorThunk
// @covers:coreThunk:exportSoulThunk
// @covers:coreThunk:generateEmbeddingThunk
// @covers:coreThunk:getBridgeRulesThunk
// @covers:coreThunk:getGhostHistoryThunk
// @covers:coreThunk:getGhostResultsThunk
// @covers:coreThunk:getGhostStatusThunk
// @covers:coreThunk:getSoulListThunk
// @covers:coreThunk:importNpcFromSoulThunk
// @covers:coreThunk:importSoulFromArweaveThunk
// @covers:coreThunk:importSoulThunk
// @covers:coreThunk:initNodeMemoryThunk
// @covers:coreThunk:initVectorThunk
// @covers:coreThunk:listMemoryRemoteThunk
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
// @covers:coreThunk:recallMemoryRemoteThunk
// @covers:coreThunk:recallNodeMemoryThunk
// @covers:coreThunk:registerRulesetThunk
// @covers:coreThunk:remoteExportSoulThunk
// @covers:coreThunk:startGhostThunk
// @covers:coreThunk:stopGhostThunk
// @covers:coreThunk:storeMemoryRemoteThunk
// @covers:coreThunk:storeNodeMemoryThunk
// @covers:coreThunk:validateBridgeThunk
// @covers:coreThunk:verifySoulThunk

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProtocolStoreWiringTest,
                                 "ForbocAI.Integration.Protocol.StoreWiring",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)

bool FProtocolStoreWiringTest::RunTest(const FString &Parameters) {
  rtk::EnhancedStore<FRuntimeState> TestStore = createRuntimeStore();

  FNPCInternalState Npc;
  Npc.Id = TEXT("ag_test1");
  Npc.Persona = TEXT("A test knight");
  TestStore.dispatch(NPCActions::setNPCInfo(Npc));
  TestStore.dispatch(NPCActions::setActiveNPC(TEXT("ag_test1")));

  FRuntimeState State = TestStore.getState();
  func::Maybe<FNPCInternalState> Found =
      NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_test1"));
  TestTrue("NPC found in store", Found.hasValue);
  if (Found.hasValue) {
    TestEqual("NPC persona", Found.value.Persona,
              FString(TEXT("A test knight")));
  }
  TestEqual("Active NPC set", NPCSelectors::selectActiveNpcId(State.NPCs),
            FString(TEXT("ag_test1")));

  FAgentState NewState;
  NewState.JsonData = FString(TEXT("{") TEXT("\"health\":100}"));
  TestStore.dispatch(NPCActions::updateNPCState(TEXT("ag_test1"), NewState));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_test1"));
  TestTrue("NPC still found", Found.hasValue);
  if (Found.hasValue) {
    TestTrue("State updated",
             Found.value.State.JsonData.Contains(TEXT("health")));
  }

  TestStore.dispatch(NPCActions::addToHistory(
      TEXT("ag_test1"), TEXT("player"), TEXT("Hello")));
  TestStore.dispatch(NPCActions::addToHistory(
      TEXT("ag_test1"), TEXT("npc"), TEXT("Greetings")));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_test1"));
  TestTrue("NPC found after history", Found.hasValue);
  if (Found.hasValue) {
    TestEqual("Two history entries", Found.value.History.Num(), 2);
    TestEqual("First history role", Found.value.History[0].Role,
              FString(TEXT("player")));
    TestEqual("First history content", Found.value.History[0].Content,
              FString(TEXT("Hello")));
  }

  FAgentAction Attack;
  Attack.Type = TEXT("ATTACK");
  Attack.Target = TEXT("goblin");
  Attack.Reason = TEXT("In combat");
  const rtk::AnyAction ActionReceived =
      NPCActions::actionReceived(TEXT("ag_test1"), Attack);
  TestTrue("Action event matches",
           NPCActions::actionReceivedActionCreator().match(ActionReceived));
  const auto ActionPayload =
      NPCActions::actionReceivedActionCreator().extract(ActionReceived);
  TestTrue("Action event has payload", ActionPayload.hasValue);
  if (ActionPayload.hasValue) {
    TestEqual("Action event type", ActionPayload.value.Action.Type,
              FString(TEXT("ATTACK")));
    TestEqual("Action event target", ActionPayload.value.Action.Target,
              FString(TEXT("goblin")));
  }
  TestStore.dispatch(ActionReceived);

  TestStore.dispatch(NPCActions::blockAction(
      TEXT("ag_test1"), TEXT("Cannot attack civilians")));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_test1"));
  TestTrue("NPC found after block", Found.hasValue);
  if (Found.hasValue) {
    TestTrue("NPC is blocked", Found.value.bIsBlocked);
    TestEqual("Block reason", Found.value.BlockReason,
              FString(TEXT("Cannot attack civilians")));
  }

  TestStore.dispatch(NPCActions::clearBlock(TEXT("ag_test1")));
  State = TestStore.getState();
  Found = NPCSelectors::selectNPCById(State.NPCs, TEXT("ag_test1"));
  TestTrue("NPC found after unblock", Found.hasValue);
  if (Found.hasValue) {
    TestFalse("NPC is unblocked", Found.value.bIsBlocked);
  }
  return true;
}
