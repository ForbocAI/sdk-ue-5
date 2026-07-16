#pragma once

#include "Features/Vector/VectorTypes.h"

namespace VectorSelectors {

/** User Story: As a features vector consumer, I need to invoke select vector ready through a stable signature so the features vector workflow remains explicit and composable. @fn inline bool selectVectorReady(const FVectorState &State) */
inline bool selectVectorReady(const FVectorState &State) {
  return State.bIsReady;
}

/** User Story: As a features vector consumer, I need to invoke select vector busy through a stable signature so the features vector workflow remains explicit and composable. @fn inline bool selectVectorBusy(const FVectorState &State) */
inline bool selectVectorBusy(const FVectorState &State) {
  return State.Status == TEXT("initializing");
}

/** User Story: As a features vector consumer, I need to invoke select vector error through a stable signature so the features vector workflow remains explicit and composable. @fn inline const FString &selectVectorError(const FVectorState &State) */
inline const FString &selectVectorError(const FVectorState &State) {
  return State.Error;
}

/** User Story: As a features vector consumer, I need to invoke select vector state through a stable signature so the features vector workflow remains explicit and composable. @fn template <typename RootState> inline const FVectorState &selectVectorState(const RootState &State) */
template <typename RootState>
inline const FVectorState &selectVectorState(const RootState &State) {
  return State.Vector;
}

/** User Story: As a features vector consumer, I need to invoke select vector busy through a stable signature so the features vector workflow remains explicit and composable. @fn template <typename RootState> inline bool selectVectorBusy(const RootState &State) */
template <typename RootState>
inline bool selectVectorBusy(const RootState &State) {
  return selectVectorBusy(selectVectorState(State));
}

} // namespace VectorSelectors
