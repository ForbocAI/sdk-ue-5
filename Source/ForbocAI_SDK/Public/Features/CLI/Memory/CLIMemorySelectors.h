#pragma once

#include "Core/fp.hpp"
#include "Features/CLI/Memory/CLIMemoryTypes.h"
#include "Features/Memory/MemoryTypes.h"

namespace ForbocAI {
namespace CLI {
struct FCLIState;
namespace Memory {

/** User Story: As a features cli memory consumer, I need to invoke select cli memory through a stable signature so the features cli memory workflow remains explicit and composable. @fn const FCLIMemoryState &selectCliMemory(const FCLIState &State) */
const FCLIMemoryState &selectCliMemory(const FCLIState &State);

/** User Story: As a features cli memory consumer, I need to invoke select memory items through a stable signature so the features cli memory workflow remains explicit and composable. @fn inline TArray<FMemoryItem> selectMemoryItems(const TArray<FMemoryItem> &Items, int32 Limit) */
inline TArray<FMemoryItem> selectMemoryItems(const TArray<FMemoryItem> &Items,
                                            int32 Limit) {
  TArray<FMemoryItem> Selected = Items;
  Selected.SetNum(FMath::Min(Selected.Num(), Limit));
  return Selected;
}

/** User Story: As a features cli memory consumer, I need to invoke select memory snippet through a stable signature so the features cli memory workflow remains explicit and composable. @fn inline FString selectMemorySnippet(const FMemoryItem &Item, const FCLIMemoryState &State) */
inline FString selectMemorySnippet(const FMemoryItem &Item,
                                   const FCLIMemoryState &State) {
  return Item.Text.Left(State.Limits.SnippetLength);
}

/** User Story: As a CLI memory consumer, I need list rows derived from persisted memory data so command results expose the same evidence as presentation logs. @fn inline FString selectMemoryListOutput(const TArray<FMemoryItem> &Items, const FCLIMemoryState &State) */
inline FString selectMemoryListOutput(const TArray<FMemoryItem> &Items,
                                      const FCLIMemoryState &State) {
  const TArray<FString> Lines =
      func::fold_array<FMemoryItem, TArray<FString>>(
          Items, TArray<FString>(),
          [&State](const TArray<FString> &Output,
                   const FMemoryItem &Item) {
            const int32 Index =
                State.Limits.FirstItemIndex +
                Output.Num() * State.Limits.NextItemOffset +
                State.Limits.DisplayIndexOffset;
            return func::append_value<FString>(
                Output,
                FString::Format(*State.Messages.ListItem,
                                {Index, selectMemorySnippet(Item, State)}));
          });
  return Lines.IsEmpty()
             ? State.Messages.None
             : FString::Join(Lines, *State.Syntax.ListSeparator);
}

/** User Story: As a CLI memory consumer, I need recall rows derived from vector results so command success carries the retrieved evidence. @fn inline FString selectMemoryRecallOutput(const TArray<FMemoryItem> &Items, const FCLIMemoryState &State) */
inline FString selectMemoryRecallOutput(const TArray<FMemoryItem> &Items,
                                        const FCLIMemoryState &State) {
  const TArray<FString> Lines =
      func::fold_array<FMemoryItem, TArray<FString>>(
          Items, TArray<FString>(),
          [&State](const TArray<FString> &Output,
                   const FMemoryItem &Item) {
            const int32 Index =
                State.Limits.FirstItemIndex +
                Output.Num() * State.Limits.NextItemOffset +
                State.Limits.DisplayIndexOffset;
            return func::append_value<FString>(
                Output, FString::Format(*State.Messages.RecallItem,
                                        {Index, Item.Text}));
          });
  return Lines.IsEmpty()
             ? State.Messages.NoneRelevant
             : FString::Join(Lines, *State.Syntax.ListSeparator);
}

} // namespace Memory
} // namespace CLI
} // namespace ForbocAI
