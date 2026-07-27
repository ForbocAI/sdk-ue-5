#include "Core/rtk.hpp"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Entities/Soul/SoulSelectors.h"
#include "Entities/Soul/SoulSlice.h"
#include "Systems/Soul/Storage/Configuration/StorageConfigurationAdapters.h"
#include "Systems/Soul/Storage/Provider/StorageProviderAdapters.h"
#include "Systems/Testing/Soul/TestingSoulAdapters.h"
#include "Misc/AutomationTest.h"

using namespace Testing::Soul;
using namespace SoulSlice;

namespace {

struct FSoulTestActionInput {
  FSoulSliceState State;
  FSoulTestAction Action;
};

typedef func::ArgDispatcher<ESoulTestActionKind, FSoulTestActionInput,
                            FSoulSliceState>
    FSoulTestActionDispatcher;

/** User Story: As a tests slices consumer, I need to invoke required soul field message through a stable signature so the tests slices workflow remains explicit and composable. @fn inline std::string RequiredSoulFieldMessage() */
inline std::string RequiredSoulFieldMessage() {
  return std::string(
      TCHAR_TO_UTF8(*TestingSoulFixtures().Labels.RequiredField));
}

/** User Story: As a tests slices consumer, I need to invoke required soul field through a stable signature so the tests slices workflow remains explicit and composable. @fn template <typename T> T RequiredSoulField(const func::Maybe<T> &Value) */
template <typename T>
T RequiredSoulField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredSoulFieldMessage());
}

/** User Story: As a tests slices consumer, I need to invoke soul test slice through a stable signature so the tests slices workflow remains explicit and composable. @fn inline const rtk::Slice<FSoulSliceState> &SoulTestSlice() */
inline const rtk::Slice<FSoulSliceState> &SoulTestSlice() {
  static const rtk::Slice<FSoulSliceState> Slice =
      SoulSlice::createSoulSlice();
  return Slice;
}

/** User Story: As a tests slices consumer, I need to invoke soul list items through a stable signature so the tests slices workflow remains explicit and composable. @fn inline TArray<FSoulListItem> SoulListItems(const TArray<FString> &TxIds) */
inline TArray<FSoulListItem>
SoulListItems(const TArray<FString> &TxIds) {
  return func::map_array<FString, FSoulListItem>(
      TxIds, [](const FString &TxId) {
        FSoulListItem Item;
        Item.TxId = TxId;
        return Item;
      });
}

/** User Story: As a tests slices consumer, I need to invoke build soul test action dispatcher through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FSoulTestActionDispatcher BuildSoulTestActionDispatcher() */
inline FSoulTestActionDispatcher BuildSoulTestActionDispatcher() {
  FSoulTestActionDispatcher Dispatcher =
      func::create_arg_dispatcher<ESoulTestActionKind,
                                  FSoulTestActionInput,
                                  FSoulSliceState>();
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ExportPending,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, rtk::exportSoulThunk().pending(FString()));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ExportSuccess,
      [](const FSoulTestActionInput &Input) {
        FSoulExportResult Result;
        Result.TxId = RequiredSoulField(Input.Action.TxId);
        return SoulTestSlice().Reducer(
            Input.State, rtk::exportSoulThunk().fulfilled(Result));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ExportFailed,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, rtk::exportSoulThunk().rejected(
                             RequiredSoulField(Input.Action.Error)));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ImportPending,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, rtk::importSoulThunk().pending(FString()));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ImportSuccess,
      [](const FSoulTestActionInput &Input) {
        FSoul Soul;
        Soul.Id = RequiredSoulField(Input.Action.SoulId);
        Soul.StructuredPersona = RequiredSoulField(Input.Action.Persona);
        return SoulTestSlice().Reducer(
            Input.State, rtk::importSoulThunk().fulfilled(Soul));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ImportFailed,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, rtk::importSoulThunk().rejected(
                             RequiredSoulField(Input.Action.Error)));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::SetList,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, rtk::listSoulsThunk().fulfilled(
                             SoulListItems(Input.Action.TxIds)));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::Clear,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, SoulSlice::Actions::clearSoulState());
      });
  return func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::Inspect,
      [](const FSoulTestActionInput &Input) { return Input.State; });
}

/** User Story: As a tests slices consumer, I need to invoke soul test action dispatcher through a stable signature so the tests slices workflow remains explicit and composable. @fn inline const FSoulTestActionDispatcher &SoulTestActionDispatcher() */
inline const FSoulTestActionDispatcher &SoulTestActionDispatcher() {
  static const FSoulTestActionDispatcher Dispatcher =
      BuildSoulTestActionDispatcher();
  return Dispatcher;
}

/** User Story: As a tests slices consumer, I need to invoke apply soul test action through a stable signature so the tests slices workflow remains explicit and composable. @fn inline FSoulSliceState ApplySoulTestAction( const FSoulSliceState &State, const FSoulTestAction &Action) */
inline FSoulSliceState ApplySoulTestAction(
    const FSoulSliceState &State, const FSoulTestAction &Action) {
  const FSoulTestActionInput Input{State, Action};
  const FSoulTestActionDispatcher &Dispatcher =
      SoulTestActionDispatcher();
  return RequiredSoulField(func::arg_dispatcher_dispatch_maybe(
      func::ArgDispatcherDispatch<ESoulTestActionKind,
                                  FSoulTestActionInput,
                                  FSoulSliceState>{
          &Dispatcher, &Action.Kind, &Input}));
}

} // namespace

IMPLEMENT_COMPLEX_AUTOMATION_TEST(
    FSoulTest, FORBOCAI_SDK_AUTHORED_STRINGVD267C50F1D37,
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

/** User Story: As a tests slices consumer, I need to invoke get tests through a stable signature so the tests slices workflow remains explicit and composable. @fn void FSoulTest::GetTests(TArray<FString> &OutBeautifiedNames, TArray<FString> &OutTestCommands) const */
void FSoulTest::GetTests(TArray<FString> &OutBeautifiedNames,
                         TArray<FString> &OutTestCommands) const {
  func::for_each_array<FSoulTestScenario>(
      TestingSoulFixtures().Scenarios,
      [&OutBeautifiedNames,
       &OutTestCommands](const FSoulTestScenario &Scenario) {
        OutBeautifiedNames.Add(Scenario.Name);
        OutTestCommands.Add(Scenario.Name);
      });
}

/** User Story: As a tests slices consumer, I need to invoke run test through a stable signature so the tests slices workflow remains explicit and composable. @fn bool FSoulTest::RunTest(const FString &Parameters) */
bool FSoulTest::RunTest(const FString &Parameters) {
  const FSoulTestFixtures &Fixtures = TestingSoulFixtures();
  const SoulStorage::Configuration::FSoulStorageConfigurationData &Data =
      SoulStorage::Configuration::soulStorageData();
  TestEqual(
      Fixtures.Labels.ProviderUrls,
      SoulStorage::Provider::soulProviderUrlsAdapter(
          Fixtures.ProviderUrls.Response),
      Fixtures.ProviderUrls.Expected);
  TestEqual(
      Fixtures.Labels.ProviderRetryWithinCycle,
      SoulStorage::Provider::soulProviderRetryDelayAdapter(
          Fixtures.ProviderRetry.WithinCycleAttempt,
          Fixtures.ProviderRetry.UrlCount),
      Data.Numbers.Zero);
  TestEqual(
      Fixtures.Labels.ProviderRetryCycleBoundary,
      SoulStorage::Provider::soulProviderRetryDelayAdapter(
          Fixtures.ProviderRetry.CycleBoundaryAttempt,
          Fixtures.ProviderRetry.UrlCount),
      Data.Retrieval.DelayMs);
  const int32 MaximumAttempt =
      Data.Retrieval.MaximumCycles * Fixtures.ProviderRetry.UrlCount;
  TestFalse(Fixtures.Labels.ProviderRetryBeforeMaximumCycle,
            SoulStorage::Provider::soulProviderRetryExhaustedAdapter(
                MaximumAttempt - Data.Retrieval.AttemptStep,
                Fixtures.ProviderRetry.UrlCount));
  TestTrue(Fixtures.Labels.ProviderRetryAtMaximumCycle,
           SoulStorage::Provider::soulProviderRetryExhaustedAdapter(
               MaximumAttempt, Fixtures.ProviderRetry.UrlCount));
  TestTrue(Fixtures.Labels.ProviderRetryWithoutGateways,
           SoulStorage::Provider::soulProviderRetryExhaustedAdapter(
               Data.Retrieval.InitialAttempt, Data.Numbers.Zero));
  const Testing::Soul::Storage::Serialization::FStorageSerializationResult
      StorageResult =
          Testing::Soul::Storage::Serialization::
              evaluateStorageSerializationAdapter(
                  Fixtures.StorageSerialization);
  TestTrue(Fixtures.Labels.StorageWalletRoundTrip,
           StorageResult.bWalletRoundTrip);
  TestTrue(Fixtures.Labels.StorageEnvelopeRoundTrip,
           StorageResult.bEnvelopeRoundTrip);
  TestTrue(Fixtures.Labels.StorageCatalogRoundTrip,
           StorageResult.bCatalogRoundTrip);
  const func::Maybe<FSoulTestScenario> Scenario =
      FindSoulTestScenario(Parameters);
  TestTrue(Fixtures.Labels.ScenarioPresent, Scenario.hasValue);
  return func::match(
      Scenario,
      [this, &Fixtures](const FSoulTestScenario &Value) {
        const auto TestValue =
            [this](const FString &Label, const auto &Expected,
                   const auto &Actual) {
              return func::match(
                  Expected,
                  [this, &Label, &Actual](const auto &ExpectedValue) {
                    TestEqual(Label, Actual, ExpectedValue);
                    return true;
                  },
                  []() { return true; });
            };
        func::fold_array<FSoulTestStep, FSoulSliceState>(
            Value.Steps, SoulTestSlice().InitialState,
            [this, &Fixtures, &TestValue](const FSoulSliceState &State,
                                         const FSoulTestStep &Step) {
              const FSoulSliceState Next =
                  ApplySoulTestAction(State, Step.Action);
              TestValue(Fixtures.Labels.ExportStatus,
                        Step.Expected.ExportStatus,
                        SoulSelectors::selectSoulExportStatus(Next));
              TestValue(Fixtures.Labels.ImportStatus,
                        Step.Expected.ImportStatus,
                        SoulSelectors::selectSoulImportStatus(Next));
              TestValue(Fixtures.Labels.HasExport,
                        Step.Expected.HasExport,
                        SoulSelectors::selectSoulHasExportResult(Next));
              TestValue(Fixtures.Labels.ExportTxId,
                        Step.Expected.ExportTxId,
                        SoulSelectors::selectSoulExportResult(Next).TxId);
              TestValue(Fixtures.Labels.HasImport,
                        Step.Expected.HasImport,
                        SoulSelectors::selectSoulHasImportedSoul(Next));
              TestValue(Fixtures.Labels.ImportId,
                        Step.Expected.ImportId,
                        SoulSelectors::selectSoulImportedSoul(Next).Id);
              TestValue(Fixtures.Labels.ImportPersona,
                        Step.Expected.ImportPersona,
                        SoulSelectors::selectSoulImportedSoul(Next)
                            .StructuredPersona);
              const TArray<FSoulListItem> &AvailableSouls =
                  SoulSelectors::selectSoulAvailableSouls(Next);
              TestValue(Fixtures.Labels.AvailableSoulCount,
                        Step.Expected.AvailableSoulCount,
                        AvailableSouls.Num());
              func::match(
                  Step.Expected.AvailableSoulFirstTxId,
                  [this, &Fixtures,
                   &AvailableSouls](const FString &Expected) {
                    const func::Maybe<FSoulListItem> First =
                        func::find_array<FSoulListItem>(
                            AvailableSouls,
                            [](const FSoulListItem &) { return true; });
                    TestTrue(Fixtures.Labels.AvailableSoulFirstTxId,
                             First.hasValue);
                    return func::match(
                        First,
                        [this, &Fixtures,
                         &Expected](const FSoulListItem &Actual) {
                          TestEqual(
                              Fixtures.Labels.AvailableSoulFirstTxId,
                              Actual.TxId, Expected);
                          return true;
                        },
                        []() { return false; });
                  },
                  []() { return true; });
              TestValue(Fixtures.Labels.Error, Step.Expected.Error,
                        SoulSelectors::selectSoulError(Next));
              return Next;
            });
        return true;
      },
      []() { return false; });
}
