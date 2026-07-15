#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Components/Inventory/InventoryActions.h"
#include "TestGame/Features/Components/Inventory/InventoryTypes.h"

namespace TestGame {

namespace InventorySelectors {
inline TMap<FString, TArray<FString>>
SelectInventoryByOwner(const FInventoryState &S) {
  return S.ByOwner;
}

inline TArray<FString> SelectOwnerInventory(const FInventoryState &S,
                                            const FString &OwnerId) {
  const TArray<FString> *Items = S.ByOwner.Find(OwnerId);
  return Items == nullptr ? TArray<FString>() : *Items;
}
} // namespace InventorySelectors

inline rtk::Slice<FInventoryState> CreateInventorySlice() {
  return rtk::createSlice<FInventoryState>(
      TEXT("testgame/inventory"), FInventoryState(),
      [](rtk::ActionReducerMapBuilder<FInventoryState> &Builder) {
        Builder.addCase(
            InventoryActions::setOwnerInventoryActionCreator(),
            [](const FInventoryState &S,
               const rtk::Action<FSetOwnerInventoryPayload> &A)
                -> FInventoryState {
              FInventoryState Next = S;
              Next.ByOwner.Add(A.PayloadValue.OwnerId, A.PayloadValue.Items);
              return Next;
            });
      });
}

} // namespace TestGame
