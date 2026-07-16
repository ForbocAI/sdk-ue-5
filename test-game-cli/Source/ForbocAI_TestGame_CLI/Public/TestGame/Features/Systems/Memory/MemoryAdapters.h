#pragma once
/**
 * Test-game memory adapters own normalized memory indexing.
 */

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"

namespace TestGame {

/**
 * Returns the entity adapter used for game-side memory records.
 * User Story: As test-game memory reducers, I need one shared adapter so local
 * memory records use a consistent normalized state structure.
 * @fn inline rtk::EntityAdapter<FMemoryRecord> &GetGameMemoryAdapter()
 */
inline rtk::EntityAdapter<FMemoryRecord> &GetGameMemoryAdapter() {
  static rtk::EntityAdapter<FMemoryRecord> Adapter =
      rtk::createEntityAdapter<FMemoryRecord>(
          [](const FMemoryRecord &R) { return R.Id; });
  return Adapter;
}

} // namespace TestGame
