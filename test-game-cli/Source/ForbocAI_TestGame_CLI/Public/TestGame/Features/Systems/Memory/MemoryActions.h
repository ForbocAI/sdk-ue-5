#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Memory/MemoryTypes.h"

namespace TestGame {
namespace GameMemoryActions {

inline rtk::ActionCreator<FMemoryRecord> storeMemoryActionCreator() {
  static auto C =
      rtk::createAction<FMemoryRecord>(TEXT("testgame/memory/storeMemory"));
  return C;
}

inline rtk::ActionCreator<FString> clearMemoryForNpcActionCreator() {
  static auto C =
      rtk::createAction<FString>(TEXT("testgame/memory/clearMemoryForNpc"));
  return C;
}

inline rtk::AnyAction storeMemory(const FMemoryRecord &R) {
  return storeMemoryActionCreator()(R);
}

inline rtk::AnyAction clearMemoryForNpc(const FString &NpcId) {
  return clearMemoryForNpcActionCreator()(NpcId);
}

} // namespace GameMemoryActions
} // namespace TestGame
