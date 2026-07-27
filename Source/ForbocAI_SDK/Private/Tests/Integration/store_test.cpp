#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Entities/Bridge/BridgeSelectors.h"
#include "Entities/Memory/MemorySelectors.h"
#include "Entities/NPC/NPCActions.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Entities/Protocol/Logger/LoggerSelectors.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

using namespace rtk;

/**
 * Test: StoreReducer processes NPC creation across all slices
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreNPCCreationTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGVA2D472BE3E9D,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FStoreNPCCreationTest::RunTest(const FString &Parameters)
 */
bool FStoreNPCCreationTest::RunTest(const FString &Parameters) {
  FRuntimeState State = StoreInternal::createRuntimeInitialState();

  FNPCInternalState Info;
  Info.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV38F7A6ADE050);
  Info.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV8FF4092C7217);

  State = StoreReducer(State, NPCActions::setNPCInfo(Info));

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV1A6AA8C8B59A, State.NPCs.ActiveNpcId,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV38F7A6ADE050)));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVE4D4A3BCDD66,
           NPCSelectors::selectNPCById(State.NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV38F7A6ADE050)).hasValue);

  /**
   * Other slices remain at initial state
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV647DD6D80B34, State.Memory.StorageStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVF0FC8AA1DE8F, State.Ghost.Status, FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVAD4BF2CD96ED, State.Bridge.Status, FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV7AF9B1C63133, State.Soul.ExportStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));

  return true;
}

/**
 * Test: NPC removal middleware cascades clears
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStoreRemovalCascadeTest,
    FORBOCAI_SDK_AUTHORED_STRINGVCA0473A14182,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FStoreRemovalCascadeTest::RunTest(const FString &Parameters)
 */
bool FStoreRemovalCascadeTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  /**
   * Create NPC
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FNPCInternalState Info;
  Info.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2);
  Info.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD82F4DE3E20E);
  Store.dispatch(NPCActions::setNPCInfo(Info));

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVF16429000669, Store.getState().NPCs.ActiveNpcId,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2)));

  /**
   * Add some memory
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FMemoryItem MemItem;
  MemItem.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV381D1959A3E2);
  MemItem.Text = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4B005DBA5938);
  MemItem.Importance = FORBOCAI_SDK_AUTHORED_NUMBERVA03A0A41C51F;
  Store.dispatch(MemorySlice::Actions::memoryStoreSuccess(MemItem));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV303DC226738C, MemorySelectors::selectAllMemories(
                                  Store.getState().Memory)
                                  .Num(),
            FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  /**
   * Add bridge validation
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(BridgeSlice::Actions::validationRequested());
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVDA6107BBCAF3, Store.getState().Bridge.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9474FED15011)));

  FDirectiveRuleSet Preset;
  Preset.RulesetId = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV26067698B6B4);
  Store.dispatch(BridgeSlice::Actions::activePresetAdded(Preset));

  TArray<FDirectiveRuleSet> Rulesets;
  FDirectiveRuleSet Ruleset;
  Ruleset.RulesetId = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVCDE018B89DE3);
  Rulesets.Add(Ruleset);
  Store.dispatch(BridgeSlice::Actions::rulesetsReceived(Rulesets));

  TArray<FString> PresetIds;
  PresetIds.Add(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV26067698B6B4));
  Store.dispatch(BridgeSlice::Actions::presetIdsReceived(PresetIds));

  /**
   * Start ghost session
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(
      GhostSlice::Actions::ghostSessionStarted(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1D6E1452B2F0), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E2678FEDB3E)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV6308A15BBE34, Store.getState().Ghost.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E2678FEDB3E)));

  /**
   * Add a directive
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(DirectiveSlice::Actions::directiveRunStarted(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV65A10EB122F7), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2), TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD1CCE25A3B47)));

  /**
   * Remove NPC — should cascade clear
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(NPCActions::removeNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2)));

  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVD794830A5AF2,
           !NPCSelectors::selectNPCById(Store.getState().NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C79CD9F0E2))
                .hasValue);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVA3CEC75A37E6,
           Store.getState().NPCs.ActiveNpcId.IsEmpty());

  /**
   * Cascade effects
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVFCAE70C55CED,
            MemorySelectors::selectAllMemories(Store.getState().Memory).Num(),
            FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVA1447EDD1491, Store.getState().Bridge.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVD62FAAAACE1C,
            BridgeSelectors::selectActivePresets(Store.getState().Bridge).Num(),
            FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVE695C68674BC,
            BridgeSelectors::selectAvailableRulesets(Store.getState().Bridge)
                .Num(),
            FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV643E6FF9CB81,
            Store.getState().Bridge.AvailablePresetIds.Num(), FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVEED094E72E7D, Store.getState().Ghost.Status,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV837AB03F2703,
           Store.getState().Directives.ActiveDirectiveId.IsEmpty());
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV75B2E664BED9, Store.getState().Soul.ExportStatus,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E32DA346F92)));

  return true;
}

/**
 * Test: Multiple NPCs, remove non-active — no cascade on memory
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStoreRemoveNonActiveTest,
    FORBOCAI_SDK_AUTHORED_STRINGV4584204201A0,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FStoreRemoveNonActiveTest::RunTest(const FString &Parameters)
 */
bool FStoreRemoveNonActiveTest::RunTest(const FString &Parameters) {
  EnhancedStore<FRuntimeState> Store = createRuntimeStore();

  /**
   * Create two NPCs
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FNPCInternalState A;
  A.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV83A033E59251);
  A.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV64140FC9EA63);
  Store.dispatch(NPCActions::setNPCInfo(A));

  FNPCInternalState B;
  B.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC03F7465F012);
  B.Persona = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVBD44A23A4D4D);
  Store.dispatch(NPCActions::setNPCInfo(B));

  /**
   * Active is now B
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV9B1944C64D17, Store.getState().NPCs.ActiveNpcId,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC03F7465F012)));

  /**
   * Store memory
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  FMemoryItem Mem;
  Mem.Id = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV8F9E67C3219C);
  Mem.Text = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3C808DE3889C);
  Mem.Importance = FORBOCAI_SDK_AUTHORED_NUMBERVD72733281456;
  Store.dispatch(MemorySlice::Actions::memoryStoreSuccess(Mem));

  /**
   * Remove A (not active) — memory should NOT be cleared
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(NPCActions::removeNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV83A033E59251)));

  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV402A28457064,
            NPCSelectors::selectNPCById(Store.getState().NPCs, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV83A033E59251))
                .hasValue);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV180E7F45FDBC, Store.getState().NPCs.ActiveNpcId,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC03F7465F012)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV97646A8CC932,
            MemorySelectors::selectAllMemories(Store.getState().Memory).Num(),
            FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  return true;
}

/**
 * Test: Logger helpers summarize action payloads and slice deltas
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuntimeProtocolLoggerSummaryTest,
    FORBOCAI_SDK_AUTHORED_STRINGVF7BEB26EA5F3,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRuntimeProtocolLoggerSummaryTest::RunTest(const FString &Parameters)
 */
bool FRuntimeProtocolLoggerSummaryTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  const rtk::AnyAction SetActiveAction =
      NPCActions::setActiveNPC(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD7812F8CA949));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV6896CCDD12B2,
            SetActiveAction.describePayload(), FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD7812F8CA949)));
  TestEqual(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD4D03B87C276),
            LoggerAdapters::describeProtocolPayloadAdapter(),
            LoggerConfiguration::loggerData().Text.OpaquePayload);

  FRuntimeState Before = StoreInternal::createRuntimeInitialState();
  FRuntimeState After = StoreReducer(Before, SetActiveAction);
  FRuntimeState ErrorAfter = After;
  ErrorAfter.Dependencies.Error =
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD7812F8CA949);
  const FString ErrorDelta =
      LoggerSelectors::describeStateDelta(After, ErrorAfter);
  TestTrue(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD7F4EA429631),
           ErrorDelta.Contains(
               LoggerConfiguration::loggerData().Text.OpaquePayload));
  TestFalse(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV001D686FB3B6),
            ErrorDelta.Contains(
                TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD7812F8CA949)));

  const FString Delta =
      LoggerSelectors::describeStateDelta(Before, After);

  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVE07AA534CD9C, Delta.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6191C2903AAE)));
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV7D04F36B6BCF,
           Delta.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD7812F8CA949)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV5764B8E19025,
            LoggerSelectors::describeStateDelta(After, After),
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV781ACD34ECFB)));

  rtk::AnyAction ProtocolAction;
  ProtocolAction.Type = TEXT(FORBOCAI_SDK_AUTHORED_STRINGVB4193769DADF);
  rtk::AnyAction UnrelatedAction;
  UnrelatedAction.Type = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7E05F3C19671);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVCB51572DBE91,
           LoggerSelectors::selectIsProtocolAction(ProtocolAction));
  TestFalse(FORBOCAI_SDK_AUTHORED_STRINGV80CA71FFF3DE,
            LoggerSelectors::selectIsProtocolAction(UnrelatedAction));

  return true;
}
