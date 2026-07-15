#pragma once

#include "Features/Ghost/GhostSlice.h"

namespace GhostSelectors {

using GhostSlice::FGhostSliceState;

inline const FString &
selectGhostActiveSessionId(const FGhostSliceState &State) {
  return State.ActiveSessionId;
}

inline const FString &selectGhostStatus(const FGhostSliceState &State) {
  return State.Status;
}

inline float selectGhostProgress(const FGhostSliceState &State) {
  return State.Progress;
}

inline const FGhostTestReport &
selectGhostResults(const FGhostSliceState &State) {
  return State.Results;
}

inline bool selectGhostHasResults(const FGhostSliceState &State) {
  return State.bHasResults;
}

inline const TArray<FGhostHistoryEntry> &
selectGhostHistory(const FGhostSliceState &State) {
  return State.History;
}

inline bool selectGhostLoading(const FGhostSliceState &State) {
  return State.bLoading;
}

inline const FString &selectGhostError(const FGhostSliceState &State) {
  return State.Error;
}

} // namespace GhostSelectors
