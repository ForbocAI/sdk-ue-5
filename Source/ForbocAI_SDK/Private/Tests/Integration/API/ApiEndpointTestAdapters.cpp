#include "ApiEndpointTestAdapters.h"

#include "Features/Config/ConfigAdapters.h"
#include "HAL/PlatformProcess.h"

namespace ApiEndpointTests {

rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey) {
  rtk::FetchBaseQueryArgs Options;
  !ApiKey.IsEmpty()
      ? (Options.headers.Add(TEXT("Authorization"),
                             FString(TEXT("Bearer ")) + ApiKey),
         void())
      : void();
  return Options;
}

rtk::FetchArgs FetchArgs(const FString &Method, const FString &Url) {
  rtk::FetchArgs Args;
  Args.method = Method;
  Args.url = Url;
  return Args;
}

int32 StatusCode(const rtk::QueryReturnValue<FString> &Result) {
  return Result.meta.hasValue && Result.meta.value.response.hasValue
             ? Result.meta.value.response.value.status
             : 0;
}

FString Body(const rtk::QueryReturnValue<FString> &Result) {
  return Result.data.hasValue ? Result.data.value : TEXT("");
}

FString ErrorMessage(const rtk::QueryReturnValue<FString> &Result) {
  return Result.error.hasValue
             ? (!Result.error.value.error.IsEmpty() ? Result.error.value.error
                                                    : Result.error.value.status)
             : TEXT("");
}

FString BaseUrl() { return SDKConfig::GetApiUrl(); }

bool ShouldSkip() {
  return FPlatformMisc::GetEnvironmentVariable(
             TEXT("FORBOC_RUN_API_ENDPOINT_TESTS"))
             .IsEmpty()
         ? []() {
             UE_LOG(LogTemp, Display,
                    TEXT("Skipping API endpoint integration tests until API work resumes."));
             return true;
           }()
         : false;
}

} // namespace ApiEndpointTests

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
