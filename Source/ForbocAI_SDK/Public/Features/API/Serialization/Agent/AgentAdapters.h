#pragma once

#include "Core/JsonInterop.h"
#include "Features/Contracts/ContractsTypes.h"

namespace JsonInterop {

inline TSharedRef<FJsonObject> EmptyStructuredPersonaObject() {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (SetStringArrayField(Object, TEXT("traits"), TArray<FString>()),
          SetStringArrayField(Object, TEXT("goals"), TArray<FString>()),
          SetStringArrayField(Object, TEXT("relationships"), TArray<FString>()),
          SetStringArrayField(Object, TEXT("world"), TArray<FString>()),
          SetStringArrayField(Object, TEXT("speakingStyle"), TArray<FString>()),
          SetStringArrayField(Object, TEXT("constraints"), TArray<FString>()),
          Object);
}

inline TSharedRef<FJsonObject>
StructuredPersonaToObject(const FString &PersonaOrJson) {
  TSharedPtr<FJsonObject> Parsed;
  return ParseJsonObject(PersonaOrJson, Parsed) && Parsed.IsValid() &&
                 Parsed->HasField(TEXT("traits"))
             ? Parsed.ToSharedRef()
             : [&]() {
                 const TSharedRef<FJsonObject> Object =
                     EmptyStructuredPersonaObject();
                 const FString Trait = PersonaOrJson.TrimStartAndEnd();
                 !Trait.IsEmpty()
                     ? [&]() {
                         TArray<FString> Traits;
                         Traits.Add(Trait);
                         SetStringArrayField(Object, TEXT("traits"), Traits);
                       }()
                     : void();
                 return Object;
               }();
}

inline FAgentState StateFromObject(const TSharedPtr<FJsonObject> &Object) {
  return TypeFactory::AgentState(StringifyObject(Object));
}

inline FAgentState StateFromField(const TSharedPtr<FJsonObject> &Object,
                                  const FString &FieldName) {
  return TypeFactory::AgentState(JsonStringFromField(Object, FieldName));
}

inline TSharedPtr<FJsonObject> StateToObject(const FAgentState &State) {
  return ParseJsonObjectOrEmpty(State.JsonData);
}

inline TSharedRef<FJsonObject> ActionToObject(const FAgentAction &Action) {
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (detail::SetIfNonEmpty(Object, TEXT("type"), Action.Type),
          detail::SetIfNonEmpty(Object, TEXT("target"), Action.Target),
          detail::SetIfNonEmpty(Object, TEXT("reason"), Action.Reason),
          (!FMath::IsNearlyEqual(Action.Confidence, 1.0f) ||
           !Action.Type.IsEmpty())
              ? (Object->SetNumberField(TEXT("confidence"), Action.Confidence),
                 void())
              : void(),
          detail::SetIfNonEmpty(Object, TEXT("signature"), Action.Signature),
          SetFieldFromJsonString(Object, TEXT("payload"), Action.PayloadJson),
          Object);
}

inline FString ExtractWithAlias(const TSharedPtr<FJsonObject> &Object,
                                const FString &Primary,
                                const FString &Alias) {
  const FString PrimaryValue = OptionalStringFromField(Object, Primary);
  return !PrimaryValue.IsEmpty() ? PrimaryValue
                                 : OptionalStringFromField(Object, Alias);
}

inline FAgentAction ActionFromObject(const TSharedPtr<FJsonObject> &Object) {
  FAgentAction Action;
  return !Object.IsValid()
             ? Action
             : (Action.Type =
                    ExtractWithAlias(Object, TEXT("gaType"), TEXT("type")),
                Action.Target = ExtractWithAlias(
                    Object, TEXT("actionTarget"), TEXT("target")),
                Action.Reason = ExtractWithAlias(
                    Object, TEXT("actionReason"), TEXT("reason")),
                Action.Confidence = detail::TryGetNumberAs<float>(
                    Object, TEXT("confidence"), Action.Confidence),
                Action.Signature =
                    OptionalStringFromField(Object, TEXT("signature")),
                Action.PayloadJson =
                    JsonStringFromField(Object, TEXT("payload"), TEXT("{}")),
                Action);
}

} // namespace JsonInterop
