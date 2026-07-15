#include "ApiEndpointTestAdapters.h"

#include "Features/Config/ConfigAdapters.h"
#include "Features/API/Endpoints/Arweave/ArweaveApi.h"
#include "Store.h"

// @covers:api:getApiStatus
// @covers:api:postArweaveUpload
// @covers:api:postArweaveDownload

namespace {

void TestArweaveMissingUrls(FAutomationTestBase &Test) {
  rtk::EnhancedStore<FRuntimeState> Store = createRuntimeStore();
  bool bUploadResolved = false;
  bool bUploadRejected = false;
  bool bDownloadResolved = false;
  bool bDownloadRejected = false;
  FString UploadError;
  FString DownloadError;
  FArweaveUploadInstruction UploadInstruction;
  FArweaveDownloadInstruction DownloadInstruction;

  Store.dispatch(APISlice::Endpoints::postArweaveUpload(
                     UploadInstruction, FString()))
      .then([&bUploadResolved](const FArweaveUploadResult &) {
        bUploadResolved = true;
      })
      .catch_([&bUploadRejected, &UploadError](std::string Message) {
        bUploadRejected = true;
        UploadError = UTF8_TO_TCHAR(Message.c_str());
      })
      .execute();
  Store.dispatch(
           APISlice::Endpoints::postArweaveDownload(DownloadInstruction))
      .then([&bDownloadResolved](const FArweaveDownloadResult &) {
        bDownloadResolved = true;
      })
      .catch_([&bDownloadRejected, &DownloadError](std::string Message) {
        bDownloadRejected = true;
        DownloadError = UTF8_TO_TCHAR(Message.c_str());
      })
      .execute();

  const FString Label(ANSI_TO_TCHAR(__FUNCTION__));
  Test.TestFalse(Label, bUploadResolved);
  Test.TestTrue(Label, bUploadRejected && !UploadError.IsEmpty());
  Test.TestFalse(Label, bDownloadResolved);
  Test.TestTrue(Label, bDownloadRejected && !DownloadError.IsEmpty());
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointStatusNoAuthTest,
    "ForbocAI.Integration.API.Endpoint.StatusNoAuth",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

bool FApiEndpointStatusNoAuthTest::RunTest(const FString &Parameters) {
  TestArweaveMissingUrls(*this);
  if (ApiEndpointTests::ShouldSkip()) {
    return true;
  }
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

bool FApiEndpointNotFoundTest::RunTest(const FString &Parameters) {
  if (ApiEndpointTests::ShouldSkip()) {
    return true;
  }
  SDKConfig::SetApiConfig(SDKConfig::GetApiUrl(), TEXT(""));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT("/nonexistent-path-404"), TEXT(""),
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
