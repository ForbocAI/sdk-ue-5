#pragma once

#include "Core/rtk.hpp"
#include "Components/Memory/MemoryTypes.h"

struct FRuntimeState;

namespace rtk {

struct FProtocolHandlerContext {
  std::function<ThunkAction<FMemoryItem, FRuntimeState>(const FMemoryItem &)>
      StoreMemory;
  std::function<ThunkAction<TArray<FMemoryItem>, FRuntimeState>(
      const FMemoryRecallRequest &)>
      RecallMemory;

  /** User Story: As a features protocol handlers consumer, I need to invoke has memory through a stable signature so the features protocol handlers workflow remains explicit and composable. @fn bool HasMemory() const */
  bool HasMemory() const {
    return static_cast<bool>(StoreMemory) && static_cast<bool>(RecallMemory);
  }
};

} // namespace rtk
