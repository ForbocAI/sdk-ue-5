#pragma once

#include "Components/API/Endpoints/NPC/Process/Configuration/ProcessConfigurationTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace APISlice::NPCProcessConfiguration {

/** User Story: As the NPC process codec, I need one immutable authored contract so TS and UE field names and instruction discriminators cannot drift independently. @fn inline const FProcessContractData &processContractData() */
inline const FProcessContractData &processContractData() {
  static const FProcessContractData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                     TEXT("Data/api/npc.json"));
    const TSharedRef<FJsonObject> Response =
        DataAdapters::ReadObjectField(Source, TEXT("response"));
    const TSharedRef<FJsonObject> Request =
        DataAdapters::ReadObjectField(Source, TEXT("request"));
    const TSharedRef<FJsonObject> Tape =
        DataAdapters::ReadObjectField(Source, TEXT("tape"));
    const TSharedRef<FJsonObject> Actor =
        DataAdapters::ReadObjectField(Source, TEXT("actor"));
    const TSharedRef<FJsonObject> DecisionIntent =
        DataAdapters::ReadObjectField(Source, TEXT("decisionIntent"));
    const TSharedRef<FJsonObject> ReasoningOutput =
        DataAdapters::ReadObjectField(Source, TEXT("reasoningOutput"));
    const TSharedRef<FJsonObject> PromptConstraints =
        DataAdapters::ReadObjectField(Source, TEXT("promptConstraints"));
    const TSharedRef<FJsonObject> Instruction =
        DataAdapters::ReadObjectField(Source, TEXT("instruction"));
    const TSharedRef<FJsonObject> InstructionTypes =
        DataAdapters::ReadObjectField(Source, TEXT("instructionTypes"));
    const TSharedRef<FJsonObject> ResultTypes =
        DataAdapters::ReadObjectField(Source, TEXT("resultTypes"));
    const TSharedRef<FJsonObject> Finalize =
        DataAdapters::ReadObjectField(Source, TEXT("finalize"));
    const TSharedRef<FJsonObject> RuleAudit =
        DataAdapters::ReadObjectField(Source, TEXT("ruleAudit"));
    const TSharedRef<FJsonObject> RuleVerdict =
        DataAdapters::ReadObjectField(Source, TEXT("ruleVerdict"));
    const TSharedRef<FJsonObject> Verdicts =
        DataAdapters::ReadObjectField(Source, TEXT("verdicts"));
    return FProcessContractData{
        {DataAdapters::ReadStringField(Response, TEXT("instruction")),
         DataAdapters::ReadStringField(Response, TEXT("tape"))},
        {DataAdapters::ReadStringField(Request, TEXT("tape")),
         DataAdapters::ReadStringField(Request, TEXT("lastResult"))},
        {DataAdapters::ReadStringField(Tape, TEXT("observation")),
         DataAdapters::ReadStringField(Tape, TEXT("context")),
         DataAdapters::ReadStringField(Tape, TEXT("npcState")),
         DataAdapters::ReadStringField(Tape, TEXT("structuredPersona")),
         DataAdapters::ReadStringField(Tape, TEXT("thoughtProfile")),
         DataAdapters::ReadStringField(Tape, TEXT("actor")),
         DataAdapters::ReadStringField(Tape, TEXT("memories")),
         DataAdapters::ReadStringField(Tape, TEXT("decisionIntent")),
         DataAdapters::ReadStringField(Tape, TEXT("reasoningOutput")),
         DataAdapters::ReadStringField(Tape, TEXT("prompt")),
         DataAdapters::ReadStringField(Tape, TEXT("constraints")),
         DataAdapters::ReadStringField(Tape, TEXT("generatedOutput")),
         DataAdapters::ReadStringField(Tape, TEXT("rulesetId")),
         DataAdapters::ReadStringField(Tape, TEXT("vectorQueried")),
         DataAdapters::ReadStringField(Tape, TEXT("legalActions")),
         DataAdapters::ReadStringField(Tape, TEXT("visitedActions")),
         DataAdapters::ReadStringField(Tape, TEXT("avoidActions"))},
        {DataAdapters::ReadStringField(Actor, TEXT("id")),
         DataAdapters::ReadStringField(Actor, TEXT("structuredPersona")),
         DataAdapters::ReadStringField(Actor, TEXT("data"))},
        {DataAdapters::ReadStringField(DecisionIntent, TEXT("goal")),
         DataAdapters::ReadStringField(DecisionIntent, TEXT("actionType")),
         DataAdapters::ReadStringField(DecisionIntent, TEXT("target")),
         DataAdapters::ReadStringField(DecisionIntent, TEXT("metadata"))},
        {DataAdapters::ReadStringField(ReasoningOutput,
                                       TEXT("reasoningText")),
         DataAdapters::ReadStringField(ReasoningOutput,
                                       TEXT("responseText"))},
        {DataAdapters::ReadStringField(PromptConstraints, TEXT("maxTokens")),
         DataAdapters::ReadStringField(PromptConstraints,
                                       TEXT("temperature")),
         DataAdapters::ReadStringField(PromptConstraints, TEXT("stop")),
         DataAdapters::ReadStringField(PromptConstraints,
                                       TEXT("repeatPenalty")),
         DataAdapters::ReadStringField(PromptConstraints, TEXT("seed"))},
        {DataAdapters::ReadStringField(Instruction, TEXT("type")),
         DataAdapters::ReadStringField(Instruction, TEXT("query")),
         DataAdapters::ReadStringField(Instruction, TEXT("limit")),
         DataAdapters::ReadStringField(Instruction, TEXT("threshold"))},
        {DataAdapters::ReadStringField(InstructionTypes,
                                       TEXT("queryVector")),
         DataAdapters::ReadStringField(InstructionTypes, TEXT("finalize"))},
        {DataAdapters::ReadStringField(ResultTypes, TEXT("queryVector"))},
        {DataAdapters::ReadStringField(Finalize, TEXT("valid")),
         DataAdapters::ReadStringField(Finalize, TEXT("signature")),
         DataAdapters::ReadStringField(Finalize, TEXT("memoryStore")),
         DataAdapters::ReadStringField(Finalize, TEXT("stateTransform")),
         DataAdapters::ReadStringField(Finalize, TEXT("action")),
         DataAdapters::ReadStringField(Finalize, TEXT("dialogue")),
         DataAdapters::ReadStringField(Finalize, TEXT("ruleAudit"))},
        {DataAdapters::ReadStringField(RuleAudit, TEXT("preset")),
         DataAdapters::ReadStringField(RuleAudit, TEXT("appliedRules"))},
        {DataAdapters::ReadStringField(RuleVerdict, TEXT("ruleId")),
         DataAdapters::ReadStringField(RuleVerdict, TEXT("verdict")),
         DataAdapters::ReadStringField(RuleVerdict, TEXT("reason"))},
        {DataAdapters::ReadStringField(Verdicts, TEXT("pass")),
         DataAdapters::ReadStringField(Verdicts, TEXT("blocked"))}};
  }();
  return Data;
}

} // namespace APISlice::NPCProcessConfiguration
