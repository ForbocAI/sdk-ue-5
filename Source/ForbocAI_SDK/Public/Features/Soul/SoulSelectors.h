#pragma once

#include "Features/Soul/SoulSlice.h"

namespace SoulSelectors {

using SoulSlice::FSoulSliceState;

/** User Story: As a features soul consumer, I need to invoke select soul export status through a stable signature so the features soul workflow remains explicit and composable. @fn inline const FString &selectSoulExportStatus(const FSoulSliceState &State) */
inline const FString &selectSoulExportStatus(const FSoulSliceState &State) {
  return State.ExportStatus;
}

/** User Story: As a features soul consumer, I need to invoke select soul import status through a stable signature so the features soul workflow remains explicit and composable. @fn inline const FString &selectSoulImportStatus(const FSoulSliceState &State) */
inline const FString &selectSoulImportStatus(const FSoulSliceState &State) {
  return State.ImportStatus;
}

/** User Story: As a features soul consumer, I need to invoke the Soul export result through a stable signature so the features soul workflow remains explicit and composable. @fn inline const FSoulExportResult & selectSoulExportResult(const FSoulSliceState &State) */
inline const FSoulExportResult &
selectSoulExportResult(const FSoulSliceState &State) {
  return State.ExportResult;
}

/** User Story: As a features soul consumer, I need to test whether a Soul export result exists through a stable signature so the features soul workflow remains explicit and composable. @fn inline bool selectSoulHasExportResult(const FSoulSliceState &State) */
inline bool selectSoulHasExportResult(const FSoulSliceState &State) {
  return State.bHasExportResult;
}

/** User Story: As a features soul consumer, I need to select the imported Soul through a stable signature so the features soul workflow remains explicit and composable. @fn inline const FSoul &selectSoulImportedSoul(const FSoulSliceState &State) */
inline const FSoul &selectSoulImportedSoul(const FSoulSliceState &State) {
  return State.ImportedSoul;
}

/** User Story: As a features soul consumer, I need to test whether an imported Soul exists through a stable signature so the features soul workflow remains explicit and composable. @fn inline bool selectSoulHasImportedSoul(const FSoulSliceState &State) */
inline bool selectSoulHasImportedSoul(const FSoulSliceState &State) {
  return State.bHasImportedSoul;
}

/** User Story: As a features soul consumer, I need to invoke select soul available souls through a stable signature so the features soul workflow remains explicit and composable. @fn inline const TArray<FSoulListItem> & selectSoulAvailableSouls(const FSoulSliceState &State) */
inline const TArray<FSoulListItem> &
selectSoulAvailableSouls(const FSoulSliceState &State) {
  return State.AvailableSouls;
}

/** User Story: As a Soul consumer, I need catalog activity selected without inspecting thunk internals. @fn inline bool selectSoulIsListing(const FSoulSliceState &State) */
inline bool selectSoulIsListing(const FSoulSliceState &State) {
  return State.bListing;
}

/** User Story: As a Soul consumer, I need verification activity selected without inspecting provider internals. @fn inline bool selectSoulIsVerifying(const FSoulSliceState &State) */
inline bool selectSoulIsVerifying(const FSoulSliceState &State) {
  return State.bVerifying;
}

/** User Story: As a features soul consumer, I need to invoke select soul error through a stable signature so the features soul workflow remains explicit and composable. @fn inline const FString &selectSoulError(const FSoulSliceState &State) */
inline const FString &selectSoulError(const FSoulSliceState &State) {
  return State.Error;
}

} // namespace SoulSelectors
