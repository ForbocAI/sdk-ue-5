#pragma once

#include "Core/rtk.hpp"
#include "Features/Memory/MemoryTypes.h"

struct FRuntimeState;

// Handler classification is pinned by the canonical instruction sequence.
//
// | Instruction        | Classification | Reason                                                       |
// | ------------------ | -------------- | ------------------------------------------------------------ |
// | IdentifyActor      | Local          | SDK resolves actor information from the host registry        |
// | QueryVector        | Local          | SDK runs the configured memory-engine recall                 |
// | Decision           | Local          | SDK applies the local decision policy                        |
// | Reasoning          | Pass-through   | API hosts the SLM; SDK acknowledges and continues            |
// | Finalize           | Local          | SDK persists memory, applies state, and dispatches verdict   |

namespace rtk {

struct FProtocolHandlerContext {
  std::function<ThunkAction<FMemoryItem, FRuntimeState>(const FMemoryItem &)>
      StoreMemory;
  std::function<ThunkAction<TArray<FMemoryItem>, FRuntimeState>(
      const FMemoryRecallRequest &)>
      RecallMemory;

  bool HasMemory() const {
    return static_cast<bool>(StoreMemory) && static_cast<bool>(RecallMemory);
  }
};

} // namespace rtk
