#pragma once

#include "MicroGame/Features/Components/Testing/Harness/TwoNpcChat/Fixture/FixtureTypes.h"
#include "MicroGame/Features/Data/DataAdapters.h"

namespace MicroGame::Testing::TwoNpcChat {

/** User Story: As two-NPC adapter tests, I need all registration and assertion labels loaded from authored test data. @fn inline const FTwoNpcChatTestingData &TwoNpcChatTestingData() */
inline const FTwoNpcChatTestingData &TwoNpcChatTestingData() {
  static const FTwoNpcChatTestingData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("tests/harness/two-npc-chat.json"));
    const TSharedRef<FJsonObject> Labels =
        DataAdapters::ReadObjectField(Source, TEXT("labels"));
    FTwoNpcChatTestingData Value;
    Value.AutomationName =
        DataAdapters::ReadStringField(Source.Root, TEXT("automationName"));
    Value.Labels = {
        DataAdapters::ReadStringField(Labels, TEXT("configuration")),
        DataAdapters::ReadStringField(Labels, TEXT("numbers")),
        DataAdapters::ReadStringField(Labels, TEXT("generateValues")),
        DataAdapters::ReadStringField(Labels, TEXT("generateTokens")),
        DataAdapters::ReadStringField(Labels, TEXT("createToken")),
        DataAdapters::ReadStringField(Labels, TEXT("decideTokens")),
        DataAdapters::ReadStringField(Labels, TEXT("memoryTokens")),
        DataAdapters::ReadStringField(Labels, TEXT("lineTokens")),
        DataAdapters::ReadStringField(Labels, TEXT("context")),
    };
    return Value;
  }();
  return Data;
}

} // namespace MicroGame::Testing::TwoNpcChat
