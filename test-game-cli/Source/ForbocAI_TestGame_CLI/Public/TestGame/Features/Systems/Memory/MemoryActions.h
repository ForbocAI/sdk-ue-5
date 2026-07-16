#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"

namespace TestGame {
namespace GameMemoryActions {

/** User Story: As a features systems memory consumer, I need to invoke store memory action creator through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::ActionCreator<FMemoryRecord> storeMemoryActionCreator() */
inline rtk::ActionCreator<FMemoryRecord> storeMemoryActionCreator() {
  static auto C =
      rtk::createAction<FMemoryRecord>(TEXT("testgame/memory/storeMemory"));
  return C;
}

/** User Story: As a features systems memory consumer, I need to invoke clear memory for npc action creator through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> clearMemoryForNpcActionCreator() */
inline rtk::ActionCreator<FString> clearMemoryForNpcActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/memory/clearMemoryForNpc"));
  return C;
}

/** User Story: As a features systems memory consumer, I need to invoke store memory through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::AnyAction storeMemory(const FMemoryRecord &R) */
inline rtk::AnyAction storeMemory(const FMemoryRecord &R) {
  return storeMemoryActionCreator()(R);
}

/** User Story: As a features systems memory consumer, I need to invoke clear memory for npc through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::AnyAction clearMemoryForNpc(const FString &NpcId) */
inline rtk::AnyAction clearMemoryForNpc(const FString &NpcId) {
  return clearMemoryForNpcActionCreator()(NpcId);
}

} // namespace GameMemoryActions
} // namespace TestGame
