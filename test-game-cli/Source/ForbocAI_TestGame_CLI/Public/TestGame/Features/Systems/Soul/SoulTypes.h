#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FSoulTrackingState {
  TMap<FString, FString> ExportsByNpc;
  TArray<FString> ImportedSoulTxIds;
};

typedef FSoulTrackingState FSoulState;

} // namespace TestGame
