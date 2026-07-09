#include "Misc/AutomationTest.h"
#include "TestGame/TestGameSlices.h"

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
