#include "Misc/AutomationTest.h"
#include "TestGame/Features/TestGameSlices.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGamePlayerPatchAndSelectorsTest,
    "ForbocAI.Slices.TestGame.PlayerPatchAndSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FTestGamePlayerPatchAndSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  FPatchPlayerPayload Patch;
  Patch.Hp = 42;
  Patch.bHasHp = true;
  Patch.bHidden = false;
  Patch.bHasHidden = true;
  Patch.Position = FPosition(3, 4);
  Patch.bHasPosition = true;
  Patch.Inventory = TArray<FString>();
  Patch.bHasInventory = true;

  const FPlayerState State = CreatePlayerSlice().Reducer(
      FPlayerState(), PlayerActions::PatchPlayer(Patch));

  TestEqual("Player name remains owned by reducer", PlayerSelectors::SelectPlayerName(State),
            FString(TEXT("Scout")));
  TestEqual("Player hp updates", PlayerSelectors::SelectPlayerHp(State), 42);
  TestFalse("Player hidden flag updates",
            PlayerSelectors::SelectPlayerHidden(State));
  TestEqual("Player position x updates",
            PlayerSelectors::SelectPlayerPosition(State).X, 3);
  TestEqual("Player position y updates",
            PlayerSelectors::SelectPlayerPosition(State).Y, 4);
  TestEqual("Player inventory can be emptied",
            PlayerSelectors::SelectPlayerInventory(State).Num(), 0);

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameBridgeRulePatchAndSelectorsTest,
    "ForbocAI.Slices.TestGame.BridgeRulePatchAndSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FTestGameBridgeRulePatchAndSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;

  GameBridgeActions::FSetBridgeRulesPayload Patch;
  Patch.MaxJumpForce = 800;
  Patch.bHasMaxJumpForce = true;
  Patch.ActivePreset = TEXT("heavy");
  Patch.bHasActivePreset = true;

  const rtk::Slice<FBridgeRulesState> Slice = CreateGameBridgeSlice();
  FBridgeRulesState State =
      Slice.Reducer(FBridgeRulesState(), GameBridgeActions::SetBridgeRules(Patch));

  TestEqual("Bridge jump force updates",
            GameBridgeSelectors::SelectBridgeMaxJumpForce(State), 800);
  TestEqual("Bridge move distance remains unchanged",
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State), 2);
  TestEqual("Bridge preset updates",
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            FString(TEXT("heavy")));

  State =
      Slice.Reducer(State, GameBridgeActions::LoadBridgePreset(TEXT("social")));
  TestEqual("Social preset narrows move distance",
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State), 1);
  TestEqual("Social preset is selected",
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            FString(TEXT("social")));

  State =
      Slice.Reducer(State, GameBridgeActions::LoadBridgePreset(TEXT("custom")));
  TestEqual("Custom preset preserves move distance",
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State), 1);
  TestEqual("Custom preset is selected",
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            FString(TEXT("custom")));

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameNpcPatchAndSelectorsTest,
    "ForbocAI.Slices.TestGame.NpcPatchAndSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FTestGameNpcPatchAndSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  FGameNPC Miller;
  Miller.Id = TEXT("miller");
  Miller.Name = TEXT("Miller");
  Miller.Faction = TEXT("Neutral");
  Miller.Hp = 100;
  Miller.Suspicion = 50;
  Miller.Position = FPosition(5, 12);

  const rtk::Slice<FNPCsSliceState> Slice = CreateNPCsSlice();
  FNPCsSliceState State =
      Slice.Reducer(FNPCsSliceState(), NPCsActions::UpsertNPC(Miller));

  NPCsActions::FPatchNPCPayload Patch;
  Patch.Id = TEXT("miller");
  Patch.Patch.Name = TEXT("Miller Prime");
  Patch.Patch.bHasName = true;
  Patch.Patch.Hp = 84;
  Patch.Patch.bHasHp = true;
  Patch.Patch.Suspicion = 75;
  Patch.Patch.bHasSuspicion = true;
  Patch.Patch.Inventory.Add(TEXT("medkit"));
  Patch.Patch.bHasInventory = true;
  Patch.Patch.Position = FPosition(6, 13);
  Patch.Patch.bHasPosition = true;

  State = Slice.Reducer(State, NPCsActions::PatchNPC(Patch));

  const func::Maybe<FGameNPC> MaybeNpc =
      NPCsSelectors::SelectNpcById(State, TEXT("miller"));
  TestTrue("NPC selector finds patched entity", MaybeNpc.hasValue);
  TestEqual("NPC name patches explicitly", MaybeNpc.value.Name,
            FString(TEXT("Miller Prime")));
  TestEqual("NPC hp patches explicitly", MaybeNpc.value.Hp, 84);
  TestEqual("NPC suspicion patches explicitly", MaybeNpc.value.Suspicion, 75);
  TestEqual("NPC inventory patches explicitly", MaybeNpc.value.Inventory.Num(),
            1);
  TestEqual("NPC position x patches explicitly", MaybeNpc.value.Position.X, 6);
  TestEqual("NPC total selector reads adapter state",
            NPCsSelectors::SelectNpcTotal(State), 1);
  TestEqual("NPC ids selector reads adapter state",
            NPCsSelectors::SelectNpcIds(State).Num(), 1);
  TestEqual("NPC all selector reads adapter state",
            NPCsSelectors::SelectAllNpcs(State).Num(), 1);

  NPCsActions::FApplyNpcVerdictPayload Verdict;
  Verdict.Id = TEXT("miller");
  Verdict.Action.Type = TEXT("MOVE");
  Verdict.Action.TargetHex = FPosition(7, 14);
  Verdict.Action.bHasTargetHex = true;
  Verdict.StateDelta.Suspicion = 55;
  Verdict.StateDelta.bHasSuspicion = true;
  State = Slice.Reducer(State, NPCsActions::ApplyNpcVerdict(Verdict));

  const func::Maybe<FGameNPC> VerdictNpc =
      NPCsSelectors::SelectNpcById(State, TEXT("miller"));
  TestTrue("NPC verdict selector finds entity", VerdictNpc.hasValue);
  TestEqual("NPC verdict applies state delta explicitly",
            VerdictNpc.value.Suspicion, 55);
  TestEqual("NPC verdict applies MOVE target", VerdictNpc.value.Position.X, 7);

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameStoreDomainSelectorsTest,
    "ForbocAI.Slices.TestGame.StoreDomainSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FTestGameStoreDomainSelectorsTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  const rtk::Slice<FGameMemorySliceState> MemorySlice =
      CreateGameMemorySlice();
  FMemoryRecord FirstMemory;
  FirstMemory.Id = TEXT("mem-1");
  FirstMemory.NpcId = TEXT("miller");
  FirstMemory.Text = TEXT("Keeps a medkit hidden.");
  FMemoryRecord SecondMemory;
  SecondMemory.Id = TEXT("mem-2");
  SecondMemory.NpcId = TEXT("scout");
  SecondMemory.Text = TEXT("Saw the exit route.");
  FGameMemorySliceState MemoryState = MemorySlice.Reducer(
      FGameMemorySliceState(), GameMemoryActions::StoreMemory(FirstMemory));
  MemoryState =
      MemorySlice.Reducer(MemoryState, GameMemoryActions::StoreMemory(SecondMemory));

  TestEqual("Memory total selector reads adapter state",
            GameMemorySelectors::SelectMemoryTotal(MemoryState), 2);
  TestTrue("Memory by id selector finds entity",
           GameMemorySelectors::SelectMemoryById(MemoryState, TEXT("mem-1"))
               .hasValue);
  TestEqual("Memory by NPC selector filters records",
            GameMemorySelectors::SelectMemoriesByNpcId(MemoryState,
                                                       TEXT("miller"))
                .Num(),
            1);

  FSetOwnerInventoryPayload InventoryPayload;
  InventoryPayload.OwnerId = TEXT("scout");
  InventoryPayload.Items.Add(TEXT("coin-pouch"));
  InventoryPayload.Items.Add(TEXT("signal-key"));
  const FInventoryState InventoryState = CreateInventorySlice().Reducer(
      FInventoryState(), InventoryActions::SetOwnerInventory(InventoryPayload));
  TestEqual("Owner inventory selector reads keyed state",
            InventorySelectors::SelectOwnerInventory(InventoryState,
                                                     TEXT("scout"))
                .Num(),
            2);

  FMarkSoulExportedPayload ExportPayload;
  ExportPayload.NpcId = TEXT("miller");
  ExportPayload.TxId = TEXT("tx-001");
  const rtk::Slice<FSoulTrackingState> SoulSlice = CreateGameSoulSlice();
  FSoulTrackingState SoulState = SoulSlice.Reducer(
      FSoulTrackingState(), GameSoulActions::MarkSoulExported(ExportPayload));
  SoulState = SoulSlice.Reducer(
      SoulState, GameSoulActions::MarkSoulImported(TEXT("tx-001")));

  const func::Maybe<FString> TxId =
      GameSoulSelectors::SelectSoulExportTxId(SoulState, TEXT("miller"));
  TestTrue("Soul export selector finds tx id", TxId.hasValue);
  TestEqual("Soul import selector reads tx ids",
            GameSoulSelectors::SelectImportedSoulTxIds(SoulState).Num(), 1);

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameMechanicsTerminalSelectorsTest,
    "ForbocAI.Slices.TestGame.MechanicsTerminalSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FTestGameMechanicsTerminalSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;

  FSetGridSizePayload GridPayload;
  GridPayload.Width = 10;
  GridPayload.Height = 12;
  const FGridState GridState = CreateGridSlice().Reducer(
      FGridState(), GridActions::SetGridSize(GridPayload));
  TestEqual("Grid width selector reads state",
            GridSelectors::SelectGridWidth(GridState), 10);
  TestEqual("Grid height selector reads state",
            GridSelectors::SelectGridHeight(GridState), 12);

  FStealthState StealthState = CreateStealthSlice().Reducer(
      FStealthState(), StealthActions::SetDoorOpen(true));
  StealthState =
      CreateStealthSlice().Reducer(StealthState, StealthActions::BumpAlert(30));
  TestTrue("Stealth door selector reads state",
           StealthSelectors::SelectStealthDoorOpen(StealthState));
  TestEqual("Stealth alert selector reads state",
            StealthSelectors::SelectStealthAlertLevel(StealthState), 30);

  FTradeOffer Offer;
  Offer.NpcId = TEXT("miller");
  Offer.Item = TEXT("medkit");
  Offer.Price = 100;
  FSocialState SocialState = CreateSocialSlice().Reducer(
      FSocialState(), SocialActions::SetDialogue(TEXT("Trade?")));
  SocialState =
      CreateSocialSlice().Reducer(SocialState, SocialActions::SetTradeOffer(Offer));
  TestEqual("Social dialogue selector reads state",
            SocialSelectors::SelectSocialActiveDialogue(SocialState),
            FString(TEXT("Trade?")));
  TestTrue("Social trade selector reads optional trade",
           SocialSelectors::SelectSocialActiveTrade(SocialState).hasValue);

  FUIState UIState =
      CreateUISlice().Reducer(FUIState(), UIActions::SetMode(EPlayMode::Manual));
  UIState = CreateUISlice().Reducer(UIState,
                                    UIActions::AddMessage(TEXT("ready")));
  TestTrue("UI mode selector reads state",
           UISelectors::SelectUiMode(UIState) == EPlayMode::Manual);
  TestEqual("UI messages selector reads state",
            UISelectors::SelectUiMessages(UIState).Num(), 2);

  TranscriptActions::FRecordTranscriptPayload TranscriptPayload;
  TranscriptPayload.ScenarioId = TEXT("s1");
  TranscriptPayload.CommandGroup = ECommandGroup::Status;
  TranscriptPayload.Command = TEXT("forbocai status");
  TranscriptPayload.Status = ETranscriptStatus::Ok;
  FTranscriptState TranscriptState = CreateTranscriptSlice().Reducer(
      FTranscriptState(), TranscriptActions::RecordTranscript(TranscriptPayload));
  TranscriptPayload.CommandGroup = ECommandGroup::NpcLifecycle;
  TranscriptPayload.Command = TEXT("forbocai npc create doomguard");
  TranscriptPayload.Status = ETranscriptStatus::Error;
  TranscriptState = CreateTranscriptSlice().Reducer(
      TranscriptState, TranscriptActions::RecordTranscript(TranscriptPayload));
  TestEqual("Transcript selector reads entries",
            TranscriptSelectors::SelectTranscriptEntries(TranscriptState).Num(),
            2);
  TestEqual("Transcript error selector derives failed command count",
            TranscriptSelectors::SelectTranscriptErrorCount(TranscriptState),
            1);

  const FHarnessState HarnessState = CreateHarnessSlice().Reducer(
      FHarnessState(), HarnessActions::MarkCovered(ECommandGroup::Status));
  TestEqual("Harness covered selector reads state",
            HarnessSelectors::SelectHarnessCovered(HarnessState).Num(), 1);
  TestEqual("Harness missing selector derives coverage",
            HarnessSelectors::SelectHarnessMissingGroups(HarnessState,
                                                         RequiredGroups())
                .Num(),
            RequiredGroups().Num() - 1);

  FScenarioSliceState ScenarioState;
  FScenarioStep Step;
  Step.Id = TEXT("s1");
  ScenarioState.Steps.Add(Step);
  TestEqual("Scenario steps selector reads state",
            ScenarioSelectors::SelectScenarioSteps(ScenarioState).Num(), 1);

  return true;
}
