#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "MicroGame/Features/Systems/Soul/SoulActions.h"
#include "MicroGame/Features/Systems/Soul/SoulTypes.h"

namespace MicroGame {

namespace GameSoulSelectors {
/** User Story: As a features systems soul consumer, I need to invoke select soul exports by npc through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline TMap<FString, FString> SelectSoulExportsByNpc(const FSoulTrackingState &S) */
inline TMap<FString, FString>
SelectSoulExportsByNpc(const FSoulTrackingState &S) {
  return S.ExportsByNpc;
}

/** User Story: As a features systems soul consumer, I need to invoke select imported soul tx ids through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline TArray<FString> SelectImportedSoulTxIds(const FSoulTrackingState &S) */
inline TArray<FString> SelectImportedSoulTxIds(const FSoulTrackingState &S) {
  return S.ImportedSoulTxIds;
}

/** User Story: As a features systems soul consumer, I need to invoke select soul export tx id through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline func::Maybe<FString> SelectSoulExportTxId( const FSoulTrackingState &S, const FString &NpcId) */
inline func::Maybe<FString> SelectSoulExportTxId(
    const FSoulTrackingState &S, const FString &NpcId) {
  const FString *TxId = S.ExportsByNpc.Find(NpcId);
  return TxId == nullptr ? func::nothing<FString>()
                         : func::just<FString>(*TxId);
}
} // namespace GameSoulSelectors

/** User Story: As a features systems soul consumer, I need to invoke create game soul slice through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline rtk::Slice<FSoulTrackingState> CreateGameSoulSlice() */
inline rtk::Slice<FSoulTrackingState> CreateGameSoulSlice() {
  return rtk::createSlice<FSoulTrackingState>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVD51D35127DA5), FSoulTrackingState(),
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

} // namespace MicroGame
