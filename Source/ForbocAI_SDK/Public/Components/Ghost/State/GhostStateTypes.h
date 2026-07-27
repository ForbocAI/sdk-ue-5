#pragma once

#include "Components/Ghost/GhostTypes.h"

namespace GhostSlice {

struct FGhostSliceState {
  FString ActiveSessionId;
  FString Status;
  float Progress{};
  FGhostTestReport Results;
  bool bHasResults{};
  TArray<FGhostHistoryEntry> History;
  bool bLoading{};
  FString Error;
};

} // namespace GhostSlice
