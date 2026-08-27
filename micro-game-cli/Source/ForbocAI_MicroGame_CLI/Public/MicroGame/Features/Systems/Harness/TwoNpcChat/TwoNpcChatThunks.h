#pragma once

#include "CoreMinimal.h"
#include "Internationalization/Regex.h"
#include "Core/JsonInterop.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Harness/Command/CommandTypes.h"
#include "MicroGame/Features/Systems/Harness/Verification/Command/CommandThunks.h"
// The two-NPC chat is the GAME's own orchestration (it lives in the game, not
// the SDK core). It composes the granular CLI commands, each of which executes
// an SDK core primitive: npc generate x N -> npc create -> memory store x N,
// then per turn npc decide -> memory store. Any SDK consumer (Platform,
// demo-ue) can do the same by composing those core primitives directly.

namespace MicroGame {

namespace TwoNpcChat {

struct FTwoNpcChatConfig {
  FString Group;
  FString Header;
  FString Footer;
  int32 TurnCount{};
  FString NameAttribute;
  TArray<FString> PersonaAttributes;
  FString SeedAttribute;
  FString GenerateCommand;
  FString CreateCommand;
  FString DecideCommand;
  FString MemoryStoreCommand;
  FString ValueField;
  FString PersonaSeparator;
  FString ContextSeparator;
  FString EmptyContext;
  FString IdPattern;
  FString LineFormat;
  FString DialogueFallback;
  FString ContextQuote;
};

struct FChatNpc {
  FString Id;
  FString Name;
  FString Seed;
};

/** User Story: As the two-NPC chat game orchestrator, I need the authored chat configuration loaded once from game.json. @fn inline const FTwoNpcChatConfig &TwoNpcChatConfig() */
inline const FTwoNpcChatConfig &TwoNpcChatConfig() {
  static const FTwoNpcChatConfig Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("harness/game.json"));
    const TSharedRef<FJsonObject> Chat =
        DataAdapters::ReadObjectField(Source, TEXT("twoNpcChat"));
    FTwoNpcChatConfig Config;
    Config.Group = DataAdapters::ReadStringField(Chat, TEXT("group"));
    Config.Header = DataAdapters::ReadStringField(Chat, TEXT("header"));
    Config.Footer = DataAdapters::ReadStringField(Chat, TEXT("footer"));
    Config.TurnCount = DataAdapters::ReadNumberField(Chat, TEXT("turnCount"));
    Config.NameAttribute =
        DataAdapters::ReadStringField(Chat, TEXT("nameAttribute"));
    Config.PersonaAttributes =
        DataAdapters::ReadStringArrayField(Chat, TEXT("personaAttributes"));
    Config.SeedAttribute =
        DataAdapters::ReadStringField(Chat, TEXT("seedAttribute"));
    Config.GenerateCommand =
        DataAdapters::ReadStringField(Chat, TEXT("generateCommand"));
    Config.CreateCommand =
        DataAdapters::ReadStringField(Chat, TEXT("createCommand"));
    Config.DecideCommand =
        DataAdapters::ReadStringField(Chat, TEXT("decideCommand"));
    Config.MemoryStoreCommand =
        DataAdapters::ReadStringField(Chat, TEXT("memoryStoreCommand"));
    Config.ValueField = DataAdapters::ReadStringField(Chat, TEXT("valueField"));
    Config.PersonaSeparator =
        DataAdapters::ReadStringField(Chat, TEXT("personaSeparator"));
    Config.ContextSeparator =
        DataAdapters::ReadStringField(Chat, TEXT("contextSeparator"));
    Config.EmptyContext =
        DataAdapters::ReadStringField(Chat, TEXT("emptyContext"));
    Config.IdPattern = DataAdapters::ReadStringField(Chat, TEXT("idPattern"));
    Config.LineFormat =
        DataAdapters::ReadStringField(Chat, TEXT("lineFormat"));
    Config.DialogueFallback =
        DataAdapters::ReadStringField(Chat, TEXT("dialogueFallback"));
    Config.ContextQuote =
        DataAdapters::ReadStringField(Chat, TEXT("contextQuote"));
    return Config;
  }();
  return Data;
}

/** User Story: As a two-NPC chat adapter, I need a granular npc generate command composed for one attribute conditioned on the prior attributes supplied as context. @fn inline FString FormatGenerate(const FTwoNpcChatConfig &Config, const FString &Attribute, const FString &Context) */
inline FString FormatGenerate(const FTwoNpcChatConfig &Config,
                              const FString &Attribute,
                              const FString &Context) {
  const FString Quoted = Context.IsEmpty()
                             ? Config.EmptyContext
                             : Config.ContextQuote + Context + Config.ContextQuote;
  return Config.GenerateCommand.Replace(TEXT("{attribute}"), *Attribute)
      .Replace(TEXT("{context}"), *Quoted);
}

/** User Story: As a two-NPC chat adapter, I need an npc create command composed from an SLM-assembled persona. @fn inline FString FormatCreate(const FTwoNpcChatConfig &Config, const FString &Persona) */
inline FString FormatCreate(const FTwoNpcChatConfig &Config,
                            const FString &Persona) {
  return Config.CreateCommand.Replace(TEXT("{persona}"), *Persona);
}

/** User Story: As a two-NPC chat adapter, I need the decide-only npc command composed from one NPC id and the line it must react to. @fn inline FString FormatDecide(const FTwoNpcChatConfig &Config, const FString &Id, const FString &Line) */
inline FString FormatDecide(const FTwoNpcChatConfig &Config, const FString &Id,
                            const FString &Line) {
  return Config.DecideCommand.Replace(TEXT("{id}"), *Id)
      .Replace(TEXT("{line}"), *Line);
}

/** User Story: As a two-NPC chat adapter, I need a granular memory store command composed so a generated attribute is persisted to the NPC's vector db. @fn inline FString FormatMemoryStore(const FTwoNpcChatConfig &Config, const FString &Id, const FString &Memory) */
inline FString FormatMemoryStore(const FTwoNpcChatConfig &Config,
                                 const FString &Id, const FString &Memory) {
  return Config.MemoryStoreCommand.Replace(TEXT("{id}"), *Id)
      .Replace(TEXT("{memory}"), *Memory);
}

/** User Story: As a two-NPC chat adapter, I need each spoken turn rendered as one transcript line. @fn inline FString FormatLine(const FTwoNpcChatConfig &Config, const FString &Name, const FString &Dialogue) */
inline FString FormatLine(const FTwoNpcChatConfig &Config, const FString &Name,
                          const FString &Dialogue) {
  return Config.LineFormat.Replace(TEXT("{name}"), *Name)
      .Replace(TEXT("{dialogue}"), *Dialogue);
}

/** User Story: As a two-NPC chat adapter, I need the SLM-generated attribute value read from the structured npc generate output. @fn inline FString ReadGeneratedValue(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadGeneratedValue(const FTwoNpcChatConfig &Config,
                                  const FString &Output) {
  TSharedPtr<FJsonObject> Root;
  return (JsonInterop::ParseJsonObject(Output, Root) && Root.IsValid() &&
          Root->HasTypedField<EJson::String>(Config.ValueField))
             ? Root->GetStringField(Config.ValueField)
             : Config.EmptyContext;
}

/** User Story: As a two-NPC chat adapter, I need the created NPC id extracted from the real npc create output. @fn inline FString ReadNpcId(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadNpcId(const FTwoNpcChatConfig &Config,
                         const FString &Output) {
  FRegexMatcher Matcher(FRegexPattern(Config.IdPattern), Output);
  return Matcher.FindNext() ? Matcher.GetCaptureGroup(0) : Config.EmptyContext;
}

/** User Story: As a two-NPC chat adapter, I need the spoken dialogue read from the real npc decide output, falling back when the NPC said nothing. @fn inline FString ReadDialogue(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadDialogue(const FTwoNpcChatConfig &Config,
                            const FString &Output) {
  const FString Spoken = Output.TrimStartAndEnd();
  return Spoken.IsEmpty() ? Config.DialogueFallback : Spoken;
}

/**
 * User Story: As the two-NPC chat game orchestrator, I need each generated command run through the real forbocai CLI so game state reflects the exercised SDK primitive.
 * @fn inline CommandRunner::FCommandOutput RunChatCommand( FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FString &Command)
 */
inline CommandRunner::FCommandOutput RunChatCommand(
    FMicroGameStore &Store, const FTwoNpcChatConfig &Config,
    const FString &Command) {
  FCommandSpec Spec;
  Spec.Group = Config.Group;
  Spec.Command = Command;
  return VerificationThunksDetail::ExecuteCommand(Spec, Store);
}

/** User Story: As the two-NPC chat game orchestrator, I need each persona attribute SLM-generated in order and threaded as context so the persona stays consistent. @fn inline TArray<FString> GenerateAttributes(FMicroGameStore &Store, const FTwoNpcChatConfig &Config, int32 Index, const FString &Context, TArray<FString> Values) */
inline TArray<FString> GenerateAttributes(FMicroGameStore &Store,
                                          const FTwoNpcChatConfig &Config,
                                          int32 Index, const FString &Context,
                                          TArray<FString> Values) {
  return Index >= Config.PersonaAttributes.Num()
             ? Values
             : [&]() {
                 const FString Value = ReadGeneratedValue(
                     Config, RunChatCommand(Store, Config,
                                            FormatGenerate(Config,
                                                           Config.PersonaAttributes[Index],
                                                           Context))
                                 .Output);
                 const FString Next =
                     Context.IsEmpty() ? Value
                                       : Context + Config.ContextSeparator + Value;
                 Values.Add(Value);
                 return GenerateAttributes(Store, Config, Index + 1, Next,
                                           Values);
               }();
}

/** User Story: As the two-NPC chat game orchestrator, I need each generated attribute persisted to the NPC's vector db through a composed memory store command so a later session can recall the same NPC. @fn inline void PersistAttributes(FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FString &Id, const TArray<FString> &Memories, int32 Index) */
inline void PersistAttributes(FMicroGameStore &Store,
                              const FTwoNpcChatConfig &Config, const FString &Id,
                              const TArray<FString> &Memories, int32 Index) {
  Index >= Memories.Num()
      ? void()
      : (RunChatCommand(Store, Config,
                        FormatMemoryStore(Config, Id, Memories[Index])),
         PersistAttributes(Store, Config, Id, Memories, Index + 1));
}

/** User Story: As the two-NPC chat game orchestrator, I need one participant SLM-generated, created Redux-only, then its persona persisted to the vector db via composed memory store commands. @fn inline FChatNpc BuildNpc(FMicroGameStore &Store, const FTwoNpcChatConfig &Config) */
inline FChatNpc BuildNpc(FMicroGameStore &Store,
                         const FTwoNpcChatConfig &Config) {
  const FString Name = ReadGeneratedValue(
      Config, RunChatCommand(Store, Config,
                             FormatGenerate(Config, Config.NameAttribute,
                                            Config.EmptyContext))
                  .Output);
  const TArray<FString> Values = GenerateAttributes(
      Store, Config, 0, Config.EmptyContext, TArray<FString>());
  const FString Persona = FString::Join(Values, *Config.PersonaSeparator);
  const FString Id = ReadNpcId(
      Config, RunChatCommand(Store, Config, FormatCreate(Config, Persona)).Output);
  TArray<FString> Memories;
  Memories.Add(Name);
  Memories.Append(Values);
  PersistAttributes(Store, Config, Id, Memories, 0);
  const int32 SeedIndex = Config.PersonaAttributes.IndexOfByKey(Config.SeedAttribute);
  const FString Seed = Values.IsValidIndex(SeedIndex) ? Values[SeedIndex]
                                                      : Config.DialogueFallback;
  return FChatNpc{Id, Name, Seed};
}

/** User Story: As the two-NPC chat game orchestrator, I need each NPC to react to the other's last line via the decide-only command, then persist that line via a composed memory store, alternating for the authored turn count. @fn inline TArray<FString> RunTurns(FMicroGameStore &Store, const FTwoNpcChatConfig &Config, const FChatNpc &Speaker, const FChatNpc &Listener, const FString &InputLine, int32 Turn, TArray<FString> Lines) */
inline TArray<FString> RunTurns(FMicroGameStore &Store,
                                const FTwoNpcChatConfig &Config,
                                const FChatNpc &Speaker, const FChatNpc &Listener,
                                const FString &InputLine, int32 Turn,
                                TArray<FString> Lines) {
  return Turn >= Config.TurnCount
             ? Lines
             : [&]() {
                 const FString Dialogue = ReadDialogue(
                     Config, RunChatCommand(Store, Config,
                                            FormatDecide(Config, Speaker.Id,
                                                         InputLine))
                                 .Output);
                 const FString Line = FormatLine(Config, Speaker.Name, Dialogue);
                 RunChatCommand(Store, Config,
                                FormatMemoryStore(Config, Speaker.Id, Line));
                 Lines.Add(Line);
                 return RunTurns(Store, Config, Listener, Speaker, Dialogue,
                                 Turn + 1, Lines);
               }();
}

/** User Story: As the micro-game, I need a client-orchestrated two-NPC chat that SLM-generates both personas, opens from a generated seed, alternates real decide turns, and persists every line to the vector db. @fn inline TArray<FString> RunTwoNpcChat(FMicroGameStore &Store) */
inline TArray<FString> RunTwoNpcChat(FMicroGameStore &Store) {
  const FTwoNpcChatConfig &Config = TwoNpcChatConfig();
  const FChatNpc NpcA = BuildNpc(Store, Config);
  const FChatNpc NpcB = BuildNpc(Store, Config);
  TArray<FString> Lines;
  Lines.Add(Config.Header);
  Lines.Append(
      RunTurns(Store, Config, NpcA, NpcB, NpcA.Seed, 0, TArray<FString>()));
  Lines.Add(Config.Footer);
  return Lines;
}

} // namespace TwoNpcChat

} // namespace MicroGame
