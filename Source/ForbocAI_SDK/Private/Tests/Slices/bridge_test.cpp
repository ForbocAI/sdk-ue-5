#include "Core/fp.hpp"
#include "Features/Bridge/BridgeSelectors.h"
#include "Features/Testing/Bridge/TestingBridgeAdapters.h"
#include "Misc/AutomationTest.h"

using namespace Testing::Bridge;

namespace {

struct FBridgeTestActionInput {
  BridgeSlice::FBridgeSliceState State;
  FBridgeTestAction Action;
};

using FBridgeTestDispatcher =
    func::ArgDispatcher<EBridgeTestActionKind, FBridgeTestActionInput,
                        BridgeSlice::FBridgeSliceState>;

inline std::string RequiredBridgeFieldMessage() {
  return std::string(
      TCHAR_TO_UTF8(*TestingBridgeFixtures().Labels.RequiredField));
}

template <typename T>
T RequiredBridgeField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredBridgeFieldMessage());
}

inline const rtk::Slice<BridgeSlice::FBridgeSliceState> &BridgeTestSlice() {
  static const rtk::Slice<BridgeSlice::FBridgeSliceState> Slice =
      BridgeSlice::createBridgeSlice();
  return Slice;
}

inline FDirectiveRuleSet FirstBridgeRuleset(const FBridgeTestAction &Action) {
  return RequiredBridgeField(func::find_array<FDirectiveRuleSet>(
      Action.Rulesets, [](const FDirectiveRuleSet &) { return true; }));
}

inline FBridgeTestDispatcher BuildBridgeTestDispatcher() {
  FBridgeTestDispatcher Dispatcher =
      func::create_arg_dispatcher<EBridgeTestActionKind,
                                  FBridgeTestActionInput,
                                  BridgeSlice::FBridgeSliceState>();
  const auto Register = [&Dispatcher](EBridgeTestActionKind Kind,
                                      auto Reducer) {
    Dispatcher = func::arg_dispatcher_register<
        EBridgeTestActionKind, FBridgeTestActionInput,
        BridgeSlice::FBridgeSliceState>(Dispatcher, Kind, Reducer);
  };
  Register(EBridgeTestActionKind::ValidationRequested,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::validationRequested());
           });
  Register(EBridgeTestActionKind::ValidationSucceeded,
           [](const FBridgeTestActionInput &Input) {
             FValidationResult Result;
             Result.bValid = RequiredBridgeField(Input.Action.Valid);
             Result.Reason = RequiredBridgeField(Input.Action.Reason);
             return BridgeTestSlice().Reducer(
                 Input.State,
                 BridgeSlice::Actions::validationSucceeded(Result));
           });
  Register(EBridgeTestActionKind::ValidationFailed,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::validationFailed(
                                  RequiredBridgeField(Input.Action.Reason)));
           });
  Register(EBridgeTestActionKind::ActivePresetsReceived,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::activePresetsReceived(
                                  Input.Action.Rulesets));
           });
  Register(EBridgeTestActionKind::ActivePresetAdded,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::activePresetAdded(
                                  FirstBridgeRuleset(Input.Action)));
           });
  Register(EBridgeTestActionKind::RulesetsReceived,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::rulesetsReceived(
                                  Input.Action.Rulesets));
           });
  Register(EBridgeTestActionKind::RulesetRegistered,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::rulesetRegistered(
                                  FirstBridgeRuleset(Input.Action)));
           });
  Register(EBridgeTestActionKind::RulesetDeleted,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::rulesetDeleted(
                                  RequiredBridgeField(Input.Action.TargetId)));
           });
  Register(EBridgeTestActionKind::PresetIdsReceived,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::presetIdsReceived(
                                  Input.Action.PresetIds));
           });
  Register(EBridgeTestActionKind::ValidationCleared,
           [](const FBridgeTestActionInput &Input) {
             return BridgeTestSlice().Reducer(
                 Input.State, BridgeSlice::Actions::validationCleared());
           });
  Register(EBridgeTestActionKind::Inspect,
           [](const FBridgeTestActionInput &Input) { return Input.State; });
  return Dispatcher;
}

inline BridgeSlice::FBridgeSliceState
ApplyBridgeTestAction(const BridgeSlice::FBridgeSliceState &State,
                      const FBridgeTestAction &Action) {
  static const FBridgeTestDispatcher Dispatcher = BuildBridgeTestDispatcher();
  const FBridgeTestActionInput Input{State, Action};
  return RequiredBridgeField(func::arg_dispatcher_dispatch_maybe(
      func::ArgDispatcherDispatch<EBridgeTestActionKind,
                                  FBridgeTestActionInput,
                                  BridgeSlice::FBridgeSliceState>{
          &Dispatcher, &Action.Kind, &Input}));
}

template <typename T>
void TestBridgeExpected(FAutomationTestBase &Test, const FString &Label,
                        const func::Maybe<T> &Expected,
                        const func::Maybe<T> &Actual) {
  func::match(
      Expected,
      [&Test, &Label, &Actual](const T &ExpectedValue) {
        Test.TestTrue(Label, Actual.hasValue);
        return func::match(
            Actual,
            [&Test, &Label, &ExpectedValue](const T &ActualValue) {
              Test.TestEqual(Label, ActualValue, ExpectedValue);
              return true;
            },
            []() { return false; });
      },
      []() { return true; });
}

template <typename T>
void TestBridgeExpected(FAutomationTestBase &Test, const FString &Label,
                        const func::Maybe<T> &Expected, const T &Actual) {
  TestBridgeExpected(Test, Label, Expected, func::just<T>(Actual));
}

inline func::Maybe<FString>
FirstBridgeRulesetId(const TArray<FDirectiveRuleSet> &Rulesets) {
  return func::fmap(
      func::find_array<FDirectiveRuleSet>(
          Rulesets, [](const FDirectiveRuleSet &) { return true; }),
      [](const FDirectiveRuleSet &Ruleset) { return Ruleset.Id; });
}

} // namespace

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FBridgeTest, "ForbocAI.Slices.Bridge",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

void FBridgeTest::GetTests(TArray<FString> &Names,
                           TArray<FString> &Commands) const {
  func::for_each_array<FBridgeTestScenario>(
      TestingBridgeFixtures().Scenarios,
      [&Names, &Commands](const FBridgeTestScenario &Scenario) {
        Names.Add(Scenario.Name);
        Commands.Add(Scenario.Name);
      });
}

bool FBridgeTest::RunTest(const FString &Parameters) {
  const FBridgeTestFixtures &Fixtures = TestingBridgeFixtures();
  const func::Maybe<FBridgeTestScenario> Scenario =
      FindBridgeTestScenario(Parameters);
  TestTrue(Fixtures.Labels.ScenarioPresent, Scenario.hasValue);
  return func::match(
      Scenario,
      [this, &Fixtures](const FBridgeTestScenario &Value) {
        func::fold_array<FBridgeTestStep, BridgeSlice::FBridgeSliceState>(
            Value.Steps, BridgeSlice::FBridgeSliceState(),
            [this, &Fixtures](const BridgeSlice::FBridgeSliceState &State,
                              const FBridgeTestStep &Step) {
              const auto Next = ApplyBridgeTestAction(State, Step.Action);
              const auto Validation =
                  BridgeSelectors::selectBridgeLastValidation(Next);
              const auto Active = BridgeSelectors::selectActivePresets(Next);
              const auto Available =
                  BridgeSelectors::selectAvailableRulesets(Next);
              TestBridgeExpected(*this, Fixtures.Labels.Status,
                  Step.Expected.Status,
                  BridgeSelectors::selectBridgeStatus(Next));
              TestBridgeExpected(*this, Fixtures.Labels.Error,
                  Step.Expected.Error, BridgeSelectors::selectBridgeError(Next));
              TestBridgeExpected(*this, Fixtures.Labels.HasValidation,
                  Step.Expected.HasValidation,
                  BridgeSelectors::selectBridgeHasLastValidation(Next));
              TestBridgeExpected(*this, Fixtures.Labels.ValidationValid,
                  Step.Expected.ValidationValid,
                  func::fmap(Validation, [](const FValidationResult &Result) {
                    return Result.bValid;
                  }));
              TestBridgeExpected(*this, Fixtures.Labels.ValidationReason,
                  Step.Expected.ValidationReason,
                  func::fmap(Validation, [](const FValidationResult &Result) {
                    return Result.Reason;
                  }));
              TestBridgeExpected(*this, Fixtures.Labels.ActiveCount,
                  Step.Expected.ActiveCount, Active.Num());
              TestBridgeExpected(*this, Fixtures.Labels.ActiveFirstId,
                  Step.Expected.ActiveFirstId, FirstBridgeRulesetId(Active));
              TestBridgeExpected(*this, Fixtures.Labels.AvailableCount,
                  Step.Expected.AvailableCount, Available.Num());
              TestBridgeExpected(*this, Fixtures.Labels.AvailableFirstId,
                  Step.Expected.AvailableFirstId,
                  FirstBridgeRulesetId(Available));
              const auto Selected = func::mbind(
                  Step.Action.TargetId,
                  [&Next](const FString &Id) {
                    return BridgeSelectors::selectAvailableRulesetById(Next,
                                                                         Id);
                  });
              TestBridgeExpected(*this,
                  Fixtures.Labels.AvailableSelectedRulesetId,
                  Step.Expected.AvailableSelectedRulesetId,
                  func::fmap(Selected, [](const FDirectiveRuleSet &Ruleset) {
                    return Ruleset.RulesetId;
                  }));
              TestBridgeExpected(*this, Fixtures.Labels.PresetIdCount,
                  Step.Expected.PresetIdCount,
                  BridgeSelectors::selectAvailablePresetIds(Next).Num());
              return Next;
            });
        return true;
      },
      []() { return false; });
}
