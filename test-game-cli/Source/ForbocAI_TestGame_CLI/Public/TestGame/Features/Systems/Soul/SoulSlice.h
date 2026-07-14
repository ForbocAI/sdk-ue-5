#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Soul/SoulActions.h"
#include "TestGame/Features/Systems/Soul/SoulTypes.h"

namespace TestGame {

namespace GameSoulSelectors {
inline TMap<FString, FString>
SelectSoulExportsByNpc(const FSoulTrackingState &S) {
  return S.ExportsByNpc;
}

inline TArray<FString> SelectImportedSoulTxIds(const FSoulTrackingState &S) {
  return S.ImportedSoulTxIds;
}

inline func::Maybe<FString> SelectSoulExportTxId(
    const FSoulTrackingState &S, const FString &NpcId) {
  const FString *TxId = S.ExportsByNpc.Find(NpcId);
  return TxId == nullptr ? func::nothing<FString>()
                         : func::just<FString>(*TxId);
}
} // namespace GameSoulSelectors

inline rtk::Slice<FSoulTrackingState> CreateGameSoulSlice() {
  return rtk::createSlice<FSoulTrackingState>(
      TEXT("testgame/soul"), FSoulTrackingState(),
      [](rtk::ActionReducerMapBuilder<FSoulTrackingState> &Builder) {
        Builder.addCase(
            GameSoulActions::markSoulExportedActionCreator(),
            [](const FSoulTrackingState &S,
               const rtk::Action<FMarkSoulExportedPayload> &A)
                -> FSoulTrackingState {
              FSoulTrackingState Next = S;
              Next.ExportsByNpc.Add(A.PayloadValue.NpcId, A.PayloadValue.TxId);
              return Next;
            });
        Builder.addCase(
            GameSoulActions::markSoulImportedActionCreator(),
            [](const FSoulTrackingState &S,
               const rtk::Action<FString> &A) -> FSoulTrackingState {
              FSoulTrackingState Next = S;
              Next.ImportedSoulTxIds.Add(A.PayloadValue);
              return Next;
            });
      });
}

} // namespace TestGame
