#pragma once

#include "Features/Vector/VectorTypes.h"

namespace VectorSelectors {

inline bool selectVectorReady(const FVectorState &State) {
  return State.bIsReady;
}

inline bool selectVectorBusy(const FVectorState &State) {
  return State.Status == TEXT("initializing");
}

inline const FString &selectVectorError(const FVectorState &State) {
  return State.Error;
}

template <typename RootState>
inline const FVectorState &selectVectorState(const RootState &State) {
  return State.Vector;
}

template <typename RootState>
inline bool selectVectorBusy(const RootState &State) {
  return selectVectorBusy(selectVectorState(State));
}

} // namespace VectorSelectors
