#pragma once

#include "Containers/SharedString.h"
#include "TestGame/Features/Components/Inventory/InventoryTypes.h"
#include "TestGame/Features/Data/DataAdapters.h"

namespace TestGame::InventoryAdapters {

/** User Story: As an inventory consumer, I need initial owner inventories loaded from authored data so feature types and reducers contain no scenario content. @fn inline FInventoryState ReadInitialInventoryState() */
inline FInventoryState ReadInitialInventoryState() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("components/inventory/inventory.json"));
  const TSharedRef<FJsonObject> InitialState =
      DataAdapters::ReadObjectField(Source, TEXT("initialState"));
  const TSharedRef<FJsonObject> ByOwner =
      DataAdapters::ReadObjectField(InitialState, TEXT("byOwner"));
  TArray<UE::FSharedString> AuthoredOwnerIds;
  ByOwner->Values.GetKeys(AuthoredOwnerIds);
  const TArray<FString> OwnerIds =
      func::map_array<UE::FSharedString, FString>(
          AuthoredOwnerIds,
          [](const UE::FSharedString &OwnerId) {
            return FString(*OwnerId);
          });
  return FInventoryState{func::fold_array<FString, TMap<FString, TArray<FString>>>(
      OwnerIds, {}, [&ByOwner](const TMap<FString, TArray<FString>> &Owners,
                              const FString &OwnerId) {
        return func::upsert_map_value<FString, TArray<FString>>(
            Owners, OwnerId, {}, [&ByOwner, &OwnerId](const TArray<FString> &) {
              return DataAdapters::ReadStringArrayField(ByOwner, OwnerId);
            });
      })};
}

/** User Story: As an inventory consumer, I need one immutable initial-state value so every new root store starts from the same authored inventory. @fn inline const FInventoryState &InitialInventoryState() */
inline const FInventoryState &InitialInventoryState() {
  static const FInventoryState State = ReadInitialInventoryState();
  return State;
}

} // namespace TestGame::InventoryAdapters
