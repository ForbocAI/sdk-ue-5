#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Features/Soul/SoulSlice.h"

using namespace rtk;
using namespace SoulSlice;
namespace SoulActions = SoulSlice::Actions;

/**
 * Test: Import failure followed by successful retry recovers state
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSoulImportRecoveryTest,
                                 "ForbocAI.Slices.Soul.ImportFailureRecovery",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSoulImportRecoveryTest::RunTest(const FString &Parameters) {
  Slice<FSoulSliceState> SSlice = createSoulSlice();
  FSoulSliceState State;

  /**
   * First attempt: fails
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = SSlice.Reducer(State, SoulActions::importSoulPending());
  State = SSlice.Reducer(State,
                         SoulActions::importSoulFailed(TEXT("Network timeout")));
  TestEqual("ImportStatus failed", State.ImportStatus,
            FString(TEXT("failed")));
  TestEqual("Error set", State.Error, FString(TEXT("Network timeout")));

  /**
   * Second attempt: succeeds
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = SSlice.Reducer(State, SoulActions::importSoulPending());
  TestEqual("ImportStatus importing again", State.ImportStatus,
            FString(TEXT("importing")));
  TestTrue("Error cleared on pending", State.Error.IsEmpty());

  FSoul Soul;
  Soul.Id = TEXT("retry_soul");
  Soul.Persona = TEXT("Recovered");
  State = SSlice.Reducer(State, SoulActions::importSoulSuccess(Soul));
  TestEqual("ImportStatus success", State.ImportStatus,
            FString(TEXT("success")));
  TestTrue("Has last import", State.bHasLastImport);
  TestEqual("Soul id matches", State.LastImport.Id,
            FString(TEXT("retry_soul")));

  return true;
}

/**
 * Test: Export failure followed by successful retry recovers state
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSoulExportRecoveryTest,
                                 "ForbocAI.Slices.Soul.ExportFailureRecovery",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSoulExportRecoveryTest::RunTest(const FString &Parameters) {
  Slice<FSoulSliceState> SSlice = createSoulSlice();
  FSoulSliceState State;

  /**
   * First: fail
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = SSlice.Reducer(State, SoulActions::remoteExportSoulPending());
  State = SSlice.Reducer(State,
                         SoulActions::remoteExportSoulFailed(TEXT("Upload error")));
  TestEqual("Export failed", State.ExportStatus, FString(TEXT("failed")));

  /**
   * Second: succeed
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = SSlice.Reducer(State, SoulActions::remoteExportSoulPending());
  TestEqual("Re-exporting", State.ExportStatus, FString(TEXT("exporting")));

  FSoulExportResult Result;
  Result.TxId = TEXT("retry_tx");
  State = SSlice.Reducer(State, SoulActions::remoteExportSoulSuccess(Result));
  TestEqual("Export success", State.ExportStatus, FString(TEXT("success")));
  TestEqual("TxId matches", State.LastExport.TxId, FString(TEXT("retry_tx")));

  return true;
}

/**
 * Test: Empty soul list is handled correctly
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSoulEmptyListTest,
                                 "ForbocAI.Slices.Soul.EmptyList",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSoulEmptyListTest::RunTest(const FString &Parameters) {
  Slice<FSoulSliceState> SSlice = createSoulSlice();
  FSoulSliceState State;

  /**
   * Set empty list
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TArray<FSoulListItem> EmptyList;
  State = SSlice.Reducer(State, SoulActions::setSoulList(EmptyList));
  TestEqual("Empty soul list", State.AvailableSouls.Num(), 0);

  /**
   * Replace non-empty with empty
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TArray<FSoulListItem> NonEmpty;
  FSoulListItem Item;
  Item.TxId = TEXT("tx_1");
  NonEmpty.Add(Item);
  State = SSlice.Reducer(State, SoulActions::setSoulList(NonEmpty));
  TestEqual("One soul", State.AvailableSouls.Num(), 1);

  State = SSlice.Reducer(State, SoulActions::setSoulList(EmptyList));
  TestEqual("Back to empty", State.AvailableSouls.Num(), 0);

  return true;
}

/**
 * Test: Soul list replacement is atomic (not additive)
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSoulListReplacementTest,
                                 "ForbocAI.Slices.Soul.ListReplacement",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSoulListReplacementTest::RunTest(const FString &Parameters) {
  Slice<FSoulSliceState> SSlice = createSoulSlice();
  FSoulSliceState State;

  TArray<FSoulListItem> List1;
  FSoulListItem Item1;
  Item1.TxId = TEXT("old_tx");
  List1.Add(Item1);
  State = SSlice.Reducer(State, SoulActions::setSoulList(List1));
  TestEqual("First list count", State.AvailableSouls.Num(), 1);

  TArray<FSoulListItem> List2;
  FSoulListItem Item2;
  Item2.TxId = TEXT("new_tx_1");
  FSoulListItem Item3;
  Item3.TxId = TEXT("new_tx_2");
  List2.Add(Item2);
  List2.Add(Item3);
  State = SSlice.Reducer(State, SoulActions::setSoulList(List2));
  TestEqual("Second list replaces first", State.AvailableSouls.Num(), 2);
  TestEqual("First item is new", State.AvailableSouls[0].TxId,
            FString(TEXT("new_tx_1")));

  return true;
}

/**
 * Test: clearSoulState does not affect soul list set afterward
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSoulClearThenSetTest,
                                 "ForbocAI.Slices.Soul.ClearThenSet",
                                 EAutomationTestFlags_ApplicationContextMask |
                                     EAutomationTestFlags::EngineFilter)
/**
 * User Story: As a developer, I need RunTest to fulfill its role in the module.
 */
bool FSoulClearThenSetTest::RunTest(const FString &Parameters) {
  Slice<FSoulSliceState> SSlice = createSoulSlice();
  FSoulSliceState State;

  /**
   * Export and import something
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  State = SSlice.Reducer(State, SoulActions::remoteExportSoulPending());
  FSoulExportResult Result;
  Result.TxId = TEXT("export_tx");
  State = SSlice.Reducer(State, SoulActions::remoteExportSoulSuccess(Result));
  TestTrue("Has export", State.bHasLastExport);

  /**
   * Clear
   * User Story: As a maintainer, I need this step note so I can follow the scenario progression and reason about the expected state changes.
   */
  State = SSlice.Reducer(State, SoulActions::clearSoulState());
  TestFalse("Export cleared", State.bHasLastExport);
  TestFalse("Import cleared", State.bHasLastImport);
  TestEqual("ExportStatus idle", State.ExportStatus, FString(TEXT("idle")));

  /**
   * Set new list after clear
   * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
   */
  TArray<FSoulListItem> NewList;
  FSoulListItem NewItem;
  NewItem.TxId = TEXT("post_clear");
  NewList.Add(NewItem);
  State = SSlice.Reducer(State, SoulActions::setSoulList(NewList));
  TestEqual("List works after clear", State.AvailableSouls.Num(), 1);

  return true;
}
