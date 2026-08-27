#pragma once

#include "Core/JsonInterop.h"
#include "Internationalization/Regex.h"
#include "MicroGame/Features/Components/Harness/TwoNpcChat/TwoNpcChatTypes.h"
#include "MicroGame/Features/Data/DataAdapters.h"

namespace MicroGame::TwoNpcChat {

/** User Story: As the two-NPC chat game orchestrator, I need one typed authored configuration so command composition and parsing share one JSON authority. @fn inline const FTwoNpcChatConfig &TwoNpcChatConfig() */
inline const FTwoNpcChatConfig &TwoNpcChatConfig() {
  static const FTwoNpcChatConfig Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("harness/two-npc-chat.json"));
    const TSharedRef<FJsonObject> Root = Source.Root;
    const TSharedRef<FJsonObject> Tokens =
        DataAdapters::ReadObjectField(Source, TEXT("tokens"));
    const TSharedRef<FJsonObject> Numbers =
        DataAdapters::ReadObjectField(Source, TEXT("numbers"));
    FTwoNpcChatConfig Config;
    Config.Group = DataAdapters::ReadStringField(Root, TEXT("group"));
    Config.Header = DataAdapters::ReadStringField(Root, TEXT("header"));
    Config.Footer = DataAdapters::ReadStringField(Root, TEXT("footer"));
    Config.TurnCount = DataAdapters::ReadNumberField(Root, TEXT("turnCount"));
    Config.ParticipantCount =
        DataAdapters::ReadNumberField(Root, TEXT("participantCount"));
    Config.NameAttribute =
        DataAdapters::ReadStringField(Root, TEXT("nameAttribute"));
    Config.PersonaAttributes =
        DataAdapters::ReadStringArrayField(Root, TEXT("personaAttributes"));
    Config.SeedAttribute =
        DataAdapters::ReadStringField(Root, TEXT("seedAttribute"));
    Config.GenerateCommand =
        DataAdapters::ReadStringField(Root, TEXT("generateCommand"));
    Config.CreateCommand =
        DataAdapters::ReadStringField(Root, TEXT("createCommand"));
    Config.DecideCommand =
        DataAdapters::ReadStringField(Root, TEXT("decideCommand"));
    Config.MemoryStoreCommand =
        DataAdapters::ReadStringField(Root, TEXT("memoryStoreCommand"));
    Config.ValueField =
        DataAdapters::ReadStringField(Root, TEXT("valueField"));
    Config.PersonaSeparator =
        DataAdapters::ReadStringField(Root, TEXT("personaSeparator"));
    Config.ContextSeparator =
        DataAdapters::ReadStringField(Root, TEXT("contextSeparator"));
    Config.EmptyContext =
        DataAdapters::ReadStringField(Root, TEXT("emptyContext"));
    Config.IdPattern =
        DataAdapters::ReadStringField(Root, TEXT("idPattern"));
    Config.LineFormat =
        DataAdapters::ReadStringField(Root, TEXT("lineFormat"));
    Config.DialogueFallback =
        DataAdapters::ReadStringField(Root, TEXT("dialogueFallback"));
    Config.ContextQuote =
        DataAdapters::ReadStringField(Root, TEXT("contextQuote"));
    Config.Tokens = {
        DataAdapters::ReadStringField(Tokens, TEXT("attribute")),
        DataAdapters::ReadStringField(Tokens, TEXT("context")),
        DataAdapters::ReadStringField(Tokens, TEXT("persona")),
        DataAdapters::ReadStringField(Tokens, TEXT("id")),
        DataAdapters::ReadStringField(Tokens, TEXT("line")),
        DataAdapters::ReadStringField(Tokens, TEXT("name")),
        DataAdapters::ReadStringField(Tokens, TEXT("dialogue")),
        DataAdapters::ReadStringField(Tokens, TEXT("memory")),
    };
    Config.Numbers = {
        DataAdapters::ReadNumberField(Numbers, TEXT("initialIndex")),
        DataAdapters::ReadNumberField(Numbers, TEXT("nextIndex")),
    };
    return Config;
  }();
  return Data;
}

/** User Story: As a two-NPC chat adapter, I need one authored token replaced without duplicating template syntax. @fn inline FString ApplyToken(const FString &Template, const FString &Token, const FString &Value) */
inline FString ApplyToken(const FString &Template, const FString &Token,
                          const FString &Value) {
  return Template.Replace(*Token, *Value);
}

/** User Story: As a two-NPC chat adapter, I need a granular generate command composed from authored tokens. @fn inline FString FormatGenerate(const FTwoNpcChatConfig &Config, const FString &Attribute, const FString &Context) */
inline FString FormatGenerate(const FTwoNpcChatConfig &Config,
                              const FString &Attribute,
                              const FString &Context) {
  const FString Quoted = Context.IsEmpty()
                             ? Config.EmptyContext
                             : Config.ContextQuote + Context + Config.ContextQuote;
  return ApplyToken(
      ApplyToken(Config.GenerateCommand, Config.Tokens.Attribute, Attribute),
      Config.Tokens.Context, Quoted);
}

/** User Story: As a two-NPC chat adapter, I need an NPC create command composed from an authored persona token. @fn inline FString FormatCreate(const FTwoNpcChatConfig &Config, const FString &Persona) */
inline FString FormatCreate(const FTwoNpcChatConfig &Config,
                            const FString &Persona) {
  return ApplyToken(Config.CreateCommand, Config.Tokens.Persona, Persona);
}

/** User Story: As a two-NPC chat adapter, I need a decide command composed from authored identity and line tokens. @fn inline FString FormatDecide(const FTwoNpcChatConfig &Config, const FString &Id, const FString &Line) */
inline FString FormatDecide(const FTwoNpcChatConfig &Config, const FString &Id,
                            const FString &Line) {
  return ApplyToken(ApplyToken(Config.DecideCommand, Config.Tokens.Id, Id),
                    Config.Tokens.Line, Line);
}

/** User Story: As a two-NPC chat adapter, I need a memory command composed from authored identity and memory tokens. @fn inline FString FormatMemoryStore(const FTwoNpcChatConfig &Config, const FString &Id, const FString &Memory) */
inline FString FormatMemoryStore(const FTwoNpcChatConfig &Config,
                                 const FString &Id, const FString &Memory) {
  return ApplyToken(
      ApplyToken(Config.MemoryStoreCommand, Config.Tokens.Id, Id),
      Config.Tokens.Memory, Memory);
}

/** User Story: As a two-NPC chat adapter, I need one transcript line composed from authored speaker and dialogue tokens. @fn inline FString FormatLine(const FTwoNpcChatConfig &Config, const FString &Name, const FString &Dialogue) */
inline FString FormatLine(const FTwoNpcChatConfig &Config, const FString &Name,
                          const FString &Dialogue) {
  return ApplyToken(ApplyToken(Config.LineFormat, Config.Tokens.Name, Name),
                    Config.Tokens.Dialogue, Dialogue);
}

/** User Story: As a two-NPC chat adapter, I need the generated value read through the authored response field. @fn inline FString ReadGeneratedValue(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadGeneratedValue(const FTwoNpcChatConfig &Config,
                                  const FString &Output) {
  TSharedPtr<FJsonObject> Root;
  return (JsonInterop::ParseJsonObject(Output, Root) && Root.IsValid() &&
          Root->HasTypedField<EJson::String>(Config.ValueField))
             ? Root->GetStringField(Config.ValueField)
             : Config.EmptyContext;
}

/** User Story: As a two-NPC chat adapter, I need a created NPC id parsed with the authored pattern and index. @fn inline FString ReadNpcId(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadNpcId(const FTwoNpcChatConfig &Config,
                         const FString &Output) {
  FRegexMatcher Matcher(FRegexPattern(Config.IdPattern), Output);
  return Matcher.FindNext()
             ? Matcher.GetCaptureGroup(Config.Numbers.InitialIndex)
             : Config.EmptyContext;
}

/** User Story: As a two-NPC chat adapter, I need spoken dialogue normalized with the authored fallback. @fn inline FString ReadDialogue(const FTwoNpcChatConfig &Config, const FString &Output) */
inline FString ReadDialogue(const FTwoNpcChatConfig &Config,
                            const FString &Output) {
  const FString Spoken = Output.TrimStartAndEnd();
  return Spoken.IsEmpty() ? Config.DialogueFallback : Spoken;
}

/** User Story: As a two-NPC chat adapter, I need generated values joined through the authored persona separator. @fn inline FString JoinPersona(const FTwoNpcChatConfig &Config, const TArray<FString> &Values) */
inline FString JoinPersona(const FTwoNpcChatConfig &Config,
                           const TArray<FString> &Values) {
  return FString::Join(Values, *Config.PersonaSeparator);
}

/** User Story: As a two-NPC chat adapter, I need generated context extended through the authored separator. @fn inline FString ExtendContext(const FTwoNpcChatConfig &Config, const FString &Context, const FString &Value) */
inline FString ExtendContext(const FTwoNpcChatConfig &Config,
                             const FString &Context, const FString &Value) {
  return Context.IsEmpty() ? Value
                           : Context + Config.ContextSeparator + Value;
}

/** User Story: As a two-NPC chat adapter, I need the opening seed selected by authored attribute name. @fn inline FString SeedValue(const FTwoNpcChatConfig &Config, const TArray<FString> &Values) */
inline FString SeedValue(const FTwoNpcChatConfig &Config,
                         const TArray<FString> &Values) {
  const int32 Index =
      Config.PersonaAttributes.IndexOfByKey(Config.SeedAttribute);
  return Values.IsValidIndex(Index) ? Values[Index] : Config.DialogueFallback;
}

} // namespace MicroGame::TwoNpcChat
