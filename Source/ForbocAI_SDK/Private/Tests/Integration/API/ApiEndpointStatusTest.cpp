#include "ApiEndpointTestAdapters.h"

#include "Features/API/APIApi.h"
#include "Features/Config/ConfigAdapters.h"

// @covers:api:getApiStatus
// @covers:api:postNpcProcess
// @covers:api:getBridgeValidation
// @covers:api:getBridgeRules
// @covers:api:postGhostRun
// @covers:api:getGhostStatus
// @covers:api:getGhostResults
// @covers:api:postGhostStop
// @covers:api:getGhostHistory
// @covers:api:postBridgePreset
// @covers:api:getRulesets
// @covers:api:getRulePresets
// @covers:api:postSoulExportPreparation
// @covers:api:postSoulExportConfirmation
// @covers:api:postSoulVerification
// @covers:api:postSoulStorageUpload
// @covers:api:getSoulStorageDownload
// @covers:api:getSoulStorageVerification
// @covers:api:postSoulStoragePreparation
// @covers:api:deleteSoulStoragePreparation
// @covers:api:postSoulStorageCommit
// @covers:api:getSoulStorageCatalog
// @covers:api:getSoulStorageEntry
// @covers:coreThunk:MakePostQueryWithCodec

namespace {

/**
 * User Story: As an SDK maintainer, I need every authored endpoint builder to
 * produce an executable RTK thunk so declarations cannot satisfy coverage
 * without real endpoint construction.
 * @fn template <typename EndpointThunk> void TestEndpointConstructed(FAutomationTestBase &Test, const FString &EndpointName, const EndpointThunk &Thunk)
 */
template <typename EndpointThunk>
void TestEndpointConstructed(FAutomationTestBase &Test,
                             const FString &EndpointName,
                             const EndpointThunk &Thunk) {
  Test.TestTrue(EndpointName + TEXT(" constructs an RTK Query thunk"),
                static_cast<bool>(Thunk));
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointConstructionTest,
    "ForbocAI.Integration.API.Endpoint.Construction",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As an SDK maintainer, I need one executable inventory test for
 * every package-root RTK Query endpoint so endpoint data and builders remain
 * synchronized across TS and UE.
 * @fn bool FApiEndpointConstructionTest::RunTest(const FString &Parameters)
 */
bool FApiEndpointConstructionTest::RunTest(const FString &Parameters) {
  using namespace APISlice::Endpoints;
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const FString Empty;

  TestEndpointConstructed(*this, Data.Names.GetApiStatus, getApiStatus());
  TestEndpointConstructed(*this, Data.Names.PostNpcProcess,
                          postNpcProcess(Empty, FNPCProcessRequest{}));
  TestEndpointConstructed(*this, Data.Names.GetBridgeValidation,
                          getBridgeValidation(Empty,
                                              FBridgeValidateRequest{}));
  TestEndpointConstructed(*this, Data.Names.GetBridgeRules,
                          getBridgeRules());
  TestEndpointConstructed(*this, Data.Names.PostGhostRun,
                          postGhostRun(FGhostRunRequest{}));
  TestEndpointConstructed(*this, Data.Names.GetGhostStatus,
                          getGhostStatus(Empty));
  TestEndpointConstructed(*this, Data.Names.GetGhostResults,
                          getGhostResults(Empty));
  TestEndpointConstructed(*this, Data.Names.PostGhostStop,
                          postGhostStop(Empty));
  TestEndpointConstructed(*this, Data.Names.GetGhostHistory,
                          getGhostHistory(Data.Defaults.GhostHistoryLimit));
  TestEndpointConstructed(*this, Data.Names.PostBridgePreset,
                          postBridgePreset(Empty));
  TestEndpointConstructed(*this, Data.Names.GetRulesets, getRulesets());
  TestEndpointConstructed(*this, Data.Names.GetRulePresets,
                          getRulePresets());
  TestEndpointConstructed(*this, Data.Names.PostSoulExportPreparation,
                          postSoulExportPreparation(
                              Empty, FSoulExportPreparationRequest{}));
  TestEndpointConstructed(*this, Data.Names.PostSoulExportConfirmation,
                          postSoulExportConfirmation(
                              Empty, FSoulExportConfirmationRequest{}));
  TestEndpointConstructed(*this, Data.Names.PostSoulVerification,
                          postSoulVerification(
                              Empty, FSoulVerificationRequest{}));
  TestEndpointConstructed(*this, Data.Names.PostSoulStorageUpload,
                          postSoulStorageUpload(Empty));
  TestEndpointConstructed(*this, Data.Names.GetSoulStorageDownload,
                          getSoulStorageDownload(Empty));
  TestEndpointConstructed(*this, Data.Names.GetSoulStorageVerification,
                          getSoulStorageVerification(Empty));
  TestEndpointConstructed(*this, Data.Names.PostSoulStoragePreparation,
                          postSoulStoragePreparation(FSoul{}));
  TestEndpointConstructed(*this, Data.Names.DeleteSoulStoragePreparation,
                          deleteSoulStoragePreparation(Empty));
  TestEndpointConstructed(*this, Data.Names.PostSoulStorageCommit,
                          postSoulStorageCommit(FSoulStorageCommit{}));
  TestEndpointConstructed(*this, Data.Names.GetSoulStorageCatalog,
                          getSoulStorageCatalog(Data.Defaults.SoulListLimit));
  TestEndpointConstructed(*this, Data.Names.GetSoulStorageEntry,
                          getSoulStorageEntry(Empty));

  return true;
}

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
