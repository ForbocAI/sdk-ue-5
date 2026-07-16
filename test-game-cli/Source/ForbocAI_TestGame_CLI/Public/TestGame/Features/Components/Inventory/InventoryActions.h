#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace TestGame {

struct FSetOwnerInventoryPayload {
  FString OwnerId;
  TArray<FString> Items;
};

namespace InventoryActions {

/** User Story: As a features components inventory consumer, I need to invoke set owner inventory action creator through a stable signature so the features components inventory workflow remains explicit and composable. @fn inline rtk::ActionCreator<FSetOwnerInventoryPayload> setOwnerInventoryActionCreator() */
inline rtk::ActionCreator<FSetOwnerInventoryPayload>
setOwnerInventoryActionCreator() {
  static auto C = rtk::createAction<FSetOwnerInventoryPayload>(
      TEXT("testgame/inventory/setOwnerInventory"));
  return C;
}

/** User Story: As a features components inventory consumer, I need to invoke set owner inventory through a stable signature so the features components inventory workflow remains explicit and composable. @fn inline rtk::AnyAction setOwnerInventory(const FSetOwnerInventoryPayload &P) */
inline rtk::AnyAction setOwnerInventory(const FSetOwnerInventoryPayload &P) {
  return setOwnerInventoryActionCreator()(P);
}

} // namespace InventoryActions
} // namespace TestGame
