#pragma once

#include "Features/Protocol/Serialization/ProtocolSerializationAdapters.h"

namespace rtk::detail {

inline func::AsyncResult<rtk::FEmptyPayload>
PersistMemoryInstructions(const TArray<FMemoryStoreInstruction> &Instructions,
                          int32 Index, const FProtocolHandlerContext &Runtime,
                          std::function<AnyAction(const AnyAction &)> Dispatch,
                          std::function<const FRuntimeState &()> GetState);

func::AsyncResult<FAgentResponse>
RunProtocolTurn(const FString &NpcId, const FString &Input,
                const FString &RunId, const FNPCProcessTape &Tape,
                const FString &LastResult, bool bHasLastResult,
                int32 Turn, const FProtocolHandlerContext &Runtime,
                std::function<AnyAction(const AnyAction &)> Dispatch,
                std::function<const FRuntimeState &()> GetState);

} // namespace rtk::detail
