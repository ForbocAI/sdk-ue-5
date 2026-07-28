#include "ApiEndpointTestAdapters.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Systems/API/APIApi.h"
#include "Systems/API/APIThunks.h"
#include "Systems/CLI/Config/ConfigThunks.h"
#include "Entities/Config/ConfigSelectors.h"
#include "Store.h"

// @covers:api:getApiStatus
// @covers:api:getMicroGameContract
// @covers:api:postNpcProcess
// @covers:api:postNpcGenerateAttribute
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
// @covers:coreThunk:MakePostNoBodyWithCodec
// @covers:coreThunk:getMicroGameContractThunk
// @covers:cliOp:commitApiConfiguration

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
  Test.TestTrue(EndpointName + TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD44F9D90D1ED),
                static_cast<bool>(Thunk));
}

} // namespace

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointConstructionTest,
    FORBOCAI_SDK_AUTHORED_STRINGV68306F30B4F8,
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
  APISlice::api.Endpoints.Empty();
  const Configuration::FEndpointConfigurationData &Data =
      Configuration::endpointData();
  const FString Empty;

  TestEndpointConstructed(*this, Data.Names.GetApiStatus, getApiStatus());
  TestEndpointConstructed(*this, Data.Names.GetMicroGameContract,
                          getMicroGameContract());
  TestEndpointConstructed(*this, Data.Names.GetMicroGameContract,
                          rtk::getMicroGameContractThunk());
  TestEndpointConstructed(*this, Data.Names.PostNpcProcess,
                          postNpcProcess(Empty, FNPCProcessRequest{}));
  TestEndpointConstructed(*this, Data.Names.PostNpcGenerateAttribute,
                          postNpcGenerateAttribute(TEXT("role"),
                                                   FNpcAttributeGenerateRequest{}));
  TestTrue(Data.Names.PostNpcProcess,
           Data.Timeouts.NpcProcessMs >
               APISlice::Transport::transportQueryData().Timeouts.Disabled);
  TestTrue(Data.Names.PostNpcGenerateAttribute,
           Data.Timeouts.NpcGenerateMs >
               APISlice::Transport::transportQueryData().Timeouts.Disabled);
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

  const TArray<FString> AuthoredNames =
      Configuration::endpointNames(Data.Names);
  TestEqual(Data.Names.GetApiStatus,
            APISlice::api.Endpoints.Num(), AuthoredNames.Num());
  for (const FString &EndpointName : AuthoredNames) {
    TestTrue(EndpointName, APISlice::api.Endpoints.Contains(EndpointName));
  }

  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointStatusNoAuthTest,
    FORBOCAI_SDK_AUTHORED_STRINGV6C51C1676B84,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests integration api consumer, I need to invoke run test through a stable signature so the tests integration api workflow remains explicit and composable. @fn bool FApiEndpointStatusNoAuthTest::RunTest(const FString &Parameters) */
bool FApiEndpointStatusNoAuthTest::RunTest(const FString &Parameters) {
  Ops::commitApiConfiguration(
      store(), ConfigSelectors::selectApiUrl(store().getState()), TEXT(""));

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT(FORBOCAI_SDK_AUTHORED_STRINGV640F4A43A4A8), TEXT(""), State));
  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV1068614E082D, State->bDone);
        if (!State->bDone) {
          return;
        }
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV60FF726E854A, State->bSuccess);
        TestEqual(FORBOCAI_SDK_AUTHORED_STRINGVD96BA6A6D90C, State->HttpCode, FORBOCAI_SDK_AUTHORED_NUMBERVEF1B5401B507);
        if (State->bSuccess) {
          TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV6F50C396470E, State->Body.Len() > FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA);
        }
      },
      FORBOCAI_SDK_AUTHORED_NUMBERVEC53E6A2E194));
  return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FApiEndpointNotFoundTest, FORBOCAI_SDK_AUTHORED_STRINGVFC39450631A0,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests integration api consumer, I need to invoke run test through a stable signature so the tests integration api workflow remains explicit and composable. @fn bool FApiEndpointNotFoundTest::RunTest(const FString &Parameters) */
bool FApiEndpointNotFoundTest::RunTest(const FString &Parameters) {
  const FString Key = ApiEndpointTests::RequiredApiKey(*this);
  if (Key.IsEmpty()) {
    return true;
  }
  Ops::commitApiConfiguration(
      store(), ConfigSelectors::selectApiUrl(store().getState()), Key);

  auto State = MakeShared<FApiEndpointTestState>();
  ADD_LATENT_AUTOMATION_COMMAND(FHttpGetWaitComplete(
      ApiEndpointTests::BaseUrl() + TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5D25F69A2458), Key,
      State));
  ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(
      [this, State]() {
        TestTrue(FORBOCAI_SDK_AUTHORED_STRINGV1068614E082D, State->bDone);
        if (!State->bDone) {
          return;
        }
        TestFalse(FORBOCAI_SDK_AUTHORED_STRINGVCF6FAA867FC9, State->bSuccess);
        TestEqual(FORBOCAI_SDK_AUTHORED_STRINGV7BC91F71337B, State->HttpCode, FORBOCAI_SDK_AUTHORED_NUMBERV2D91EE8B369D);
      },
      FORBOCAI_SDK_AUTHORED_NUMBERVEC53E6A2E194));
  return true;
}
