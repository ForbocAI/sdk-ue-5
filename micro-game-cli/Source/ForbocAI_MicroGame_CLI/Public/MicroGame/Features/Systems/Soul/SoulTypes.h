#pragma once

#include "CoreMinimal.h"

namespace MicroGame {

struct FSoulTrackingState {
  TMap<FString, FString> ExportsByNpc;
  TArray<FString> ImportedSoulTxIds;
};

typedef FSoulTrackingState FSoulState;

} // namespace MicroGame
