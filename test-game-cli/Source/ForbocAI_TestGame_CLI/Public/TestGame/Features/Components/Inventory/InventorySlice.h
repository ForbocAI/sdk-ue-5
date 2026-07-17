#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Components/Inventory/InventoryActions.h"
#include "TestGame/Features/Components/Inventory/InventoryAdapters.h"
#include "TestGame/Features/Components/Inventory/InventoryTypes.h"

namespace TestGame {

/** User Story: As a features components inventory consumer, I need to invoke create inventory slice through a stable signature so the features components inventory workflow remains explicit and composable. @fn inline rtk::Slice<FInventoryState> CreateInventorySlice() */
inline rtk::Slice<FInventoryState> CreateInventorySlice() {
  return rtk::createSlice<FInventoryState>(
      TEXT("testgame/inventory"), InventoryAdapters::InitialInventoryState(),
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
