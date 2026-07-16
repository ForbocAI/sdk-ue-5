#include "ApiEndpointTestAdapters.h"

#include "Features/Config/ConfigAdapters.h"

// @covers:api:getApiStatus

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointStatusNoAuthTest,
    "ForbocAI.Integration.API.Endpoint.StatusNoAuth",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests integration api consumer, I need to invoke run test through a stable signature so the tests integration api workflow remains explicit and composable. @fn bool FApiEndpointStatusNoAuthTest::RunTest(const FString &Parameters) */
bool FApiEndpointStatusNoAuthTest::RunTest(const FString &Parameters) {
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), TEXT(""));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT("/status"), TEXT(""), State));
  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone) {
          return;
        }
        TestTrue("Status endpoint succeeded", State->bSuccess);
        TestEqual("Status returns 200", State->HttpCode, 200);
        if (State->bSuccess) {
          TestTrue("Response body non-empty", State->Body.Len() > 0);
        }
      },
      0.01f));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointNotFoundTest, "ForbocAI.Integration.API.Endpoint.NotFound",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests integration api consumer, I need to invoke run test through a stable signature so the tests integration api workflow remains explicit and composable. @fn bool FApiEndpointNotFoundTest::RunTest(const FString &Parameters) */
bool FApiEndpointNotFoundTest::RunTest(const FString &Parameters) {
  const FString Key = ApiEndpointTests::RequiredApiKey(*this);
  if (Key.IsEmpty()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), Key);

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT("/nonexistent-path-404"), Key,
      State));
  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone) {
          return;
        }
        TestFalse("404 path fails", State->bSuccess);
        TestEqual("Returns 404", State->HttpCode, 404);
      },
      0.01f));
  return true;
}
