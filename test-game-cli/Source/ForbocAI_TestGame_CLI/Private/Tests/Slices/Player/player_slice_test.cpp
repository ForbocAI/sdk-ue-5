#include "Misc/AutomationTest.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGamePlayerPatchAndSelectorsTest,
    "ForbocAI.Slices.TestGame.PlayerPatchAndSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FTestGamePlayerPatchAndSelectorsTest::RunTest(
    const FString &Parameters) {
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
      CreatePlayerInitialState(), PlayerActions::patchPlayer(Patch));

  TestEqual("Player name remains owned by reducer",
            PlayerSelectors::SelectPlayerName(State), FString(TEXT("Scout")));
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
