#include "Core/fp.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Entities/NPC/NPCSlice.h"
#include "Systems/Testing/NPC/TestingNPCAdapters.h"
#include "Misc/AutomationTest.h"

using namespace Testing::NPC;

namespace {

struct FNPCTestActionInput {
  NPCSlice::FNPCSliceState State;
  FNPCTestAction Action;
};

using FNPCTestDispatcher =
    func::ArgDispatcher<ENPCTestActionKind, FNPCTestActionInput,
                        NPCSlice::FNPCSliceState>;

/** User Story: As a tests slices consumer, I need to invoke required npcfield message through a stable signature so the tests slices workflow remains explicit and composable. @fn inline std::string RequiredNPCFieldMessage() */
inline std::string RequiredNPCFieldMessage() {
  return std::string(TCHAR_TO_UTF8(*TestingNPCFixtures().Labels.RequiredField));
}

/** User Story: As a tests slices consumer, I need to invoke required npcfield through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> T RequiredNPCField(const func::Maybe<T> &Value) */
template <typename T>
T RequiredNPCField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredNPCFieldMessage());
}

/** User Story: As a tests slices consumer, I need to invoke npctest slice through a stable signature so the tests slices workflow remains explicit and composable. @fn inline const rtk::Slice<NPCSlice::FNPCSliceState> &NPCTestSlice() */
inline const rtk::Slice<NPCSlice::FNPCSliceState> &NPCTestSlice() {
  static const rtk::Slice<NPCSlice::FNPCSliceState> Slice =
      NPCSlice::createNPCSlice();
  return Slice;
}

/** User Story: As a tests slices consumer, I need to invoke npctest info through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FNPCInternalState NPCTestInfo(const FNPCTestAction &Action) */
inline FNPCInternalState NPCTestInfo(const FNPCTestAction &Action) {
  FNPCInternalState Info;
  Info.Id = RequiredNPCField(Action.Id);
  Info.Persona = RequiredNPCField(Action.Persona);
  Info.State = TypeFactory::AgentState(RequiredNPCField(Action.StateJson));
  return Info;
}

/** User Story: As a tests slices consumer, I need to invoke build npctest dispatcher through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FNPCTestDispatcher BuildNPCTestDispatcher() */
inline FNPCTestDispatcher BuildNPCTestDispatcher() {
  FNPCTestDispatcher Dispatcher =
      func::create_arg_dispatcher<ENPCTestActionKind, FNPCTestActionInput,
                                  NPCSlice::FNPCSliceState>();
  const auto Register = [&Dispatcher](ENPCTestActionKind Kind, auto Reducer) {
    Dispatcher = func::arg_dispatcher_register<
        ENPCTestActionKind, FNPCTestActionInput, NPCSlice::FNPCSliceState>(
        Dispatcher, Kind, Reducer);
  };
  Register(ENPCTestActionKind::InfoReceived,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State, NPCActions::setNPCInfo(NPCTestInfo(Input.Action)));
           });
  Register(ENPCTestActionKind::ActiveChanged,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State, NPCActions::setActiveNPC(
                                  RequiredNPCField(Input.Action.Id)));
           });
  Register(ENPCTestActionKind::StateReplaced,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State,
                 NPCActions::setNPCState(
                     RequiredNPCField(Input.Action.Id),
                     TypeFactory::AgentState(
                         RequiredNPCField(Input.Action.StateJson))));
           });
  Register(ENPCTestActionKind::StateUpdated,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State,
                 NPCActions::updateNPCState(
                     RequiredNPCField(Input.Action.Id),
                     TypeFactory::AgentState(
                         RequiredNPCField(Input.Action.DeltaJson))));
           });
  Register(ENPCTestActionKind::HistoryAppended,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State,
                 NPCActions::addToHistory(
                     RequiredNPCField(Input.Action.Id),
                     RequiredNPCField(Input.Action.Role),
                     RequiredNPCField(Input.Action.Content)));
           });
  Register(ENPCTestActionKind::HistoryReceived,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State,
                 NPCActions::setHistory(RequiredNPCField(Input.Action.Id),
                                        Input.Action.History));
           });
  Register(ENPCTestActionKind::Blocked,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State,
                 NPCActions::blockAction(
                     RequiredNPCField(Input.Action.Id),
                     RequiredNPCField(Input.Action.Reason)));
           });
  Register(ENPCTestActionKind::BlockCleared,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State, NPCActions::clearBlock(
                                  RequiredNPCField(Input.Action.Id)));
           });
  Register(ENPCTestActionKind::Removed,
           [](const FNPCTestActionInput &Input) {
             return NPCTestSlice().Reducer(
                 Input.State, NPCActions::removeNPC(
                                  RequiredNPCField(Input.Action.Id)));
           });
  Register(ENPCTestActionKind::Inspect,
           [](const FNPCTestActionInput &Input) { return Input.State; });
  return Dispatcher;
}

/** User Story: As a tests slices consumer, I need to invoke apply npctest action through a stable signature so the tests slices workflow remains explicit and composable. @fn inline NPCSlice::FNPCSliceState ApplyNPCTestAction(const NPCSlice::FNPCSliceState &State, const FNPCTestAction &Action) */
inline NPCSlice::FNPCSliceState
ApplyNPCTestAction(const NPCSlice::FNPCSliceState &State,
                   const FNPCTestAction &Action) {
  static const FNPCTestDispatcher Dispatcher = BuildNPCTestDispatcher();
  const FNPCTestActionInput Input{State, Action};
  return RequiredNPCField(func::arg_dispatcher_dispatch_maybe(
      func::ArgDispatcherDispatch<ENPCTestActionKind, FNPCTestActionInput,
                                  NPCSlice::FNPCSliceState>{
          &Dispatcher, &Action.Kind, &Input}));
}

/** User Story: As a tests slices consumer, I need to invoke test npcexpected through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> void TestNPCExpected(FAutomationTestBase &Test, const FString &Label, const func::Maybe<T> &Expected, const func::Maybe<T> &Actual) */
template <typename T>
void TestNPCExpected(FAutomationTestBase &Test, const FString &Label,
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

/** User Story: As a tests slices consumer, I need to invoke test npcexpected through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> void TestNPCExpected(FAutomationTestBase &Test, const FString &Label, const func::Maybe<T> &Expected, const T &Actual) */
template <typename T>
void TestNPCExpected(FAutomationTestBase &Test, const FString &Label,
                     const func::Maybe<T> &Expected, const T &Actual) {
  TestNPCExpected(Test, Label, Expected, func::just<T>(Actual));
}

} // namespace

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FNPCTest, FORBOCAI_SDK_AUTHORED_STRINGVC3527C8743CC,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices consumer, I need to invoke get tests through a stable signature so the tests slices workflow remains explicit and composable. @fn void FNPCTest::GetTests(TArray<FString> &Names, TArray<FString> &Commands) const */
void FNPCTest::GetTests(TArray<FString> &Names,
                        TArray<FString> &Commands) const {
  func::for_each_array<FNPCTestScenario>(
      TestingNPCFixtures().Scenarios,
      [&Names, &Commands](const FNPCTestScenario &Scenario) {
        Names.Add(Scenario.Name);
        Commands.Add(Scenario.Name);
      });
}

/** User Story: As a tests slices consumer, I need to invoke run test through a stable signature so the tests slices workflow remains explicit and composable. @fn bool FNPCTest::RunTest(const FString &Parameters) */
bool FNPCTest::RunTest(const FString &Parameters) {
  const FNPCTestFixtures &Fixtures = TestingNPCFixtures();
  const func::Maybe<FNPCTestScenario> Scenario =
      FindNPCTestScenario(Parameters);
  TestTrue(Fixtures.Labels.ScenarioPresent, Scenario.hasValue);
  return func::match(
      Scenario,
      [this, &Fixtures](const FNPCTestScenario &Value) {
        func::fold_array<FNPCTestStep, NPCSlice::FNPCSliceState>(
            Value.Steps, NPCTestSlice().InitialState,
            [this, &Fixtures](const NPCSlice::FNPCSliceState &State,
                              const FNPCTestStep &Step) {
              const auto Next = ApplyNPCTestAction(State, Step.Action);
              const auto Selected = func::mbind(
                  Step.Action.Id, [&Next](const FString &Id) {
                    return NPCSelectors::selectNPCById(Next, Id);
                  });
              const auto SelectedState = func::mbind(
                  Step.Action.Id, [&Next](const FString &Id) {
                    return NPCSelectors::selectNPCState(Next, Id);
                  });
              const auto History = func::mbind(
                  Step.Action.Id, [&Next](const FString &Id) {
                    return NPCSelectors::selectNPCHistory(Next, Id);
                  });
              const auto StateLog = func::mbind(
                  Step.Action.Id, [&Next](const FString &Id) {
                    return NPCSelectors::selectNPCStateLog(Next, Id);
                  });
              const auto BlockReason = func::mbind(
                  Step.Action.Id, [&Next](const FString &Id) {
                    return NPCSelectors::selectNPCBlockReason(Next, Id);
                  });
              TestNPCExpected(*this, Fixtures.Labels.ActiveId,
                  Step.Expected.ActiveId,
                  NPCSelectors::selectActiveNpcId(Next));
              TestNPCExpected(*this, Fixtures.Labels.NpcCount,
                  Step.Expected.NpcCount,
                  NPCSelectors::selectTotalNPCs(Next));
              TestNPCExpected(*this, Fixtures.Labels.HasNpc,
                  Step.Expected.HasNpc, Selected.hasValue);
              TestNPCExpected(*this, Fixtures.Labels.SelectedPersona,
                  Step.Expected.SelectedPersona,
                  func::fmap(Selected, [](const FNPCInternalState &Npc) {
                    return Npc.Persona;
                  }));
              TestNPCExpected(*this, Fixtures.Labels.SelectedStateJson,
                  Step.Expected.SelectedStateJson,
                  func::fmap(SelectedState, [](const FAgentState &NpcState) {
                    return NpcState.JsonData;
                  }));
              TestNPCExpected(*this, Fixtures.Labels.HistoryCount,
                  Step.Expected.HistoryCount,
                  func::fmap(History, [](const TArray<FNPCHistoryEntry> &Items) {
                    return Items.Num();
                  }));
              const bool Blocked = func::match(
                  Step.Action.Id,
                  [&Next](const FString &Id) {
                    return NPCSelectors::selectNPCBlocked(Next, Id);
                  },
                  []() { return false; });
              TestNPCExpected(*this, Fixtures.Labels.Blocked,
                  Step.Expected.Blocked, Blocked);
              TestNPCExpected(*this, Fixtures.Labels.BlockReason,
                  Step.Expected.BlockReason, BlockReason);
              TestNPCExpected(*this, Fixtures.Labels.StateLogCount,
                  Step.Expected.StateLogCount,
                  func::fmap(StateLog,
                    [](const TArray<FNPCStateLogEntry> &Items) {
                      return Items.Num();
                    }));
              TestNPCExpected(*this, Fixtures.Labels.ActiveExists,
                  Step.Expected.ActiveExists,
                  NPCSelectors::selectActiveNPC(Next).hasValue);
              TestNPCExpected(*this, Fixtures.Labels.IdCount,
                  Step.Expected.IdCount,
                  NPCSelectors::selectNPCIds(Next).Num());
              TestNPCExpected(*this, Fixtures.Labels.EntityCount,
                  Step.Expected.EntityCount,
                  NPCSelectors::selectNPCEntities(Next).Num());
              return Next;
            });
        return true;
      },
      []() { return false; });
}
