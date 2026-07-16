#pragma once

#include "Core/rtk.hpp"
#include "Features/Vector/VectorAdapters.h"
#include "Features/Vector/VectorSelectors.h"

struct FRuntimeState;

namespace rtk {

/** User Story: As a features vector consumer, I need to invoke init vector thunk through a stable signature so the features vector workflow remains explicit and composable. @fn inline const AsyncThunkConfig<FEmptyPayload, FEmptyPayload, FRuntimeState> & initVectorThunk() */
inline const AsyncThunkConfig<FEmptyPayload, FEmptyPayload, FRuntimeState> &
initVectorThunk() {
  const ConditionCallback<FEmptyPayload, FRuntimeState> Condition =
      [](const FEmptyPayload &, const ThunkApi<FRuntimeState> &Api) {
        return !VectorSelectors::selectVectorBusy(Api.getState());
      };
  static const AsyncThunkConfig<FEmptyPayload, FEmptyPayload, FRuntimeState>
      Thunk = rtk::createAsyncThunk<FEmptyPayload, FEmptyPayload, FRuntimeState>(
          TEXT("node/vector/init"),
          [](const FEmptyPayload &, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<FEmptyPayload> {
            const TArray<float> Probe = VectorAdapters::embedVectorAdapter(
                TEXT("vector-readiness"));
            return Probe.Num() == 384
                       ? detail::ResolveAsync(FEmptyPayload{})
                       : detail::RejectAsync<FEmptyPayload>(
                             TEXT("SDK vectorizer returned an invalid dimension"));
          }, Condition);
  return Thunk;
}

/** User Story: As a features vector consumer, I need to invoke generate embedding thunk through a stable signature so the features vector workflow remains explicit and composable. @fn inline const AsyncThunkConfig<TArray<float>, FString, FRuntimeState> & generateEmbeddingThunk() */
inline const AsyncThunkConfig<TArray<float>, FString, FRuntimeState> &
generateEmbeddingThunk() {
  static const AsyncThunkConfig<TArray<float>, FString, FRuntimeState> Thunk =
      rtk::createAsyncThunk<TArray<float>, FString, FRuntimeState>(
          TEXT("node/vector/generate"),
          [](const FString &Text, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<TArray<float>> {
            const TArray<float> Embedding =
                VectorAdapters::embedVectorAdapter(Text);
            return Embedding.Num() == 384
                       ? detail::ResolveAsync(Embedding)
                       : detail::RejectAsync<TArray<float>>(
                             TEXT("SDK vectorizer returned an invalid dimension"));
          });
  return Thunk;
}

} // namespace rtk
