#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace TestGame {

struct FMarkSoulExportedPayload {
  FString NpcId;
  FString TxId;
};

namespace GameSoulActions {

inline rtk::ActionCreator<FMarkSoulExportedPayload>
markSoulExportedActionCreator() {
  static auto C = rtk::createAction<FMarkSoulExportedPayload>(
      TEXT("testgame/soul/markSoulExported"));
  return C;
}

inline rtk::ActionCreator<FString> markSoulImportedActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/soul/markSoulImported"));
  return C;
}

inline rtk::AnyAction markSoulExported(const FMarkSoulExportedPayload &P) {
  return markSoulExportedActionCreator()(P);
}

inline rtk::AnyAction markSoulImported(const FString &TxId) {
  return markSoulImportedActionCreator()(TxId);
}

} // namespace GameSoulActions
} // namespace TestGame
