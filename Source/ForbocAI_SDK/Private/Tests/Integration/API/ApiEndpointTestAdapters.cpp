#include "ApiEndpointTestAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Entities/Config/ConfigSelectors.h"
#include "HAL/PlatformProcess.h"
#include "Store.h"

namespace ApiEndpointTests {

/** User Story: As a tests integration api consumer, I need to invoke base query options through a stable signature so the tests integration api workflow remains explicit and composable. @fn rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey) */
rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey) {
  rtk::FetchBaseQueryArgs Options;
  !ApiKey.IsEmpty()
      ? (Options.headers.Add(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV4CB3798C30EF),
                             FString(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV8F076706E1D9)) + ApiKey),
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
             : FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
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
      FPlatformMisc::GetEnvironmentVariable(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5F0C966E7042));
  ApiKey.IsEmpty()
      ? Test.AddError(
            TEXT(FORBOCAI_SDK_AUTHORED_STRINGV63C10FC07058))
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
        ApiEndpointTests::FetchArgs(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9FB15D43C73B), Url), rtk::BaseQueryApi(),
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
  FPlatformProcess::Sleep(FORBOCAI_SDK_AUTHORED_NUMBERV4B582E8E76C5);
  return false;
}
