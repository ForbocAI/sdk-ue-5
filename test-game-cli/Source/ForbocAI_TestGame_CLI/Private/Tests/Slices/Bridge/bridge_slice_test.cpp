#include "Misc/AutomationTest.h"
#include "TestGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameBridgeRulePatchAndSelectorsTest,
    VerificationAdapters::ArchitectureTestData().rtk.automationNames.bridge,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices bridge consumer, I need to invoke run test through a stable signature so the tests slices bridge workflow remains explicit and composable. @fn bool FTestGameBridgeRulePatchAndSelectorsTest::RunTest( const FString &Parameters) */
bool FTestGameBridgeRulePatchAndSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const Verification::FRtkVerificationData &Data =
      VerificationAdapters::ArchitectureTestData().rtk;

  GameBridgeActions::FSetBridgeRulesPayload Patch;
  Patch.MaxJumpForce = Data.bridge.patch.maxJumpForce;
  Patch.bHasMaxJumpForce = true;
  Patch.ActivePreset = Data.bridge.patch.activePreset;
  Patch.bHasActivePreset = true;

  FTestGameStore Store = createTestGameStore();
  Store.dispatch(GameBridgeActions::setBridgeRules(Patch));
  FBridgeRulesState State = Store.getState().Bridge;

  TestEqual(Data.stories.bridge,
            GameBridgeSelectors::SelectBridgeMaxJumpForce(State),
            Data.bridge.patch.maxJumpForce);
  TestEqual(Data.stories.bridge,
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State),
            Data.bridge.expectedInitialDistance);
  TestEqual(Data.stories.bridge,
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            Data.bridge.patch.activePreset);

  Store.dispatch(GameBridgeActions::loadBridgePreset(
      Data.bridge.presets.social.name));
  State = Store.getState().Bridge;
  TestEqual(Data.stories.bridge,
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State),
            Data.bridge.presets.social.expectedDistance);
  TestEqual(Data.stories.bridge,
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            Data.bridge.presets.social.name);

  Store.dispatch(GameBridgeActions::loadBridgePreset(
      Data.bridge.presets.custom.name));
  State = Store.getState().Bridge;
  TestEqual(Data.stories.bridge,
            GameBridgeSelectors::SelectBridgeMaxMoveDistance(State),
            Data.bridge.presets.custom.expectedDistance);
  TestEqual(Data.stories.bridge,
            GameBridgeSelectors::SelectBridgeActivePreset(State),
            Data.bridge.presets.custom.name);

  return true;
}
