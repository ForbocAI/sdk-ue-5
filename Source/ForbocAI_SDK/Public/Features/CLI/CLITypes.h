#pragma once

#include "CoreMinimal.h"

namespace ForbocAI {
namespace CLI {

struct FCommandRoute {
  FString Key;
  FString Group;
  FString Subcommand;
};

struct FCommandParseResult {
  bool bMatched;
  FString CommandKey;
  TArray<FString> Args;
};

} // namespace CLI
} // namespace ForbocAI
