#pragma once

#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace TestGame {
namespace CoverageSelectors {

/** User Story: As a systems harness coverage consumer, I need to invoke select harness covered through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline TMap<FString, bool> SelectHarnessCovered(const FHarnessState &S) */
inline TMap<FString, bool> SelectHarnessCovered(const FHarnessState &S) {
  return S.Covered;
}

/** User Story: As a systems harness coverage consumer, I need missing groups derived from the active API contract so coverage cannot pass against a stale local list. @fn inline TArray<FString> SelectHarnessMissingGroups( const FHarnessState &State, const TArray<FString> &Groups) */
inline TArray<FString> SelectHarnessMissingGroups(
    const FHarnessState &State, const TArray<FString> &Groups) {
  return func::filter_array<FString>(Groups, [&State](const FString &Group) {
    return !func::map_value_or<FString, bool>(State.Covered, Group, false);
  });
}

} // namespace CoverageSelectors
} // namespace TestGame
