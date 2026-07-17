#pragma once

#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace TestGame {
namespace CoverageSelectors {

/** User Story: As a systems harness coverage consumer, I need to invoke select harness covered through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline TMap<FString, bool> SelectHarnessCovered(const FHarnessState &S) */
inline TMap<FString, bool> SelectHarnessCovered(const FHarnessState &S) {
  return S.Covered;
}

/** User Story: As a systems harness coverage consumer, I need to invoke collect missing groups through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline void CollectMissingGroups(const TMap<FString, bool> &Covered, const TArray<FString> &Groups, int32 Index, TArray<FString> &Missing) */
inline void CollectMissingGroups(const TMap<FString, bool> &Covered,
                                 const TArray<FString> &Groups,
                                 int32 Index,
                                 TArray<FString> &Missing) {
  Index >= Groups.Num()
      ? void()
      : ((!Covered.Contains(Groups[Index]) ||
          !(*Covered.Find(Groups[Index])))
             ? (Missing.Add(Groups[Index]), void())
             : void(),
         CollectMissingGroups(Covered, Groups, Index + 1, Missing));
}

/** User Story: As a systems harness coverage consumer, I need missing groups derived from the active API contract so coverage cannot pass against a stale local list. @fn inline TArray<FString> SelectHarnessMissingGroups( const FHarnessState &State, const TArray<FString> &Groups) */
inline TArray<FString> SelectHarnessMissingGroups(
    const FHarnessState &State, const TArray<FString> &Groups) {
  TArray<FString> Missing;
  CollectMissingGroups(State.Covered, Groups, 0, Missing);
  return Missing;
}

} // namespace CoverageSelectors
} // namespace TestGame
