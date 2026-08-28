#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Entities/Bridge/BridgeSelectors.h"
#include "Entities/Memory/MemorySelectors.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Systems/Testing/API/Codec/Ghost/CodecGhostAdapters.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStoreRemovalCascadeTest,
    FORBOCAI_SDK_AUTHORED_STRINGVCA0473A14182,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As root-store integration, I need active-NPC removal to clear every dependent slice while retaining API-owned Ghost identity at session start. @fn bool FStoreRemovalCascadeTest::RunTest(const FString &Parameters) */
bool FStoreRemovalCascadeTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  FNPCInternalState Info;
  Info.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2);
  Info.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD82F4DE3E20E);
  Store.dispatch(NPCActions::setNPCInfo(Info));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVF16429000669,
            Store.getState().NPCs.ActiveNpcId,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2)));

  FMemoryItem MemItem;
  MemItem.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV381D1959A3E2);
  MemItem.Text = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4B005DBA5938);
  MemItem.Importance = FORBOCAI_SDK_AUTHORED_NUMBERVA03A0A41C51F;
  Store.dispatch(MemorySlice::Actions::memoryStoreSuccess(MemItem));
  TestEqual(
      FORBOCAI_SDK_AUTHORED_STRINGV303DC226738C,
      MemorySelectors::selectAllMemories(Store.getState().Memory).Num(),
      FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  Store.dispatch(BridgeSlice::Actions::validationRequested());
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVDA6107BBCAF3,
            Store.getState().Bridge.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9474FED15011)));

  FDirectiveRuleSet Preset;
  Preset.RulesetId = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV26067698B6B4);
  Store.dispatch(BridgeSlice::Actions::activePresetAdded(Preset));

  FDirectiveRuleSet Ruleset;
  Ruleset.RulesetId = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVCDE018B89DE3);
  Store.dispatch(BridgeSlice::Actions::rulesetsReceived({Ruleset}));
  Store.dispatch(BridgeSlice::Actions::presetIdsReceived(
      {TEXT(FORBOCAI_SDK_AUTHORED_STRINGV26067698B6B4)}));

  const FGhostRunResponse &GhostIdentity =
      Testing::API::Codec::Ghost::GhostCodecFixtures().RunExpected;
  Store.dispatch(GhostSlice::Actions::ghostSessionStarted(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1D6E1452B2F0),
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E2678FEDB3E),
      GhostIdentity.GhostName, GhostIdentity.RuntimeIdentity));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV6308A15BBE34,
            Store.getState().Ghost.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E2678FEDB3E)));

  Store.dispatch(DirectiveSlice::Actions::directiveRunStarted(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV65A10EB122F7),
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2),
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD1CCE25A3B47)));
  Store.dispatch(NPCActions::removeNPC(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2)));

  TestTrue(
      FORBOCAI_SDK_AUTHORED_STRINGVD794830A5AF2,
      !NPCSelectors::selectNPCById(
           Store.getState().NPCs,
           TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2))
           .hasValue);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVA3CEC75A37E6,
           Store.getState().NPCs.ActiveNpcId.IsEmpty());
  TestEqual(
      FORBOCAI_SDK_AUTHORED_STRINGVFCAE70C55CED,
      MemorySelectors::selectAllMemories(Store.getState().Memory).Num(),
      FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVA1447EDD1491,
            Store.getState().Bridge.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  TestEqual(
      FORBOCAI_SDK_AUTHORED_STRINGVD62FAAAACE1C,
      BridgeSelectors::selectActivePresets(Store.getState().Bridge).Num(),
      FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestEqual(
      FORBOCAI_SDK_AUTHORED_STRINGVE695C68674BC,
      BridgeSelectors::selectAvailableRulesets(Store.getState().Bridge).Num(),
      FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV643E6FF9CB81,
            Store.getState().Bridge.AvailablePresetIds.Num(),
            FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVEED094E72E7D,
            Store.getState().Ghost.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV837AB03F2703,
           Store.getState().Directives.ActiveDirectiveId.IsEmpty());
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV75B2E664BED9,
            Store.getState().Soul.ExportStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  return true;
}
