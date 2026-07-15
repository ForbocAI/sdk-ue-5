#pragma once

#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

struct FApiEndpointTestState {
  bool bDone = false;
  bool bSuccess = false;
  int32 HttpCode = 0;
  FString Body;
  FString Error;
  bool bStarted = false;
};

namespace ApiEndpointTests {

rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey);
rtk::FetchArgs FetchArgs(const FString &Method, const FString &Url);
int32 StatusCode(const rtk::QueryReturnValue<FString> &Result);
FString Body(const rtk::QueryReturnValue<FString> &Result);
FString ErrorMessage(const rtk::QueryReturnValue<FString> &Result);
FString BaseUrl();
bool ShouldSkip();

} // namespace ApiEndpointTests

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
    FHttpGetWaitComplete, FString, Url, FString, ApiKey,
    TSharedPtr<FApiEndpointTestState>, State);
