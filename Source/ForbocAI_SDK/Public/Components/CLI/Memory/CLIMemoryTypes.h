#pragma once

#include "CoreMinimal.h"

namespace ForbocAI {
namespace CLI {
namespace Memory {

struct FCLIMemoryLimits {
  int32 SingleArgumentCount;
  int32 DoubleArgumentCount;
  int32 ListItemLimit;
  int32 RecallItemLimit;
  int32 FirstItemIndex;
  int32 NextItemOffset;
  int32 DisplayIndexOffset;
  int32 SnippetLength;
  int32 JsonIndent;
};

struct FCLIMemoryDefaults {
  float Importance;
  float RecallSimilarity;
  FString Confirmation;
};

struct FCLIMemoryFields {
  FString Text;
  FString Type;
  FString Importance;
  FString Memories;
};

struct FCLIMemorySyntax {
  FString ListSeparator;
};

struct FCLIMemoryMessages {
  FString TypeOption;
  FString UnknownError;
  FString ListUsage;
  FString Listing;
  FString None;
  FString ListItem;
  FString RecallUsage;
  FString NoneRelevant;
  FString RecallItem;
  FString StoreUsage;
  FString Stored;
  FString ClearUsage;
  FString ClearWarning;
  FString ClearPrompt;
  FString Cleared;
  FString Aborted;
  FString ExportUsage;
  FString Exporting;
  FString Found;
  FString Recalled;
  FString Listed;
  FString RecallDone;
  FString StoreDone;
  FString ClearDone;
  FString ExportDone;
};

struct FCLIMemoryState {
  FCLIMemoryLimits Limits;
  FCLIMemoryDefaults Defaults;
  FCLIMemoryFields Fields;
  FCLIMemorySyntax Syntax;
  FCLIMemoryMessages Messages;
};

} // namespace Memory
} // namespace CLI
} // namespace ForbocAI
