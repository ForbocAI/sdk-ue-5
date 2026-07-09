#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"

namespace TestGame {

struct FSetOwnerInventoryPayload {
  FString OwnerId;
  TArray<FString> Items;
};

namespace InventoryActions {

inline rtk::ActionCreator<FSetOwnerInventoryPayload>
setOwnerInventoryActionCreator() {
  static auto C = rtk::createAction<FSetOwnerInventoryPayload>(
      TEXT("testgame/inventory/setOwnerInventory"));
  return C;
}

inline rtk::AnyAction setOwnerInventory(const FSetOwnerInventoryPayload &P) {
  return setOwnerInventoryActionCreator()(P);
}

} // namespace InventoryActions
} // namespace TestGame
