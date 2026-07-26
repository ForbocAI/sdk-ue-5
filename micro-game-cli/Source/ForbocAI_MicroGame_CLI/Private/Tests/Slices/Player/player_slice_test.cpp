#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/CrossSdkConformanceAdapters.h"
#include "MicroGame/MicroGameStore.h"

using namespace MicroGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMicroGamePlayerPatchAndSelectorsTest,
    CrossSdkConformanceAdapters::CrossSdkConformanceData().rtk.automationNames.player,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices player consumer, I need to invoke run test through a stable signature so the tests slices player workflow remains explicit and composable. @fn bool FMicroGamePlayerPatchAndSelectorsTest::RunTest( const FString &Parameters) */
bool FMicroGamePlayerPatchAndSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const CrossSdkConformance::FRtkVerificationData &Data =
      CrossSdkConformanceAdapters::CrossSdkConformanceData().rtk;

  FPatchPlayerPayload Patch;
  Patch.Hp = Data.player.patch.hp;
  Patch.bHasHp = true;
  Patch.bHidden = Data.player.patch.hidden;
  Patch.bHasHidden = true;
  Patch.Position = Data.player.patch.position;
  Patch.bHasPosition = true;
  Patch.Inventory = Data.player.patch.inventory;
  Patch.bHasInventory = true;

  FMicroGameStore Store = createMicroGameStore();
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
