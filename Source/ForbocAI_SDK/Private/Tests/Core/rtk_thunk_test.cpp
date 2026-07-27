#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkAsyncThunkTest,
                                 FORBOCAI_SDK_AUTHORED_STRINGV12179AEB05AF,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkAsyncThunkTest::RunTest(const FString &Parameters)
 */
bool FRtkAsyncThunkTest::RunTest(const FString &Parameters) {
  /**
   * Build a deterministic async thunk
   * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
   */
  auto TestThunk = createAsyncThunk<int, FString, FAppFixtureState>(
      TEXT(FORBOCAI_SDK_AUTHORED_STRINGV249DF08722C0),
      [](const FString &Arg,
         const ThunkApi<FAppFixtureState> &Api) -> func::AsyncResult<int> {
        return func::AsyncResult<int>::create(
            [Arg](std::function<void(int)> Resolve,
                  std::function<void(std::string)> Reject) {
              if (Arg == TEXT(FORBOCAI_SDK_AUTHORED_STRINGV704327E48BCF)) {
                Reject(FORBOCAI_SDK_AUTHORED_STRINGV335FA0760E94);
              } else {
                Resolve(FORBOCAI_SDK_AUTHORED_NUMBERV2158B370F94E);
              }
            });
      });

  TArray<FString> DispatchedActions;
  std::function<AnyAction(const AnyAction &)> RecordDispatch =
      [&DispatchedActions](const AnyAction &Action) {
        DispatchedActions.Add(Action.Type);
        return Action;
      };

  const FAppFixtureState State{};
  std::function<const FAppFixtureState &()> ReadState =
      [&State]() -> const FAppFixtureState & {
    return State;
  };

  /**
   * Test Success Path
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  auto ThunkActionSuccess = TestThunk(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4CC9B79D80CC));
  auto ResultSuccess = ThunkActionSuccess(RecordDispatch, ReadState);
  ResultSuccess.execute();

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVA893B6D7D5D8, DispatchedActions[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA],
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1FBB7F8409C3)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV37D3F73C3CCD, DispatchedActions[FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4],
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV2876388CAC6B)));
  DispatchedActions.Empty();

  /**
   * Test Failure Path
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  auto ThunkActionFail = TestThunk(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV704327E48BCF));
  auto ResultFail = ThunkActionFail(RecordDispatch, ReadState);
  ResultFail.execute();

  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV8F15F7369C78, DispatchedActions[FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA],
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1FBB7F8409C3)));
  TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV294660045F71, DispatchedActions[FORBOCAI_SDK_AUTHORED_NUMBERV0063C33F45B4],
            FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGVADFCB52E0CFE)));

  return true;
}
