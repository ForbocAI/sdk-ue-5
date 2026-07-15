#include "ApiEndpointTestAdapters.h"

#include "Features/Config/ConfigAdapters.h"
#include "Features/Soul/SoulTypes.h"
#include "JsonObjectConverter.h"

// @covers:api:deleteMemoryClear
// @covers:api:deleteRule
// @covers:api:getGhostHistory
// @covers:api:getGhostResults
// @covers:api:getGhostStatus
// @covers:api:getNPC
// @covers:api:getSoulImport
// @covers:api:getSouls
// @covers:api:postBridgePreset
// @covers:api:postBridgeValidate
// @covers:api:postGhostRun
// @covers:api:postGhostStop
// @covers:api:postMemoryStore
// @covers:api:postNPC
// @covers:api:postNpcImport
// @covers:api:postNpcImportConfirm
// @covers:api:postNpcProcess
// @covers:api:postRuleRegister
// @covers:api:postSoulExport
// @covers:api:postSoulExportConfirm
// @covers:api:postSoulVerify

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointSoulsValidKeyTest,
    "ForbocAI.Integration.API.Endpoint.SoulsValidKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FApiEndpointSoulsValidKeyTest::RunTest(const FString &Parameters) {
  if (ApiEndpointTests::ShouldSkip()) {
    return true;
  }
  const FString Key =
      FPlatformMisc::GetEnvironmentVariable(TEXT("FORBOCAI_API_KEY"));
  if (Key.IsEmpty()) {
    AddInfo(TEXT("Skip: FORBOCAI_API_KEY not set"));
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), Key);

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT("/souls?limit=10"), Key, State));
  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone) {
          return;
        }
        TestTrue("getSouls succeeded", State->bSuccess);
        TestEqual("Returns 200", State->HttpCode, 200);
        if (State->bSuccess && State->Body.Len() > 0) {
          FSoulListResponse Decoded;
          TestTrue("Response parses as FSoulListResponse",
                   FJsonObjectConverter::JsonObjectStringToUStruct(
                       State->Body, &Decoded, 0, 0));
        }
      },
      0.01f));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointSoulsNoKeyTest,
    "ForbocAI.Integration.API.Endpoint.SoulsNoKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FApiEndpointSoulsNoKeyTest::RunTest(const FString &Parameters) {
  if (ApiEndpointTests::ShouldSkip()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), TEXT(""));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT("/souls?limit=10"), TEXT(""), State));
  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone) {
          return;
        }
        TestFalse("getSouls without key fails", State->bSuccess);
        TestEqual("Returns 401", State->HttpCode, 401);
      },
      0.01f));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointSoulsInvalidKeyTest,
    "ForbocAI.Integration.API.Endpoint.SoulsInvalidKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FApiEndpointSoulsInvalidKeyTest::RunTest(const FString &Parameters) {
  if (ApiEndpointTests::ShouldSkip()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(),
                          TEXT("invalid_key_12345"));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT("/souls?limit=10"),
      TEXT("invalid_key_12345"), State));
  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone) {
          return;
        }
        TestFalse("getSouls with invalid key fails", State->bSuccess);
        TestEqual("Returns 401", State->HttpCode, 401);
      },
      0.01f));
  return true;
}
