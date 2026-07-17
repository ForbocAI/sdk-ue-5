#pragma once

#include "TestGame/Features/Components/Inventory/InventoryTypes.h"

namespace TestGame::InventorySelectors {

/** User Story: As an inventory consumer, I need all owner inventories selected without exposing reducer implementation details. @fn inline TMap<FString, TArray<FString>> SelectInventoryByOwner(const FInventoryState &State) */
inline TMap<FString, TArray<FString>>
SelectInventoryByOwner(const FInventoryState &State) {
  return State.ByOwner;
}

/** User Story: As an inventory consumer, I need one owner's items selected from root-owned state so views and systems never query authored files directly. @fn inline TArray<FString> SelectOwnerInventory(const FInventoryState &State, const FString &OwnerId) */
inline TArray<FString> SelectOwnerInventory(const FInventoryState &State,
                                            const FString &OwnerId) {
  const TArray<FString> *Items = State.ByOwner.Find(OwnerId);
  return Items == nullptr ? TArray<FString>() : *Items;
}

} // namespace TestGame::InventorySelectors
