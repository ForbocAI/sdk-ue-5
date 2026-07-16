#pragma once

#include "Features/API/Serialization/Agent/AgentAdapters.h"
#include "Features/API/Serialization/Bridge/Configuration/ConfigurationAdapters.h"

namespace JsonInterop {

/** User Story: As a Bridge caller, I need validation context encoded through canonical authored fields so transport does not own host-domain names. @fn inline TSharedRef<FJsonObject> ValidationContextToObject(const FBridgeValidationContext &Context) */
inline TSharedRef<FJsonObject>
ValidationContextToObject(const FBridgeValidationContext &Context) {
  const BridgeConfiguration::FBridgeSerializationData &Data =
      BridgeConfiguration::bridgeSerializationData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (SetFieldFromJsonString(Object, Data.Context.NpcState,
                                 Context.NpcStateJson),
          SetFieldFromJsonString(Object, Data.Context.WorldState,
                                 Context.WorldStateJson),
          SetFieldFromJsonString(Object, Data.Context.Constraints,
                                 Context.ConstraintsJson),
          Object);
}

/** User Story: As a Bridge caller, I need canonical validation results decoded atomically so malformed corrected actions cannot masquerade as successful responses. @fn inline func::Maybe<FValidationResult> DecodeValidationResultObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FValidationResult>
DecodeValidationResultObject(const TSharedPtr<FJsonObject> &Object) {
  const BridgeConfiguration::FBridgeSerializationData &Data =
      BridgeConfiguration::bridgeSerializationData();
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::Boolean>(Data.ValidationResult.Valid) &&
      HasOptionalFieldType(Object, Data.ValidationResult.Reason,
                           EJson::String) &&
      HasOptionalFieldType(Object, Data.ValidationResult.CorrectedAction,
                           EJson::Object);
  return !bValid
             ? func::nothing<FValidationResult>()
             : [&]() {
                 FValidationResult Result;
                 Result.bValid =
                     Object->GetBoolField(Data.ValidationResult.Valid);
                 Result.Reason = OptionalStringFromField(
                     Object, Data.ValidationResult.Reason);
                 return !HasNonNullField(
                            Object, Data.ValidationResult.CorrectedAction)
                            ? func::just(Result)
                            : func::match(
                                  DecodeActionObject(Object->GetObjectField(
                                      Data.ValidationResult.CorrectedAction)),
                                  [&Result](const FAgentAction &Action) {
                                    Result.CorrectedAction = Action;
                                    return func::just(Result);
                                  },
                                  []() {
                                    return func::nothing<FValidationResult>();
                                  });
               }();
}

} // namespace JsonInterop
