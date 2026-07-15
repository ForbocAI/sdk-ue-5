#include "Core/fp.hpp"
#include "Features/NPC/NPCSelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Testing/NPC/TestingNPCAdapters.h"
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

inline std::string RequiredNPCFieldMessage() {
  return std::string(TCHAR_TO_UTF8(*TestingNPCFixtures().Labels.RequiredField));
}

template <typename T>
T RequiredNPCField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredNPCFieldMessage());
}

inline const rtk::Slice<NPCSlice::FNPCSliceState> &NPCTestSlice() {
  static const rtk::Slice<NPCSlice::FNPCSliceState> Slice =
      NPCSlice::createNPCSlice();
  return Slice;
}

inline FNPCInternalState NPCTestInfo(const FNPCTestAction &Action) {
  FNPCInternalState Info;
  Info.Id = RequiredNPCField(Action.Id);
  Info.Persona = RequiredNPCField(Action.Persona);
  Info.State = TypeFactory::AgentState(RequiredNPCField(Action.StateJson));
  return Info;
}

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

template <typename T>
void TestNPCExpected(FAutomationTestBase &Test, const FString &Label,
                     const func::Maybe<T> &Expected, const T &Actual) {
  TestNPCExpected(Test, Label, Expected, func::just<T>(Actual));
}

} // namespace

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FNPCTest, "ForbocAI.Slices.NPC",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

void FNPCTest::GetTests(TArray<FString> &Names,
                        TArray<FString> &Commands) const {
  func::for_each_array<FNPCTestScenario>(
      TestingNPCFixtures().Scenarios,
      [&Names, &Commands](const FNPCTestScenario &Scenario) {
        Names.Add(Scenario.Name);
        Commands.Add(Scenario.Name);
      });
}

bool FNPCTest::RunTest(const FString &Parameters) {
  const FNPCTestFixtures &Fixtures = TestingNPCFixtures();
  const func::Maybe<FNPCTestScenario> Scenario =
      FindNPCTestScenario(Parameters);
  TestTrue(Fixtures.Labels.ScenarioPresent, Scenario.hasValue);
  return func::match(
      Scenario,
      [this, &Fixtures](const FNPCTestScenario &Value) {
        func::fold_array<FNPCTestStep, NPCSlice::FNPCSliceState>(
            Value.Steps, NPCSlice::FNPCSliceState(),
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
