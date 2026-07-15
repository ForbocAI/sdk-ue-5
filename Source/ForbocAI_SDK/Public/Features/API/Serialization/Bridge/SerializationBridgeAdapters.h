#pragma once

#include "Features/API/Serialization/Agent/AgentAdapters.h"

namespace JsonInterop {

inline TSharedRef<FJsonObject>
ValidationContextToObject(const FBridgeValidationContext &Context) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (SetFieldFromJsonString(Object, TEXT("npcState"),
                                 Context.NpcStateJson),
          SetFieldFromJsonString(Object, TEXT("worldState"),
                                 Context.WorldStateJson),
          SetFieldFromJsonString(Object, TEXT("constraints"),
                                 Context.ConstraintsJson),
          Object);
}

inline FValidationResult
ValidationResultFromObject(const TSharedPtr<FJsonObject> &Object) {
  FValidationResult Result;
  return !Object.IsValid()
             ? (Result.bValid = false,
                Result.Reason = TEXT("Invalid validation response"), Result)
             : (Result.bValid = detail::TryGetBoolAs(Object, TEXT("valid"),
                                                     Result.bValid),
                Result.Reason =
                    OptionalStringFromField(Object, TEXT("reason")),
                Object->HasTypedField<EJson::Object>(TEXT("correctedAction"))
                    ? (Result.CorrectedAction = ActionFromObject(
                           Object->GetObjectField(TEXT("correctedAction"))),
                       void())
                    : void(),
                Result);
}

} // namespace JsonInterop
