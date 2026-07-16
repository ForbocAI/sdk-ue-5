#pragma once

// Imperative local ghost workflows live at this thunk boundary.

#include "Core/rtk.hpp"
#include "Features/Ghost/GhostTypes.h"
#include "NPC/NPCBaseTypes.h"

struct FRuntimeState;

namespace rtk {

/** User Story: As a features ghost local consumer, I need to invoke run local ghost test thunk through a stable signature so the features ghost local workflow remains explicit and composable. @fn ThunkAction<FGhostTestResult, FRuntimeState> runLocalGhostTestThunk(const FAgent &Agent, const FString &Scenario) */
ThunkAction<FGhostTestResult, FRuntimeState>
runLocalGhostTestThunk(const FAgent &Agent, const FString &Scenario);

} // namespace rtk
