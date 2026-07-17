#include "ApiEndpointTestAdapters.h"

#include "Features/Config/ConfigSelectors.h"
#include "HAL/PlatformProcess.h"
#include "Store.h"

namespace ApiEndpointTests {

/** User Story: As a tests integration api consumer, I need to invoke base query options through a stable signature so the tests integration api workflow remains explicit and composable. @fn rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey) */
rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey) {
  rtk::FetchBaseQueryArgs Options;
  !ApiKey.IsEmpty()
      ? (Options.headers.Add(TEXT("Authorization"),
                             FString(TEXT("Bearer ")) + ApiKey),
         void())
      : void();
  return Options;
}

/** User Story: As a tests integration api consumer, I need to invoke fetch args through a stable signature so the tests integration api workflow remains explicit and composable. @fn rtk::FetchArgs FetchArgs(const FString &Method, const FString &Url) */
rtk::FetchArgs FetchArgs(const FString &Method, const FString &Url) {
  rtk::FetchArgs Args;
  Args.method = Method;
  Args.url = Url;
  return Args;
}

/** User Story: As a tests integration api consumer, I need to invoke status code through a stable signature so the tests integration api workflow remains explicit and composable. @fn int32 StatusCode(const rtk::QueryReturnValue<FString> &Result) */
int32 StatusCode(const rtk::QueryReturnValue<FString> &Result) {
  return Result.meta.hasValue && Result.meta.value.response.hasValue
             ? Result.meta.value.response.value.status
             : 0;
}

/** User Story: As a tests integration api consumer, I need to invoke body through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString Body(const rtk::QueryReturnValue<FString> &Result) */
FString Body(const rtk::QueryReturnValue<FString> &Result) {
  return Result.data.hasValue ? Result.data.value : TEXT("");
}

/** User Story: As a tests integration api consumer, I need to invoke error message through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString ErrorMessage(const rtk::QueryReturnValue<FString> &Result) */
FString ErrorMessage(const rtk::QueryReturnValue<FString> &Result) {
  return Result.error.hasValue
             ? (!Result.error.value.error.IsEmpty() ? Result.error.value.error
                                                    : Result.error.value.status)
             : TEXT("");
}

/** User Story: As a tests integration api consumer, I need to invoke base url through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString BaseUrl() */
FString BaseUrl() {
  return ConfigSelectors::selectApiUrl(store().getState());
}

/** User Story: As a tests integration api consumer, I need to invoke required api key through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString RequiredApiKey(FAutomationTestBase &Test) */
FString RequiredApiKey(FAutomationTestBase &Test) {
  const FString ApiKey =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_KEY"));
  ApiKey.IsEmpty()
      ? Test.AddError(
            TEXT("FORBOCAI_API_KEY is required for live endpoint tests"))
      : void();
  return ApiKey;
}

} // namespace ApiEndpointTests

/** User Story: As a tests integration api consumer, I need to invoke update through a stable signature so the tests integration api workflow remains explicit and composable. @fn bool FHttpGetWaitComplete::Update() */
bool FHttpGetWaitComplete::Update() {
  if (!State->bStarted) {
    const TSharedPtr<FApiEndpointTestState> SharedState = State;
    State->bStarted = true;
    rtk::fetchBaseQuery<FString>(ApiEndpointTests::BaseQueryOptions(ApiKey))(
        ApiEndpointTests::FetchArgs(TEXT("GET"), Url), rtk::BaseQueryApi(),
        rtk::FEmptyPayload{})
        .then([SharedState](const rtk::QueryReturnValue<FString> &Result) {
          SharedState->bDone = true;
          SharedState->bSuccess = Result.data.hasValue;
          SharedState->HttpCode = ApiEndpointTests::StatusCode(Result);
          Result.data.hasValue
              ? (SharedState->Body = ApiEndpointTests::Body(Result), void())
              : (SharedState->Error =
                     ApiEndpointTests::ErrorMessage(Result),
                 void());
        })
        .execute();
    return false;
  }
  if (State->bDone) {
    return true;
  }
  FPlatformProcess::Sleep(0.05f);
  return false;
}
