#pragma once

#include "Systems/API/Endpoints/NPC/Process/Tape/TapeAdapters.h"

namespace APISlice::Detail {

/**
 * User Story: As the process tape decoder, I need optional string arrays traversed atomically so malformed action constraints reject the complete tape.
 * @fn inline func::Maybe<TArray<FString>> DecodeOptionalTapeStringArray( const TSharedPtr<FJsonObject> &Object, const FString &Field)
 */
inline func::Maybe<TArray<FString>> DecodeOptionalTapeStringArray(
    const TSharedPtr<FJsonObject> &Object, const FString &Field) {
  const bool bHasValue = JsonInterop::HasNonNullField(Object, Field);
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  return !bHasValue
             ? func::just(TArray<FString>())
         : !Object->TryGetArrayField(Field, Values) || !Values
             ? func::nothing<TArray<FString>>()
             : func::traverse_maybe_array<TSharedPtr<FJsonValue>, FString>(
                   *Values, [](const TSharedPtr<FJsonValue> &Value) {
                     return Value.IsValid() && Value->Type == EJson::String
                                ? func::just(Value->AsString())
                                : func::nothing<FString>();
                   });
}

/** User Story: As the process tape decoder, I need every nested tape value decoded atomically so malformed API output cannot partially replace runtime state. @fn inline bool DecodeProcessTapeObject(const TSharedPtr<FJsonObject> &Object, FNPCProcessTape &Tape) */
inline bool DecodeProcessTapeObject(const TSharedPtr<FJsonObject> &Object,
                                    FNPCProcessTape &Tape) {
  const auto &Data = NPCProcessConfiguration::processContractData();
  const TArray<TSharedPtr<FJsonValue>> *MemoryValues = nullptr;
  const bool bBaseValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.Tape.Observation) &&
      Object->HasTypedField<EJson::Object>(Data.Tape.Context) &&
      Object->HasTypedField<EJson::Object>(Data.Tape.NpcState) &&
      JsonInterop::HasOptionalFieldType(
          Object, Data.Tape.StructuredPersona, EJson::Object) &&
      JsonInterop::HasOptionalFieldType(
          Object, Data.Tape.ThoughtProfile, EJson::String) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.Actor,
                                        EJson::Object) &&
      Object->TryGetArrayField(Data.Tape.Memories, MemoryValues) &&
      MemoryValues &&
      JsonInterop::HasOptionalFieldType(
          Object, Data.Tape.DecisionIntent, EJson::Object) &&
      JsonInterop::HasOptionalFieldType(
          Object, Data.Tape.ReasoningOutput, EJson::Object) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.Prompt,
                                        EJson::String) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.Constraints,
                                        EJson::Object) &&
      JsonInterop::HasOptionalFieldType(
          Object, Data.Tape.GeneratedOutput, EJson::String) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.RulesetId,
                                        EJson::String) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.VectorQueried,
                                        EJson::Boolean) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.LegalActions,
                                        EJson::Array) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.VisitedActions,
                                        EJson::Array) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Tape.AvoidActions,
                                        EJson::Array);
  return !bBaseValid
             ? false
             : [&]() {
  const bool bHasPersona =
      JsonInterop::HasNonNullField(Object, Data.Tape.StructuredPersona);
  const func::Maybe<FString> Persona =
      bHasPersona
          ? JsonInterop::DecodeStructuredPersonaObject(
                Object->GetObjectField(Data.Tape.StructuredPersona))
          : func::just(FString());
  const bool bHasActor =
      JsonInterop::HasNonNullField(Object, Data.Tape.Actor);
  const TSharedPtr<FJsonObject> Actor =
      bHasActor ? Object->GetObjectField(Data.Tape.Actor) : nullptr;
  const bool bActorValid =
      !bHasActor ||
      (Actor.IsValid() &&
       Actor->HasTypedField<EJson::String>(Data.Actor.Id) &&
       JsonInterop::HasOptionalFieldType(
           Actor, Data.Actor.StructuredPersona, EJson::Object) &&
       Actor->HasTypedField<EJson::Object>(Data.Actor.Data));
  const bool bHasActorPersona =
      bActorValid && bHasActor &&
      JsonInterop::HasNonNullField(Actor, Data.Actor.StructuredPersona);
  const func::Maybe<FString> ActorPersona =
      bHasActorPersona
          ? JsonInterop::DecodeStructuredPersonaObject(
                Actor->GetObjectField(Data.Actor.StructuredPersona))
          : func::just(FString());

  const bool bHasDecision =
      JsonInterop::HasNonNullField(Object, Data.Tape.DecisionIntent);
  const TSharedPtr<FJsonObject> Intent =
      bHasDecision ? Object->GetObjectField(Data.Tape.DecisionIntent) : nullptr;
  const bool bDecisionValid =
      !bHasDecision ||
      (Intent.IsValid() &&
       Intent->HasTypedField<EJson::String>(Data.DecisionIntent.Goal) &&
       Intent->HasTypedField<EJson::String>(
           Data.DecisionIntent.ActionType) &&
       JsonInterop::HasOptionalFieldType(
           Intent, Data.DecisionIntent.Target, EJson::String) &&
       JsonInterop::HasOptionalFieldType(
           Intent, Data.DecisionIntent.Metadata, EJson::Object));

  const bool bHasReasoning =
      JsonInterop::HasNonNullField(Object, Data.Tape.ReasoningOutput);
  const TSharedPtr<FJsonObject> Reasoning =
      bHasReasoning ? Object->GetObjectField(Data.Tape.ReasoningOutput)
                    : nullptr;
  const bool bReasoningValid =
      !bHasReasoning ||
      (Reasoning.IsValid() &&
       Reasoning->HasTypedField<EJson::String>(
           Data.ReasoningOutput.ReasoningText) &&
       Reasoning->HasTypedField<EJson::String>(
           Data.ReasoningOutput.ResponseText));

  const bool bHasConstraints =
      JsonInterop::HasNonNullField(Object, Data.Tape.Constraints);
  const func::Maybe<FPromptConstraints> Constraints =
      bHasConstraints
          ? DecodePromptConstraintsObject(
                Object->GetObjectField(Data.Tape.Constraints))
          : func::just(FPromptConstraints());
  const func::Maybe<TArray<FRecalledMemory>> Memories =
      func::traverse_maybe_array<TSharedPtr<FJsonValue>, FRecalledMemory>(
          *MemoryValues, JsonInterop::DecodeRecalledMemoryValue);
  const func::Maybe<TArray<FString>> LegalActions =
      DecodeOptionalTapeStringArray(Object, Data.Tape.LegalActions);
  const func::Maybe<TArray<FString>> VisitedActions =
      DecodeOptionalTapeStringArray(Object, Data.Tape.VisitedActions);
  const func::Maybe<TArray<FString>> AvoidActions =
      DecodeOptionalTapeStringArray(Object, Data.Tape.AvoidActions);
  const bool bNestedValid =
      bActorValid && bDecisionValid && bReasoningValid &&
      func::is_just(Persona) && func::is_just(ActorPersona) &&
      func::is_just(Constraints) && func::is_just(Memories) &&
      func::is_just(LegalActions) && func::is_just(VisitedActions) &&
      func::is_just(AvoidActions);
  return !bNestedValid
             ? false
             : [&]() {
  FNPCProcessTape Decoded;
  Decoded.Observation = Object->GetStringField(Data.Tape.Observation);
  Decoded.ContextJson =
      JsonInterop::JsonStringFromField(Object, Data.Tape.Context);
  Decoded.NpcState = JsonInterop::StateFromField(Object, Data.Tape.NpcState);
  Decoded.bHasStructuredPersona = bHasPersona;
  Decoded.Persona = func::or_else(Persona, FString());
  Decoded.bHasThoughtProfile =
      JsonInterop::HasNonNullField(Object, Data.Tape.ThoughtProfile);
  Decoded.ThoughtProfile = JsonInterop::OptionalStringFromField(
      Object, Data.Tape.ThoughtProfile);
  Decoded.bHasActor = bHasActor;
  bHasActor
      ? (Decoded.Actor.NpcId = Actor->GetStringField(Data.Actor.Id),
         Decoded.Actor.bHasStructuredPersona = bHasActorPersona,
         Decoded.Actor.Persona = func::or_else(ActorPersona, FString()),
         Decoded.Actor.Data =
             JsonInterop::StateFromField(Actor, Data.Actor.Data),
         void())
      : void();
  Decoded.Memories = func::or_else(Memories, TArray<FRecalledMemory>());
  Decoded.bHasDecisionIntent = bHasDecision;
  bHasDecision
      ? (Decoded.DecisionIntent.Goal =
             Intent->GetStringField(Data.DecisionIntent.Goal),
         Decoded.DecisionIntent.ActionType =
             Intent->GetStringField(Data.DecisionIntent.ActionType),
         Decoded.DecisionIntent.Target = JsonInterop::OptionalStringFromField(
             Intent, Data.DecisionIntent.Target),
         Decoded.DecisionIntent.bHasMetadata =
             JsonInterop::HasNonNullField(Intent,
                                          Data.DecisionIntent.Metadata),
         Decoded.DecisionIntent.MetadataJson = JsonInterop::JsonStringFromField(
             Intent, Data.DecisionIntent.Metadata, TEXT("")),
         void())
      : void();
  Decoded.bHasReasoningOutput = bHasReasoning;
  bHasReasoning
      ? (Decoded.ReasoningOutput.ReasoningText = Reasoning->GetStringField(
             Data.ReasoningOutput.ReasoningText),
         Decoded.ReasoningOutput.ResponseText = Reasoning->GetStringField(
             Data.ReasoningOutput.ResponseText),
         void())
      : void();
  Decoded.bHasPrompt =
      JsonInterop::HasNonNullField(Object, Data.Tape.Prompt);
  Decoded.Prompt =
      JsonInterop::OptionalStringFromField(Object, Data.Tape.Prompt);
  Decoded.bHasConstraints = bHasConstraints;
  Decoded.Constraints = func::or_else(Constraints, FPromptConstraints());
  Decoded.bHasGeneratedOutput =
      JsonInterop::HasNonNullField(Object, Data.Tape.GeneratedOutput);
  Decoded.GeneratedOutput = JsonInterop::OptionalStringFromField(
      Object, Data.Tape.GeneratedOutput);
  Decoded.bHasRulesetId =
      JsonInterop::HasNonNullField(Object, Data.Tape.RulesetId);
  Decoded.RulesetId =
      JsonInterop::OptionalStringFromField(Object, Data.Tape.RulesetId);
  Decoded.bHasVectorQueried =
      JsonInterop::HasNonNullField(Object, Data.Tape.VectorQueried);
  Decoded.bVectorQueried = Decoded.bHasVectorQueried
                               ? Object->GetBoolField(Data.Tape.VectorQueried)
                               : false;
  Decoded.bHasLegalActions =
      JsonInterop::HasNonNullField(Object, Data.Tape.LegalActions);
  Decoded.LegalActions = func::or_else(LegalActions, TArray<FString>());
  Decoded.bHasVisitedActions =
      JsonInterop::HasNonNullField(Object, Data.Tape.VisitedActions);
  Decoded.VisitedActions = func::or_else(VisitedActions, TArray<FString>());
  Decoded.bHasAvoidActions =
      JsonInterop::HasNonNullField(Object, Data.Tape.AvoidActions);
  Decoded.AvoidActions = func::or_else(AvoidActions, TArray<FString>());
  Tape = Decoded;
  return true;
               }();
               }();
}

} // namespace APISlice::Detail
