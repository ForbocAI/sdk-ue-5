#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageActions.h"
#include "TestGame/Features/Systems/Harness/Coverage/CoverageTypes.h"

namespace TestGame {

inline rtk::Slice<FHarnessState> CreateHarnessSlice() {
  return rtk::createSlice<FHarnessState>(
      TEXT("testgame/harness"), FHarnessState(),
      [](rtk::ActionReducerMapBuilder<FHarnessState> &Builder) {
        Builder.addCase(
            HarnessActions::markCoveredActionCreator(),
            [](const FHarnessState &S,
               const rtk::Action<ECommandGroup> &A) -> FHarnessState {
              FHarnessState Next = S;
              Next.Covered.Add(A.PayloadValue, true);
              return Next;
            });
        Builder.addCase(
            HarnessActions::resetCoverageActionCreator(),
            [](const FHarnessState &,
               const rtk::Action<rtk::FEmptyPayload> &) -> FHarnessState {
              return FHarnessState();
            });
      });
}

inline TArray<ECommandGroup>
SelectMissingGroups(const TMap<ECommandGroup, bool> &Covered,
                    const TArray<ECommandGroup> &Groups) {
  struct CollectMissing {
    static void apply(const TMap<ECommandGroup, bool> &C,
                      const TArray<ECommandGroup> &G,
                      TArray<ECommandGroup> &Out, int32 Idx) {
      Idx >= G.Num()
          ? void()
          : ((!C.Contains(G[Idx]) || !(*C.Find(G[Idx])))
                 ? (Out.Add(G[Idx]), void())
                 : void(),
             apply(C, G, Out, Idx + 1), void());
    }
  };
  TArray<ECommandGroup> Missing;
  CollectMissing::apply(Covered, Groups, Missing, 0);
  return Missing;
}

namespace HarnessSelectors {
inline TMap<ECommandGroup, bool> SelectHarnessCovered(const FHarnessState &S) {
  return S.Covered;
}

inline TArray<ECommandGroup> SelectHarnessMissingGroups(
    const FHarnessState &S, const TArray<ECommandGroup> &Groups) {
  return SelectMissingGroups(S.Covered, Groups);
}
} // namespace HarnessSelectors

} // namespace TestGame
