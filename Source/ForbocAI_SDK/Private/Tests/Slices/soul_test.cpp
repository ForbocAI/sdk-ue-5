#include "Core/rtk.hpp"
#include "Features/Soul/SoulSelectors.h"
#include "Features/Soul/SoulSlice.h"
#include "Features/Testing/Soul/TestingSoulAdapters.h"
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

inline std::string RequiredSoulFieldMessage() {
  return std::string(
      TCHAR_TO_UTF8(*TestingSoulFixtures().Labels.RequiredField));
}

template <typename T>
T RequiredSoulField(const func::Maybe<T> &Value) {
  return func::requireJust<T>(Value, RequiredSoulFieldMessage());
}

inline const rtk::Slice<FSoulSliceState> &SoulTestSlice() {
  static const rtk::Slice<FSoulSliceState> Slice =
      SoulSlice::createSoulSlice();
  return Slice;
}

inline TArray<FSoulListItem>
SoulListItems(const TArray<FString> &TxIds) {
  return func::map_array<FString, FSoulListItem>(
      TxIds, [](const FString &TxId) {
        FSoulListItem Item;
        Item.TxId = TxId;
        return Item;
      });
}

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
            Input.State, SoulSlice::Actions::remoteExportSoulPending());
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ExportSuccess,
      [](const FSoulTestActionInput &Input) {
        FSoulExportResult Result;
        Result.TxId = RequiredSoulField(Input.Action.TxId);
        return SoulTestSlice().Reducer(
            Input.State,
            SoulSlice::Actions::remoteExportSoulSuccess(Result));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ExportFailed,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, SoulSlice::Actions::remoteExportSoulFailed(
                             RequiredSoulField(Input.Action.Error)));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ImportPending,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, SoulSlice::Actions::importSoulPending());
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ImportSuccess,
      [](const FSoulTestActionInput &Input) {
        FSoul Soul;
        Soul.Id = RequiredSoulField(Input.Action.SoulId);
        Soul.Persona = RequiredSoulField(Input.Action.Persona);
        return SoulTestSlice().Reducer(
            Input.State, SoulSlice::Actions::importSoulSuccess(Soul));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::ImportFailed,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, SoulSlice::Actions::importSoulFailed(
                             RequiredSoulField(Input.Action.Error)));
      });
  Dispatcher = func::arg_dispatcher_register<
      ESoulTestActionKind, FSoulTestActionInput, FSoulSliceState>(
      Dispatcher, ESoulTestActionKind::SetList,
      [](const FSoulTestActionInput &Input) {
        return SoulTestSlice().Reducer(
            Input.State, SoulSlice::Actions::setSoulList(
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

inline const FSoulTestActionDispatcher &SoulTestActionDispatcher() {
  static const FSoulTestActionDispatcher Dispatcher =
      BuildSoulTestActionDispatcher();
  return Dispatcher;
}

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
    FSoulTest, "ForbocAI.Slices.Soul",
    EAutomationTestFlags_ApplicationContextMask |
        EAutomationTestFlags::EngineFilter)

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

bool FSoulTest::RunTest(const FString &Parameters) {
  const FSoulTestFixtures &Fixtures = TestingSoulFixtures();
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
            Value.Steps, FSoulSliceState(),
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
                        SoulSelectors::selectSoulHasLastExport(Next));
              TestValue(Fixtures.Labels.ExportTxId,
                        Step.Expected.ExportTxId,
                        SoulSelectors::selectSoulLastExport(Next).TxId);
              TestValue(Fixtures.Labels.HasImport,
                        Step.Expected.HasImport,
                        SoulSelectors::selectSoulHasLastImport(Next));
              TestValue(Fixtures.Labels.ImportId,
                        Step.Expected.ImportId,
                        SoulSelectors::selectSoulLastImport(Next).Id);
              TestValue(Fixtures.Labels.ImportPersona,
                        Step.Expected.ImportPersona,
                        SoulSelectors::selectSoulLastImport(Next).Persona);
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
