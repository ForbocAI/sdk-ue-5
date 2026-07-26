#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/Testing/RTK/ToolkitSurface/ToolkitSurfaceAdapters.h"
#include "Misc/AutomationTest.h"
#include "rtk_test_fixtures.h"

using namespace rtk;
namespace ToolkitSurface = Testing::RTK::ToolkitSurface;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRtkApiTest,
                                 ToolkitSurface::ToolkitSurfaceFixtures().Names.Api,
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 * @fn bool FRtkApiTest::RunTest(const FString &Parameters)
 */
bool FRtkApiTest::RunTest(const FString &Parameters) {
  const ToolkitSurface::FToolkitSurfaceFixtures &Fixture =
      ToolkitSurface::ToolkitSurfaceFixtures();
  const ToolkitSurface::FApiFixture &ApiFixture = Fixture.Api;
  /**
   * 1. Define an API Endpoint
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  ApiEndpoint<FString, int32, FAppFixtureState> GetUserEndpoint;
  GetUserEndpoint.EndpointName = ApiFixture.EndpointName;
  GetUserEndpoint.providesTags = {{ApiFixture.TagType, ApiFixture.TagId}};

  /**
   * Deterministic endpoint request builder that resolves after parsing
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  GetUserEndpoint.RequestBuilder =
      [&ApiFixture](const FString &UserId,
                    const ApiContext<FAppFixtureState> &Context) {
    const int32 StateResult = Context.getState().ActiveNpc.Health;
    return func::AsyncResult<QueryReturnValue<int32>>::create(
        [UserId, &ApiFixture, StateResult](auto Resolve, auto Reject) {
          if (UserId == ApiFixture.FailureArgument) {
            Resolve(QueryReturnValue<int32>::failure(
                FetchBaseQueryError::fetchError(ApiFixture.FailureMessage)));
          } else {
            Resolve(QueryReturnValue<int32>::success(StateResult));
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
  TestTrue(Fixture.Labels.ApiEndpointType,
           BuiltGetUserEndpoint.Type == DefinitionType::mutation);

  CreateApiOptions<FAppFixtureState> ApiOptions;
  ApiOptions.reducerPath = ApiFixture.ReducerPath;
  ApiOptions.tagTypes.Add(ApiFixture.TagType);
  ApiOptions.endpoints = [](EndpointBuilder<FAppFixtureState> &EndpointBuilder) {
    (void)EndpointBuilder;
  };
  Api<FAppFixtureState> TestApi = buildCreateApi<FAppFixtureState>()(ApiOptions);
  TestEqual(Fixture.Labels.ApiReducerPath, TestApi.ReducerPath,
            ApiFixture.ReducerPath);
  TestEqual(Fixture.Labels.ApiTagTypes, TestApi.TagTypes.Num(),
            ApiOptions.tagTypes.Num());

  Api<FAppFixtureState> &InjectedApi =
      injectEndpoints(TestApi, BuiltGetUserEndpoint);
  const TArray<FString> ExpectedEndpointNames = {
      BuiltGetUserEndpoint.EndpointName};
  TestTrue(Fixture.Labels.ApiIdentity, &InjectedApi == &TestApi);
  TestEqual(Fixture.Labels.ApiEndpointCount, TestApi.Endpoints.Num(),
            ExpectedEndpointNames.Num());
  TestTrue(Fixture.Labels.ApiEndpointLookup,
           TestApi.Endpoints.Contains(BuiltGetUserEndpoint.EndpointName));
  const FApiEndpointMetadata &RegisteredEndpoint =
      TestApi.Endpoints.FindChecked(BuiltGetUserEndpoint.EndpointName);
  TestTrue(Fixture.Labels.ApiDefinitionType,
           RegisteredEndpoint.Type == DefinitionType::mutation);
  TestEqual(Fixture.Labels.ApiProvidesTags,
            RegisteredEndpoint.providesTags.Num(),
            BuiltGetUserEndpoint.providesTags.Num());
  const int32 RegisteredEndpointCount = TestApi.Endpoints.Num();
  injectEndpoints(TestApi, BuiltGetUserEndpoint);
  TestEqual(Fixture.Labels.ApiIdempotence, TestApi.Endpoints.Num(),
            RegisteredEndpointCount);

  auto GetUserThunk = initiate(TestApi, BuiltGetUserEndpoint);

  TArray<FString> EventLog;
  std::function<AnyAction(const AnyAction &)> RecordDispatch =
      [&EventLog](const AnyAction &Action) {
        EventLog.Add(Action.Type);
        return Action;
      };
  const FAppFixtureState State{
      FNpcFixtureState{ApiFixture.SuccessArgument, ApiFixture.EndpointResult}};
  std::function<const FAppFixtureState &()> ReadState =
      [&State]() -> const FAppFixtureState & {
    return State;
  };

  /**
   * 3. Test Successful HTTP Call
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  auto SuccessOp = GetUserThunk(ApiFixture.SuccessArgument);
  SuccessOp(RecordDispatch, ReadState).execute();

  TestEqual(Fixture.Labels.ApiSuccessPending,
            EventLog[ApiFixture.PendingEventIndex],
            ApiFixture.PendingActionType);
  TestEqual(Fixture.Labels.ApiSuccessFulfilled,
            EventLog[ApiFixture.TerminalEventIndex],
            ApiFixture.FulfilledActionType);
  EventLog.Empty();

  /**
   * 4. Test Failed HTTP Call
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  auto FailOp = GetUserThunk(ApiFixture.FailureArgument);
  FailOp(RecordDispatch, ReadState).execute();

  TestEqual(Fixture.Labels.ApiFailurePending,
            EventLog[ApiFixture.PendingEventIndex],
            ApiFixture.PendingActionType);
  TestEqual(Fixture.Labels.ApiFailureRejected,
            EventLog[ApiFixture.TerminalEventIndex],
            ApiFixture.RejectedActionType);

  FString HttpErrorMessage;
  unwrapEndpointResult<int32>(QueryReturnValue<int32>::failure(
      FetchBaseQueryError::httpError(ApiFixture.HttpErrorStatus,
                                     ApiFixture.HttpErrorBody)))
      .catch_([&HttpErrorMessage](std::string Error) {
        HttpErrorMessage = UTF8_TO_TCHAR(Error.c_str());
      })
      .execute();
  TestEqual(Fixture.Labels.ApiHttpErrorBody, HttpErrorMessage,
            ApiFixture.HttpErrorBody);

  SerializeQueryArgsOptions SerializeOptions;
  SerializeOptions.endpointName = ApiFixture.EndpointName;
  SerializeOptions.queryArgs = ApiFixture.SuccessArgument;
  TestEqual(Fixture.Labels.ApiSerializedArgs,
            defaultSerializeQueryArgs(SerializeOptions),
            ApiFixture.SerializedQueryKey);

  int32 RetryAttempts = ApiFixture.InitialRetryAttempts;
  BaseQueryFn<rtk::FEmptyPayload, int32> FlakyBaseQuery =
      [&RetryAttempts, &ApiFixture](const rtk::FEmptyPayload &,
                                    const BaseQueryApi &,
                                    const rtk::FEmptyPayload &) {
        return func::AsyncResult<QueryReturnValue<int32>>::create(
            [&RetryAttempts, &ApiFixture](
                std::function<void(QueryReturnValue<int32>)> Resolve,
                std::function<void(std::string)> Reject) {
              (void)Reject;
              ++RetryAttempts == ApiFixture.FirstFailureAttempt
                  ? Resolve(QueryReturnValue<int32>::failure(
                        FetchBaseQueryError::fetchError(
                            ApiFixture.RetryFailureMessage)))
                  : Resolve(QueryReturnValue<int32>::success(
                        ApiFixture.RetryResult));
            });
      };
  RetryOptions RetryConfig;
  RetryConfig.maxRetries = ApiFixture.MaxRetries;
  int32 RetryValue = ApiFixture.InitialRetryValue;
  retry<rtk::FEmptyPayload, int32>(FlakyBaseQuery, RetryConfig)(
      rtk::FEmptyPayload{}, BaseQueryApi(), rtk::FEmptyPayload{})
      .then([&RetryValue, &ApiFixture](QueryReturnValue<int32> Value) {
        RetryValue = Value.data.hasValue ? Value.data.value
                                         : ApiFixture.MissingRetryValue;
      })
      .execute();
  TestEqual(Fixture.Labels.ApiRetryAttempts, RetryAttempts,
            ApiFixture.ExpectedRetryAttempts);
  TestEqual(Fixture.Labels.ApiRetryResult, RetryValue,
            ApiFixture.RetryResult);

  return true;
}
