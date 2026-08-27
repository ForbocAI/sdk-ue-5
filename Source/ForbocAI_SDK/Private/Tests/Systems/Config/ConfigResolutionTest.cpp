#include "Entities/CLI/Presentation/PresentationSelectors.h"
#include "Misc/AutomationTest.h"
#include "Systems/Config/ConfigAdapters.h"
#include "Systems/Config/Resolution/ResolutionThunks.h"
#include "Systems/Errors/ErrorsAdapters.h"
#include "Systems/Testing/Config/TestingConfigAdapters.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FConfigRuntimeEndpointAuthorityTest,
    TestingConfigAdapters::testingConfigData().AutomationName,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/**
 * User Story: As an SDK maintainer, I need endpoint precedence, probe bounds, and credential redaction verified against authored runtime data.
 * @fn bool FConfigRuntimeEndpointAuthorityTest::RunTest( const FString &Parameters)
 */
bool FConfigRuntimeEndpointAuthorityTest::RunTest(
    const FString &Parameters) {
  (void)Parameters;
  const TestingConfigAdapters::FTestingConfigData &TestData =
      TestingConfigAdapters::testingConfigData();
  const ConfigSlice::FConfigRuntimeData &Runtime =
      ConfigSlice::configRuntimeData();

  int32 ExplicitProbeCalls = TestData.Counts.None;
  const ConfigSlice::FConfigState ExplicitState =
      ConfigSlice::readConfigState(
          {TestData.Values.ExplicitApiUrl, TestData.Values.ApiKey},
          TMap<FString, FString>(), TestData.Values.MissingConfigPath);
  const ConfigSlice::FConfigState ExplicitResolved =
      ConfigSlice::resolveApiConfiguration(
          ExplicitState,
          [&ExplicitProbeCalls, &TestData](
              const ConfigSlice::FConfigConnectionData &) {
            ExplicitProbeCalls += TestData.Counts.Increment;
            return TestData.Values.bLocalAvailable;
          });
  TestEqual(*TestData.Assertions.ExplicitUrl, ExplicitResolved.ApiUrl,
            TestData.Values.NormalizedExplicitApiUrl);
  TestEqual(*TestData.Assertions.ExplicitSource,
            ExplicitResolved.ApiUrlSource,
            Runtime.Connection.Sources.Explicit);
  TestEqual(*TestData.Assertions.ExplicitProbe, ExplicitProbeCalls,
            TestData.Counts.None);

  const ConfigSlice::FConfigState AutomaticState =
      ConfigSlice::readConfigState(
          {TestData.Values.Empty, TestData.Values.ApiKey},
          TMap<FString, FString>(), TestData.Values.MissingConfigPath);
  int32 LocalProbeCalls = TestData.Counts.None;
  const ConfigSlice::FConfigState LocalResolved =
      ConfigSlice::resolveApiConfiguration(
          AutomaticState,
          [&LocalProbeCalls, &TestData](
              const ConfigSlice::FConfigConnectionData &) {
            LocalProbeCalls += TestData.Counts.Increment;
            return TestData.Values.bLocalAvailable;
          });
  TestEqual(*TestData.Assertions.LocalUrl, LocalResolved.ApiUrl,
            Runtime.Connection.LocalApiUrl);
  TestEqual(*TestData.Assertions.LocalSource, LocalResolved.ApiUrlSource,
            Runtime.Connection.Sources.Local);
  TestEqual(*TestData.Assertions.LocalProbe, LocalProbeCalls,
            TestData.Counts.Once);

  int32 TimeoutProbeCalls = TestData.Counts.None;
  const ConfigSlice::FConfigState ProductionResolved =
      ConfigSlice::resolveApiConfiguration(
          AutomaticState,
          [&TimeoutProbeCalls, &TestData](
              const ConfigSlice::FConfigConnectionData &) {
            TimeoutProbeCalls += TestData.Counts.Increment;
            return TestData.Values.bLocalUnavailable;
          });
  TestEqual(*TestData.Assertions.ProductionUrl,
            ProductionResolved.ApiUrl, Runtime.Defaults.ApiUrl);
  TestEqual(*TestData.Assertions.ProductionSource,
            ProductionResolved.ApiUrlSource,
            Runtime.Connection.Sources.Production);
  TestEqual(*TestData.Assertions.TimeoutProbe, TimeoutProbeCalls,
            TestData.Counts.Once);

  TestTrue(*TestData.Assertions.ProductionKey,
           Errors::requireApiKeyGuidance(
               ProductionResolved.ApiUrl, TestData.Values.Empty)
               .hasValue);
  TestFalse(*TestData.Assertions.LocalKey,
            Errors::requireApiKeyGuidance(LocalResolved.ApiUrl,
                                          TestData.Values.Empty)
                .hasValue);

  const ForbocAI::CLI::Presentation::FCLIPresentationState Presentation =
      ForbocAI::CLI::Presentation::readCliPresentation();
  const FString EndpointDiagnostics =
      ForbocAI::CLI::Presentation::selectCliApiUrlLine(
          Presentation, ProductionResolved.ApiUrl) +
      ForbocAI::CLI::Presentation::selectCliApiUrlSourceLine(
          Presentation, ProductionResolved.ApiUrlSource) +
      ForbocAI::CLI::Presentation::selectCliApiKeyLine(Presentation, true);
  TestFalse(*TestData.Assertions.RedactedUrl,
            EndpointDiagnostics.Contains(TestData.Values.ApiKey));
  TestTrue(*TestData.Assertions.RedactedKey,
           EndpointDiagnostics.Contains(Presentation.Common.MaskedValue));
  return true;
}
