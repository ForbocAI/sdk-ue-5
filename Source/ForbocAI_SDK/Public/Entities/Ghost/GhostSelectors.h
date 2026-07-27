#pragma once

#include "Entities/Ghost/GhostSlice.h"

namespace GhostSelectors {

using GhostSlice::FGhostSliceState;

/** User Story: As a features ghost consumer, I need to invoke select ghost active session id through a stable signature so the features ghost workflow remains explicit and composable. @fn inline const FString & selectGhostActiveSessionId(const FGhostSliceState &State) */
inline const FString &
selectGhostActiveSessionId(const FGhostSliceState &State) {
  return State.ActiveSessionId;
}

/** User Story: As a features ghost consumer, I need to invoke select ghost status through a stable signature so the features ghost workflow remains explicit and composable. @fn inline const FString &selectGhostStatus(const FGhostSliceState &State) */
inline const FString &selectGhostStatus(const FGhostSliceState &State) {
  return State.Status;
}

/** User Story: As a features ghost consumer, I need to invoke select ghost progress through a stable signature so the features ghost workflow remains explicit and composable. @fn inline float selectGhostProgress(const FGhostSliceState &State) */
inline float selectGhostProgress(const FGhostSliceState &State) {
  return State.Progress;
}

/** User Story: As a features ghost consumer, I need to invoke select ghost results through a stable signature so the features ghost workflow remains explicit and composable. @fn inline const FGhostTestReport & selectGhostResults(const FGhostSliceState &State) */
inline const FGhostTestReport &
selectGhostResults(const FGhostSliceState &State) {
  return State.Results;
}

/** User Story: As a features ghost consumer, I need to invoke select ghost has results through a stable signature so the features ghost workflow remains explicit and composable. @fn inline bool selectGhostHasResults(const FGhostSliceState &State) */
inline bool selectGhostHasResults(const FGhostSliceState &State) {
  return State.bHasResults;
}

/** User Story: As a features ghost consumer, I need to invoke select ghost history through a stable signature so the features ghost workflow remains explicit and composable. @fn inline const TArray<FGhostHistoryEntry> & selectGhostHistory(const FGhostSliceState &State) */
inline const TArray<FGhostHistoryEntry> &
selectGhostHistory(const FGhostSliceState &State) {
  return State.History;
}

/** User Story: As a features ghost consumer, I need to invoke select ghost loading through a stable signature so the features ghost workflow remains explicit and composable. @fn inline bool selectGhostLoading(const FGhostSliceState &State) */
inline bool selectGhostLoading(const FGhostSliceState &State) {
  return State.bLoading;
}

/** User Story: As a features ghost consumer, I need to invoke select ghost error through a stable signature so the features ghost workflow remains explicit and composable. @fn inline const FString &selectGhostError(const FGhostSliceState &State) */
inline const FString &selectGhostError(const FGhostSliceState &State) {
  return State.Error;
}

} // namespace GhostSelectors
