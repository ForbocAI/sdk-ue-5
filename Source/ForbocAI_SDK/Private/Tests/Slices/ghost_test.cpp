#include "Core/rtk.hpp"
#include "Features/Ghost/GhostSelectors.h"
#include "Features/Ghost/GhostSlice.h"
#include "Features/Testing/Ghost/TestingGhostAdapters.h"
#include "Misc/AutomationTest.h"

using namespace Testing::Ghost;
using namespace GhostSlice;

namespace {

struct FGhostTestActionInput {
  FGhostSliceState State;
  FGhostTestAction Action;
};

typedef func::ArgDispatcher<EGhostTestActionKind, FGhostTestActionInput,
                            FGhostSliceState>
    FGhostTestActionDispatcher;

inline std::string RequiredGhostFieldMessage() {
  return std::string(
      TCHAR_TO_UTF8(*TestingGhostFixtures().Labels.RequiredField));
}

template <typename T>
T RequiredGhostField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredGhostFieldMessage());
}

inline const rtk::Slice<FGhostSliceState> &GhostTestSlice() {
  static const rtk::Slice<FGhostSliceState> Slice =
      GhostSlice::createGhostSlice();
  return Slice;
}

inline TArray<FGhostTestResult>
GhostTestResults(const TArray<FString> &Scenarios) {
  return func::map_array<FString, FGhostTestResult>(
      Scenarios, [](const FString &Scenario) {
        FGhostTestResult Result;
        Result.Scenario = Scenario;
        Result.bPassed = true;
        return Result;
      });
}

inline TArray<FGhostHistoryEntry>
GhostHistory(const TArray<FString> &SessionIds) {
  return func::map_array<FString, FGhostHistoryEntry>(
      SessionIds, [](const FString &SessionId) {
        FGhostHistoryEntry Entry;
        Entry.SessionId = SessionId;
        return Entry;
      });
}

inline FGhostTestActionDispatcher BuildGhostTestActionDispatcher() {
  FGhostTestActionDispatcher Dispatcher =
      func::create_arg_dispatcher<EGhostTestActionKind,
                                  FGhostTestActionInput,
                                  FGhostSliceState>();
  Dispatcher = func::arg_dispatcher_register<
      EGhostTestActionKind, FGhostTestActionInput, FGhostSliceState>(
      Dispatcher, EGhostTestActionKind::Start,
      [](const FGhostTestActionInput &Input) {
        return GhostTestSlice().Reducer(
            Input.State, GhostSlice::Actions::ghostSessionStarted(
                             RequiredGhostField(Input.Action.SessionId),
                             RequiredGhostField(Input.Action.Status)));
      });
  Dispatcher = func::arg_dispatcher_register<
      EGhostTestActionKind, FGhostTestActionInput, FGhostSliceState>(
      Dispatcher, EGhostTestActionKind::Progress,
      [](const FGhostTestActionInput &Input) {
        return GhostTestSlice().Reducer(
            Input.State, GhostSlice::Actions::ghostSessionProgress(
                             RequiredGhostField(Input.Action.SessionId),
                             RequiredGhostField(Input.Action.Status),
                             RequiredGhostField(Input.Action.Progress)));
      });
  Dispatcher = func::arg_dispatcher_register<
      EGhostTestActionKind, FGhostTestActionInput, FGhostSliceState>(
      Dispatcher, EGhostTestActionKind::Complete,
      [](const FGhostTestActionInput &Input) {
        FGhostTestReport Report;
        Report.SessionId = RequiredGhostField(Input.Action.SessionId);
        Report.Results = GhostTestResults(Input.Action.ResultScenarios);
        return GhostTestSlice().Reducer(
            Input.State,
            GhostSlice::Actions::ghostSessionCompleted(Report));
      });
  Dispatcher = func::arg_dispatcher_register<
      EGhostTestActionKind, FGhostTestActionInput, FGhostSliceState>(
      Dispatcher, EGhostTestActionKind::Fail,
      [](const FGhostTestActionInput &Input) {
        return GhostTestSlice().Reducer(
            Input.State, GhostSlice::Actions::ghostSessionFailed(
                             RequiredGhostField(Input.Action.SessionId),
                             RequiredGhostField(Input.Action.Error)));
      });
  Dispatcher = func::arg_dispatcher_register<
      EGhostTestActionKind, FGhostTestActionInput, FGhostSliceState>(
      Dispatcher, EGhostTestActionKind::LoadHistory,
      [](const FGhostTestActionInput &Input) {
        return GhostTestSlice().Reducer(
            Input.State, GhostSlice::Actions::ghostHistoryLoaded(
                             GhostHistory(Input.Action.HistorySessionIds)));
      });
  Dispatcher = func::arg_dispatcher_register<
      EGhostTestActionKind, FGhostTestActionInput, FGhostSliceState>(
      Dispatcher, EGhostTestActionKind::Clear,
      [](const FGhostTestActionInput &Input) {
        return GhostTestSlice().Reducer(
            Input.State, GhostSlice::Actions::clearGhostSession());
      });
  return func::arg_dispatcher_register<
      EGhostTestActionKind, FGhostTestActionInput, FGhostSliceState>(
      Dispatcher, EGhostTestActionKind::Inspect,
      [](const FGhostTestActionInput &Input) { return Input.State; });
}

inline const FGhostTestActionDispatcher &GhostTestActionDispatcher() {
  static const FGhostTestActionDispatcher Dispatcher =
      BuildGhostTestActionDispatcher();
  return Dispatcher;
}

inline FGhostSliceState ApplyGhostTestAction(
    const FGhostSliceState &State, const FGhostTestAction &Action) {
  const FGhostTestActionInput Input{State, Action};
  const FGhostTestActionDispatcher &Dispatcher =
      GhostTestActionDispatcher();
  return RequiredGhostField(func::arg_dispatcher_dispatch_maybe(
      func::ArgDispatcherDispatch<EGhostTestActionKind,
                                  FGhostTestActionInput,
                                  FGhostSliceState>{
          &Dispatcher, &Action.Kind, &Input}));
}

} // namespace

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FGhostTest, "ForbocAI.Slices.Ghost",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

void FGhostTest::GetTests(TArray<FString> &OutBeautifiedNames,
                          TArray<FString> &OutTestCommands) const {
  func::for_each_array<FGhostTestScenario>(
      TestingGhostFixtures().Scenarios,
      [&OutBeautifiedNames,
       &OutTestCommands](const FGhostTestScenario &Scenario) {
        OutBeautifiedNames.Add(Scenario.Name);
        OutTestCommands.Add(Scenario.Name);
      });
}

bool FGhostTest::RunTest(const FString &Parameters) {
  const FGhostTestFixtures &Fixtures = TestingGhostFixtures();
  const func::Maybe<FGhostTestScenario> Scenario =
      FindGhostTestScenario(Parameters);
  TestTrue(Fixtures.Labels.ScenarioPresent, Scenario.hasValue);
  return func::match(
      Scenario,
      [this, &Fixtures](const FGhostTestScenario &Value) {
        const auto TestValue =
            [this](const FString &Label, const auto &Expected,
                   const auto &Actual) {
              return func::match(
                  Expected,
                  [this, &Label, &Actual](const auto &ExpectedValue) {
                    TestEqual(Label, Actual, ExpectedValue);
                    return true;
                  },
                  []() { return true; });
            };
        func::fold_array<FGhostTestStep, FGhostSliceState>(
            Value.Steps, FGhostSliceState(),
            [this, &Fixtures, &TestValue](const FGhostSliceState &State,
                                         const FGhostTestStep &Step) {
              const FGhostSliceState Next =
                  ApplyGhostTestAction(State, Step.Action);
              TestValue(Fixtures.Labels.ActiveSessionId,
                        Step.Expected.ActiveSessionId,
                        GhostSelectors::selectGhostActiveSessionId(Next));
              TestValue(Fixtures.Labels.Status, Step.Expected.Status,
                        GhostSelectors::selectGhostStatus(Next));
              TestValue(Fixtures.Labels.Progress, Step.Expected.Progress,
                        GhostSelectors::selectGhostProgress(Next));
              TestValue(Fixtures.Labels.HasResults,
                        Step.Expected.HasResults,
                        GhostSelectors::selectGhostHasResults(Next));
              TestValue(Fixtures.Labels.ResultCount,
                        Step.Expected.ResultCount,
                        GhostSelectors::selectGhostResults(Next)
                            .Results.Num());
              const TArray<FGhostHistoryEntry> &History =
                  GhostSelectors::selectGhostHistory(Next);
              TestValue(Fixtures.Labels.HistoryCount,
                        Step.Expected.HistoryCount, History.Num());
              func::match(
                  Step.Expected.HistoryFirstSessionId,
                  [this, &Fixtures, &History](const FString &Expected) {
                    const func::Maybe<FGhostHistoryEntry> First =
                        func::find_array<FGhostHistoryEntry>(
                            History,
                            [](const FGhostHistoryEntry &) { return true; });
                    TestTrue(Fixtures.Labels.HistoryFirstSessionId,
                             First.hasValue);
                    return func::match(
                        First,
                        [this, &Fixtures,
                         &Expected](const FGhostHistoryEntry &Actual) {
                          TestEqual(Fixtures.Labels.HistoryFirstSessionId,
                                    Actual.SessionId, Expected);
                          return true;
                        },
                        []() { return false; });
                  },
                  []() { return true; });
              TestValue(Fixtures.Labels.Loading, Step.Expected.Loading,
                        GhostSelectors::selectGhostLoading(Next));
              TestValue(Fixtures.Labels.Error, Step.Expected.Error,
                        GhostSelectors::selectGhostError(Next));
              return Next;
            });
        return true;
      },
      []() { return false; });
}
