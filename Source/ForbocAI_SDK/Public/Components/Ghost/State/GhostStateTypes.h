#pragma once

#include "Core/fp.hpp"
#include "Components/Ghost/Configuration/GhostConfigurationTypes.h"
#include "Components/Ghost/Identity/IdentityTypes.h"
#include "Components/Ghost/Lifecycle/LifecycleTypes.h"
#include "Components/Ghost/Results/ResultsTypes.h"

namespace GhostSlice {

struct FGhostSliceState {
  FString ActiveSessionId;
  func::Maybe<FString> ActiveGhostName{};
  func::Maybe<FGhostRuntimeIdentity> RuntimeIdentity{};
  FString Status;
  float Progress{};
  FGhostResults Results;
  bool bHasResults{};
  TArray<FGhostHistoryEntry> History;
  bool bLoading{};
  FString Error;
  FGhostConfiguration Configuration{};
};

} // namespace GhostSlice
