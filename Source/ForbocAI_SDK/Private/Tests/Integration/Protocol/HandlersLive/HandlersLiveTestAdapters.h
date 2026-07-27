#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Systems/API/APIApi.h"
#include "Systems/API/Serialization/APISerializationAdapters.h"

namespace ProtocolHandlersLiveTestAdapters {

/** User Story: As a live protocol test maintainer, I need instruction names through a stable signature so transition failures report the exact API stage. @fn inline FString InstructionTypeName(const ENPCInstructionType Type) */
inline FString InstructionTypeName(const ENPCInstructionType Type) {
  return Type == ENPCInstructionType::IdentifyActor
             ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV9648236DCEE6)
             : Type == ENPCInstructionType::QueryVector
                   ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGVA1B9D4047349)
                   : Type == ENPCInstructionType::Decision
                         ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV6B5B6E4359EF)
                         : Type == ENPCInstructionType::Reasoning
                               ? TEXT(FORBOCAI_SDK_AUTHORED_STRINGV128754029BBF)
                               : TEXT(FORBOCAI_SDK_AUTHORED_STRINGV1EFFA2D43442);
}

/** User Story: As a live protocol test maintainer, I need stage mismatch messages through a stable signature so failed API transitions remain diagnosable. @fn inline FString ExpectedInstructionError(const FString &Expected, const ENPCInstructionType Actual) */
inline FString ExpectedInstructionError(const FString &Expected,
                                        const ENPCInstructionType Actual) {
  return FString::Printf(TEXT(FORBOCAI_SDK_AUTHORED_STRINGV5D8A2F625E59), *Expected,
                         *InstructionTypeName(Actual));
}

/** User Story: As a live protocol test maintainer, I need prior results serialized through a stable signature so each real API turn receives canonical JSON. @fn inline FString LastResultJson(const TSharedPtr<FJsonObject> &Object) */
inline FString LastResultJson(const TSharedPtr<FJsonObject> &Object) {
  return JsonInterop::StringifyObject(Object);
}

} // namespace ProtocolHandlersLiveTestAdapters
