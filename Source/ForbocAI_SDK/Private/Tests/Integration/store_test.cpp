#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Bridge/BridgeSelectors.h"
#include "Features/Memory/MemorySelectors.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSelectors.h"
#include "Features/Protocol/Logger/LoggerSelectors.h"
#include "Misc/AutomationTest.h"
#include "Store.h"

using namespace rtk;

/**
 * Test: StoreReducer processes NPC creation across all slices
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStoreNPCCreationTest,
                                 "ForbocAI.Integration.Store.NPCCreation",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FStoreNPCCreationTest::RunTest(const FString &Parameters)
 */
bool FStoreNPCCreationTest::RunTest(const FString &Parameters) {
  FRuntimeState State = StoreInternal::createRuntimeInitialState();

  FNPCInternalState Info;
  Info.Id = TEXT("int_npc_1");
  Info.Persona = TEXT("Test NPC");

  State = StoreReducer(State, NPCActions::setNPCInfo(Info));

  TestEqual("NPC active", State.NPCs.ActiveNpcId,
            FString(TEXT("int_npc_1")));
  TestTrue("NPC in entities",
           NPCSelectors::selectNPCById(State.NPCs, TEXT("int_npc_1")).hasValue);

  /**
   * Other slices remain at initial state
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual("Memory idle", State.Memory.StorageStatus,
            FString(TEXT("idle")));
  TestEqual("Ghost idle", State.Ghost.Status, FString(TEXT("idle")));
  TestEqual("Bridge idle", State.Bridge.Status, FString(TEXT("idle")));
  TestEqual("Soul export idle", State.Soul.ExportStatus,
            FString(TEXT("idle")));

  return true;
}

/**
 * Test: NPC removal middleware cascades clears
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStoreRemovalCascadeTest,
    "ForbocAI.Integration.Store.RemovalCascade",
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
  Info.Id = TEXT("cascade_npc");
  Info.Persona = TEXT("Cascade test");
  Store.dispatch(NPCActions::setNPCInfo(Info));

  TestEqual("NPC active before removal", Store.getState().NPCs.ActiveNpcId,
            FString(TEXT("cascade_npc")));

  /**
   * Add some memory
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FMemoryItem MemItem;
  MemItem.Id = TEXT("cascade_mem");
  MemItem.Text = TEXT("Important memory");
  MemItem.Importance = 0.9f;
  Store.dispatch(MemorySlice::Actions::memoryStoreSuccess(MemItem));
  TestEqual("Memory exists", MemorySelectors::selectAllMemories(
                                  Store.getState().Memory)
                                  .Num(),
            1);

  /**
   * Add bridge validation
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(BridgeSlice::Actions::validationRequested());
  TestEqual("Bridge validating", Store.getState().Bridge.Status,
            FString(TEXT("validating")));

  FDirectiveRuleSet Preset;
  Preset.RulesetId = TEXT("preset_keep");
  Store.dispatch(BridgeSlice::Actions::activePresetAdded(Preset));

  TArray<FDirectiveRuleSet> Rulesets;
  FDirectiveRuleSet Ruleset;
  Ruleset.RulesetId = TEXT("ruleset_keep");
  Rulesets.Add(Ruleset);
  Store.dispatch(BridgeSlice::Actions::rulesetsReceived(Rulesets));

  TArray<FString> PresetIds;
  PresetIds.Add(TEXT("preset_keep"));
  Store.dispatch(BridgeSlice::Actions::presetIdsReceived(PresetIds));

  /**
   * Start ghost session
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(
      GhostSlice::Actions::ghostSessionStarted(TEXT("gs_1"), TEXT("running")));
  TestEqual("Ghost running", Store.getState().Ghost.Status,
            FString(TEXT("running")));

  /**
   * Add a directive
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(DirectiveSlice::Actions::directiveRunStarted(
      TEXT("dir_1"), TEXT("cascade_npc"), TEXT("observe")));

  /**
   * Remove NPC — should cascade clear
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(NPCActions::removeNPC(TEXT("cascade_npc")));

  TestTrue("NPC removed",
           !NPCSelectors::selectNPCById(Store.getState().NPCs, TEXT("cascade_npc"))
                .hasValue);
  TestTrue("ActiveNpcId cleared",
           Store.getState().NPCs.ActiveNpcId.IsEmpty());

  /**
   * Cascade effects
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual("Memory cleared after active NPC removal",
            MemorySelectors::selectAllMemories(Store.getState().Memory).Num(),
            0);
  TestEqual("Bridge reset", Store.getState().Bridge.Status,
            FString(TEXT("idle")));
  TestEqual("Bridge active presets preserved",
            BridgeSelectors::selectActivePresets(Store.getState().Bridge).Num(),
            1);
  TestEqual("Bridge rulesets preserved",
            BridgeSelectors::selectAvailableRulesets(Store.getState().Bridge)
                .Num(),
            1);
  TestEqual("Bridge preset ids preserved",
            Store.getState().Bridge.AvailablePresetIds.Num(), 1);
  TestEqual("Ghost reset", Store.getState().Ghost.Status,
            FString(TEXT("idle")));
  TestTrue("Directive cleared",
           Store.getState().Directives.ActiveDirectiveId.IsEmpty());
  TestEqual("Soul reset", Store.getState().Soul.ExportStatus,
            FString(TEXT("idle")));

  return true;
}

/**
 * Test: Multiple NPCs, remove non-active — no cascade on memory
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FStoreRemoveNonActiveTest,
    "ForbocAI.Integration.Store.RemoveNonActive",
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
  A.Id = TEXT("npc_a");
  A.Persona = TEXT("Alpha");
  Store.dispatch(NPCActions::setNPCInfo(A));

  FNPCInternalState B;
  B.Id = TEXT("npc_b");
  B.Persona = TEXT("Beta");
  Store.dispatch(NPCActions::setNPCInfo(B));

  /**
   * Active is now B
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual("Active is B", Store.getState().NPCs.ActiveNpcId,
            FString(TEXT("npc_b")));

  /**
   * Store memory
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  FMemoryItem Mem;
  Mem.Id = TEXT("keep_mem");
  Mem.Text = TEXT("Should survive");
  Mem.Importance = 0.5f;
  Store.dispatch(MemorySlice::Actions::memoryStoreSuccess(Mem));

  /**
   * Remove A (not active) — memory should NOT be cleared
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(NPCActions::removeNPC(TEXT("npc_a")));

  TestFalse("A removed",
            NPCSelectors::selectNPCById(Store.getState().NPCs, TEXT("npc_a"))
                .hasValue);
  TestEqual("Active still B", Store.getState().NPCs.ActiveNpcId,
            FString(TEXT("npc_b")));
  TestEqual("Memory preserved",
            MemorySelectors::selectAllMemories(Store.getState().Memory).Num(),
            1);

  return true;
}

/**
 * Test: Logger helpers summarize action payloads and slice deltas
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FRuntimeProtocolLoggerSummaryTest,
    "ForbocAI.Integration.Store.ProtocolLoggerSummary",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRuntimeProtocolLoggerSummaryTest::RunTest(const FString &Parameters)
 */
bool FRuntimeProtocolLoggerSummaryTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  const rtk::AnyAction SetActiveAction =
      NPCActions::setActiveNPC(TEXT("logger_npc"));
  TestEqual("String payloads are preserved for logger output",
            SetActiveAction.describePayload(), FString(TEXT("logger_npc")));

  FRuntimeState Before = StoreInternal::createRuntimeInitialState();
  FRuntimeState After = StoreReducer(Before, SetActiveAction);
  const FString Delta =
      LoggerSelectors::describeStateDelta(Before, After);

  TestTrue("Delta includes NPC slice summary", Delta.Contains(TEXT("NPCs{")));
  TestTrue("Delta includes active npc id",
           Delta.Contains(TEXT("logger_npc")));
  TestEqual("No-change deltas collapse to <none>",
            LoggerSelectors::describeStateDelta(After, After),
            FString(TEXT("<none>")));

  rtk::AnyAction ProtocolAction;
  ProtocolAction.Type = TEXT("protocol/test");
  rtk::AnyAction UnrelatedAction;
  UnrelatedAction.Type = TEXT("counter/test");
  TestTrue("Default selector includes protocol actions",
           LoggerSelectors::selectIsProtocolAction(ProtocolAction));
  TestFalse("Default selector excludes unrelated actions",
            LoggerSelectors::selectIsProtocolAction(UnrelatedAction));

  return true;
}
