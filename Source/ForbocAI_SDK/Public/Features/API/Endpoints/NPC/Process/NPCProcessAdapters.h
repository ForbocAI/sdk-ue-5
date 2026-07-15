#pragma once

#include "Features/API/Transport/Query/TransportQueryAdapters.h"

namespace APISlice::Detail {

/**
 * Encodes an NPC process tape into a JSON object.
 * User Story: As protocol request composition, I need process tapes converted
 * into JSON objects so nested runtime state can be transmitted cleanly.
 */
inline TSharedRef<FJsonObject>
EncodeProcessTapeObject(const FNPCProcessTape &Tape) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  TArray<TSharedPtr<FJsonValue>> Memories;
  return (
      Root->SetStringField(TEXT("observation"), Tape.Observation),
      JsonInterop::SetFieldFromJsonString(Root, TEXT("context"),
                                          Tape.ContextJson, false),
      Root->SetObjectField(TEXT("npcState"),
                           JsonInterop::StateToObject(Tape.NpcState)),
      Root->SetObjectField(TEXT("structuredPersona"),
                           JsonInterop::StructuredPersonaToObject(
                               Tape.Persona)),
      Tape.bHasActor
          ? [&]() {
              const TSharedRef<FJsonObject> Actor = MakeShared<FJsonObject>();
              Actor->SetStringField(TEXT("npcId"), Tape.Actor.NpcId);
              Actor->SetStringField(TEXT("persona"), Tape.Actor.Persona);
              Actor->SetObjectField(
                  TEXT("data"), JsonInterop::StateToObject(Tape.Actor.Data));
              Root->SetObjectField(TEXT("actor"), Actor);
            }()
          : void(),
      detail::BuildRecalledMemoriesRecursive(Tape.Memories, Memories, 0),
      Root->SetArrayField(TEXT("memories"), Memories),
      !Tape.RulesetId.IsEmpty()
          ? (Root->SetStringField(TEXT("rulesetId"), Tape.RulesetId), void())
          : void(),
      Root->SetBoolField(TEXT("vectorQueried"), Tape.bVectorQueried),
      Tape.bDecisionCompleted
          ? [&]() {
              const TSharedRef<FJsonObject> Intent = MakeShared<FJsonObject>();
              Intent->SetStringField(TEXT("goal"), Tape.DecisionIntent.Goal);
              Intent->SetStringField(TEXT("actionType"), Tape.DecisionIntent.ActionType);
              !Tape.DecisionIntent.Target.IsEmpty()
                  ? (Intent->SetStringField(TEXT("target"), Tape.DecisionIntent.Target), void())
                  : void();
              Root->SetObjectField(TEXT("decisionIntent"), Intent);
            }()
          : void(),
      Tape.bReasoningCompleted
          ? [&]() {
              const TSharedRef<FJsonObject> Reasoning = MakeShared<FJsonObject>();
              Reasoning->SetStringField(TEXT("reasoningText"), Tape.ReasoningOutput.ReasoningText);
              Reasoning->SetStringField(TEXT("responseText"), Tape.ReasoningOutput.ResponseText);
              Root->SetObjectField(TEXT("reasoningOutput"), Reasoning);
            }()
          : void(),
      Root);
}

/**
 * Decodes a process tape JSON object into a typed tape value.
 * User Story: As protocol response handling, I need process tapes parsed back
 * into runtime types so downstream orchestration can inspect the turn state.
 */
inline bool DecodeProcessTapeObject(const TSharedPtr<FJsonObject> &Object,
                                    FNPCProcessTape &Tape) {
  return !Object.IsValid()
             ? false
             : (Tape.Observation = Object->GetStringField(TEXT("observation")),
                Tape.ContextJson =
                    JsonInterop::JsonStringFromField(Object, TEXT("context")),
                Tape.NpcState =
                    JsonInterop::StateFromField(Object, TEXT("npcState")),
                Tape.Persona = JsonInterop::JsonStringFromField(
                    Object, TEXT("structuredPersona"), TEXT("")),
                Tape.bHasActor =
                    Object->HasTypedField<EJson::Object>(TEXT("actor")),
                Tape.bHasActor
                    ? [&]() {
                        const TSharedPtr<FJsonObject> Actor =
                            Object->GetObjectField(TEXT("actor"));
                        Tape.Actor.NpcId =
                            Actor->GetStringField(TEXT("npcId"));
                        Tape.Actor.Persona =
                            Actor->HasField(TEXT("structuredPersona"))
                                ? JsonInterop::JsonStringFromField(
                                      Actor, TEXT("structuredPersona"))
                                : JsonInterop::OptionalStringFromField(
                                      Actor, TEXT("persona"));
                        Tape.Actor.Data =
                            JsonInterop::StateFromField(Actor, TEXT("data"));
                      }()
                    : void(),
                Tape.Memories.Empty(),
                [&]() {
                  const TArray<TSharedPtr<FJsonValue>> *MemoryValues = nullptr;
                  (Object->TryGetArrayField(TEXT("memories"), MemoryValues) &&
                   MemoryValues)
                      ? (detail::ExtractRecalledMemoriesRecursive(
                             *MemoryValues, Tape.Memories, 0),
                         void())
                      : void();
                }(),
                Tape.RulesetId = JsonInterop::OptionalStringFromField(
                    Object, TEXT("rulesetId")),
                Tape.bVectorQueried = JsonInterop::detail::TryGetBoolAs(
                    Object, TEXT("vectorQueried"), false),
                Tape.bDecisionCompleted = Object->HasTypedField<EJson::Object>(TEXT("decisionIntent")),
                Tape.bDecisionCompleted
                    ? [&]() {
                        const TSharedPtr<FJsonObject> Intent = Object->GetObjectField(TEXT("decisionIntent"));
                        Tape.DecisionIntent.Goal = Intent->GetStringField(TEXT("goal"));
                        Tape.DecisionIntent.ActionType = Intent->GetStringField(TEXT("actionType"));
                        Tape.DecisionIntent.Target = JsonInterop::OptionalStringFromField(Intent, TEXT("target"));
                      }()
                    : void(),
                Tape.bReasoningCompleted = Object->HasTypedField<EJson::Object>(TEXT("reasoningOutput")),
                Tape.bReasoningCompleted
                    ? [&]() {
                        const TSharedPtr<FJsonObject> Reasoning = Object->GetObjectField(TEXT("reasoningOutput"));
                        Tape.ReasoningOutput.ReasoningText = Reasoning->GetStringField(TEXT("reasoningText"));
                        Tape.ReasoningOutput.ResponseText = Reasoning->GetStringField(TEXT("responseText"));
                      }()
                    : void(),
                true);
}

/**
 * Encodes an NPC process request into JSON text.
 * User Story: As process endpoint callers, I need a request encoder so typed
 * process requests can be posted without duplicating JSON assembly logic.
 */
inline FString EncodeNpcProcessRequest(const FNPCProcessRequest &Request) {
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  return (Root->SetObjectField(TEXT("tape"),
                               EncodeProcessTapeObject(Request.Tape)),
          Request.bHasPreviousResult
              ? (JsonInterop::SetFieldFromJsonString(
                     Root, TEXT("lastResult"), Request.PreviousResult, false),
                 void())
              : void(),
          ToJsonString(Root));
}

/**
 * Decodes an instruction object from JSON.
 * User Story: As process response handling, I need typed instruction parsing so
 * protocol actions can be reconstructed from the API payload.
 */
inline bool DecodeInstructionObject(const TSharedPtr<FJsonObject> &Object,
                                    FNPCInstruction &Instruction) {
  return !Object.IsValid()
             ? false
             : [&]() -> bool {
                 const FString Type = Object->GetStringField(TEXT("type"));
                 const func::Maybe<bool> Matched =
                     func::multi_match<FString, bool>(
                         Type,
                         {func::when<FString, bool>(
                              func::equals<FString>(TEXT("IdentifyActor")),
                              [&](const FString &) -> bool {
                                Instruction.Type =
                                    ENPCInstructionType::IdentifyActor;
                                return true;
                              }),
                          func::when<FString, bool>(
                              func::equals<FString>(TEXT("QueryVector")),
                              [&](const FString &) -> bool {
                                return (
                                    Instruction.Type =
                                        ENPCInstructionType::QueryVector,
                                    Instruction.Query =
                                        Object->GetStringField(TEXT("query")),
                                    Instruction.Limit =
                                        JsonInterop::detail::TryGetNumberAs<
                                            int32>(Object, TEXT("limit"),
                                                   Instruction.Limit),
                                    Instruction.Threshold =
                                        JsonInterop::detail::TryGetNumberAs<
                                            float>(Object, TEXT("threshold"),
                                                   Instruction.Threshold),
                                    true);
                              }),
                          func::when<FString, bool>(
                              func::equals<FString>(TEXT("Decision")),
                              [&](const FString &) -> bool {
                                Instruction.Type =
                                    ENPCInstructionType::Decision;
                                return true;
                              }),
                          func::when<FString, bool>(
                              func::equals<FString>(TEXT("Reasoning")),
                              [&](const FString &) -> bool {
                                Instruction.Type =
                                    ENPCInstructionType::Reasoning;
                                return true;
                              }),
                          func::when<FString, bool>(
                              func::equals<FString>(TEXT("Finalize")),
                              [&](const FString &) -> bool {
                                return (
                                    Instruction.Type =
                                        ENPCInstructionType::Finalize,
                                    Instruction.bValid =
                                        JsonInterop::detail::TryGetBoolAs(
                                            Object, TEXT("valid"), true),
                                    Instruction.Signature =
                                        JsonInterop::OptionalStringFromField(
                                            Object, TEXT("signature")),
                                    Instruction.StateTransform =
                                        JsonInterop::StateFromField(
                                            Object, TEXT("stateTransform")),
                                    Instruction.Dialogue =
                                        Object->GetStringField(
                                            TEXT("dialogue")),
                                    Instruction.bHasAction =
                                        Object->HasTypedField<EJson::Object>(
                                            TEXT("action")),
                                    Instruction.bHasAction
                                        ? (Instruction.Action =
                                               JsonInterop::ActionFromObject(
                                                   Object->GetObjectField(
                                                       TEXT("action"))),
                                           void())
                                        : void(),
                                    [&]() {
                                      const TArray<TSharedPtr<FJsonValue>>
                                          *MemoryValues = nullptr;
                                      (Object->TryGetArrayField(
                                           TEXT("memoryStore"),
                                           MemoryValues) &&
                                       MemoryValues)
                                          ? (Instruction.storeMemory.Empty(
                                                 MemoryValues->Num()),
                                             detail::
                                                 ExtractMemoryStoreInstructionsRecursive(
                                                     *MemoryValues,
                                                     Instruction.storeMemory,
                                                     0),
                                             void())
                                          : void();
                                    }(),
                                    true);
                              })});
                 return func::or_else(Matched, false);
               }();
}

/**
 * Decodes a process response into its instruction and tape payloads.
 * User Story: As the SDK protocol loop, I need one process-response decoder so
 * turn instructions and echoed tape state are reconstructed together.
 */
inline bool DecodeNpcProcessResponse(const FString &Json,
                                     FNPCProcessResponse &Response) {
  TSharedPtr<FJsonObject> Root;
  return (!JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid())
             ? false
             : (!Root->HasTypedField<EJson::Object>(TEXT("instruction")) ||
                !Root->HasTypedField<EJson::Object>(TEXT("tape")))
                   ? false
                   : (DecodeInstructionObject(
                          Root->GetObjectField(TEXT("instruction")),
                          Response.Instruction) &&
                      DecodeProcessTapeObject(
                          Root->GetObjectField(TEXT("tape")), Response.Tape));
}

/**
 * Encodes a directive request into JSON text.
 * User Story: As directive endpoint callers, I need a request encoder so
 * observation, state, and context fields are posted with a stable schema.
 */

} // namespace APISlice::Detail
