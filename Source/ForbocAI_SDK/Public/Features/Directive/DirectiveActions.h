#pragma once
/**
 * Directive action creators own directive event construction.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Types.h"

namespace DirectiveSlice {

using namespace rtk;
using namespace func;

struct FDirectiveRunStartedPayload {
  FString Id;
  FString NpcId;
  FString Observation;
}

} // namespace DirectiveSlice
