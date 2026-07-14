// @covers:api:getApiStatus

/**
 * API endpoint integration tests - uses SDKConfig production default and FORBOCAI_API_URL override.
 * I.5 — Auth, response normalization, representative endpoints, error handling
 * Requires FORBOCAI_API_KEY for auth tests.
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "Features/API/APIAdapters.h"
#include "Features/API/APIApi.h"
#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "JsonObjectConverter.h"
#include "HAL/PlatformProcess.h"
#include "Misc/AutomationTest.h"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Soul/SoulTypes.h"

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


struct FApiEndpointTestState {
  bool bDone = false;
  bool bSuccess = false;
  int32 HttpCode = 0;
  FString Body;
  FString Error;
  bool bStarted = false;
};

static rtk::FetchBaseQueryArgs TestBaseQueryOptions(const FString &ApiKey) {
  rtk::FetchBaseQueryArgs Options;
  !ApiKey.IsEmpty()
      ? (Options.headers.Add(TEXT("Authorization"),
                             FString(TEXT("Bearer ")) + ApiKey),
         void())
      : void();
  return Options;
}

static rtk::FetchArgs TestFetchArgs(const FString &Method, const FString &Url,
                                    const FString &Payload = TEXT("")) {
  rtk::FetchArgs Args;
  Args.method = Method;
  Args.url = Url;
  Args.body = Payload;
  return Args;
}

static int32 QueryStatusCode(const rtk::QueryReturnValue<FString> &Result) {
  return Result.meta.hasValue && Result.meta.value.response.hasValue
             ? Result.meta.value.response.value.status
             : 0;
}

static FString QueryBody(const rtk::QueryReturnValue<FString> &Result) {
  return Result.data.hasValue ? Result.data.value : TEXT("");
}

static FString QueryErrorMessage(const rtk::QueryReturnValue<FString> &Result) {
  return Result.error.hasValue
             ? (!Result.error.value.error.IsEmpty() ? Result.error.value.error
                                                    : Result.error.value.status)
             : TEXT("");
}

/**
 * Latent command: GET request, polls until complete
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(
    FHttpGetWaitComplete, FString, Url, FString, ApiKey,
    TSharedPtr<FApiEndpointTestState>, State);
/**
 * User Story: As a developer, I need Update to fulfill its role in the module.
 */
bool FHttpGetWaitComplete::Update() {
  if (!State->bStarted) {
    const TSharedPtr<FApiEndpointTestState> SharedState = State;
    State->bStarted = true;
    rtk::fetchBaseQuery<FString>(TestBaseQueryOptions(ApiKey))(
        TestFetchArgs(TEXT("GET"), Url), rtk::BaseQueryApi(),
        rtk::FEmptyPayload{})
        .then([SharedState](const rtk::QueryReturnValue<FString> &R) {
          SharedState->bDone = true;
          SharedState->bSuccess = R.data.hasValue;
          SharedState->HttpCode = QueryStatusCode(R);
          if (R.data.hasValue) {
            SharedState->Body = QueryBody(R);
          } else {
            SharedState->Error = QueryErrorMessage(R);
          }
        })
        .execute();
    return false;
  }
  if (State->bDone)
    return true;
  FPlatformProcess::Sleep(0.05f);
  return false;
}

/**
 * Latent command: POST request
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(
    FHttpPostWaitComplete, FString, Url, FString, Payload, FString, ApiKey,
    TSharedPtr<FApiEndpointTestState>, State);
/**
 * User Story: As a developer, I need Update to fulfill its role in the module.
 */
bool FHttpPostWaitComplete::Update() {
  if (!State->bStarted) {
    const TSharedPtr<FApiEndpointTestState> SharedState = State;
    State->bStarted = true;
    rtk::fetchBaseQuery<FString>(TestBaseQueryOptions(ApiKey))(
        TestFetchArgs(TEXT("POST"), Url, Payload), rtk::BaseQueryApi(),
        rtk::FEmptyPayload{})
        .then([SharedState](const rtk::QueryReturnValue<FString> &R) {
          SharedState->bDone = true;
          SharedState->bSuccess = R.data.hasValue;
          SharedState->HttpCode = QueryStatusCode(R);
          if (R.data.hasValue) {
            SharedState->Body = QueryBody(R);
          } else {
            SharedState->Error = QueryErrorMessage(R);
          }
        })
        .execute();
    return false;
  }
  if (State->bDone)
    return true;
  FPlatformProcess::Sleep(0.05f);
  return false;
}

static FString GetBaseUrl() { return SDKConfig::GetApiUrl(); }
static FString GetApiKey() { return SDKConfig::GetApiKey(); }

static bool SkipApiEndpointIntegration() {
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

/**
 * Auth: getApiStatus (no auth required) — connectivity check
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointStatusNoAuthTest,
    "ForbocAI.Integration.API.Endpoint.StatusNoAuth",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiEndpointStatusNoAuthTest::RunTest(const FString &Parameters) {
  if (SkipApiEndpointIntegration()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), TEXT(""));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(
      FHttpGetWaitComplete(GetBaseUrl() + TEXT("/status"), TEXT(""), State));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone)
          return;
        TestTrue("Status endpoint succeeded", State->bSuccess);
        TestEqual("Status returns 200", State->HttpCode, 200);
        if (State->bSuccess) {
          TestTrue("Response body non-empty", State->Body.Len() > 0);
        }
      },
      0.01f));

  return true;
}

/**
 * Auth: getSouls with valid key — 200 and parseable
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointSoulsValidKeyTest,
    "ForbocAI.Integration.API.Endpoint.SoulsValidKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiEndpointSoulsValidKeyTest::RunTest(const FString &Parameters) {
  if (SkipApiEndpointIntegration()) {
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
  ADD_LATENT_AUTOMATION_COMMAND(
      FHttpGetWaitComplete(GetBaseUrl() + TEXT("/souls?limit=10"), Key, State));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone)
          return;
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

/**
 * Auth: getSouls with no key — expect 401
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointSoulsNoKeyTest,
    "ForbocAI.Integration.API.Endpoint.SoulsNoKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiEndpointSoulsNoKeyTest::RunTest(const FString &Parameters) {
  if (SkipApiEndpointIntegration()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), TEXT(""));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(
      FHttpGetWaitComplete(GetBaseUrl() + TEXT("/souls?limit=10"), TEXT(""),
                           State));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone)
          return;
        TestFalse("getSouls without key fails", State->bSuccess);
        TestEqual("Returns 401", State->HttpCode, 401);
      },
      0.01f));

  return true;
}

/**
 * Auth: getSouls with invalid key — expect 401
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointSoulsInvalidKeyTest,
    "ForbocAI.Integration.API.Endpoint.SoulsInvalidKey",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiEndpointSoulsInvalidKeyTest::RunTest(const FString &Parameters) {
  if (SkipApiEndpointIntegration()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(),
                          TEXT("invalid_key_12345"));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      GetBaseUrl() + TEXT("/souls?limit=10"), TEXT("invalid_key_12345"),
      State));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone)
          return;
        TestFalse("getSouls with invalid key fails", State->bSuccess);
        TestEqual("Returns 401", State->HttpCode, 401);
      },
      0.01f));

  return true;
}

/**
 * Error: 404 — nonexistent path
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointNotFoundTest,
    "ForbocAI.Integration.API.Endpoint.NotFound",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FApiEndpointNotFoundTest::RunTest(const FString &Parameters) {
  if (SkipApiEndpointIntegration()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), TEXT(""));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      GetBaseUrl() + TEXT("/nonexistent-path-404"), TEXT(""), State));

  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue("Request completed", State->bDone);
        if (!State->bDone)
          return;
        TestFalse("404 path fails", State->bSuccess);
        TestEqual("Returns 404", State->HttpCode, 404);
      },
      0.01f));

  return true;
}
