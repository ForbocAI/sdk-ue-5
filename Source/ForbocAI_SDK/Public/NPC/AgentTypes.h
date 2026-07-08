#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"

/**
 * DEPRECATED: Include "NPC/NPCBaseTypes.h" instead.
 * This forwarding header keeps the prior include path compiling. All struct
 * definitions (FAgentState, FAgentAction, FAgent, FAgentConfig,
 * FAgentResponse, FImportedNpc) have moved to NPCBaseTypes.h as part
 * of the Agent → NPC terminology migration.
 * Struct names are unchanged for Blueprint continuity.
 * User Story: As an integrator upgrading SDK includes, I need this deprecation note so I can move to the supported header while existing includes keep compiling.
 */

#include "NPCBaseTypes.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicNPCAgentTypesHOptionalDomainId;
} } }
