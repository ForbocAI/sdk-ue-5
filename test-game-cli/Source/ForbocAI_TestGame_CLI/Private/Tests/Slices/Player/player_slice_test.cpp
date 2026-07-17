#include "Misc/AutomationTest.h"
#include "TestGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGamePlayerPatchAndSelectorsTest,
    VerificationAdapters::ArchitectureTestData().rtk.automationNames.player,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices player consumer, I need to invoke run test through a stable signature so the tests slices player workflow remains explicit and composable. @fn bool FTestGamePlayerPatchAndSelectorsTest::RunTest( const FString &Parameters) */
bool FTestGamePlayerPatchAndSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const Verification::FRtkVerificationData &Data =
      VerificationAdapters::ArchitectureTestData().rtk;

  FPatchPlayerPayload Patch;
  Patch.Hp = Data.player.patch.hp;
  Patch.bHasHp = true;
  Patch.bHidden = Data.player.patch.hidden;
  Patch.bHasHidden = true;
  Patch.Position = Data.player.patch.position;
  Patch.bHasPosition = true;
  Patch.Inventory = Data.player.patch.inventory;
  Patch.bHasInventory = true;

  FTestGameStore Store = createTestGameStore();
  Store.dispatch(PlayerActions::patchPlayer(Patch));
  const FPlayerState &State = Store.getState().Player;

  TestEqual(Data.stories.player, PlayerSelectors::SelectPlayerName(State),
            Data.player.expectedName);
  TestEqual(Data.stories.player, PlayerSelectors::SelectPlayerHp(State),
            Data.player.patch.hp);
  TestFalse(Data.stories.player,
            PlayerSelectors::SelectPlayerHidden(State));
  TestEqual(Data.stories.player,
            PlayerSelectors::SelectPlayerPosition(State).X,
            Data.player.patch.position.X);
  TestEqual(Data.stories.player,
            PlayerSelectors::SelectPlayerPosition(State).Y,
            Data.player.patch.position.Y);
  TestEqual(Data.stories.player,
            PlayerSelectors::SelectPlayerInventory(State),
            Data.player.patch.inventory);

  return true;
}
