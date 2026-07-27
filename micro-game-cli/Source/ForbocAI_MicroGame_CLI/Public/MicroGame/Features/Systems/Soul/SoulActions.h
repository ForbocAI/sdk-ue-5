#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace MicroGame {

struct FMarkSoulExportedPayload {
  FString NpcId;
  FString TxId;
};

namespace GameSoulActions {

/** User Story: As a features systems soul consumer, I need to invoke mark soul exported action creator through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline rtk::ActionCreator<FMarkSoulExportedPayload> markSoulExportedActionCreator() */
inline rtk::ActionCreator<FMarkSoulExportedPayload>
markSoulExportedActionCreator() {
  static auto C = rtk::createAction<FMarkSoulExportedPayload>(
      TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV2F73CF8DA810));
  return C;
}

/** User Story: As a features systems soul consumer, I need to invoke mark soul imported action creator through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> markSoulImportedActionCreator() */
inline rtk::ActionCreator<FString> markSoulImportedActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV4B2AE2A42548));
  return C;
}

/** User Story: As a features systems soul consumer, I need to invoke mark soul exported through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline rtk::AnyAction markSoulExported(const FMarkSoulExportedPayload &P) */
inline rtk::AnyAction markSoulExported(const FMarkSoulExportedPayload &P) {
  return markSoulExportedActionCreator()(P);
}

/** User Story: As a features systems soul consumer, I need to invoke mark soul imported through a stable signature so the features systems soul workflow remains explicit and composable. @fn inline rtk::AnyAction markSoulImported(const FString &TxId) */
inline rtk::AnyAction markSoulImported(const FString &TxId) {
  return markSoulImportedActionCreator()(TxId);
}

} // namespace GameSoulActions
} // namespace MicroGame
