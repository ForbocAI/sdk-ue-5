#include "Misc/AutomationTest.h"
#include "MicroGame/Features/Components/Spatial/Grid/GridSelectors.h"
#include "MicroGame/Features/Systems/Bridge/BridgeSelectors.h"
#include "MicroGame/Features/Systems/Harness/Coverage/CoverageSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Harness/Scenario/ScenarioSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/CrossSdkConformance/CrossSdkConformanceAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "MicroGame/Features/Systems/Terminal/Transcript/TranscriptSelectors.h"
#include "MicroGame/Features/Systems/Terminal/UI/UISelectors.h"
#include "MicroGame/MicroGameStore.h"

using namespace MicroGame;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FMicroGameMechanicsTerminalSelectorsTest,
    CrossSdkConformanceAdapters::CrossSdkConformanceData().mechanics.automationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices mechanics consumer, I need to invoke run test through a stable signature so the tests slices mechanics workflow remains explicit and composable. @fn bool FMicroGameMechanicsTerminalSelectorsTest::RunTest( const FString &Parameters) */
bool FMicroGameMechanicsTerminalSelectorsTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const CrossSdkConformance::FCrossSdkConformanceData &Conformance =
      CrossSdkConformanceAdapters::CrossSdkConformanceData();
  const CrossSdkConformance::FBridgeGridVerificationData &BridgeGrid =
      Conformance.bridgeGrid;
  const CrossSdkConformance::FMechanicsVerificationData &Data =
      Conformance.mechanics;
  const FGameRuntimeData &Runtime = VerificationVocabularyAdapters::GameRuntimeData();
  FMicroGameStore Store = createMicroGameStore();

  FSetGridSizePayload GridPayload;
  GridPayload.Width = BridgeGrid.grid.state.Width;
  GridPayload.Height = BridgeGrid.grid.state.Height;
  Store.dispatch(GridActions::setGridSize(GridPayload));
  const FGridState GridState = Store.getState().Grid;
  TestEqual(BridgeGrid.stories.grid,
            GridSelectors::SelectGridWidth(GridState),
            BridgeGrid.grid.state.Width);
  TestEqual(BridgeGrid.stories.grid,
            GridSelectors::SelectGridHeight(GridState),
            BridgeGrid.grid.state.Height);
  TestFalse(BridgeGrid.stories.grid,
            IsPassable(BridgeGrid.grid.state,
                       BridgeGrid.grid.blockedPosition));
  TestTrue(BridgeGrid.stories.grid,
           IsPassable(BridgeGrid.grid.state, BridgeGrid.grid.openPosition));
  TestFalse(BridgeGrid.stories.grid,
            IsPassable(BridgeGrid.grid.state,
                       BridgeGrid.grid.outsidePosition));

  const FJumpValidation Jump =
      ValidateJump(BridgeGrid.rules, BridgeGrid.jump.requestedForce);
  TestFalse(BridgeGrid.stories.jump, Jump.bValid);
  TestTrue(BridgeGrid.stories.jump,
           Jump.Reason.Contains(BridgeGrid.jump.reasonFragment));
  const FMoveDistanceResult Move =
      CapMoveDistance(BridgeGrid.rules, BridgeGrid.move.requestedDistance);
  TestEqual(BridgeGrid.stories.move, Move.AllowedDistance,
            BridgeGrid.move.expectedDistance);
  TestTrue(BridgeGrid.stories.move, Move.bCapped);

  Store.dispatch(StealthActions::setDoorOpen(Data.stealth.doorOpen));
  Store.dispatch(StealthActions::bumpAlert(Data.stealth.alertDelta));
  const FStealthState StealthState = Store.getState().Stealth;
  TestTrue(Data.story,
           StealthSelectors::SelectStealthDoorOpen(StealthState));
  TestEqual(Data.story,
            StealthSelectors::SelectStealthAlertLevel(StealthState),
            Data.stealth.alertDelta);

  Store.dispatch(SocialActions::setDialogue(Data.social.dialogue));
  Store.dispatch(SocialActions::setTradeOffer(Data.social.tradeOffer));
  const FSocialState SocialState = Store.getState().Social;
  TestEqual(Data.story,
            SocialSelectors::SelectSocialActiveDialogue(SocialState),
            Data.social.dialogue);
  TestTrue(Data.story,
           SocialSelectors::SelectSocialActiveTrade(SocialState).hasValue);

  Store.dispatch(UIActions::setMode(Runtime.modes.manual));
  Store.dispatch(UIActions::addMessage(Data.ui.message));
  const FUIState UIState = Store.getState().UI;
  TestTrue(Data.story,
           UISelectors::SelectUiMode(UIState) == Runtime.modes.manual);
  TestEqual(Data.story, UISelectors::SelectUiMessages(UIState).Num(),
            Data.ui.expectedMessageCount);

  TranscriptActions::FRecordTranscriptPayload TranscriptPayload;
  TranscriptPayload.ScenarioId = Data.transcript.scenarioId;
  TranscriptPayload.CommandGroup = Runtime.commandGroups.status;
  TranscriptPayload.Command = Data.transcript.statusCommand;
  TranscriptPayload.Status = Runtime.statuses.ok;
  Store.dispatch(TranscriptActions::recordTranscript(TranscriptPayload));
  TranscriptPayload.CommandGroup = Runtime.commandGroups.npc_lifecycle;
  TranscriptPayload.Command = Data.transcript.npcCommand;
  TranscriptPayload.Status = Runtime.statuses.error;
  Store.dispatch(TranscriptActions::recordTranscript(TranscriptPayload));
  const FTranscriptState TranscriptState = Store.getState().Transcript;
  TestEqual(Data.story,
            TranscriptSelectors::SelectTranscriptEntries(TranscriptState).Num(),
            Data.transcript.expectedEntryCount);
  TestEqual(Data.story,
            TranscriptSelectors::SelectTranscriptErrorCount(TranscriptState),
            Data.transcript.expectedErrorCount);

  Store.dispatch(CoverageActions::markCovered(Data.coverage.coveredGroup));
  const FHarnessState HarnessState = Store.getState().Harness;
  TestEqual(Data.story,
            CoverageSelectors::SelectHarnessCovered(HarnessState).Num(),
            Data.coverage.expectedCoveredCount);
  TestEqual(Data.story,
            CoverageSelectors::SelectHarnessMissingGroups(
                HarnessState, Data.coverage.requiredGroups)
                .Num(),
            Data.coverage.expectedMissingCount);

  FScenarioStep Step;
  Step.Id = Data.transcript.scenarioId;
  FScenarioContractPayload ContractPayload;
  ContractPayload.RequiredCommandGroups = Data.coverage.requiredGroups;
  ContractPayload.Steps.Add(Step);
  Store.dispatch(ScenarioActions::setContract(MoveTemp(ContractPayload)));
  TestEqual(Data.story,
            ScenarioSelectors::SelectScenarioSteps(Store.getState().Scenario)
                .Num(),
            Data.scenario.expectedStepCount);

  return true;
}
