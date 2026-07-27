#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkStoreAndSliceTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGVF03B5289EFD4,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkStoreAndSliceTest::RunTest(const FString &Parameters)
 */
bool FRtkStoreAndSliceTest::RunTest(const FString &Parameters) {
  auto SetInfoAction = createAction<FNpcFixtureState>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA00708F44B93));
  auto ResetAction = createAction(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0F21910591D8));

  Slice<FNpcFixtureState> NpcSlice = createSlice<FNpcFixtureState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV62BC7243C9C2), FNpcFixtureState{TEXT(""), FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831},
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
              return FNpcFixtureState{TEXT(""), FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831};
            });
      });
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV366A4557AE9D, NpcSlice.InitialState.Id,
            FString(TEXT("")));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV528F0D2DE3AF, NpcSlice.InitialState.Health,
            FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831);

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
  FAppFixtureState InitialState{FNpcFixtureState{TEXT(""), FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831}};
  Store<FAppFixtureState> AppStore = createStore(InitialState, RootReducer);

  int CallCount = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
  auto Unsub = AppStore.subscribe([&CallCount]() { CallCount++; });

  AppStore.dispatch(SetInfoAction(FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4F0114EF284D), FORBOCAI_SDK_AUTHORED_NUMBERV6E60C7B9AD4C}));

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVC9598C5A3570, AppStore.getState().ActiveNpc.Id,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4F0114EF284D)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV0EFD9882AA3E,
            AppStore.getState().ActiveNpc.Health, FORBOCAI_SDK_AUTHORED_NUMBERV6E60C7B9AD4C);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVA0A8D4C460D7, CallCount, FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);

  AppStore.dispatch(ResetAction());
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV7AF1958B8A44,
            AppStore.getState().ActiveNpc.Id, FString(TEXT("")));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVE4B2B3EF0781, CallCount, FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561);

  Unsub();
  AppStore.dispatch(SetInfoAction(FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0D22AEE735EE), FORBOCAI_SDK_AUTHORED_NUMBERV476E954B364F}));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVCD759C7E9014, CallCount, FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561);

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkCreateSliceTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGVE9CAB0F4D379,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkCreateSliceTest::RunTest(const FString &Parameters)
 */
bool FRtkCreateSliceTest::RunTest(const FString &Parameters) {
  rtk::ActionCreator<FNpcFixtureState> SetInfoAction;
  rtk::ActionCreatorWithoutPayload ResetAction;

  const Slice<FNpcFixtureState> NpcSlice = createSlice<FNpcFixtureState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV62BC7243C9C2), FNpcFixtureState{TEXT(""), FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831},
      [&SetInfoAction,
       &ResetAction](ActionReducerMapBuilder<FNpcFixtureState> &Builder) {
        SetInfoAction = createAction<FNpcFixtureState>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA00708F44B93));
        ResetAction = createAction(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV0F21910591D8));
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
              return FNpcFixtureState{TEXT(""), FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831};
            });
      });

  FNpcFixtureState State{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9366D8342DAC), FORBOCAI_SDK_AUTHORED_NUMBERVA9A9F21D33B0};
  State = NpcSlice.Reducer(State, SetInfoAction(FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF16DB6474C5D), FORBOCAI_SDK_AUTHORED_NUMBERV02F0ED27296D}));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV30D9B652C2DE, State.Id,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF16DB6474C5D)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVBAD6BC91AFBE, State.Health, FORBOCAI_SDK_AUTHORED_NUMBERV02F0ED27296D);

  State = NpcSlice.Reducer(State, ResetAction());
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV2F3A67C7C829, State.Id, FString(TEXT("")));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVC5AF6FC1C330, State.Health, FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831);

  const Slice<FNpcFixtureState> WrappedSlice = createSlice<FNpcFixtureState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9FBD0813748D), FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4B55EFC1C8F6), FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F},
      CaseReducer<FNpcFixtureState>(
          [](const FNpcFixtureState &PrevState, const AnyAction &Action) {
            return Action.Type == TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6F8F1A66CA02)
                       ? FNpcFixtureState{TEXT(""), FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA}
                       : PrevState;
          }));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV42CE97C2234A,
            WrappedSlice.InitialState.Id, FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4B55EFC1C8F6)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV55C85A329C2A,
            WrappedSlice.Reducer(FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV02AA397FAA7D), FORBOCAI_SDK_AUTHORED_NUMBERV2B61CCD40B6E},
                                 AnyAction{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6F8F1A66CA02),
                                           std::make_shared<FEmptyPayload>()})
                .Health,
            FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkConfigureStoreTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGVC7BBF425448B,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkConfigureStoreTest::RunTest(const FString &Parameters)
 */
bool FRtkConfigureStoreTest::RunTest(const FString &Parameters) {
  /**
   * 1. Setup Reducer
   * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
   */
  auto RootReducer = [](const FAppFixtureState &State, const AnyAction &Action) {
    FAppFixtureState Next = State;
    if (Action.Type == TEXT(FORBOCAI_SDK_AUTHORED_STRINGV345306F6CB50)) {
      Next.ActiveNpc.Health -= FORBOCAI_SDK_AUTHORED_NUMBERV14FE7CBC615F;
    }
    return Next;
  };

  FAppFixtureState PreloadState{FNpcFixtureState{TEXT(FORBOCAI_SDK_AUTHORED_STRINGVDC5A9247171E), FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831}};

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
            EventLog.Add(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV969AA3FE0F9C), *Action.Type));
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
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVFD14BA14D2D9, Store.getState().ActiveNpc.Health, FORBOCAI_SDK_AUTHORED_NUMBERV41E9A1F40831);

  /**
   * 5. Dispatch Action & Validate Middleware Chain + State update
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  Store.dispatch(
      AnyAction{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV345306F6CB50), std::make_shared<rtk::FEmptyPayload>()});

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVD3EE8F645730, Store.getState().ActiveNpc.Health, FORBOCAI_SDK_AUTHORED_NUMBERV2B63E56DFF54);
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVEE32D9D41D7E, EventLog.Num(), FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  if (EventLog.Num() == FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV487E80C18AB8, EventLog[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA],
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF8794A6BB566)));
  }

  return true;
}
