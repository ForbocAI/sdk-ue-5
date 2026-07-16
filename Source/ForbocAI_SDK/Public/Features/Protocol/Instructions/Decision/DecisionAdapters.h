#pragma once

#include "Core/fp.hpp"
#include "Features/API/Serialization/APISerializationAdapters.h"
#include "Features/NPC/NPCTypes.h"
#include "Features/Protocol/Configuration/ConfigurationAdapters.h"
#include "Features/Protocol/Requests/RequestsTypes.h"

namespace rtk::detail {

/** User Story: As decision policy, I need recent interactions selected from the tape or NPC history using the same priority as TS so metadata remains reproducible. @fn inline TArray<TSharedPtr<FJsonValue>> buildRecentInteractions(const FNPCProcessTape &Tape, const TArray<FNPCHistoryEntry> &History) */
inline TArray<TSharedPtr<FJsonValue>>
buildRecentInteractions(const FNPCProcessTape &Tape,
                        const TArray<FNPCHistoryEntry> &History) {
  const auto &Data = ProtocolConfiguration::protocolData();
  TSharedPtr<FJsonObject> Context;
  const TArray<TSharedPtr<FJsonValue>> *FromTape = nullptr;
  const bool bHasTapeInteractions =
      JsonInterop::ParseJsonObject(Tape.ContextJson, Context) &&
      Context.IsValid() &&
      Context->TryGetArrayField(
          Data.Decision.MetadataFields.RecentInteractions, FromTape) &&
      FromTape && !FromTape->IsEmpty();
  return bHasTapeInteractions
             ? *FromTape
             : [&]() {
                 const int32 Start = FMath::Max(
                     Data.Iteration.InitialIndex,
                     History.Num() - Data.History.RecentInteractionLimit);
                 return func::map_array<FNPCHistoryEntry,
                                        TSharedPtr<FJsonValue>>(
                     func::drop_array(History, Start),
                     [&](const FNPCHistoryEntry &Entry) {
                       const TSharedRef<FJsonObject> Object =
                           MakeShared<FJsonObject>();
                       Object->SetStringField(
                           Data.Decision.MetadataFields.Role, Entry.Role);
                       Object->SetStringField(
                           Data.Decision.MetadataFields.Content,
                           Entry.Content);
                       return MakeShared<FJsonValueObject>(Object);
                     });
               }();
}

/**
 * User Story: As decision policy, I need metadata composed as a pure JSON value so the thunk coordinates effects without owning transport-shaped calculations.
 * @fn inline FString buildDecisionMetadata( const FNPCProcessTape &Tape, const TArray<FNPCHistoryEntry> &History)
 */
inline FString buildDecisionMetadata(
    const FNPCProcessTape &Tape,
    const TArray<FNPCHistoryEntry> &History) {
  const auto &Data = ProtocolConfiguration::protocolData();
  const TSharedRef<FJsonObject> Context = MakeShared<FJsonObject>();
  Context->SetObjectField(Data.Decision.MetadataFields.NpcState,
                          JsonInterop::StateToObject(Tape.NpcState));
  Context->SetArrayField(
      Data.Decision.MetadataFields.Memories,
      func::map_array<FRecalledMemory, TSharedPtr<FJsonValue>>(
          Tape.Memories, [](const FRecalledMemory &Memory) {
            return MakeShared<FJsonValueObject>(
                JsonInterop::RecalledMemoryToObject(Memory));
          }));
  Context->SetArrayField(Data.Decision.MetadataFields.RecentInteractions,
                         buildRecentInteractions(Tape, History));
  const TSharedRef<FJsonObject> Metadata = MakeShared<FJsonObject>();
  Metadata->SetStringField(Data.Decision.MetadataFields.Source,
                           Data.Decision.MetadataSource);
  Metadata->SetObjectField(Data.Decision.MetadataFields.Context, Context);
  return JsonInterop::StringifyObject(Metadata);
}

/** User Story: As local Decision handling, I need a pure policy adapter that mirrors TS verb, target, goal, and metadata derivation so both SDKs advance identical tapes. @fn inline FDecisionIntent buildDecisionIntent(const FNPCProcessTape &Tape, const TArray<FNPCHistoryEntry> &History) */
inline FDecisionIntent
buildDecisionIntent(const FNPCProcessTape &Tape,
                    const TArray<FNPCHistoryEntry> &History) {
  const auto &Data = ProtocolConfiguration::protocolData();
  TArray<FString> Words;
  Tape.Observation.ToLower().ParseIntoArrayWS(Words);
  const func::Maybe<FString> MatchedVerb = func::find_array<FString>(
      Words, [&](const FString &Word) {
        return func::contains_value(Data.Decision.MoveVerbs, Word) ||
               func::contains_value(Data.Decision.InteractVerbs, Word);
      });
  const FString ActionType =
      func::is_nothing(MatchedVerb)
          ? Data.Decision.ActionTypes.Speak
      : func::contains_value(Data.Decision.MoveVerbs, MatchedVerb.value)
          ? Data.Decision.ActionTypes.Move
          : Data.Decision.ActionTypes.Interact;
  const TArray<FString> Candidates =
      func::is_nothing(MatchedVerb)
          ? TArray<FString>()
          : func::drop_array(
                Words, Words.Find(MatchedVerb.value) + Data.Iteration.Step);
  const func::Maybe<FString> Target = func::find_array<FString>(
      Candidates, [&](const FString &Word) {
        return !func::contains_value(Data.Decision.StopWords, Word);
      });
  FDecisionIntent Intent;
  Intent.ActionType = ActionType;
  Intent.Target = func::is_just(Target) ? Target.value : Data.Text.Empty;
  Intent.Goal = Data.Decision.Goal.Prefix + Tape.Observation +
                (Tape.Memories.Num() > Data.Iteration.InitialIndex
                     ? Data.Decision.Goal.MemoryPrefix +
                           FString::FromInt(Tape.Memories.Num()) +
                           Data.Decision.Goal.MemorySuffix
                     : Data.Text.Empty);
  Intent.bHasMetadata = true;
  Intent.MetadataJson = buildDecisionMetadata(Tape, History);
  return Intent;
}

} // namespace rtk::detail
