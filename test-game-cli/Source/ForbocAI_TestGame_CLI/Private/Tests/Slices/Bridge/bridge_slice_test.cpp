#include "Misc/AutomationTest.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

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
  FBridgeRulesState State = Slice.Reducer(
      CreateBridgeInitialState(), GameBridgeActions::setBridgeRules(Patch));

  TestEqual("Bridge jump force updates",
            GameBridgeSelectors::SelectBridgeMaxJumpForce(State), 800);
  TestEqual("Bridge move distance remains unchanged",
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State), 2);
  TestEqual("Bridge preset updates",
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            FString(TEXT("heavy")));

  State =
      Slice.Reducer(State, GameBridgeActions::loadBridgePreset(TEXT("social")));
  TestEqual("Social preset narrows move distance",
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State), 1);
  TestEqual("Social preset is selected",
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            FString(TEXT("social")));

  State =
      Slice.Reducer(State, GameBridgeActions::loadBridgePreset(TEXT("custom")));
  TestEqual("Custom preset preserves move distance",
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State), 1);
  TestEqual("Custom preset is selected",
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            FString(TEXT("custom")));

  return true;
}
