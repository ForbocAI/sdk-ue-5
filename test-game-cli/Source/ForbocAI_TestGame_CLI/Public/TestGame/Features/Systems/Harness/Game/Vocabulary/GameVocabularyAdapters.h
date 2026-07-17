#pragma once

#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/Vocabulary/GameVocabularyTypes.h"

namespace TestGame::GameAdapters {
namespace detail {

/** User Story: As a harness maintainer, I need enabled authored names decoded from their JSON keys so C++ and TypeScript share one vocabulary contract. @fn inline FString ReadEnabledName(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline FString ReadEnabledName(const TSharedRef<FJsonObject> &Object,
                               const FString &Field) {
  check(DataAdapters::ReadBooleanField(Object, Field));
  return Field;
}

/** User Story: As a harness maintainer, I need authored positions decoded through one adapter so all initial entities use the same coordinate contract. @fn inline FPosition ReadPosition(const TSharedRef<FJsonObject> &Object) */
inline FPosition ReadPosition(const TSharedRef<FJsonObject> &Object) {
  return FPosition(DataAdapters::ReadNumberField(Object, TEXT("x")),
                   DataAdapters::ReadNumberField(Object, TEXT("y")));
}

/** User Story: As a harness maintainer, I need authored NPCs decoded through one adapter so scenario initialization never reconstructs entity data in behavior code. @fn inline FGameNPC ReadNpc(const TSharedRef<FJsonObject> &Object) */
inline FGameNPC ReadNpc(const TSharedRef<FJsonObject> &Object) {
  FGameNPC Npc;
  Npc.Id = DataAdapters::ReadStringField(Object, TEXT("id"));
  Npc.Name = DataAdapters::ReadStringField(Object, TEXT("name"));
  Npc.Faction = DataAdapters::ReadStringField(Object, TEXT("faction"));
  Npc.Hp = DataAdapters::ReadNumberField(Object, TEXT("hp"));
  Npc.Suspicion = DataAdapters::ReadNumberField(Object, TEXT("suspicion"));
  Npc.Inventory =
      DataAdapters::ReadStringArrayField(Object, TEXT("inventory"));
  Npc.KnownSecrets =
      DataAdapters::ReadStringArrayField(Object, TEXT("knownSecrets"));
  Npc.Position = ReadPosition(
      DataAdapters::ReadObjectField(Object, TEXT("position")));
  return Npc;
}

/** User Story: As a harness maintainer, I need authored memories decoded through one adapter so scenario thunks dispatch data instead of manufacturing it. @fn inline FMemoryRecord ReadMemory(const TSharedRef<FJsonObject> &Object) */
inline FMemoryRecord ReadMemory(const TSharedRef<FJsonObject> &Object) {
  FMemoryRecord Memory;
  Memory.Id = DataAdapters::ReadStringField(Object, TEXT("id"));
  Memory.NpcId = DataAdapters::ReadStringField(Object, TEXT("npcId"));
  Memory.Text = DataAdapters::ReadStringField(Object, TEXT("text"));
  Memory.Importance =
      DataAdapters::ReadFloatField(Object, TEXT("importance"));
  return Memory;
}

/** User Story: As a harness maintainer, I need authored trade offers decoded through one adapter so social initialization remains typed and deterministic. @fn inline FTradeOffer ReadTradeOffer(const TSharedRef<FJsonObject> &Object) */
inline FTradeOffer ReadTradeOffer(const TSharedRef<FJsonObject> &Object) {
  FTradeOffer Offer;
  Offer.NpcId = DataAdapters::ReadStringField(Object, TEXT("npcId"));
  Offer.Item = DataAdapters::ReadStringField(Object, TEXT("item"));
  Offer.Price = DataAdapters::ReadNumberField(Object, TEXT("price"));
  return Offer;
}

} // namespace detail

/** User Story: As a harness maintainer, I need all runtime vocabulary and initial state loaded from authored data so no behavior layer duplicates contract values. @fn inline FGameRuntimeData ReadGameRuntimeData() */
inline FGameRuntimeData ReadGameRuntimeData() {
  const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("harness/runtime.json"));
  const TSharedRef<FJsonObject> RuntimeTypes =
      DataAdapters::ReadObjectField(Source, TEXT("runtimeTypes"));
  const TSharedRef<FJsonObject> Numbers =
      DataAdapters::ReadObjectField(Source, TEXT("numbers"));
  const TSharedRef<FJsonObject> Modes =
      DataAdapters::ReadObjectField(Source, TEXT("modes"));
  const TSharedRef<FJsonObject> ScenarioEventTypes =
      DataAdapters::ReadObjectField(Source, TEXT("scenarioEventTypes"));
  const TSharedRef<FJsonObject> CommandGroups =
      DataAdapters::ReadObjectField(Source, TEXT("commandGroups"));
  const TSharedRef<FJsonObject> Statuses =
      DataAdapters::ReadObjectField(Source, TEXT("statuses"));
  const TSharedRef<FJsonObject> OutputAssertionKinds =
      DataAdapters::ReadObjectField(Source, TEXT("outputAssertionKinds"));
  const TSharedRef<FJsonObject> LifecycleEvents =
      DataAdapters::ReadObjectField(Source, TEXT("lifecycleEvents"));
  const TSharedRef<FJsonObject> Patterns =
      DataAdapters::ReadObjectField(Source, TEXT("patterns"));
  const TSharedRef<FJsonObject> Template =
      DataAdapters::ReadObjectField(Source, TEXT("template"));
  const TSharedRef<FJsonObject> WireFields =
      DataAdapters::ReadObjectField(Source, TEXT("wireFields"));
  const TSharedRef<FJsonObject> Separators =
      DataAdapters::ReadObjectField(Source, TEXT("separators"));
  const TSharedRef<FJsonObject> Tokens =
      DataAdapters::ReadObjectField(Source, TEXT("tokens"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  const TSharedRef<FJsonObject> InitialState =
      DataAdapters::ReadObjectField(Source, TEXT("initialState"));
  FGameRuntimeData Data;

#define FORBOCAI_READ_GAME_NAME(Container, SourceObject, Name)             \
  Data.Container.Name = detail::ReadEnabledName(SourceObject, TEXT(#Name)); \
  Data.Container.all.Add(Data.Container.Name);

#define FORBOCAI_READ_RUNTIME_TYPE(Name)                                  \
  FORBOCAI_READ_GAME_NAME(runtimeTypes, RuntimeTypes, Name)
  FORBOCAI_GAME_RUNTIME_TYPE_FIELDS(FORBOCAI_READ_RUNTIME_TYPE)
#undef FORBOCAI_READ_RUNTIME_TYPE

#define FORBOCAI_READ_RUNTIME_NUMBER(Type, Name)                          \
  Data.numbers.Name =                                                     \
      DataAdapters::ReadNumberField(Numbers, TEXT(#Name));
  FORBOCAI_GAME_RUNTIME_NUMBER_FIELDS(FORBOCAI_READ_RUNTIME_NUMBER)
#undef FORBOCAI_READ_RUNTIME_NUMBER

#define FORBOCAI_READ_GAME_MODE(Name)                                     \
  FORBOCAI_READ_GAME_NAME(modes, Modes, Name)
  FORBOCAI_GAME_MODE_FIELDS(FORBOCAI_READ_GAME_MODE)
#undef FORBOCAI_READ_GAME_MODE

#define FORBOCAI_READ_SCENARIO_EVENT_TYPE(Name)                           \
  FORBOCAI_READ_GAME_NAME(scenarioEventTypes, ScenarioEventTypes, Name)
  FORBOCAI_GAME_SCENARIO_EVENT_TYPE_FIELDS(
      FORBOCAI_READ_SCENARIO_EVENT_TYPE)
#undef FORBOCAI_READ_SCENARIO_EVENT_TYPE

#define FORBOCAI_READ_COMMAND_GROUP(Name)                                 \
  FORBOCAI_READ_GAME_NAME(commandGroups, CommandGroups, Name)
  FORBOCAI_GAME_COMMAND_GROUP_FIELDS(FORBOCAI_READ_COMMAND_GROUP)
#undef FORBOCAI_READ_COMMAND_GROUP

#define FORBOCAI_READ_STATUS(Name)                                        \
  FORBOCAI_READ_GAME_NAME(statuses, Statuses, Name)
  FORBOCAI_GAME_STATUS_FIELDS(FORBOCAI_READ_STATUS)
#undef FORBOCAI_READ_STATUS

#define FORBOCAI_READ_OUTPUT_ASSERTION_KIND(Name)                         \
  FORBOCAI_READ_GAME_NAME(outputAssertionKinds, OutputAssertionKinds, Name)
  FORBOCAI_GAME_OUTPUT_ASSERTION_KIND_FIELDS(
      FORBOCAI_READ_OUTPUT_ASSERTION_KIND)
#undef FORBOCAI_READ_OUTPUT_ASSERTION_KIND

#define FORBOCAI_READ_LIFECYCLE_EVENT(Name)                               \
  FORBOCAI_READ_GAME_NAME(lifecycleEvents, LifecycleEvents, Name)
  FORBOCAI_GAME_LIFECYCLE_EVENT_FIELDS(FORBOCAI_READ_LIFECYCLE_EVENT)
#undef FORBOCAI_READ_LIFECYCLE_EVENT
#undef FORBOCAI_READ_GAME_NAME

  Data.patterns.npcCommand =
      DataAdapters::ReadStringField(Patterns, TEXT("npcCommand"));
  Data.templateData.tokenPrefix =
      DataAdapters::ReadStringField(Template, TEXT("tokenPrefix"));
  Data.templateData.tokenSuffix =
      DataAdapters::ReadStringField(Template, TEXT("tokenSuffix"));
#define FORBOCAI_READ_GAME_WIRE_FIELD(Name)                               \
  Data.wireFields.Name = DataAdapters::ReadStringField(WireFields, TEXT(#Name));
  FORBOCAI_READ_GAME_WIRE_FIELD(action)
  FORBOCAI_READ_GAME_WIRE_FIELD(type)
  FORBOCAI_READ_GAME_WIRE_FIELD(targetHex)
  FORBOCAI_READ_GAME_WIRE_FIELD(x)
  FORBOCAI_READ_GAME_WIRE_FIELD(y)
  FORBOCAI_READ_GAME_WIRE_FIELD(stateDelta)
  FORBOCAI_READ_GAME_WIRE_FIELD(suspicion)
#undef FORBOCAI_READ_GAME_WIRE_FIELD
  Data.separators.list =
      DataAdapters::ReadStringField(Separators, TEXT("list"));
  Data.tokens.alias = DataAdapters::ReadStringField(Tokens, TEXT("alias"));
  Data.tokens.command =
      DataAdapters::ReadStringField(Tokens, TEXT("command"));
  Data.tokens.count = DataAdapters::ReadStringField(Tokens, TEXT("count"));
  Data.tokens.errors = DataAdapters::ReadStringField(Tokens, TEXT("errors"));
  Data.tokens.groups = DataAdapters::ReadStringField(Tokens, TEXT("groups"));
  Data.tokens.kind = DataAdapters::ReadStringField(Tokens, TEXT("kind"));
  Data.tokens.missing =
      DataAdapters::ReadStringField(Tokens, TEXT("missing"));
  Data.tokens.output = DataAdapters::ReadStringField(Tokens, TEXT("output"));
  Data.tokens.reason = DataAdapters::ReadStringField(Tokens, TEXT("reason"));
  Data.tokens.suffix = DataAdapters::ReadStringField(Tokens, TEXT("suffix"));
  Data.tokens.value = DataAdapters::ReadStringField(Tokens, TEXT("value"));

#define FORBOCAI_READ_RUNTIME_MESSAGE(Type, Name)                         \
  Data.messages.Name =                                                    \
      DataAdapters::ReadStringField(Messages, TEXT(#Name));
  FORBOCAI_GAME_RUNTIME_MESSAGE_FIELDS(FORBOCAI_READ_RUNTIME_MESSAGE)
#undef FORBOCAI_READ_RUNTIME_MESSAGE

  const TSharedRef<FJsonObject> Stealth =
      DataAdapters::ReadObjectField(InitialState, TEXT("stealth"));
  Data.initialState.stealth.doorOpen =
      DataAdapters::ReadBooleanField(Stealth, TEXT("doorOpen"));
  Data.initialState.stealth.alertDelta =
      DataAdapters::ReadNumberField(Stealth, TEXT("alertDelta"));
  Data.initialState.stealth.npc = detail::ReadNpc(
      DataAdapters::ReadObjectField(Stealth, TEXT("npc")));
  Data.initialState.stealth.memory = detail::ReadMemory(
      DataAdapters::ReadObjectField(Stealth, TEXT("memory")));

  const TSharedRef<FJsonObject> Social =
      DataAdapters::ReadObjectField(InitialState, TEXT("social"));
  Data.initialState.social.npc =
      detail::ReadNpc(DataAdapters::ReadObjectField(Social, TEXT("npc")));
  Data.initialState.social.dialogue =
      DataAdapters::ReadStringField(Social, TEXT("dialogue"));
  Data.initialState.social.tradeOffer = detail::ReadTradeOffer(
      DataAdapters::ReadObjectField(Social, TEXT("tradeOffer")));
  Data.initialState.social.suspicion = DataAdapters::ReadNumberField(
      DataAdapters::ReadObjectField(Social, TEXT("patch")),
      TEXT("suspicion"));

  const TSharedRef<FJsonObject> Escape =
      DataAdapters::ReadObjectField(InitialState, TEXT("escape"));
  Data.initialState.escape.hidden =
      DataAdapters::ReadBooleanField(Escape, TEXT("hidden"));

  const TSharedRef<FJsonObject> Persistence =
      DataAdapters::ReadObjectField(InitialState, TEXT("persistence"));
  Data.initialState.persistence.npcId =
      DataAdapters::ReadStringField(Persistence, TEXT("npcId"));
  Data.initialState.persistence.txId =
      DataAdapters::ReadStringField(Persistence, TEXT("txId"));
  Data.initialState.sessionNpc = detail::ReadNpc(
      DataAdapters::ReadObjectField(InitialState, TEXT("sessionNpc")));
  return Data;
}

/** User Story: As a harness consumer, I need one immutable runtime-data instance so every action, selector, thunk, and test shares the same authored vocabulary. @fn inline const FGameRuntimeData &GameRuntimeData() */
inline const FGameRuntimeData &GameRuntimeData() {
  static const FGameRuntimeData Data = ReadGameRuntimeData();
  return Data;
}

} // namespace TestGame::GameAdapters
