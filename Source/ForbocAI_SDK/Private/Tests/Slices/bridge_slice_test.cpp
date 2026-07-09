#include "Bridge/BridgeSlice.h"
#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

using namespace rtk;
using namespace BridgeSlice;

/**
 * Test: bridgeValidationPending / Success / Failure lifecycle
 * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FBridgeSliceValidationLifecycleTest,
    "ForbocAI.Slices.Bridge.ValidationLifecycle",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FBridgeSliceValidationLifecycleTest::RunTest(const FString &Parameters) {
  Slice<FBridgeSliceState> BSlice = createBridgeSlice();
  FBridgeSliceState State;

  /**
   * Initial
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  TestEqual("Initial status idle", State.Status, FString(TEXT("idle")));
  TestFalse("No validation yet", State.bHasLastValidation);

  /**
   * Pending
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = BSlice.Reducer(State, BridgeSlice::Actions::bridgeValidationPending());
  TestEqual("Status validating", State.Status, FString(TEXT("validating")));
  TestTrue("Error cleared", State.Error.IsEmpty());

  /**
   * Success
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FValidationResult Result;
  Result.bValid = true;
  State = BSlice.Reducer(State, BridgeSlice::Actions::bridgeValidationSuccess(Result));
  TestEqual("Status idle after success", State.Status, FString(TEXT("idle")));
  TestTrue("Has last validation", State.bHasLastValidation);
  TestTrue("Validation is valid", State.LastValidation.bValid);

  return true;
}

/**
 * Test: bridgeValidationFailure sets error state
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBridgeSliceValidationFailTest,
                                 "ForbocAI.Slices.Bridge.ValidationFailure",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FBridgeSliceValidationFailTest::RunTest(const FString &Parameters) {
  Slice<FBridgeSliceState> BSlice = createBridgeSlice();
  FBridgeSliceState State;

  State = BSlice.Reducer(State, BridgeSlice::Actions::bridgeValidationPending());
  State = BSlice.Reducer(
      State, BridgeSlice::Actions::bridgeValidationFailure(TEXT("Rule violation")));

  TestEqual("Status error", State.Status, FString(TEXT("error")));
  TestEqual("Error message", State.Error, FString(TEXT("Rule violation")));
  TestTrue("Has last validation", State.bHasLastValidation);

  return true;
}

/**
 * Test: addActivePreset and setActivePresets
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBridgeSlicePresetsTest,
                                 "ForbocAI.Slices.Bridge.Presets",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FBridgeSlicePresetsTest::RunTest(const FString &Parameters) {
  Slice<FBridgeSliceState> BSlice = createBridgeSlice();
  FBridgeSliceState State;

  /**
   * Add preset
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FDirectiveRuleSet FirstPreset;
  FirstPreset.Id = TEXT("rpg_default");
  State = BSlice.Reducer(State, BridgeSlice::Actions::addActivePreset(FirstPreset));
  TestEqual("One active preset", State.ActivePresets.Num(), 1);
  TestEqual("Preset is rpg_default", State.ActivePresets[0].Id,
            FString(TEXT("rpg_default")));

  /**
   * Duplicate add should not duplicate
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  State = BSlice.Reducer(State, BridgeSlice::Actions::addActivePreset(FirstPreset));
  TestEqual("Still one preset (no dupe)", State.ActivePresets.Num(), 1);

  /**
   * Add another
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FDirectiveRuleSet SecondPreset;
  SecondPreset.Id = TEXT("combat");
  State = BSlice.Reducer(State, BridgeSlice::Actions::addActivePreset(SecondPreset));
  TestEqual("Two presets", State.ActivePresets.Num(), 2);
  TestEqual("selectActivePresets two", selectActivePresets(State).Num(), 2);
  TestTrue("selectActivePresetById combat",
           selectActivePresetById(State, TEXT("combat")).hasValue);

  /**
   * setActivePresets replaces all
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TArray<FDirectiveRuleSet> NewPresets;
  FDirectiveRuleSet StealthPreset;
  StealthPreset.Id = TEXT("stealth");
  NewPresets.Add(StealthPreset);
  State = BSlice.Reducer(State, BridgeSlice::Actions::setActivePresets(NewPresets));
  TestEqual("Replaced to one", State.ActivePresets.Num(), 1);
  TestEqual("Preset is stealth", State.ActivePresets[0].Id,
            FString(TEXT("stealth")));
  TestTrue("selectActivePresetById stealth",
           selectActivePresetById(State, TEXT("stealth")).hasValue);

  return true;
}

/**
 * Test: setAvailableRulesets
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FBridgeSliceRulesetsTest,
                                 "ForbocAI.Slices.Bridge.Rulesets",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FBridgeSliceRulesetsTest::RunTest(const FString &Parameters) {
  Slice<FBridgeSliceState> BSlice = createBridgeSlice();
  FBridgeSliceState State;

  TArray<FDirectiveRuleSet> Rulesets;
  FDirectiveRuleSet Rs;
  Rs.Id = TEXT("rs_1");
  Rs.RulesetId = TEXT("Default Rules");
  Rulesets.Add(Rs);

  State = BSlice.Reducer(State, BridgeSlice::Actions::setAvailableRulesets(Rulesets));
  TestEqual("One ruleset available", State.AvailableRulesets.Num(), 1);
  TestEqual("Ruleset id", State.AvailableRulesets[0].Id,
            FString(TEXT("rs_1")));

  FDirectiveRuleSet ActivePreset;
  ActivePreset.RulesetId = TEXT("preset_active");
  State = BSlice.Reducer(State, BridgeSlice::Actions::addActivePreset(ActivePreset));
  TestTrue("selectActivePresetById uses RulesetId",
           selectActivePresetById(State, TEXT("preset_active")).hasValue);

  TArray<FString> PresetIds;
  PresetIds.Add(TEXT("preset_active"));
  PresetIds.Add(TEXT("preset_alt"));
  State =
      BSlice.Reducer(State, BridgeSlice::Actions::setAvailablePresetIds(PresetIds));

  FValidationResult Result;
  Result.bValid = false;
  Result.Reason = TEXT("unsafe");
  State = BSlice.Reducer(State, BridgeSlice::Actions::bridgeValidationSuccess(Result));

  /**
   * Clear only resets validation fields.
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = BSlice.Reducer(State, BridgeSlice::Actions::clearBridgeValidation());
  TestEqual("Status reset", State.Status, FString(TEXT("idle")));
  TestFalse("No last validation", State.bHasLastValidation);
  TestTrue("Error cleared", State.Error.IsEmpty());
  TestEqual("Active presets preserved", State.ActivePresets.Num(), 1);
  TestEqual("Available rulesets preserved", State.AvailableRulesets.Num(), 1);
  TestEqual("Preset ids preserved", State.AvailablePresetIds.Num(), 2);

  return true;
}
