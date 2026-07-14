#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkApiTest, "ForbocAI.Core.RTK.Api",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FRtkApiTest::RunTest(const FString &Parameters) {
  /**
   * 1. Define an API Endpoint
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  ApiEndpoint<FString, int32> GetUserEndpoint;
  GetUserEndpoint.EndpointName = TEXT("getUser");
  GetUserEndpoint.providesTags = {{TEXT("User"), TEXT("ID")}};

  /**
   * Deterministic endpoint request builder that resolves after parsing
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  GetUserEndpoint.RequestBuilder = [](const FString &UserId) {
    return func::AsyncResult<QueryReturnValue<int32>>::create(
        [UserId](auto Resolve, auto Reject) {
          if (UserId == TEXT("error")) {
            Resolve(QueryReturnValue<int32>::failure(
                FetchBaseQueryError::fetchError(TEXT("planned network failure"))));
          } else {
            Resolve(QueryReturnValue<int32>::success(42));
          }
        });
  };

  /**
   * 2. Register Endpoint in Api
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  EndpointBuilder<FAppFixtureState> Builder;
  auto BuiltGetUserEndpoint =
      Builder.mutation<FString, int32>(GetUserEndpoint);
  TestTrue("Endpoint builder marks mutation definitions",
           BuiltGetUserEndpoint.Type == DefinitionType::mutation);

  CreateApiOptions<FAppFixtureState> ApiOptions;
  ApiOptions.reducerPath = TEXT("testApi");
  ApiOptions.tagTypes.Add(TEXT("User"));
  ApiOptions.endpoints = [](EndpointBuilder<FAppFixtureState> &EndpointBuilder) {
    (void)EndpointBuilder;
  };
  Api<FAppFixtureState> TestApi = buildCreateApi<FAppFixtureState>()(ApiOptions);
  TestEqual("createApi options keep reducerPath", TestApi.ReducerPath,
            FString(TEXT("testApi")));
  TestEqual("createApi options keep tagTypes", TestApi.TagTypes.Num(), 1);

  auto GetUserThunk = injectEndpoints(TestApi, BuiltGetUserEndpoint);

  TArray<FString> EventLog;
  std::function<AnyAction(const AnyAction &)> RecordDispatch =
      [&EventLog](const AnyAction &Action) {
        EventLog.Add(Action.Type);
        return Action;
      };
  const FAppFixtureState State{};
  std::function<const FAppFixtureState &()> ReadState =
      [&State]() -> const FAppFixtureState & {
    return State;
  };

  /**
   * 3. Test Successful HTTP Call
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  auto SuccessOp = GetUserThunk(TEXT("123"));
  SuccessOp(RecordDispatch, ReadState).execute();

  TestEqual("Dispatched pending first (API success)", EventLog[0],
            FString(TEXT("testApi/getUser/pending")));
  TestEqual("Dispatched fulfilled second (API success)", EventLog[1],
            FString(TEXT("testApi/getUser/fulfilled")));
  EventLog.Empty();

  /**
   * 4. Test Failed HTTP Call
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  auto FailOp = GetUserThunk(TEXT("error"));
  FailOp(RecordDispatch, ReadState).execute();

  TestEqual("Dispatched pending first (API fail)", EventLog[0],
            FString(TEXT("testApi/getUser/pending")));
  TestEqual("Dispatched rejected second (API fail)", EventLog[1],
            FString(TEXT("testApi/getUser/rejected")));

  SerializeQueryArgsOptions SerializeOptions;
  SerializeOptions.endpointName = TEXT("getUser");
  SerializeOptions.queryArgs = TEXT("123");
  TestEqual("defaultSerializeQueryArgs mirrors RTK cache key shape",
            defaultSerializeQueryArgs(SerializeOptions),
            FString(TEXT("getUser(123)")));

  int32 RetryAttempts = 0;
  BaseQueryFn<rtk::FEmptyPayload, int32> FlakyBaseQuery =
      [&RetryAttempts](const rtk::FEmptyPayload &, const BaseQueryApi &,
                       const rtk::FEmptyPayload &) {
        return func::AsyncResult<QueryReturnValue<int32>>::create(
            [&RetryAttempts](
                std::function<void(QueryReturnValue<int32>)> Resolve,
                std::function<void(std::string)> Reject) {
              (void)Reject;
              ++RetryAttempts == 1
                  ? Resolve(QueryReturnValue<int32>::failure(
                        FetchBaseQueryError::fetchError(TEXT("retry once"))))
                  : Resolve(QueryReturnValue<int32>::success(7));
            });
      };
  RetryOptions RetryConfig;
  RetryConfig.maxRetries = 1;
  int32 RetryValue = 0;
  retry<rtk::FEmptyPayload, int32>(FlakyBaseQuery, RetryConfig)(
      rtk::FEmptyPayload{}, BaseQueryApi(), rtk::FEmptyPayload{})
      .then([&RetryValue](QueryReturnValue<int32> Value) {
        RetryValue = Value.data.hasValue ? Value.data.value : 0;
      })
      .execute();
  TestEqual("retry replays failed RTK Query results", RetryAttempts, 2);
  TestEqual("retry resolves the successful result", RetryValue, 7);

  return true;
}
