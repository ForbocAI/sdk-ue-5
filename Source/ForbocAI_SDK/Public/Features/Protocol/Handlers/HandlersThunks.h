#pragma once

#include "Features/Memory/MemoryThunks.h"
#include "Features/Protocol/Handlers/HandlersAdapters.h"

#include <memory>

namespace rtk {

inline FProtocolHandlerContext LocalProtocolHandlerContext() {
  FProtocolHandlerContext Context;
  Context.StoreMemory = [](const FMemoryItem &Item) {
    return nodeMemoryStoreThunk(Item);
  };
  Context.RecallMemory = [](const FMemoryRecallRequest &Request) {
    return nodeMemoryRecallThunk(Request);
  };
  return Context;
}

inline FProtocolHandlerContext InMemoryProtocolHandlerContext() {
  const std::shared_ptr<TArray<FMemoryItem>> Memories =
      std::make_shared<TArray<FMemoryItem>>();

  FProtocolHandlerContext Context;
  Context.StoreMemory = [Memories](const FMemoryItem &Item) {
    return [Memories, Item](std::function<AnyAction(const AnyAction &)> Dispatch,
                            std::function<const FRuntimeState &()> GetState)
               -> func::AsyncResult<FMemoryItem> {
      Dispatch(MemorySlice::Actions::memoryStoreStart());
      Memories->Add(Item);
      Dispatch(MemorySlice::Actions::memoryStoreSuccess(Item));
      return detail::ResolveAsync(Item);
    };
  };
  Context.RecallMemory = [Memories](const FMemoryRecallRequest &Request) {
    return [Memories, Request](
               std::function<AnyAction(const AnyAction &)> Dispatch,
               std::function<const FRuntimeState &()> GetState)
               -> func::AsyncResult<TArray<FMemoryItem>> {
      Dispatch(MemorySlice::Actions::memoryRecallStart());
      TArray<FMemoryItem> Results;
      const int32 Limit = Request.Limit <= 0 ? Memories->Num() : Request.Limit;
      const int32 Count = FMath::Min(Limit, Memories->Num());

      struct PopulateResults {
        static void apply(const TArray<FMemoryItem> &Source, int32 Index,
                          int32 Count, TArray<FMemoryItem> &Output) {
          Index >= Count
              ? void()
              : (Output.Add(Source[Index]),
                 apply(Source, Index + 1, Count, Output), void());
        }
      };
      PopulateResults::apply(*Memories, 0, Count, Results);
      Dispatch(MemorySlice::Actions::memoryRecallSuccess(Results));
      return detail::ResolveAsync(Results);
    };
  };
  return Context;
}

} // namespace rtk
