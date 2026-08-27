#pragma once

#include "MicroGame/Features/Components/Harness/TwoNpcChat/Definition/DefinitionTypes.h"
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

} // namespace MicroGame::TwoNpcChat
