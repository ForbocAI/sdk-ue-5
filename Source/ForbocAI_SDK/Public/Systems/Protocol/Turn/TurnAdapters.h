#pragma once

#include "Systems/Protocol/Serialization/ProtocolSerializationAdapters.h"

namespace rtk::detail {

/**
 * User Story: As protocol persistence, I need memory effects parameterized by the package state so feature headers never import the root store.
 * @fn template <typename RuntimeState> inline func::AsyncResult<rtk::FEmptyPayload> PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions, int32 Index, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState)
 */
template <typename RuntimeState>
inline func::AsyncResult<rtk::FEmptyPayload>
PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions,
                          int32 Index, const FProtocolHandlerContext &Runtime,
                          std::function<AnyAction(const AnyAction &)> Dispatch,
                          std::function<const RuntimeState &()> GetState);

/** User Story: As protocol execution, I need turn recursion parameterized by package state so feature headers remain independent of the root store. @fn template <typename RuntimeState> func::AsyncResult<FAgentResponse> RunProtocolTurn(const FString &NpcId, const FString &Input, const FString &RunId, const FNPCProcessTape &Tape, const FString &LastResult, bool bHasLastResult, int32 Turn, const FProtocolHandlerContext &Runtime, std::function<AnyAction(const AnyAction &)> Dispatch, std::function<const RuntimeState &()> GetState) */
template <typename RuntimeState>
func::AsyncResult<FAgentResponse>
RunProtocolTurn(const FString &NpcId, const FString &Input,
                const FString &RunId, const FNPCProcessTape &Tape,
                const FString &LastResult, bool bHasLastResult,
                int32 Turn, const FProtocolHandlerContext &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const RuntimeState &()> GetState);

} // namespace rtk::detail
