#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Systems/Vector/VectorAdapters.h"
#include "Entities/Vector/VectorSelectors.h"

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
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD96FBF20CAEC),
          [](const FEmptyPayload &, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<FEmptyPayload> {
            const TArray<float> Probe = VectorAdapters::embedVectorAdapter(
                TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF703575AD259));
            return Probe.Num() == FORBOCAI_SDK_AUTHORED_NUMBERVBD585E4075C4
                       ? detail::ResolveAsync(FEmptyPayload{})
                       : detail::RejectAsync<FEmptyPayload>(
                             TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E15DC160CB4));
          }, Condition);
  return Thunk;
}

/** User Story: As a features vector consumer, I need to invoke generate embedding thunk through a stable signature so the features vector workflow remains explicit and composable. @fn inline const AsyncThunkConfig<TArray<float>, FString, FRuntimeState> & generateEmbeddingThunk() */
inline const AsyncThunkConfig<TArray<float>, FString, FRuntimeState> &
generateEmbeddingThunk() {
  static const AsyncThunkConfig<TArray<float>, FString, FRuntimeState> Thunk =
      rtk::createAsyncThunk<TArray<float>, FString, FRuntimeState>(
          TEXT(FORBOCAI_SDK_AUTHORED_STRINGVD83C1BC68A82),
          [](const FString &Text, const ThunkApi<FRuntimeState> &)
              -> func::AsyncResult<TArray<float>> {
            const TArray<float> Embedding =
                VectorAdapters::embedVectorAdapter(Text);
            return Embedding.Num() == FORBOCAI_SDK_AUTHORED_NUMBERVBD585E4075C4
                       ? detail::ResolveAsync(Embedding)
                       : detail::RejectAsync<TArray<float>>(
                             TEXT(FORBOCAI_SDK_AUTHORED_STRINGV3E15DC160CB4));
          });
  return Thunk;
}

} // namespace rtk
