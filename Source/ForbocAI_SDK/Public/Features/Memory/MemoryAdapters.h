#pragma once
/**
 * Memory adapters own memory entity indexing.
 */

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"

namespace MemorySlice {

using namespace rtk;
using namespace func;

/**
 * Returns the entity id for a memory item.
 * User Story: As memory entity adapters, I need a stable id selector so memory
 * items can be indexed and updated by identifier.
 */
inline FString MemoryItemIdSelector(const FMemoryItem &Item) { return Item.Id; }

/**
 * Returns the entity adapter used to manage memory records by id.
 * User Story: As memory reducers and selectors, I need one shared adapter so
 * entity operations stay consistent across the slice.
 */
inline rtk::EntityAdapter<FMemoryItem> GetMemoryAdapter() {
  return rtk::createEntityAdapter<FMemoryItem>(&MemoryItemIdSelector);
}

} // namespace MemorySlice
