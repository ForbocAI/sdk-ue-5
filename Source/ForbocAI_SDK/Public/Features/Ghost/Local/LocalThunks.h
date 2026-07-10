#pragma once

#include "Core/rtk.hpp"
#include "Ghost/GhostTypes.h"
#include "NPC/NPCBaseTypes.h"
#include "RuntimeStore.h"

namespace rtk {

ThunkAction<FGhostTestResult, FRuntimeState>
runLocalGhostTestThunk(const FAgent &Agent, const FString &Scenario);

} // namespace rtk
