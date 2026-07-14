#pragma once

#include "CoreMinimal.h"

struct FVectorState {
  FString Status = TEXT("idle");
  bool bIsReady = false;
  FString Error;
};
