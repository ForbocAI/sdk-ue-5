#pragma once

#include "Systems/API/Endpoints/NPC/Process/Tape/TapeAdapters.h"

namespace APISlice::Detail {

/** User Story: As a rule-audit consumer, I need each applied-rule verdict validated against the authored closed set before it enters protocol state. @fn inline func::Maybe<FRuleVerdictEntry> DecodeRuleVerdictObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FRuleVerdictEntry>
DecodeRuleVerdictObject(const TSharedPtr<FJsonObject> &Object) {
  const auto &Data = NPCProcessConfiguration::processContractData();
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.RuleVerdict.RuleId) &&
      Object->HasTypedField<EJson::String>(Data.RuleVerdict.Verdict) &&
      JsonInterop::HasOptionalFieldType(Object, Data.RuleVerdict.Reason,
                                        EJson::String);
  return !bValid
             ? func::nothing<FRuleVerdictEntry>()
             : [&]() {
  const FString Verdict =
      Object->GetStringField(Data.RuleVerdict.Verdict);
  return Verdict != Data.Verdicts.Pass && Verdict != Data.Verdicts.Blocked
             ? func::nothing<FRuleVerdictEntry>()
             : [&]() {
                 FRuleVerdictEntry Decoded;
                 Decoded.RuleId =
                     Object->GetStringField(Data.RuleVerdict.RuleId);
                 Decoded.Verdict = Verdict;
                 Decoded.bHasReason = JsonInterop::HasNonNullField(
                     Object, Data.RuleVerdict.Reason);
                 Decoded.Reason = JsonInterop::OptionalStringFromField(
                     Object, Data.RuleVerdict.Reason);
                 return func::just(Decoded);
               }();
               }();
}

/** User Story: As a rule-audit consumer, I need the complete applied-rule path traversed atomically so malformed entries cannot produce a partial audit. @fn inline func::Maybe<FRuleAudit> DecodeRuleAuditObject(const TSharedPtr<FJsonObject> &Object) */
inline func::Maybe<FRuleAudit>
DecodeRuleAuditObject(const TSharedPtr<FJsonObject> &Object) {
  const auto &Data = NPCProcessConfiguration::processContractData();
  const TArray<TSharedPtr<FJsonValue>> *Values = nullptr;
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.RuleAudit.Preset) &&
      Object->TryGetArrayField(Data.RuleAudit.AppliedRules, Values) && Values;
  return !bValid
             ? func::nothing<FRuleAudit>()
             : func::match(
      func::traverse_maybe_array<TSharedPtr<FJsonValue>, FRuleVerdictEntry>(
          *Values, [](const TSharedPtr<FJsonValue> &Value) {
            return Value.IsValid() && Value->Type == EJson::Object
                       ? DecodeRuleVerdictObject(Value->AsObject())
                       : func::nothing<FRuleVerdictEntry>();
          }),
      [&](const TArray<FRuleVerdictEntry> &DecodedEntries) {
        FRuleAudit Decoded;
        Decoded.Preset = Object->GetStringField(Data.RuleAudit.Preset);
        Decoded.AppliedRules = DecodedEntries;
        return func::just(Decoded);
      },
      []() { return func::nothing<FRuleAudit>(); });
}

/**
 * User Story: As the process instruction decoder, I need finalization decoded atomically so action, memory, state, dialogue, and audit data cannot disagree.
 * @fn inline bool DecodeFinalizeInstructionObject( const TSharedPtr<FJsonObject> &Object, FNPCInstruction &Instruction)
 */
inline bool DecodeFinalizeInstructionObject(
    const TSharedPtr<FJsonObject> &Object, FNPCInstruction &Instruction) {
  const auto &Data = NPCProcessConfiguration::processContractData();
  const TArray<TSharedPtr<FJsonValue>> *MemoryValues = nullptr;
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::Boolean>(Data.Finalize.Valid) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Finalize.Signature,
                                        EJson::String) &&
      Object->TryGetArrayField(Data.Finalize.MemoryStore, MemoryValues) &&
      MemoryValues &&
      Object->HasTypedField<EJson::Object>(Data.Finalize.StateTransform) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Finalize.Action,
                                        EJson::Object) &&
      Object->HasTypedField<EJson::String>(Data.Finalize.Dialogue) &&
      JsonInterop::HasOptionalFieldType(Object, Data.Finalize.RuleAudit,
                                        EJson::Object);
  return !bValid
             ? false
             : [&]() {
  const bool bHasAction =
      JsonInterop::HasNonNullField(Object, Data.Finalize.Action);
  const func::Maybe<FAgentAction> Action =
      bHasAction
          ? JsonInterop::DecodeActionObject(
                Object->GetObjectField(Data.Finalize.Action))
          : func::just(FAgentAction());
  const bool bHasRuleAudit =
      JsonInterop::HasNonNullField(Object, Data.Finalize.RuleAudit);
  const func::Maybe<FRuleAudit> RuleAudit =
      bHasRuleAudit
          ? DecodeRuleAuditObject(
                Object->GetObjectField(Data.Finalize.RuleAudit))
          : func::just(FRuleAudit());
  const func::Maybe<TArray<FMemoryStoreInstruction>> Memories =
      func::traverse_maybe_array<TSharedPtr<FJsonValue>,
                                 FMemoryStoreInstruction>(
          *MemoryValues, JsonInterop::DecodeMemoryStoreInstructionValue);
  const bool bDecoded = func::is_just(Action) && func::is_just(RuleAudit) &&
                        func::is_just(Memories);
  return !bDecoded
             ? false
             : [&]() {
                 FNPCInstruction Decoded;
                 Decoded.Type = ENPCInstructionType::Finalize;
                 Decoded.bValid =
                     Object->GetBoolField(Data.Finalize.Valid);
                 Decoded.Signature = JsonInterop::OptionalStringFromField(
                     Object, Data.Finalize.Signature);
                 Decoded.storeMemory = Memories.value;
                 Decoded.StateTransform = JsonInterop::StateFromField(
                     Object, Data.Finalize.StateTransform);
                 Decoded.bHasAction = bHasAction;
                 Decoded.Action = Action.value;
                 Decoded.Dialogue =
                     Object->GetStringField(Data.Finalize.Dialogue);
                 Decoded.bHasRuleAudit = bHasRuleAudit;
                 Decoded.RuleAudit = RuleAudit.value;
                 Instruction = Decoded;
                 return true;
               }();
               }();
}

/** User Story: As process response handling, I need the instruction discriminator decoded through the authored closed table so unknown commands fail visibly. @fn inline bool DecodeInstructionObject(const TSharedPtr<FJsonObject> &Object, FNPCInstruction &Instruction) */
inline bool DecodeInstructionObject(const TSharedPtr<FJsonObject> &Object,
                                    FNPCInstruction &Instruction) {
  const auto &Data = NPCProcessConfiguration::processContractData();
  const bool bValid =
      Object.IsValid() &&
      Object->HasTypedField<EJson::String>(Data.Instruction.Type);
  return !bValid
             ? false
             : [&]() {
  const FString Type = Object->GetStringField(Data.Instruction.Type);
  FNPCInstruction Decoded;
  const func::Maybe<bool> Matched = func::multi_match<FString, bool>(
      Type,
      {func::when<FString, bool>(
           func::equals<FString>(Data.InstructionTypes.QueryVector),
           [&](const FString &) {
             const bool bQueryValid =
                 Object->HasTypedField<EJson::String>(
                     Data.Instruction.Query) &&
                 Object->HasTypedField<EJson::Number>(
                     Data.Instruction.Limit) &&
                 Object->HasTypedField<EJson::Number>(
                     Data.Instruction.Threshold) &&
                 HasOptionalIntegerField(Object, Data.Instruction.Limit) &&
                 HasOptionalFiniteNumberField(Object,
                                              Data.Instruction.Threshold);
             return !bQueryValid
                        ? false
                        : (Decoded.Type = ENPCInstructionType::QueryVector,
                           Decoded.Query = Object->GetStringField(
                               Data.Instruction.Query),
                           Decoded.Limit =
                               JsonInterop::detail::TryGetNumberAs<int32>(
                                   Object, Data.Instruction.Limit,
                                   Decoded.Limit),
                           Decoded.Threshold =
                               JsonInterop::detail::TryGetNumberAs<float>(
                                   Object, Data.Instruction.Threshold,
                                   Decoded.Threshold),
                           true);
           }),
       func::when<FString, bool>(
           func::equals<FString>(Data.InstructionTypes.Finalize),
           [&](const FString &) {
             return DecodeFinalizeInstructionObject(Object, Decoded);
           })});
  return func::is_nothing(Matched) || !Matched.value
             ? false
             : (Instruction = Decoded, true);
               }();
}

} // namespace APISlice::Detail
