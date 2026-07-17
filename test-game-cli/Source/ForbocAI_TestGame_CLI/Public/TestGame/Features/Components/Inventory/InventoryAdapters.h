#pragma once

#include "Containers/SharedString.h"
#include "TestGame/Features/Components/Inventory/InventoryTypes.h"
#include "TestGame/Features/Data/DataAdapters.h"

namespace TestGame::InventoryAdapters {
namespace detail {

/** User Story: As an inventory maintainer, I need each authored owner inventory decoded recursively so map construction stays deterministic and side effects remain local to the adapter. @fn inline FInventoryState ReadOwners(const TSharedRef<FJsonObject> &ByOwner, const TArray<FString> &OwnerIds, int32 Index, FInventoryState State) */
inline FInventoryState ReadOwners(const TSharedRef<FJsonObject> &ByOwner,
                                  const TArray<FString> &OwnerIds,
                                  int32 Index, FInventoryState State) {
  return Index >= OwnerIds.Num()
             ? State
             : (State.ByOwner.Add(
                    OwnerIds[Index], DataAdapters::ReadStringArrayField(
                                         ByOwner, OwnerIds[Index])),
                ReadOwners(ByOwner, OwnerIds, Index + 1, MoveTemp(State)));
}

} // namespace detail

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
  return detail::ReadOwners(ByOwner, OwnerIds, 0, {});
}

/** User Story: As an inventory consumer, I need one immutable initial-state value so every new root store starts from the same authored inventory. @fn inline const FInventoryState &InitialInventoryState() */
inline const FInventoryState &InitialInventoryState() {
  static const FInventoryState State = ReadInitialInventoryState();
  return State;
}

} // namespace TestGame::InventoryAdapters
