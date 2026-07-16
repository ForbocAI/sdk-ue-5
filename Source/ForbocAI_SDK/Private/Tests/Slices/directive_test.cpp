#include "Core/rtk.hpp"
#include "Features/Directive/DirectiveSelectors.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Testing/Directive/TestingDirectiveAdapters.h"
#include "Misc/AutomationTest.h"

using namespace DirectiveSlice;
using namespace Testing::Directive;

namespace {

struct FDirectiveTestActionInput {
  FDirectiveSliceState State;
  FDirectiveTestAction Action;
};

typedef func::ArgDispatcher<EDirectiveTestActionKind,
                            FDirectiveTestActionInput,
                            FDirectiveSliceState>
    FDirectiveTestActionDispatcher;

/** User Story: As a tests slices consumer, I need to invoke required field message through a stable signature so the tests slices workflow remains explicit and composable. @fn inline std::string RequiredFieldMessage() */
inline std::string RequiredFieldMessage() {
  return std::string(
      TCHAR_TO_UTF8(*DirectiveTestFixtures().Labels.RequiredField));
}

/** User Story: As a tests slices consumer, I need to invoke required action field through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> T RequiredActionField(const func::Maybe<T> &Value) */
template <typename T>
T RequiredActionField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredFieldMessage());
}

/** User Story: As a tests slices consumer, I need to invoke directive test slice through a stable signature so the tests slices workflow remains explicit and composable. @fn inline const rtk::Slice<FDirectiveSliceState> &DirectiveTestSlice() */
inline const rtk::Slice<FDirectiveSliceState> &DirectiveTestSlice() {
  static const rtk::Slice<FDirectiveSliceState> Slice = createDirectiveSlice();
  return Slice;
}

/** User Story: As a tests slices consumer, I need to invoke build directive test action dispatcher through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FDirectiveTestActionDispatcher BuildDirectiveTestActionDispatcher() */
inline FDirectiveTestActionDispatcher BuildDirectiveTestActionDispatcher() {
  FDirectiveTestActionDispatcher Dispatcher =
      func::create_arg_dispatcher<EDirectiveTestActionKind,
                                  FDirectiveTestActionInput,
                                  FDirectiveSliceState>();
  Dispatcher = func::arg_dispatcher_register<
      EDirectiveTestActionKind, FDirectiveTestActionInput,
      FDirectiveSliceState>(
      Dispatcher, EDirectiveTestActionKind::Start,
      [](const FDirectiveTestActionInput &Input) {
        return DirectiveTestSlice().Reducer(
            Input.State, DirectiveSlice::Actions::directiveRunStarted(
                             RequiredActionField(Input.Action.Id),
                             RequiredActionField(Input.Action.NpcId),
                             RequiredActionField(Input.Action.Observation)));
      });
  Dispatcher = func::arg_dispatcher_register<
      EDirectiveTestActionKind, FDirectiveTestActionInput,
      FDirectiveSliceState>(
      Dispatcher, EDirectiveTestActionKind::Receive,
      [](const FDirectiveTestActionInput &Input) {
        FDirectiveResponse Response;
        Response.recallMemory.Query =
            RequiredActionField(Input.Action.Query);
        Response.recallMemory.Limit =
            RequiredActionField(Input.Action.Limit);
        Response.recallMemory.Threshold =
            RequiredActionField(Input.Action.Threshold);
        return DirectiveTestSlice().Reducer(
            Input.State,
            DirectiveSlice::Actions::directiveReceived(
                RequiredActionField(Input.Action.Id), Response));
      });
  Dispatcher = func::arg_dispatcher_register<
      EDirectiveTestActionKind, FDirectiveTestActionInput,
      FDirectiveSliceState>(
      Dispatcher, EDirectiveTestActionKind::Validate,
      [](const FDirectiveTestActionInput &Input) {
        FVerdictResponse Verdict;
        Verdict.bValid =
            RequiredActionField(Input.Action.VerdictValid);
        Verdict.Dialogue =
            RequiredActionField(Input.Action.Dialogue);
        Verdict.bHasAction =
            RequiredActionField(Input.Action.HasAction);
        check(!Verdict.bHasAction ||
              (Input.Action.ActionType.hasValue &&
               Input.Action.ActionTarget.hasValue &&
               Input.Action.ActionReason.hasValue));
        func::match(
            Input.Action.ActionType,
            [&Verdict](const FString &Value) {
              Verdict.Action.Type = Value;
              return true;
            },
            []() { return true; });
        func::match(
            Input.Action.ActionTarget,
            [&Verdict](const FString &Value) {
              Verdict.Action.Target = Value;
              return true;
            },
            []() { return true; });
        func::match(
            Input.Action.ActionReason,
            [&Verdict](const FString &Value) {
              Verdict.Action.Reason = Value;
              return true;
            },
            []() { return true; });
        return DirectiveTestSlice().Reducer(
            Input.State,
            DirectiveSlice::Actions::verdictValidated(
                RequiredActionField(Input.Action.Id), Verdict));
      });
  Dispatcher = func::arg_dispatcher_register<
      EDirectiveTestActionKind, FDirectiveTestActionInput,
      FDirectiveSliceState>(
      Dispatcher, EDirectiveTestActionKind::Fail,
      [](const FDirectiveTestActionInput &Input) {
        return DirectiveTestSlice().Reducer(
            Input.State,
            DirectiveSlice::Actions::directiveRunFailed(
                RequiredActionField(Input.Action.Id),
                RequiredActionField(Input.Action.Error)));
      });
  return func::arg_dispatcher_register<
      EDirectiveTestActionKind, FDirectiveTestActionInput,
      FDirectiveSliceState>(
      Dispatcher, EDirectiveTestActionKind::ClearNpc,
      [](const FDirectiveTestActionInput &Input) {
        return DirectiveTestSlice().Reducer(
            Input.State,
            DirectiveSlice::Actions::clearDirectivesForNpc(
                RequiredActionField(Input.Action.NpcId)));
      });
}

/** User Story: As a tests slices consumer, I need to invoke directive test action dispatcher through a stable signature so the tests slices workflow remains explicit and composable. @fn inline const FDirectiveTestActionDispatcher & DirectiveTestActionDispatcher() */
inline const FDirectiveTestActionDispatcher &
DirectiveTestActionDispatcher() {
  static const FDirectiveTestActionDispatcher Dispatcher =
      BuildDirectiveTestActionDispatcher();
  return Dispatcher;
}

/** User Story: As a tests slices consumer, I need to invoke apply directive test action through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FDirectiveSliceState ApplyDirectiveTestAction(const FDirectiveSliceState &State, const FDirectiveTestAction &Action) */
inline FDirectiveSliceState
ApplyDirectiveTestAction(const FDirectiveSliceState &State,
                         const FDirectiveTestAction &Action) {
  const FDirectiveTestActionInput Input{State, Action};
  const FDirectiveTestActionDispatcher &Dispatcher =
      DirectiveTestActionDispatcher();
  return RequiredActionField(
      func::arg_dispatcher_dispatch_maybe(
          func::ArgDispatcherDispatch<
              EDirectiveTestActionKind, FDirectiveTestActionInput,
              FDirectiveSliceState>{&Dispatcher, &Action.Kind, &Input}));
}

} // namespace

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FDirectiveTest, "ForbocAI.Slices.Directive",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices consumer, I need to invoke get tests through a stable signature so the tests slices workflow remains explicit and composable. @fn void FDirectiveTest::GetTests(TArray<FString> &OutBeautifiedNames, TArray<FString> &OutTestCommands) const */
void FDirectiveTest::GetTests(TArray<FString> &OutBeautifiedNames,
                              TArray<FString> &OutTestCommands) const {
  func::for_each_array<FDirectiveTestScenario>(
      DirectiveTestFixtures().Scenarios,
      [&OutBeautifiedNames,
       &OutTestCommands](const FDirectiveTestScenario &Scenario) {
        OutBeautifiedNames.Add(Scenario.Name);
        OutTestCommands.Add(Scenario.Name);
      });
}

/** User Story: As a tests slices consumer, I need to invoke run test through a stable signature so the tests slices workflow remains explicit and composable. @fn bool FDirectiveTest::RunTest(const FString &Parameters) */
bool FDirectiveTest::RunTest(const FString &Parameters) {
  const FDirectiveTestFixtures &Fixtures = DirectiveTestFixtures();
  const func::Maybe<FDirectiveTestScenario> Scenario =
      FindDirectiveTestScenario(Parameters);
  TestTrue(Fixtures.Labels.ScenarioPresent, Scenario.hasValue);
  return func::match(
      Scenario,
      [this, &Fixtures](const FDirectiveTestScenario &Value) {
        const FDirectiveSliceState State =
            func::fold_array<FDirectiveTestAction, FDirectiveSliceState>(
                Value.Actions, DirectiveTestSlice().InitialState,
                ApplyDirectiveTestAction);
        TestEqual(Fixtures.Labels.DirectiveCount,
                  selectAllDirectives(State).Num(),
                  Value.Expected.DirectiveCount);
        TestEqual(Fixtures.Labels.ActiveId,
                  selectActiveDirectiveId(State), Value.Expected.ActiveId);
        TestEqual(Fixtures.Labels.ActivePresence,
                  selectActiveDirective(State).hasValue,
                  !Value.Expected.ActiveId.IsEmpty());

        const auto TestString =
            [this](const FString &Label,
                   const func::Maybe<FString> &Expected,
                   const FString &Actual) {
              return func::match(
                  Expected,
                  [this, &Label, &Actual](const FString &ExpectedValue) {
                    TestEqual(Label, Actual, ExpectedValue);
                    return true;
                  },
                  []() { return true; });
            };
        const auto TestInteger =
            [this](const FString &Label,
                   const func::Maybe<int32> &Expected, int32 Actual) {
              return func::match(
                  Expected,
                  [this, &Label, Actual](int32 ExpectedValue) {
                    TestEqual(Label, Actual, ExpectedValue);
                    return true;
                  },
                  []() { return true; });
            };
        const auto TestFloat =
            [this](const FString &Label,
                   const func::Maybe<float> &Expected, float Actual) {
              return func::match(
                  Expected,
                  [this, &Label, Actual](float ExpectedValue) {
                    TestEqual(Label, Actual, ExpectedValue);
                    return true;
                  },
                  []() { return true; });
            };
        const auto TestBoolean =
            [this](const FString &Label,
                   const func::Maybe<bool> &Expected, bool Actual) {
              return func::match(
                  Expected,
                  [this, &Label, Actual](bool ExpectedValue) {
                    TestEqual(Label, Actual, ExpectedValue);
                    return true;
                  },
                  []() { return true; });
            };

        func::for_each_array<FDirectiveTestRunExpected>(
            Value.Expected.Runs,
            [this, &State, &Fixtures, &TestString, &TestInteger,
             &TestFloat,
             &TestBoolean](const FDirectiveTestRunExpected &Expected) {
              const func::Maybe<FDirectiveRun> Run =
                  selectDirectiveById(State, Expected.Id);
              TestEqual(Fixtures.Labels.RunPresence, Run.hasValue,
                        Expected.bExists);
              func::match(
                  Run,
                  [&Fixtures, &Expected, &TestString, &TestInteger,
                   &TestFloat, &TestBoolean](const FDirectiveRun &Actual) {
                    TestInteger(Fixtures.Labels.Status, Expected.Status,
                                static_cast<int32>(Actual.Status));
                    TestString(Fixtures.Labels.NpcId, Expected.NpcId,
                               Actual.NpcId);
                    TestString(Fixtures.Labels.Observation,
                               Expected.Observation, Actual.Observation);
                    TestString(Fixtures.Labels.Error, Expected.Error,
                               Actual.Error);
                    TestString(Fixtures.Labels.RecallQuery,
                               Expected.RecallQuery,
                               Actual.MemoryRecallQuery);
                    TestInteger(Fixtures.Labels.RecallLimit,
                                Expected.RecallLimit,
                                Actual.MemoryRecallLimit);
                    TestFloat(Fixtures.Labels.RecallThreshold,
                              Expected.RecallThreshold,
                              Actual.MemoryRecallThreshold);
                    TestBoolean(Fixtures.Labels.VerdictValid,
                                Expected.VerdictValid,
                                Actual.bVerdictValid);
                    TestString(Fixtures.Labels.Dialogue,
                               Expected.Dialogue,
                               Actual.VerdictDialogue);
                    TestString(Fixtures.Labels.ActionType,
                               Expected.ActionType,
                               Actual.VerdictActionType);
                    return true;
                  },
                  []() { return true; });
            });
        return true;
      },
      []() { return false; });
}
