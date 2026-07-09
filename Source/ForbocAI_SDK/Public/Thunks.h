// User Story: As a developer, I need this module to function.
#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"

#include "Features/API/APIThunks.h"
#include "Features/Bridge/BridgeThunks.h"
#include "Features/Ghost/GhostThunks.h"
#include "Features/Memory/MemoryThunks.h"
#include "Features/Protocol/ProtocolThunks.h"
#include "Features/Soul/SoulThunks.h"

namespace ForbocAI { namespace SDK { namespace FunctionalCoreContracts {
typedef func::Maybe<FString> FForbocAISDKPublicThunksHOptionalDomainId;
} } }
