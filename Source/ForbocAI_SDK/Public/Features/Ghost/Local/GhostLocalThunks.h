#pragma once

// Imperative local ghost workflows live at this thunk boundary.

#include "Core/rtk.hpp"
#include "Features/Ghost/GhostTypes.h"
#include "NPC/NPCBaseTypes.h"

struct FRuntimeState;

namespace rtk {

ThunkAction<FGhostTestResult, FRuntimeState>
runLocalGhostTestThunk(const FAgent &Agent, const FString &Scenario);

} // namespace rtk
