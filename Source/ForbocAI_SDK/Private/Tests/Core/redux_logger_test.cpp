#include "Core/redux_logger.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

using namespace rtk;
using namespace rtk::logger;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FReduxLoggerMiddlewareTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGV9EFBAB311CD2,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/** User Story: As a tests core consumer, I need to invoke run test through a stable signature so the tests core workflow remains explicit and composable. @fn bool FReduxLoggerMiddlewareTest::RunTest(const FString &Parameters) */
bool FReduxLoggerMiddlewareTest::RunTest(const FString &Parameters) {
  (void)Parameters;

  TArray<FString> Lines;

  ReduxLoggerOptions<FString> Options;
  Options.bTimestamp = false;
  Options.bDuration = true;
  Options.Logger = [&Lines](const FString &Line) { Lines.Add(Line); };
  Options.StateTransformer = [](const FString &State) { return State; };

  int32 DispatchCount = FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
  MiddlewareApi<FString> Api;
  Api.dispatch = [](const AnyAction &Action) { return Action; };
  const FString State = TEXT(FORBOCAI_SDK_AUTHORED_STRINGV05D2DE39A21B);
  Api.getState = [&State]() -> const FString & { return State; };

  Dispatcher Next = [&DispatchCount](const AnyAction &Action) -> AnyAction {
    ++DispatchCount;
    return Action;
  };

  const ActionCreator<FString> SetName =
      createAction<FString>(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5B426036ABDC));
  const Dispatcher Dispatch = createLogger<FString>(Options)(Api)(Next);
  const AnyAction Result = Dispatch(SetName(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4DDFECB0E3E1)));

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVBE141EE3497F, Result.Type,
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5B426036ABDC)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV559A8F34B07A, DispatchCount, FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4);
  TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV6CD9C916626A, Lines.Num() >= FORBOCAI_SDK_AUTHORED_NUMBERV17F0DE0DDF4A);

  if (Lines.Num() >= FORBOCAI_SDK_AUTHORED_NUMBERV17F0DE0DDF4A) {
    TestTrue(FORBOCAI_SDK_AUTHORED_STRINGVA0A202172FC9,
             Lines[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA].Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5B426036ABDC)));
    TestTrue(
        FORBOCAI_SDK_AUTHORED_STRINGVE6736D0BDBB1,
        Lines.ContainsByPredicate([](const FString &Line) {
          return Line.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF673571534B7)) && Line.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4DDFECB0E3E1));
        }));
    TestTrue(
        FORBOCAI_SDK_AUTHORED_STRINGV0079A89A9C08,
        Lines.ContainsByPredicate([](const FString &Line) {
          return Line.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD9EED4E7F70D)) &&
                 Line.Contains(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV05D2DE39A21B));
        }));
  }

  return true;
}
