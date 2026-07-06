#include "Core/redux_logger.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

using namespace rtk;
using namespace rtk::logger;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReduxLoggerMiddlewareTest,
                                 "ForbocAI.Core.ReduxLogger.Middleware",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
bool FReduxLoggerMiddlewareTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  TArray<FString> Lines;

  ReduxLoggerOptions<FString> Options;
  Options.bTimestamp = false;
  Options.bDuration = true;
  Options.Logger = [&Lines](const FString &Line) { Lines.Add(Line); };
  Options.StateTransformer = [](const FString &State) { return State; };

  int32 DispatchCount = 0;
  MiddlewareApi<FString> Api;
  Api.dispatch = [](const AnyAction &Action) { return Action; };
  const FString State = TEXT("ready");
  Api.getState = [&State]() -> const FString & { return State; };

  Dispatcher Next = [&DispatchCount](const AnyAction &Action) -> AnyAction {
    ++DispatchCount;
    return Action;
  };

  const ActionCreator<FString> SetName =
      createAction<FString>(TEXT("test/setName"));
  const Dispatcher Dispatch = createLogger<FString>(Options)(Api)(Next);
  const AnyAction Result = Dispatch(SetName(TEXT("Ada")));

  TestEqual("Logger returns next(action)", Result.Type,
            FString(TEXT("test/setName")));
  TestEqual("Logger calls next once", DispatchCount, 1);
  TestTrue("Logger emits grouped rows", Lines.Num() >= 4);

  if (Lines.Num() >= 4) {
    TestTrue("Title contains action type",
             Lines[0].Contains(TEXT("test/setName")));
    TestTrue(
        "Action row contains payload",
        Lines.ContainsByPredicate([](const FString &Line) {
          return Line.Contains(TEXT("action")) && Line.Contains(TEXT("Ada"));
        }));
    TestTrue(
        "Next state row contains transformed state",
        Lines.ContainsByPredicate([](const FString &Line) {
          return Line.Contains(TEXT("next state")) &&
                 Line.Contains(TEXT("ready"));
        }));
  }

  return true;
}
