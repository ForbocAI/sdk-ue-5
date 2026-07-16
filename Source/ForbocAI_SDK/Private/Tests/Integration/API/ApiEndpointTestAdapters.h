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

/** User Story: As a tests integration api consumer, I need to invoke base query options through a stable signature so the tests integration api workflow remains explicit and composable. @fn rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey) */
rtk::FetchBaseQueryArgs BaseQueryOptions(const FString &ApiKey);
/** User Story: As a tests integration api consumer, I need to invoke fetch args through a stable signature so the tests integration api workflow remains explicit and composable. @fn rtk::FetchArgs FetchArgs(const FString &Method, const FString &Url) */
rtk::FetchArgs FetchArgs(const FString &Method, const FString &Url);
/** User Story: As a tests integration api consumer, I need to invoke status code through a stable signature so the tests integration api workflow remains explicit and composable. @fn int32 StatusCode(const rtk::QueryReturnValue<FString> &Result) */
int32 StatusCode(const rtk::QueryReturnValue<FString> &Result);
/** User Story: As a tests integration api consumer, I need to invoke body through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString Body(const rtk::QueryReturnValue<FString> &Result) */
FString Body(const rtk::QueryReturnValue<FString> &Result);
/** User Story: As a tests integration api consumer, I need to invoke error message through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString ErrorMessage(const rtk::QueryReturnValue<FString> &Result) */
FString ErrorMessage(const rtk::QueryReturnValue<FString> &Result);
/** User Story: As a tests integration api consumer, I need to invoke base url through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString BaseUrl() */
FString BaseUrl();
/** User Story: As a tests integration api consumer, I need to invoke required api key through a stable signature so the tests integration api workflow remains explicit and composable. @fn FString RequiredApiKey(FAutomationTestBase &Test) */
FString RequiredApiKey(FAutomationTestBase &Test);

} // namespace ApiEndpointTests

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
    FHttpGetWaitComplete, FString, Url, FString, ApiKey,
    TSharedPtr<FApiEndpointTestState>, State);
