#pragma once

#include "Features/Memory/MemoryThunks.h"
#include "Features/Protocol/Handlers/HandlersAdapters.h"

namespace rtk {

/** User Story: As protocol execution, I need memory handlers bound to the NPC database so every turn reads and writes SDK-owned vector storage. @fn inline FProtocolHandlerContext LocalProtocolHandlerContext(const FString &DatabaseName) */
inline FProtocolHandlerContext
LocalProtocolHandlerContext(const FString &DatabaseName) {
  FProtocolHandlerContext Context;
  Context.StoreMemory = [DatabaseName](const FMemoryItem &Item) {
    return upsertNodeMemoryThunk(Item, DatabaseName);
  };
  Context.RecallMemory = [DatabaseName](const FMemoryRecallRequest &Request) {
    return recallNodeMemoryThunk(Request.Query, Request.Limit,
                                 Request.Threshold, DatabaseName);
  };
  return Context;
}

} // namespace rtk
