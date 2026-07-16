#pragma once

#include "Features/API/APIApi.h"
#include "Features/API/Serialization/APISerializationAdapters.h"

namespace ProtocolHandlersLiveTestAdapters {

/** User Story: As a live protocol test maintainer, I need instruction names through a stable signature so transition failures report the exact API stage. @fn inline FString InstructionTypeName(const ENPCInstructionType Type) */
inline FString InstructionTypeName(const ENPCInstructionType Type) {
  return Type == ENPCInstructionType::IdentifyActor
             ? TEXT("IdentifyActor")
             : Type == ENPCInstructionType::QueryVector
                   ? TEXT("QueryVector")
                   : Type == ENPCInstructionType::Decision
                         ? TEXT("Decision")
                         : Type == ENPCInstructionType::Reasoning
                               ? TEXT("Reasoning")
                               : TEXT("Finalize");
}

/** User Story: As a live protocol test maintainer, I need stage mismatch messages through a stable signature so failed API transitions remain diagnosable. @fn inline FString ExpectedInstructionError(const FString &Expected, const ENPCInstructionType Actual) */
inline FString ExpectedInstructionError(const FString &Expected,
                                        const ENPCInstructionType Actual) {
  return FString::Printf(TEXT("Expected %s, got %s"), *Expected,
                         *InstructionTypeName(Actual));
}

/** User Story: As a live protocol test maintainer, I need prior results serialized through a stable signature so each real API turn receives canonical JSON. @fn inline FString LastResultJson(const TSharedPtr<FJsonObject> &Object) */
inline FString LastResultJson(const TSharedPtr<FJsonObject> &Object) {
  return JsonInterop::StringifyObject(Object);
}

} // namespace ProtocolHandlersLiveTestAdapters
