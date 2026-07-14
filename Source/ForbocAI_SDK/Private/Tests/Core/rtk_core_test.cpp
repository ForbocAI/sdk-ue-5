#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkStoreAndSliceTest,
                                 "ForbocAI.Core.RTK.StoreAndSlice",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FRtkStoreAndSliceTest::RunTest(const FString &Parameters) {
  auto SetInfoAction = createAction<FNpcFixtureState>(TEXT("npc/setInfo"));
  auto ResetAction = createAction(TEXT("npc/reset"));

  Slice<FNpcFixtureState> NpcSlice = createSlice<FNpcFixtureState>(
      TEXT("npc"), FNpcFixtureState{TEXT(""), 100},
      [SetInfoAction, ResetAction](
          ActionReducerMapBuilder<FNpcFixtureState> &Builder) {
        Builder.addCase(
            SetInfoAction,
            [](const FNpcFixtureState &State,
               const Action<FNpcFixtureState> &Action) {
              return Action.PayloadValue;
            });
        Builder.addCase(
            ResetAction,
            [](const FNpcFixtureState &State,
               const Action<rtk::FEmptyPayload> &Action) {
              return FNpcFixtureState{TEXT(""), 100};
            });
      });
  TestEqual("Slice keeps initial state ID", NpcSlice.InitialState.Id,
            FString(TEXT("")));
  TestEqual("Slice keeps initial state Health", NpcSlice.InitialState.Health,
            100);

  /**
   * Combine
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  ReducersMapObject<FAppFixtureState> Reducers;
  Reducers.reducer(&FAppFixtureState::ActiveNpc, NpcSlice.Reducer);
  auto RootReducer = combineReducers(Reducers);

  /**
   * Create Store
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  FAppFixtureState InitialState{FNpcFixtureState{TEXT(""), 100}};
  Store<FAppFixtureState> AppStore = createStore(InitialState, RootReducer);

  int CallCount = 0;
  auto Unsub = AppStore.subscribe([&CallCount]() { CallCount++; });

  AppStore.dispatch(SetInfoAction(FNpcFixtureState{TEXT("npc_1"), 80}));

  TestEqual("Dispatch updates root state ID", AppStore.getState().ActiveNpc.Id,
            FString(TEXT("npc_1")));
  TestEqual("Dispatch updates root state Health",
            AppStore.getState().ActiveNpc.Health, 80);
  TestEqual("Subscriber triggered", CallCount, 1);

  AppStore.dispatch(ResetAction());
  TestEqual("Empty payload dispatch clears ID",
            AppStore.getState().ActiveNpc.Id, FString(TEXT("")));
  TestEqual("Subscriber triggered again", CallCount, 2);

  Unsub();
  AppStore.dispatch(SetInfoAction(FNpcFixtureState{TEXT("npc_2"), 50}));
  TestEqual("Subscriber not triggered after Unsub", CallCount, 2);

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkCreateSliceTest,
                                 "ForbocAI.Core.RTK.CreateSlice",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FRtkCreateSliceTest::RunTest(const FString &Parameters) {
  rtk::ActionCreator<FNpcFixtureState> SetInfoAction;
  rtk::ActionCreatorWithoutPayload ResetAction;

  const Slice<FNpcFixtureState> NpcSlice = createSlice<FNpcFixtureState>(
      TEXT("npc"), FNpcFixtureState{TEXT(""), 100},
      [&SetInfoAction,
       &ResetAction](ActionReducerMapBuilder<FNpcFixtureState> &Builder) {
        SetInfoAction = createAction<FNpcFixtureState>(TEXT("npc/setInfo"));
        ResetAction = createAction(TEXT("npc/reset"));
        Builder.addCase(
            SetInfoAction,
            [](const FNpcFixtureState &State,
               const Action<FNpcFixtureState> &Action) {
              return Action.PayloadValue;
            });
        Builder.addCase(
            ResetAction,
            [](const FNpcFixtureState &State,
               const Action<rtk::FEmptyPayload> &Action) {
              return FNpcFixtureState{TEXT(""), 100};
            });
      });

  FNpcFixtureState State{TEXT("old"), 40};
  State = NpcSlice.Reducer(State, SetInfoAction(FNpcFixtureState{TEXT("new"), 75}));
  TestEqual("createSlice typed action updates ID", State.Id,
            FString(TEXT("new")));
  TestEqual("createSlice typed action updates Health", State.Health, 75);

  State = NpcSlice.Reducer(State, ResetAction());
  TestEqual("createSlice empty action resets ID", State.Id, FString(TEXT("")));
  TestEqual("createSlice empty action resets Health", State.Health, 100);

  const Slice<FNpcFixtureState> WrappedSlice = createSlice<FNpcFixtureState>(
      TEXT("npcWrapped"), FNpcFixtureState{TEXT("initial"), 10},
      CaseReducer<FNpcFixtureState>(
          [](const FNpcFixtureState &PrevState, const AnyAction &Action) {
            return Action.Type == TEXT("npcWrapped/clear")
                       ? FNpcFixtureState{TEXT(""), 0}
                       : PrevState;
          }));
  TestEqual("createSlice reducer overload keeps initial ID",
            WrappedSlice.InitialState.Id, FString(TEXT("initial")));
  TestEqual("createSlice reducer overload updates state",
            WrappedSlice.Reducer(FNpcFixtureState{TEXT("value"), 5},
                                 AnyAction{TEXT("npcWrapped/clear"),
                                           std::make_shared<FEmptyPayload>()})
                .Health,
            0);

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkConfigureStoreTest,
                                 "ForbocAI.Core.RTK.ConfigureStore",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FRtkConfigureStoreTest::RunTest(const FString &Parameters) {
  /**
   * 1. Setup Reducer
   * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
   */
  auto RootReducer = [](const FAppFixtureState &State, const AnyAction &Action) {
    FAppFixtureState Next = State;
    if (Action.Type == TEXT("trigger")) {
      Next.ActiveNpc.Health -= 10;
    }
    return Next;
  };

  FAppFixtureState PreloadState{FNpcFixtureState{TEXT("FixtureNpc"), 100}};

  /**
   * 2. Setup Middleware
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TArray<FString> EventLog;
  Middleware<FAppFixtureState> AuditMw =
      [&EventLog](const MiddlewareApi<FAppFixtureState> &Api)
          -> std::function<Dispatcher(Dispatcher)> {
        return [&EventLog](Dispatcher Next) -> Dispatcher {
          return [&EventLog, Next](const AnyAction &Action) -> AnyAction {
            EventLog.Add(FString::Printf(TEXT("MW:%s"), *Action.Type));
            return Next(Action);
          };
        };
      };

  std::vector<Middleware<FAppFixtureState>> Middlewares = {AuditMw};

  /**
   * 3. Configure Store
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  auto Store =
      configureStore<FAppFixtureState>(RootReducer, PreloadState, Middlewares);

  /**
   * 4. Assert Preload
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual("Preloaded Health", Store.getState().ActiveNpc.Health, 100);

  /**
   * 5. Dispatch Action & Validate Middleware Chain + State update
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(
      AnyAction{TEXT("trigger"), std::make_shared<rtk::FEmptyPayload>()});

  TestEqual("State Updated", Store.getState().ActiveNpc.Health, 90);
  TestEqual("Middleware Log Length", EventLog.Num(), 1);
  if (EventLog.Num() == 1) {
    TestEqual("Middleware intercept fired", EventLog[0],
              FString(TEXT("MW:trigger")));
  }

  return true;
}
