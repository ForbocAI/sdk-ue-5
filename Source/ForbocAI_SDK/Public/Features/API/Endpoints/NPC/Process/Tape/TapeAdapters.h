#pragma once

#include "Features/API/Endpoints/NPC/Process/Configuration/ProcessConfigurationAdapters.h"
#include "Features/API/Transport/Query/TransportQueryAdapters.h"

namespace APISlice::Detail {

/**
 * User Story: As the process tape codec, I need optional finite-number fields validated before numeric values enter the protocol domain.
 * @fn inline bool HasOptionalFiniteNumberField( const TSharedPtr<FJsonObject> &Object, const FString &Field)
 */
inline bool HasOptionalFiniteNumberField(
    const TSharedPtr<FJsonObject> &Object, const FString &Field) {
  const TSharedPtr<FJsonValue> Value =
      Object.IsValid() ? Object->TryGetField(Field) : nullptr;
  return JsonInterop::HasOptionalFieldType(Object, Field, EJson::Number) &&
         (!Value.IsValid() || Value->Type == EJson::Null ||
          FMath::IsFinite(Value->AsNumber()));
}

/** User Story: As the process tape codec, I need optional integer fields distinguished from fractional numbers so token counts and seeds preserve their contract. @fn inline bool HasOptionalIntegerField(const TSharedPtr<FJsonObject> &Object, const FString &Field) */
inline bool HasOptionalIntegerField(const TSharedPtr<FJsonObject> &Object,
                                    const FString &Field) {
  const TSharedPtr<FJsonValue> Value =
      Object.IsValid() ? Object->TryGetField(Field) : nullptr;
  return HasOptionalFiniteNumberField(Object, Field) &&
         (!Value.IsValid() || Value->Type == EJson::Null ||
          FMath::IsNearlyEqual(Value->AsNumber(),
                               FMath::RoundToDouble(Value->AsNumber())));
}

/** User Story: As the process tape encoder, I need optional API-owned inference constraints serialized through authored fields without manufactured defaults. @fn inline TSharedRef<FJsonObject> EncodePromptConstraintsObject(const FPromptConstraints &Constraints) */
inline TSharedRef<FJsonObject>
EncodePromptConstraintsObject(const FPromptConstraints &Constraints) {
  const auto &Fields =
      NPCProcessConfiguration::processContractData().PromptConstraints;
  const TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
  return (Constraints.bHasMaxTokens
              ? (Object->SetNumberField(Fields.MaxTokens,
                                        Constraints.MaxTokens),
                 void())
              : void(),
          Constraints.bHasTemperature
              ? (Object->SetNumberField(Fields.Temperature,
                                        Constraints.Temperature),
                 void())
              : void(),
          Constraints.bHasStop
              ? (JsonInterop::SetStringArrayField(Object, Fields.Stop,
                                                  Constraints.Stop),
                 void())
              : void(),
          Constraints.bHasRepeatPenalty
              ? (Object->SetNumberField(Fields.RepeatPenalty,
                                        Constraints.RepeatPenalty),
                 void())
              : void(),
          Constraints.bHasSeed
              ? (Object->SetNumberField(Fields.Seed, Constraints.Seed), void())
              : void(),
          Object);
}

/** User Story: As the process tape decoder, I need inference constraints decoded atomically so invalid optional values reject the complete tape. @fn inline func::Maybe<FPromptConstraints> DecodePromptConstraintsObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FPromptConstraints>
DecodePromptConstraintsObject(const TSharedPtr<FJsonObject> &Object) {
  const auto &Fields =
      NPCProcessConfiguration::processContractData().PromptConstraints;
  const bool bHasStop =
      JsonInterop::HasNonNullField(Object, Fields.Stop);
  const TArray<TSharedPtr<FJsonValue>> *StopValues = nullptr;
  const bool bValid =
      Object.IsValid() && HasOptionalIntegerField(Object, Fields.MaxTokens) &&
      HasOptionalFiniteNumberField(Object, Fields.Temperature) &&
      JsonInterop::HasOptionalFieldType(Object, Fields.Stop, EJson::Array) &&
      HasOptionalFiniteNumberField(Object, Fields.RepeatPenalty) &&
      HasOptionalIntegerField(Object, Fields.Seed) &&
      (!bHasStop ||
       (Object->TryGetArrayField(Fields.Stop, StopValues) && StopValues));
  return !bValid
             ? func::nothing<FPromptConstraints>()
             : [&]() {
  const func::Maybe<TArray<FString>> Stop =
      bHasStop
          ? func::traverse_maybe_array<TSharedPtr<FJsonValue>, FString>(
                *StopValues, [](const TSharedPtr<FJsonValue> &Value) {
                  return Value.IsValid() && Value->Type == EJson::String
                             ? func::just(Value->AsString())
                             : func::nothing<FString>();
                })
          : func::just(TArray<FString>());
  return func::match(
      Stop,
      [&](const TArray<FString> &DecodedStop) {
        FPromptConstraints Decoded;
        Decoded.bHasMaxTokens =
            JsonInterop::HasNonNullField(Object, Fields.MaxTokens);
        Decoded.MaxTokens = JsonInterop::detail::TryGetNumberAs<int32>(
            Object, Fields.MaxTokens, Decoded.MaxTokens);
        Decoded.bHasTemperature =
            JsonInterop::HasNonNullField(Object, Fields.Temperature);
        Decoded.Temperature = JsonInterop::detail::TryGetNumberAs<float>(
            Object, Fields.Temperature, Decoded.Temperature);
        Decoded.bHasStop = bHasStop;
        Decoded.Stop = DecodedStop;
        Decoded.bHasRepeatPenalty =
            JsonInterop::HasNonNullField(Object, Fields.RepeatPenalty);
        Decoded.RepeatPenalty = JsonInterop::detail::TryGetNumberAs<float>(
            Object, Fields.RepeatPenalty, Decoded.RepeatPenalty);
        Decoded.bHasSeed =
            JsonInterop::HasNonNullField(Object, Fields.Seed);
        Decoded.Seed = JsonInterop::detail::TryGetNumberAs<int32>(
            Object, Fields.Seed, Decoded.Seed);
        return func::just(Decoded);
      },
      []() { return func::nothing<FPromptConstraints>(); });
               }();
}

/** User Story: As the process tape encoder, I need every represented tape field serialized through the shared authored contract so UE requests match TS requests. @fn inline TSharedRef<FJsonObject> EncodeProcessTapeObject(const FNPCProcessTape &Tape) */
inline TSharedRef<FJsonObject>
EncodeProcessTapeObject(const FNPCProcessTape &Tape) {
  const auto &Data = NPCProcessConfiguration::processContractData();
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  const TArray<TSharedPtr<FJsonValue>> Memories =
      func::map_array<FRecalledMemory>(
          Tape.Memories,
          [](const FRecalledMemory &Memory) -> TSharedPtr<FJsonValue> {
            return MakeShared<FJsonValueObject>(
                JsonInterop::RecalledMemoryToObject(Memory));
          });
  Root->SetStringField(Data.Tape.Observation, Tape.Observation);
  JsonInterop::SetFieldFromJsonString(Root, Data.Tape.Context,
                                      Tape.ContextJson, false);
  Root->SetObjectField(Data.Tape.NpcState,
                       JsonInterop::StateToObject(Tape.NpcState));
  Tape.bHasStructuredPersona
      ? (Root->SetObjectField(
             Data.Tape.StructuredPersona,
             JsonInterop::StructuredPersonaToObject(Tape.Persona)),
         void())
      : void();
  Tape.bHasSamplerProfile
      ? (Root->SetStringField(Data.Tape.SamplerProfile, Tape.SamplerProfile),
         void())
      : void();
  Tape.bHasActor
      ? [&]() {
          const TSharedRef<FJsonObject> Actor = MakeShared<FJsonObject>();
          Actor->SetStringField(Data.Actor.Id, Tape.Actor.NpcId);
          Tape.Actor.bHasStructuredPersona
              ? (Actor->SetObjectField(
                     Data.Actor.StructuredPersona,
                     JsonInterop::StructuredPersonaToObject(
                         Tape.Actor.Persona)),
                 void())
              : void();
          Actor->SetObjectField(Data.Actor.Data,
                                JsonInterop::StateToObject(Tape.Actor.Data));
          Root->SetObjectField(Data.Tape.Actor, Actor);
        }()
      : void();
  Root->SetArrayField(Data.Tape.Memories, Memories);
  Tape.bDecisionCompleted
      ? [&]() {
          const TSharedRef<FJsonObject> Intent = MakeShared<FJsonObject>();
          Intent->SetStringField(Data.DecisionIntent.Goal,
                                 Tape.DecisionIntent.Goal);
          Intent->SetStringField(Data.DecisionIntent.ActionType,
                                 Tape.DecisionIntent.ActionType);
          JsonInterop::detail::SetIfNonEmpty(
              Intent, Data.DecisionIntent.Target, Tape.DecisionIntent.Target);
          Tape.DecisionIntent.bHasMetadata
              ? (JsonInterop::SetFieldFromJsonString(
                     Intent, Data.DecisionIntent.Metadata,
                     Tape.DecisionIntent.MetadataJson, false),
                 void())
              : void();
          Root->SetObjectField(Data.Tape.DecisionIntent, Intent);
        }()
      : void();
  Tape.bReasoningCompleted
      ? [&]() {
          const TSharedRef<FJsonObject> Reasoning = MakeShared<FJsonObject>();
          Reasoning->SetStringField(Data.ReasoningOutput.ReasoningText,
                                    Tape.ReasoningOutput.ReasoningText);
          Reasoning->SetStringField(Data.ReasoningOutput.ResponseText,
                                    Tape.ReasoningOutput.ResponseText);
          Root->SetObjectField(Data.Tape.ReasoningOutput, Reasoning);
        }()
      : void();
  Tape.bHasPrompt
      ? (Root->SetStringField(Data.Tape.Prompt, Tape.Prompt), void())
      : void();
  Tape.bHasConstraints
      ? (Root->SetObjectField(
             Data.Tape.Constraints,
             EncodePromptConstraintsObject(Tape.Constraints)),
         void())
      : void();
  Tape.bHasGeneratedOutput
      ? (Root->SetStringField(Data.Tape.GeneratedOutput,
                              Tape.GeneratedOutput),
         void())
      : void();
  Tape.bHasRulesetId
      ? (Root->SetStringField(Data.Tape.RulesetId, Tape.RulesetId), void())
      : void();
  Tape.bHasVectorQueried
      ? (Root->SetBoolField(Data.Tape.VectorQueried, Tape.bVectorQueried),
         void())
      : void();
  return Root;
}

} // namespace APISlice::Detail
