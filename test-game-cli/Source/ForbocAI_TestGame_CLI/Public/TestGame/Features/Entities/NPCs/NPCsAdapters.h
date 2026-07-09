#pragma once
/**
 * Test-game adapters own normalized entity indexing.
 */

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsTypes.h"

namespace TestGame {

/**
 * Returns the entity adapter used for test-game NPC state.
 * User Story: As test-game entity reducers, I need one shared adapter so NPC
 * CRUD actions update and query a consistent normalized state shape.
 */
inline rtk::EntityAdapter<FGameNPC> &GetNPCAdapter() {
  static rtk::EntityAdapter<FGameNPC> Adapter =
      rtk::createEntityAdapter<FGameNPC>(
          [](const FGameNPC &N) { return N.Id; });
  return Adapter;
}

} // namespace TestGame
