#pragma once
#include "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Systems/Memory/MemoryTypes.h"

namespace MicroGame {
namespace GameMemoryActions {

/** User Story: As a features systems memory consumer, I need to invoke store memory action creator through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::ActionCreator<FMemoryRecord> storeMemoryActionCreator() */
inline rtk::ActionCreator<FMemoryRecord> storeMemoryActionCreator() {
  static auto C =
      rtk::createAction<FMemoryRecord>(TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGVA091263BB4DA));
  return C;
}

/** User Story: As a features systems memory consumer, I need to invoke clear memory for npc action creator through a stable signature so the features systems memory workflow remains explicit and composable. @fn inline rtk::ActionCreator<FString> clearMemoryForNpcActionCreator() */
inline rtk::ActionCreator<FString> clearMemoryForNpcActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT(FORBOCAI_SDKCLI_AUTHORED_STRINGV129589CC9477));
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
} // namespace MicroGame
