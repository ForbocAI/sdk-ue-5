#pragma once
/**
 * Ghost action creators own ghost session event construction.
 */

#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "CoreMinimal.h"
#include "Features/Contracts/ContractsTypes.h"

namespace GhostSlice {

using namespace rtk;

struct FGhostSessionStartedPayload {
  FString SessionId;
  FString Status;
};

} // namespace GhostSlice
