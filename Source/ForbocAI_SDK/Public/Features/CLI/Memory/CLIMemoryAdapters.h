#pragma once

#include "Features/CLI/Memory/CLIMemoryTypes.h"
#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Memory/MemoryTypes.h"

namespace ForbocAI {
namespace CLI {
namespace Memory {

/** User Story: As a features cli memory consumer, I need to invoke read cli memory state through a stable signature so the features cli memory workflow remains explicit and composable. @fn inline FCLIMemoryState readCliMemoryState() */
inline FCLIMemoryState readCliMemoryState() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/cli/memory.json"));
  const TSharedRef<FJsonObject> Limits =
      DataAdapters::ReadObjectField(Source, TEXT("limits"));
  const TSharedRef<FJsonObject> Defaults =
      DataAdapters::ReadObjectField(Source, TEXT("defaults"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Source, TEXT("fields"));
  const TSharedRef<FJsonObject> Syntax =
      DataAdapters::ReadObjectField(Source, TEXT("syntax"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  return {
      {DataAdapters::ReadNumberField(Limits, TEXT("singleArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("doubleArgumentCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("listItemLimit")),
       DataAdapters::ReadNumberField(Limits, TEXT("recallItemLimit")),
       DataAdapters::ReadNumberField(Limits, TEXT("firstItemIndex")),
       DataAdapters::ReadNumberField(Limits, TEXT("nextItemOffset")),
       DataAdapters::ReadNumberField(Limits, TEXT("displayIndexOffset")),
       DataAdapters::ReadNumberField(Limits, TEXT("snippetLength")),
       DataAdapters::ReadNumberField(Limits, TEXT("jsonIndent"))},
      {DataAdapters::ReadFloatField(Defaults, TEXT("importance")),
       DataAdapters::ReadFloatField(Defaults, TEXT("recallSimilarity")),
       DataAdapters::ReadStringField(Defaults, TEXT("confirmation"))},
      {DataAdapters::ReadStringField(Fields, TEXT("text")),
       DataAdapters::ReadStringField(Fields, TEXT("type")),
       DataAdapters::ReadStringField(Fields, TEXT("importance")),
       DataAdapters::ReadStringField(Fields, TEXT("memories"))},
      {DataAdapters::ReadStringField(Syntax, TEXT("listSeparator"))},
      {DataAdapters::ReadStringField(Messages, TEXT("unknownError")),
       DataAdapters::ReadStringField(Messages, TEXT("listUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("listing")),
       DataAdapters::ReadStringField(Messages, TEXT("none")),
       DataAdapters::ReadStringField(Messages, TEXT("listItem")),
       DataAdapters::ReadStringField(Messages, TEXT("recallUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("noneRelevant")),
       DataAdapters::ReadStringField(Messages, TEXT("recallItem")),
       DataAdapters::ReadStringField(Messages, TEXT("storeUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("stored")),
       DataAdapters::ReadStringField(Messages, TEXT("clearUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("clearWarning")),
       DataAdapters::ReadStringField(Messages, TEXT("clearPrompt")),
       DataAdapters::ReadStringField(Messages, TEXT("cleared")),
       DataAdapters::ReadStringField(Messages, TEXT("aborted")),
       DataAdapters::ReadStringField(Messages, TEXT("exportUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("exporting")),
       DataAdapters::ReadStringField(Messages, TEXT("found")),
       DataAdapters::ReadStringField(Messages, TEXT("recalled")),
       DataAdapters::ReadStringField(Messages, TEXT("listed")),
       DataAdapters::ReadStringField(Messages, TEXT("recallDone")),
       DataAdapters::ReadStringField(Messages, TEXT("storeDone")),
       DataAdapters::ReadStringField(Messages, TEXT("clearDone")),
       DataAdapters::ReadStringField(Messages, TEXT("exportDone"))}};
}

/** User Story: As a features cli memory consumer, I need to invoke serialize memories through a stable signature so the features cli memory workflow remains explicit and composable. @fn inline FString serializeMemories(const TArray<FMemoryItem> &Items, const FCLIMemoryState &State) */
inline FString serializeMemories(const TArray<FMemoryItem> &Items,
                                 const FCLIMemoryState &State) {
  const TArray<TSharedPtr<FJsonValue>> JsonItems =
      func::map_array<FMemoryItem, TSharedPtr<FJsonValue>>(
          Items, [&State](const FMemoryItem &Item) {
            const TSharedPtr<FJsonObject> Object = MakeShared<FJsonObject>();
            Object->SetStringField(State.Fields.Text, Item.Text);
            Object->SetStringField(State.Fields.Type, Item.Type);
            Object->SetNumberField(State.Fields.Importance, Item.Importance);
            return MakeShared<FJsonValueObject>(Object);
          });
  const TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
  Root->SetArrayField(State.Fields.Memories, JsonItems);
  return DataAdapters::SerializeObject(Root);
}

} // namespace Memory
} // namespace CLI
} // namespace ForbocAI
