#include "Core/fp.hpp"
#include "Features/Memory/MemorySelectors.h"
#include "Features/Testing/Memory/TestingMemoryAdapters.h"
#include "Misc/AutomationTest.h"

using namespace Testing::Memory;

namespace {

struct FMemoryTestActionInput {
  MemorySlice::FMemorySliceState State;
  FMemoryTestAction Action;
};

using FMemoryTestDispatcher =
    func::ArgDispatcher<EMemoryTestActionKind, FMemoryTestActionInput,
                        MemorySlice::FMemorySliceState>;

/** User Story: As a tests slices consumer, I need to invoke required memory field message through a stable signature so the tests slices workflow remains explicit and composable. @fn inline std::string RequiredMemoryFieldMessage() */
inline std::string RequiredMemoryFieldMessage() {
  return std::string(
      TCHAR_TO_UTF8(*TestingMemoryFixtures().Labels.RequiredField));
}

/** User Story: As a tests slices consumer, I need to invoke required memory field through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> T RequiredMemoryField(const func::Maybe<T> &Value) */
template <typename T>
T RequiredMemoryField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredMemoryFieldMessage());
}

/** User Story: As a tests slices consumer, I need to invoke memory test slice through a stable signature so the tests slices workflow remains explicit and composable. @fn inline const rtk::Slice<MemorySlice::FMemorySliceState> &MemoryTestSlice() */
inline const rtk::Slice<MemorySlice::FMemorySliceState> &MemoryTestSlice() {
  static const rtk::Slice<MemorySlice::FMemorySliceState> Slice =
      MemorySlice::createMemorySlice();
  return Slice;
}

/** User Story: As a tests slices consumer, I need to invoke first memory item through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FMemoryItem FirstMemoryItem(const FMemoryTestAction &Action) */
inline FMemoryItem FirstMemoryItem(const FMemoryTestAction &Action) {
  return RequiredMemoryField(func::find_array<FMemoryItem>(
      Action.Items, [](const FMemoryItem &) { return true; }));
}

/** User Story: As a tests slices consumer, I need to invoke build memory test dispatcher through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FMemoryTestDispatcher BuildMemoryTestDispatcher() */
inline FMemoryTestDispatcher BuildMemoryTestDispatcher() {
  FMemoryTestDispatcher Dispatcher =
      func::create_arg_dispatcher<EMemoryTestActionKind,
                                  FMemoryTestActionInput,
                                  MemorySlice::FMemorySliceState>();
  const auto Register = [&Dispatcher](EMemoryTestActionKind Kind,
                                      auto Reducer) {
    Dispatcher = func::arg_dispatcher_register<
        EMemoryTestActionKind, FMemoryTestActionInput,
        MemorySlice::FMemorySliceState>(Dispatcher, Kind, Reducer);
  };
  Register(EMemoryTestActionKind::StoreStarted,
           [](const FMemoryTestActionInput &Input) {
             return MemoryTestSlice().Reducer(
                 Input.State, MemorySlice::Actions::memoryStoreStart());
           });
  Register(EMemoryTestActionKind::StoreSucceeded,
           [](const FMemoryTestActionInput &Input) {
             return MemoryTestSlice().Reducer(
                 Input.State, MemorySlice::Actions::memoryStoreSuccess(
                                  FirstMemoryItem(Input.Action)));
           });
  Register(EMemoryTestActionKind::StoreFailed,
           [](const FMemoryTestActionInput &Input) {
             return MemoryTestSlice().Reducer(
                 Input.State, MemorySlice::Actions::memoryStoreFailed(
                                  RequiredMemoryField(Input.Action.Error)));
           });
  Register(EMemoryTestActionKind::RecallStarted,
           [](const FMemoryTestActionInput &Input) {
             return MemoryTestSlice().Reducer(
                 Input.State, MemorySlice::Actions::memoryRecallStart());
           });
  Register(EMemoryTestActionKind::RecallSucceeded,
           [](const FMemoryTestActionInput &Input) {
             return MemoryTestSlice().Reducer(
                 Input.State, MemorySlice::Actions::memoryRecallSuccess(
                                  Input.Action.Items));
           });
  Register(EMemoryTestActionKind::RecallFailed,
           [](const FMemoryTestActionInput &Input) {
             return MemoryTestSlice().Reducer(
                 Input.State, MemorySlice::Actions::memoryRecallFailed(
                                  RequiredMemoryField(Input.Action.Error)));
           });
  Register(EMemoryTestActionKind::Cleared,
           [](const FMemoryTestActionInput &Input) {
             return MemoryTestSlice().Reducer(
                 Input.State, MemorySlice::Actions::memoryClear());
           });
  Register(EMemoryTestActionKind::Inspect,
           [](const FMemoryTestActionInput &Input) { return Input.State; });
  return Dispatcher;
}

/** User Story: As a tests slices consumer, I need to invoke apply memory test action through a stable signature so the tests slices workflow remains explicit and composable. @fn inline MemorySlice::FMemorySliceState ApplyMemoryTestAction(const MemorySlice::FMemorySliceState &State, const FMemoryTestAction &Action) */
inline MemorySlice::FMemorySliceState
ApplyMemoryTestAction(const MemorySlice::FMemorySliceState &State,
                      const FMemoryTestAction &Action) {
  static const FMemoryTestDispatcher Dispatcher = BuildMemoryTestDispatcher();
  const FMemoryTestActionInput Input{State, Action};
  return RequiredMemoryField(func::arg_dispatcher_dispatch_maybe(
      func::ArgDispatcherDispatch<EMemoryTestActionKind,
                                  FMemoryTestActionInput,
                                  MemorySlice::FMemorySliceState>{
          &Dispatcher, &Action.Kind, &Input}));
}

/** User Story: As a tests slices consumer, I need to invoke test memory expected through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> void TestMemoryExpected(FAutomationTestBase &Test, const FString &Label, const func::Maybe<T> &Expected, const func::Maybe<T> &Actual) */
template <typename T>
void TestMemoryExpected(FAutomationTestBase &Test, const FString &Label,
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

/** User Story: As a tests slices consumer, I need to invoke test memory expected through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> void TestMemoryExpected(FAutomationTestBase &Test, const FString &Label, const func::Maybe<T> &Expected, const T &Actual) */
template <typename T>
void TestMemoryExpected(FAutomationTestBase &Test, const FString &Label,
                        const func::Maybe<T> &Expected, const T &Actual) {
  TestMemoryExpected(Test, Label, Expected, func::just<T>(Actual));
}

/** User Story: As a tests slices consumer, I need to invoke first memory id through a stable signature so the tests slices workflow remains explicit and composable. @fn inline func::Maybe<FString> FirstMemoryId(const TArray<FMemoryItem> &Items) */
inline func::Maybe<FString>
FirstMemoryId(const TArray<FMemoryItem> &Items) {
  return func::fmap(
      func::find_array<FMemoryItem>(
          Items, [](const FMemoryItem &) { return true; }),
      [](const FMemoryItem &Item) { return Item.Id; });
}

} // namespace

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FMemoryTest, "ForbocAI.Slices.Memory",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices consumer, I need to invoke get tests through a stable signature so the tests slices workflow remains explicit and composable. @fn void FMemoryTest::GetTests(TArray<FString> &Names, TArray<FString> &Commands) const */
void FMemoryTest::GetTests(TArray<FString> &Names,
                           TArray<FString> &Commands) const {
  func::for_each_array<FMemoryTestScenario>(
      TestingMemoryFixtures().Scenarios,
      [&Names, &Commands](const FMemoryTestScenario &Scenario) {
        Names.Add(Scenario.Name);
        Commands.Add(Scenario.Name);
      });
}

/** User Story: As a tests slices consumer, I need to invoke run test through a stable signature so the tests slices workflow remains explicit and composable. @fn bool FMemoryTest::RunTest(const FString &Parameters) */
bool FMemoryTest::RunTest(const FString &Parameters) {
  const FMemoryTestFixtures &Fixtures = TestingMemoryFixtures();
  const func::Maybe<FMemoryTestScenario> Scenario =
      FindMemoryTestScenario(Parameters);
  TestTrue(Fixtures.Labels.ScenarioPresent, Scenario.hasValue);
  return func::match(
      Scenario,
      [this, &Fixtures](const FMemoryTestScenario &Value) {
        func::fold_array<FMemoryTestStep, MemorySlice::FMemorySliceState>(
            Value.Steps, MemoryTestSlice().InitialState,
            [this, &Fixtures](const MemorySlice::FMemorySliceState &State,
                              const FMemoryTestStep &Step) {
              const auto Next = ApplyMemoryTestAction(State, Step.Action);
              const auto Recalled =
                  MemorySelectors::selectRecalledMemories(Next);
              const auto Selected = func::mbind(
                  Step.Action.TargetId,
                  [&Next](const FString &Id) {
                    return MemorySelectors::selectMemoryById(Next, Id);
                  });
              TestMemoryExpected(*this, Fixtures.Labels.StorageStatus,
                  Step.Expected.StorageStatus,
                  MemorySelectors::selectMemoryStorageStatus(Next));
              TestMemoryExpected(*this, Fixtures.Labels.RecallStatus,
                  Step.Expected.RecallStatus,
                  MemorySelectors::selectMemoryRecallStatus(Next));
              TestMemoryExpected(*this, Fixtures.Labels.Error,
                  Step.Expected.Error, MemorySelectors::selectMemoryError(Next));
              TestMemoryExpected(*this, Fixtures.Labels.MemoryCount,
                  Step.Expected.MemoryCount,
                  MemorySelectors::selectMemoryCount(Next));
              TestMemoryExpected(*this, Fixtures.Labels.RecalledCount,
                  Step.Expected.RecalledCount, Recalled.Num());
              TestMemoryExpected(*this, Fixtures.Labels.FirstRecalledId,
                  Step.Expected.FirstRecalledId, FirstMemoryId(Recalled));
              TestMemoryExpected(*this, Fixtures.Labels.SelectedText,
                  Step.Expected.SelectedText,
                  func::fmap(Selected, [](const FMemoryItem &Item) {
                    return Item.Text;
                  }));
              return Next;
            });
        return true;
      },
      []() { return false; });
}
