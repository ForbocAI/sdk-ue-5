#pragma once

#include "Features/Dependencies/DependenciesTypes.h"

namespace DependenciesSelectors {

inline bool selectDependenciesBusy(const FDependenciesState &State) {
  return State.Status == TEXT("checking") ||
         State.Status == TEXT("settingUp") ||
         State.Status == TEXT("refreshing");
}

inline bool selectNativeDependenciesReady(const FDependenciesState &State) {
  return State.bHasReport && State.Report.Vectorizer.bAvailable &&
         State.Report.VectorDb.bAvailable;
}

inline bool selectDependenciesReady(const FDependenciesState &State) {
  return State.bHasResult && State.Result.Vector.bOk &&
         State.Result.Memory.bOk;
}

inline const FString &selectDependenciesError(const FDependenciesState &State) {
  return State.Error;
}

template <typename RootState>
inline const FDependenciesState &selectDependenciesState(const RootState &State) {
  return State.Dependencies;
}

template <typename RootState>
inline bool selectDependenciesBusy(const RootState &State) {
  return selectDependenciesBusy(selectDependenciesState(State));
}

} // namespace DependenciesSelectors
