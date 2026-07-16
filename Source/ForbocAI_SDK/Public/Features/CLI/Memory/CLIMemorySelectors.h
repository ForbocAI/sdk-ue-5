#pragma once

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

} // namespace Memory
} // namespace CLI
} // namespace ForbocAI
