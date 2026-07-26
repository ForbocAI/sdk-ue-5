#pragma once

#include "Core/JsonInterop.h"
#include "MicroGame/Features/Data/DataAdapters.h"

namespace MicroGame::ChatTranscriptTests {

struct FChatTestSample {
  FString Id;
  FString Command;
  FString Prompt;
  FString Response;
  double DurationMs{};
};

struct FChatTestStories {
  FString CommandParsing;
  FString Selection;
  FString MissingEvidence;
  FString ChatExecution;
  FString FinalCommand;
  FString LogOrdering;
  FString FailureOrdering;
};

struct FChatConversationEntryTestData {
  FString At;
  FString CommandGroup;
  FString Command;
  FString Status;
  FString Output;
};

struct FChatConversationTestData {
  FChatConversationEntryTestData Entry;
  FString ResponseJson;
  TArray<FString> TranscriptLines;
  FChatConversationEntryTestData FailureEntry;
};

struct FChatTestData {
  FString AutomationName;
  FChatTestStories Stories;
  TArray<FChatTestSample> Samples;
  FString Summary;
  FChatConversationTestData Conversation;
};

/** User Story: As a UE chat test, I need authored transcript examples decoded independently from production selectors. @fn inline const FChatTestData &ChatTestData() */
inline const FChatTestData &ChatTestData() {
  static const FChatTestData Data = []() {
    const TSharedRef<FJsonObject> Root =
        DataAdapters::SettingsSource(TEXT("tests/chat.json")).Root;
    const TSharedRef<FJsonObject> Stories =
        DataAdapters::ReadObjectField(Root, TEXT("stories"));
    const TSharedRef<FJsonObject> Conversation =
        DataAdapters::ReadObjectField(Root, TEXT("conversation"));
    const TSharedRef<FJsonObject> ConversationEntry =
        DataAdapters::ReadObjectField(Conversation, TEXT("entry"));
    const TSharedRef<FJsonObject> ConversationFailure =
        DataAdapters::ReadObjectField(Conversation, TEXT("failure"));
    const TSharedRef<FJsonObject> ConversationFailureEntry =
        DataAdapters::ReadObjectField(ConversationFailure, TEXT("entry"));
    const TSharedRef<FJsonObject> ConversationResponse =
        DataAdapters::ReadObjectField(Conversation, TEXT("response"));
    return FChatTestData{
        DataAdapters::ReadStringField(Root, TEXT("automationName")),
        {DataAdapters::ReadStringField(Stories, TEXT("commandParsing")),
         DataAdapters::ReadStringField(Stories, TEXT("selection")),
         DataAdapters::ReadStringField(Stories, TEXT("missingEvidence")),
         DataAdapters::ReadStringField(Stories, TEXT("chatExecution")),
         DataAdapters::ReadStringField(Stories, TEXT("finalCommand")),
         DataAdapters::ReadStringField(Stories, TEXT("logOrdering")),
         DataAdapters::ReadStringField(Stories, TEXT("failureOrdering"))},
        func::map_array<TSharedPtr<FJsonValue>, FChatTestSample>(
            DataAdapters::ReadObjectArrayField(Root, TEXT("samples")),
            [](const TSharedPtr<FJsonValue> &Value) {
              const TSharedRef<FJsonObject> Object =
                  Value->AsObject().ToSharedRef();
              return FChatTestSample{
                  DataAdapters::ReadStringField(Object, TEXT("id")),
                  DataAdapters::ReadStringField(Object, TEXT("command")),
                  DataAdapters::ReadStringField(Object, TEXT("prompt")),
                  DataAdapters::ReadStringField(Object, TEXT("response")),
                  DataAdapters::ReadDoubleField(Object, TEXT("durationMs"))};
            }),
        DataAdapters::ReadStringField(Root, TEXT("summary")),
        {{DataAdapters::ReadStringField(ConversationEntry, TEXT("at")),
          DataAdapters::ReadStringField(ConversationEntry,
                                        TEXT("commandGroup")),
          DataAdapters::ReadStringField(ConversationEntry, TEXT("command")),
          DataAdapters::ReadStringField(ConversationEntry, TEXT("status")),
          DataAdapters::ReadStringField(ConversationEntry, TEXT("output"))},
         JsonInterop::StringifyObject(ConversationResponse),
         DataAdapters::ReadStringArrayField(ConversationResponse,
                                            TEXT("transcriptLines")),
         {DataAdapters::ReadStringField(ConversationFailureEntry,
                                        TEXT("at")),
          DataAdapters::ReadStringField(ConversationFailureEntry,
                                        TEXT("commandGroup")),
          DataAdapters::ReadStringField(ConversationFailureEntry,
                                        TEXT("command")),
          DataAdapters::ReadStringField(ConversationFailureEntry,
                                        TEXT("status")),
          DataAdapters::ReadStringField(ConversationFailureEntry,
                                        TEXT("output"))}}};
  }();
  return Data;
}

} // namespace MicroGame::ChatTranscriptTests
