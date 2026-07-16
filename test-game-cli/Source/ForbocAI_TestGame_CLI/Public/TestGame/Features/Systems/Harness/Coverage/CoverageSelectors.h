#pragma once

#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace TestGame {
namespace HarnessSelectors {

/** User Story: As a systems harness coverage consumer, I need to invoke select harness covered through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline TMap<ECommandGroup, bool> SelectHarnessCovered(const FHarnessState &S) */
inline TMap<ECommandGroup, bool> SelectHarnessCovered(const FHarnessState &S) {
  return S.Covered;
}

/** User Story: As a systems harness coverage consumer, I need to invoke collect missing groups through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline void CollectMissingGroups(const TMap<ECommandGroup, bool> &Covered, const TArray<ECommandGroup> &Groups, int32 Index, TArray<ECommandGroup> &Missing) */
inline void CollectMissingGroups(const TMap<ECommandGroup, bool> &Covered,
                                 const TArray<ECommandGroup> &Groups,
                                 int32 Index,
                                 TArray<ECommandGroup> &Missing) {
  Index >= Groups.Num()
      ? void()
      : ((!Covered.Contains(Groups[Index]) ||
          !(*Covered.Find(Groups[Index])))
             ? (Missing.Add(Groups[Index]), void())
             : void(),
         CollectMissingGroups(Covered, Groups, Index + 1, Missing));
}

/** User Story: As a systems harness coverage consumer, I need to invoke select harness missing groups through a stable signature so the systems harness coverage workflow remains explicit and composable. @fn inline TArray<ECommandGroup> SelectHarnessMissingGroups( const FHarnessState &State, const TArray<ECommandGroup> &Groups) */
inline TArray<ECommandGroup> SelectHarnessMissingGroups(
    const FHarnessState &State, const TArray<ECommandGroup> &Groups) {
  TArray<ECommandGroup> Missing;
  CollectMissingGroups(State.Covered, Groups, 0, Missing);
  return Missing;
}

} // namespace HarnessSelectors
} // namespace TestGame
