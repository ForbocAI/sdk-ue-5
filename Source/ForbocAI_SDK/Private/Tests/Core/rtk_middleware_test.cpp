#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkMiddlewareTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGVA1B9794DC7CE,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkMiddlewareTest::RunTest(const FString &Parameters)
 */
bool FRtkMiddlewareTest::RunTest(const FString &Parameters) {
  TArray<FString> EventLog;

  /**
   * 1. Setup Base Dispatch and GetState
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  std::function<AnyAction(const AnyAction &)> BaseDispatch =
      [&EventLog](const AnyAction &Action) {
        EventLog.Add(FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9F5C9CF43438), *Action.Type));
        return Action;
      };

  const FAppFixtureState State{};
  std::function<const FAppFixtureState &()> GetState = [&State]() -> const FAppFixtureState & {
    return State;
  };

  /**
   * 2. Setup Middleware A (Logging before and after)
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  Middleware<FAppFixtureState> MiddlewareA =
      [&EventLog](const MiddlewareApi<FAppFixtureState> &Api)
          -> std::function<Dispatcher(Dispatcher)> {
        return [&EventLog](Dispatcher Next) -> Dispatcher {
          return [&EventLog, Next](const AnyAction &Action) -> AnyAction {
            EventLog.Add(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE5D27E5CADAF));
            auto Result = Next(Action);
            EventLog.Add(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFCEDCE7B1223));
            return Result;
          };
        };
      };

  /**
   * 3. Setup Listener Middleware (MwB)
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  ListenerMiddleware<FAppFixtureState> Listeners =
      addListener(createListenerMiddleware<FAppFixtureState>(), TEXT(FORBOCAI_SDK_AUTHORED_STRINGV345306F6CB50),
                  [&EventLog](const AnyAction &Action,
                              const MiddlewareApi<FAppFixtureState> &Api) {
                    EventLog.Add(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2B29D68E4B15));
                  });

  /**
   * 4. Compose
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  std::vector<Middleware<FAppFixtureState>> Chain = {
      MiddlewareA, buildListenerMiddleware(Listeners)};
  auto EnhancedDispatch = applyMiddleware(BaseDispatch, GetState, Chain);

  /**
   * 5. Execute
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  EnhancedDispatch(
      AnyAction{TEXT(FORBOCAI_SDK_AUTHORED_STRINGV345306F6CB50), std::make_shared<rtk::FEmptyPayload>()});

  /**
   * Validate Order
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVA0862CA90363, EventLog.Num(), FORBOCAI_SDK_AUTHORED_NUMBERV17F0DE0DDF4A);
  if (EventLog.Num() == FORBOCAI_SDK_AUTHORED_NUMBERV17F0DE0DDF4A) {
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV1C0E7B3F744A, EventLog[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA],
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVE5D27E5CADAF)));
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV534E24714ED4, EventLog[FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4],
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVC0C6CD47FB47)));
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVB550C0E8EDDD, EventLog[FORBOCAI_SDK_AUTHORED_NUMBERV6AC392A47561],
              FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2B29D68E4B15)));
    TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV3BB4EE49C1C1, EventLog[FORBOCAI_SDK_AUTHORED_NUMBERV32732DCF7787], FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVFCEDCE7B1223)));
  }

  return true;
}
