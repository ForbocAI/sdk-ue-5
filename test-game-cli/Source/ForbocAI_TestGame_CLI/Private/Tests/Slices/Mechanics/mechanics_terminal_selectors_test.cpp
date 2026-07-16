#include "Misc/AutomationTest.h"
#include "TestGame/Features/Components/Spatial/Grid/GridSelectors.h"
#include "TestGame/Features/Systems/Bridge/BridgeSelectors.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageSelectors.h"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "TestGame/Features/Systems/Terminal/Transcript/TranscriptSelectors.h"
#include "TestGame/Features/Systems/Terminal/UI/UISelectors.h"
#include "TestGame/TestGameStore.h"

using namespace TestGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FTestGameMechanicsTerminalSelectorsTest,
    "ForbocAI.Slices.TestGame.MechanicsTerminalSelectors",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices mechanics consumer, I need to invoke run test through a stable signature so the tests slices mechanics workflow remains explicit and composable. @fn bool FTestGameMechanicsTerminalSelectorsTest::RunTest( const FString &Parameters) */
bool FTestGameMechanicsTerminalSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;

  FSetGridSizePayload GridPayload;
  GridPayload.Width = 10;
  GridPayload.Height = 12;
  const FGridState GridState = CreateGridSlice().Reducer(
      CreateGridInitialState(), GridActions::setGridSize(GridPayload));
  TestEqual("Grid width selector reads state",
            GridSelectors::SelectGridWidth(GridState), 10);
  TestEqual("Grid height selector reads state",
            GridSelectors::SelectGridHeight(GridState), 12);
  TestFalse("Blocked grid positions are not passable",
            IsPassable(CreateGridInitialState(), FPosition(4, 4)));
  TestTrue("Open grid positions are passable",
           IsPassable(CreateGridInitialState(), FPosition(1, 1)));

  const FJumpValidation Jump = ValidateJump(CreateBridgeInitialState(), 800);
  TestFalse("Jump validation enforces authored limits", Jump.bValid);
  const FMoveDistanceResult Move =
      CapMoveDistance(CreateBridgeInitialState(), 6);
  TestEqual("Movement validation caps authored distance", Move.AllowedDistance,
            2);
  TestTrue("Movement validation reports capping", Move.bCapped);

  FStealthState StealthState = CreateStealthSlice().Reducer(
      CreateStealthInitialState(), StealthActions::setDoorOpen(true));
  StealthState =
      CreateStealthSlice().Reducer(StealthState, StealthActions::bumpAlert(30));
  TestTrue("Stealth door selector reads state",
           StealthSelectors::SelectStealthDoorOpen(StealthState));
  TestEqual("Stealth alert selector reads state",
            StealthSelectors::SelectStealthAlertLevel(StealthState), 30);

  FTradeOffer Offer;
  Offer.NpcId = TEXT("miller");
  Offer.Item = TEXT("medkit");
  Offer.Price = 100;
  FSocialState SocialState = CreateSocialSlice().Reducer(
      FSocialState(), SocialActions::setDialogue(TEXT("Trade?")));
  SocialState = CreateSocialSlice().Reducer(
      SocialState, SocialActions::setTradeOffer(Offer));
  TestEqual("Social dialogue selector reads state",
            SocialSelectors::SelectSocialActiveDialogue(SocialState),
            FString(TEXT("Trade?")));
  TestTrue("Social trade selector reads optional trade",
           SocialSelectors::SelectSocialActiveTrade(SocialState).hasValue);

  FUIState UIState =
      CreateUISlice().Reducer(FUIState(), UIActions::setMode(EPlayMode::Manual));
  UIState =
      CreateUISlice().Reducer(UIState, UIActions::addMessage(TEXT("ready")));
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
      FTranscriptState(),
      TranscriptActions::recordTranscript(TranscriptPayload));
  TranscriptPayload.CommandGroup = ECommandGroup::NpcLifecycle;
  TranscriptPayload.Command = TEXT("forbocai npc create doomguard");
  TranscriptPayload.Status = ETranscriptStatus::Error;
  TranscriptState = CreateTranscriptSlice().Reducer(
      TranscriptState, TranscriptActions::recordTranscript(TranscriptPayload));
  TestEqual("Transcript selector reads entries",
            TranscriptSelectors::SelectTranscriptEntries(TranscriptState).Num(),
            2);
  TestEqual("Transcript error selector derives failed command count",
            TranscriptSelectors::SelectTranscriptErrorCount(TranscriptState),
            1);

  const FHarnessState HarnessState = CreateHarnessSlice().Reducer(
      FHarnessState(), HarnessActions::markCovered(ECommandGroup::Status));
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
  TArray<FScenarioStep> ContractSteps;
  ContractSteps.Add(Step);
  ScenarioState = CreateScenarioSlice().Reducer(
      FScenarioSliceState(),
      ScenarioActions::contractReceived(MoveTemp(ContractSteps)));
  TestEqual("Scenario steps selector reads state",
            ScenarioSelectors::SelectScenarioSteps(ScenarioState).Num(), 1);

  return true;
}
