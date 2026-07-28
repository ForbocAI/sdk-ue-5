#pragma once

#include "Systems/Memory/MemoryThunks.h"
#include "Systems/Protocol/Handlers/HandlersAdapters.h"

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

/** User Story: As the decide-only npc turn, I need a protocol context with no bound memory (HasMemory() == false) so the decision engine runs without reading or writing the vector db, keeping recall and store as separate composed commands. @fn inline FProtocolHandlerContext EphemeralProtocolHandlerContext() */
inline FProtocolHandlerContext EphemeralProtocolHandlerContext() {
  return FProtocolHandlerContext();
}

} // namespace rtk
