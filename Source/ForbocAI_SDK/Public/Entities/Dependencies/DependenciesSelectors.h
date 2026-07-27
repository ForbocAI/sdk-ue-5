#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Components/Dependencies/DependenciesTypes.h"

namespace DependenciesSelectors {

/** User Story: As a features dependencies consumer, I need to invoke select dependencies busy through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline bool selectDependenciesBusy(const FDependenciesState &State) */
inline bool selectDependenciesBusy(const FDependenciesState &State) {
  return State.Status == TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5C694E6E05E0) ||
         State.Status == TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF2A8803550F5) ||
         State.Status == TEXT(FORBOCAI_SDK_AUTHORED_STRINGV311FECE34994);
}

/** User Story: As a features dependencies consumer, I need to invoke select native dependencies ready through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline bool selectNativeDependenciesReady(const FDependenciesState &State) */
inline bool selectNativeDependenciesReady(const FDependenciesState &State) {
  return State.bHasReport && State.Report.Vectorizer.bAvailable &&
         State.Report.VectorDb.bAvailable;
}

/** User Story: As a features dependencies consumer, I need to invoke select dependencies ready through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline bool selectDependenciesReady(const FDependenciesState &State) */
inline bool selectDependenciesReady(const FDependenciesState &State) {
  return State.bHasResult && State.Result.Vector.bOk &&
         State.Result.Memory.bOk;
}

/** User Story: As a features dependencies consumer, I need to invoke select dependencies error through a stable signature so the features dependencies workflow remains explicit and composable. @fn inline const FString &selectDependenciesError(const FDependenciesState &State) */
inline const FString &selectDependenciesError(const FDependenciesState &State) {
  return State.Error;
}

/** User Story: As a features dependencies consumer, I need to invoke select dependencies state through a stable signature so the features dependencies workflow remains explicit and composable. @fn template <typename RootState> inline const FDependenciesState &selectDependenciesState(const RootState &State) */
template <typename RootState>
inline const FDependenciesState &selectDependenciesState(const RootState &State) {
  return State.Dependencies;
}

/** User Story: As a features dependencies consumer, I need to invoke select dependencies busy through a stable signature so the features dependencies workflow remains explicit and composable. @fn template <typename RootState> inline bool selectDependenciesBusy(const RootState &State) */
template <typename RootState>
inline bool selectDependenciesBusy(const RootState &State) {
  return selectDependenciesBusy(selectDependenciesState(State));
}

} // namespace DependenciesSelectors
