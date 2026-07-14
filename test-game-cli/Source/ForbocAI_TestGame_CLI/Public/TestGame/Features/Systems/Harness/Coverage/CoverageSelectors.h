#pragma once

#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace TestGame {
namespace HarnessSelectors {

inline TMap<ECommandGroup, bool> SelectHarnessCovered(const FHarnessState &S) {
  return S.Covered;
}

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

inline TArray<ECommandGroup> SelectHarnessMissingGroups(
    const FHarnessState &State, const TArray<ECommandGroup> &Groups) {
  TArray<ECommandGroup> Missing;
  CollectMissingGroups(State.Covered, Groups, 0, Missing);
  return Missing;
}

} // namespace HarnessSelectors
} // namespace TestGame
