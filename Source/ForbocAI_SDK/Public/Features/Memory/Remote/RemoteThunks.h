#pragma once

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "Features/API/APIApi.h"
#include "Features/Memory/MemorySlice.h"

namespace rtk {

inline ThunkAction<rtk::FEmptyPayload, FRuntimeState>
storeMemoryRemoteThunk(const FString &NpcId, const FString &Observation,
                       float Importance = 0.8f) {
  return [NpcId, Observation,
          Importance](std::function<AnyAction(const AnyAction &)> Dispatch,
                      std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<rtk::FEmptyPayload> {
    return APISlice::Endpoints::postMemoryStore(
        NpcId, TypeFactory::RemoteMemoryStoreRequest(Observation, Importance))(
        Dispatch, GetState);
  };
}

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
listMemoryRemoteThunk(const FString &NpcId) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FMemoryItem>> {
    return func::AsyncChain::then<TArray<FMemoryItem>, TArray<FMemoryItem>>(
        APISlice::Endpoints::getMemoryList(NpcId)(Dispatch, GetState),
        [Dispatch](const TArray<FMemoryItem> &Items) {
          Dispatch(MemorySlice::Actions::memoryRecallSuccess(Items));
          return detail::ResolveAsync(Items);
        });
  };
}

inline ThunkAction<TArray<FMemoryItem>, FRuntimeState>
recallMemoryRemoteThunk(const FString &NpcId, const FString &Query,
                        float Similarity = 0.0f) {
  return [NpcId, Query,
          Similarity](std::function<AnyAction(const AnyAction &)> Dispatch,
                      std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<TArray<FMemoryItem>> {
    Dispatch(MemorySlice::Actions::memoryRecallStart());
    return func::AsyncChain::then<TArray<FMemoryItem>, TArray<FMemoryItem>>(
               APISlice::Endpoints::postMemoryRecall(
                   NpcId, TypeFactory::RemoteMemoryRecallRequest(
                              Query, Similarity))(Dispatch, GetState),
               [Dispatch](const TArray<FMemoryItem> &Items) {
                 Dispatch(MemorySlice::Actions::memoryRecallSuccess(Items));
                 return detail::ResolveAsync(Items);
               })
        .catch_([Dispatch](std::string Error) {
          Dispatch(MemorySlice::Actions::memoryRecallFailed(
              FString(UTF8_TO_TCHAR(Error.c_str()))));
        });
  };
}

inline ThunkAction<rtk::FEmptyPayload, FRuntimeState>
clearMemoryRemoteThunk(const FString &NpcId) {
  return [NpcId](std::function<AnyAction(const AnyAction &)> Dispatch,
                 std::function<const FRuntimeState &()> GetState)
             -> func::AsyncResult<rtk::FEmptyPayload> {
    return func::AsyncChain::then<rtk::FEmptyPayload, rtk::FEmptyPayload>(
        APISlice::Endpoints::deleteMemoryClear(NpcId)(Dispatch, GetState),
        [Dispatch](const rtk::FEmptyPayload &Payload) {
          Dispatch(MemorySlice::Actions::memoryClear());
          return detail::ResolveAsync(Payload);
        });
  };
}

} // namespace rtk
