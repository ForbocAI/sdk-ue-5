#pragma once

#include "Features/Soul/SoulSlice.h"

namespace SoulSelectors {

using SoulSlice::FSoulSliceState;

inline const FString &selectSoulExportStatus(const FSoulSliceState &State) {
  return State.ExportStatus;
}

inline const FString &selectSoulImportStatus(const FSoulSliceState &State) {
  return State.ImportStatus;
}

inline const FSoulExportResult &
selectSoulLastExport(const FSoulSliceState &State) {
  return State.LastExport;
}

inline bool selectSoulHasLastExport(const FSoulSliceState &State) {
  return State.bHasLastExport;
}

inline const FSoul &selectSoulLastImport(const FSoulSliceState &State) {
  return State.LastImport;
}

inline bool selectSoulHasLastImport(const FSoulSliceState &State) {
  return State.bHasLastImport;
}

inline const TArray<FSoulListItem> &
selectSoulAvailableSouls(const FSoulSliceState &State) {
  return State.AvailableSouls;
}

inline const FString &selectSoulError(const FSoulSliceState &State) {
  return State.Error;
}

} // namespace SoulSelectors
