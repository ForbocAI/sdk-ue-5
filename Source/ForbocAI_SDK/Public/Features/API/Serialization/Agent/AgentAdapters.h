#pragma once

#include "Core/JsonInterop.h"
#include "Features/API/Serialization/Agent/Configuration/ConfigurationAdapters.h"
#include "Features/Contracts/ContractsTypes.h"

namespace JsonInterop {

/**
 * User Story: As a structured-persona codec, I need one field's values traversed atomically so mixed arrays cannot be accepted as persona slots.
 * @fn inline func::Maybe<TArray<FString>> DecodePersonaStringArrayField( const TSharedPtr<FJsonObject> &Object, const FString &Field)
 */
inline func::Maybe<TArray<FString>> DecodePersonaStringArrayField(
    const TSharedPtr<FJsonObject> &Object, const FString &Field) {
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  return !Object.IsValid() || !Object->TryGetArrayField(Field, Values) || !Values
             ? func::nothing<TArray<FString>>()
             : func::traverse_maybe_array<TSharedPtr<FJsonValue>, FString>(
                   *Values, [](const TSharedPtr<FJsonValue> &Value) {
                     return Value.IsValid() && Value->Type == EJson::String
                                ? func::just(Value->AsString())
                                : func::nothing<FString>();
                   });
}

/** User Story: As an Agent transport consumer, I need every structured-persona slot validated before its canonical JSON enters protocol state. @fn inline func::Maybe<FString> DecodeStructuredPersonaObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FString>
DecodeStructuredPersonaObject(const TSharedPtr<FJsonObject> &Object) {
  const AgentConfiguration::FAgentContractData &Data =
      AgentConfiguration::agentContractData();
  const TArray<func::Maybe<TArray<FString>>> Fields{
      DecodePersonaStringArrayField(Object, Data.Persona.Traits),
      DecodePersonaStringArrayField(Object, Data.Persona.Goals),
      DecodePersonaStringArrayField(Object, Data.Persona.Relationships),
      DecodePersonaStringArrayField(Object, Data.Persona.World),
      DecodePersonaStringArrayField(Object, Data.Persona.SpeakingStyle),
      DecodePersonaStringArrayField(Object, Data.Persona.Constraints)};
  const func::Maybe<TArray<TArray<FString>>> Decoded =
      func::traverse_maybe_array<func::Maybe<TArray<FString>>,
                                 TArray<FString>>(
          Fields, [](const func::Maybe<TArray<FString>> &Value) {
            return Value;
          });
  return func::is_nothing(Decoded)
             ? func::nothing<FString>()
             : func::just(StringifyObject(Object));
}

/** User Story: As a api serialization agent consumer, I need to invoke empty structured persona object through a stable signature so the api serialization agent workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> EmptyStructuredPersonaObject() */
inline TSharedRef<FJsonObject> EmptyStructuredPersonaObject() {
  const AgentConfiguration::FAgentContractData &Data =
      AgentConfiguration::agentContractData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (SetStringArrayField(Object, Data.Persona.Traits, TArray<FString>()),
          SetStringArrayField(Object, Data.Persona.Goals, TArray<FString>()),
          SetStringArrayField(Object, Data.Persona.Relationships,
                              TArray<FString>()),
          SetStringArrayField(Object, Data.Persona.World, TArray<FString>()),
          SetStringArrayField(Object, Data.Persona.SpeakingStyle,
                              TArray<FString>()),
          SetStringArrayField(Object, Data.Persona.Constraints,
                              TArray<FString>()),
          Object);
}

/** User Story: As a api serialization agent consumer, I need to invoke structured persona to object through a stable signature so the api serialization agent workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> StructuredPersonaToObject(const FString &PersonaOrJson) */
inline TSharedRef<FJsonObject>
StructuredPersonaToObject(const FString &PersonaOrJson) {
  const AgentConfiguration::FAgentContractData &Data =
      AgentConfiguration::agentContractData();
  TSharedPtr<FJsonObject> Parsed;
  return ParseJsonObject(PersonaOrJson, Parsed) && Parsed.IsValid() &&
                 Parsed->HasField(Data.Persona.Traits)
             ? Parsed.ToSharedRef()
             : [&]() {
                 const TSharedRef<FJsonObject> Object =
                     EmptyStructuredPersonaObject();
                 const FString Trait = PersonaOrJson.TrimStartAndEnd();
                 !Trait.IsEmpty()
                     ? [&]() {
                         TArray<FString> Traits;
                         Traits.Add(Trait);
                         SetStringArrayField(Object, Data.Persona.Traits,
                                             Traits);
                       }()
                     : void();
                 return Object;
               }();
}

/** User Story: As a api serialization agent consumer, I need to invoke state from object through a stable signature so the api serialization agent workflow remains explicit and composable. @fn inline FAgentState StateFromObject(const TSharedPtr<FJsonObject> &Object) */
inline FAgentState StateFromObject(const TSharedPtr<FJsonObject> &Object) {
  return TypeFactory::AgentState(StringifyObject(Object));
}

/** User Story: As a api serialization agent consumer, I need to invoke state from field through a stable signature so the api serialization agent workflow remains explicit and composable. @fn inline FAgentState StateFromField(const TSharedPtr<FJsonObject> &Object, const FString &FieldName) */
inline FAgentState StateFromField(const TSharedPtr<FJsonObject> &Object,
                                  const FString &FieldName) {
  return TypeFactory::AgentState(JsonStringFromField(Object, FieldName));
}

/** User Story: As a api serialization agent consumer, I need to invoke state to object through a stable signature so the api serialization agent workflow remains explicit and composable. @fn inline TSharedPtr<FJsonObject> StateToObject(const FAgentState &State) */
inline TSharedPtr<FJsonObject> StateToObject(const FAgentState &State) {
  return ParseJsonObjectOrEmpty(State.JsonData);
}

/** User Story: As a api serialization agent consumer, I need to invoke action to object through a stable signature so the api serialization agent workflow remains explicit and composable. @fn inline TSharedRef<FJsonObject> ActionToObject(const FAgentAction &Action) */
inline TSharedRef<FJsonObject> ActionToObject(const FAgentAction &Action) {
  const AgentConfiguration::FAgentContractData &Data =
      AgentConfiguration::agentContractData();
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (detail::SetIfNonEmpty(Object, Data.Action.Type, Action.Type),
          detail::SetIfNonEmpty(Object, Data.Action.Target, Action.Target),
          detail::SetIfNonEmpty(Object, Data.Action.Reason, Action.Reason),
          (!FMath::IsNearlyEqual(Action.Confidence, Data.DefaultConfidence) ||
           !Action.Type.IsEmpty())
              ? (Object->SetNumberField(Data.Action.Confidence,
                                        Action.Confidence),
                 void())
              : void(),
          detail::SetIfNonEmpty(Object, Data.Action.Signature,
                                Action.Signature),
          SetFieldFromJsonString(Object, Data.Action.Payload,
                                 Action.PayloadJson),
          Object);
}

/** User Story: As an Agent transport consumer, I need only canonical, type-safe action objects admitted into domain state so malformed API responses fail at the boundary. @fn inline func::Maybe<FAgentAction> DecodeActionObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FAgentAction>
DecodeActionObject(const TSharedPtr<FJsonObject> &Object) {
  const AgentConfiguration::FAgentContractData &Data =
      AgentConfiguration::agentContractData();
  const TSharedPtr<FJsonValue> Confidence =
      Object.IsValid() ? Object->TryGetField(Data.Action.Confidence) : nullptr;
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.Action.Type) &&
      Object->GetStringField(Data.Action.Type).Len() >=
          Data.MinimumTypeLength &&
      HasOptionalFieldType(Object, Data.Action.Target, EJson::String) &&
      HasOptionalFieldType(Object, Data.Action.Reason, EJson::String) &&
      HasOptionalFieldType(Object, Data.Action.Confidence, EJson::Number) &&
      HasOptionalFieldType(Object, Data.Action.Signature, EJson::String) &&
      HasOptionalFieldType(Object, Data.Action.Payload, EJson::Object) &&
      (!Confidence.IsValid() || Confidence->Type != EJson::Number ||
       FMath::IsFinite(Confidence->AsNumber()));
  return !bValid
             ? func::nothing<FAgentAction>()
             : [&]() {
                 FAgentAction Action;
                 Action.Type = Object->GetStringField(Data.Action.Type);
                 Action.Target =
                     OptionalStringFromField(Object, Data.Action.Target);
                 Action.Reason =
                     OptionalStringFromField(Object, Data.Action.Reason);
                 Action.Confidence = detail::TryGetNumberAs<float>(
                     Object, Data.Action.Confidence, Data.DefaultConfidence);
                 Action.Signature =
                     OptionalStringFromField(Object, Data.Action.Signature);
                 Action.PayloadJson = JsonStringFromField(
                     Object, Data.Action.Payload, Data.EmptyObject);
                 return func::just(Action);
               }();
}

} // namespace JsonInterop
