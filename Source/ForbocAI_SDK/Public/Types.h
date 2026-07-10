#pragma once

#include "Core/rtk.hpp"

#include "Core/ecs.hpp"
#include "Core/frmt.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"

/**
 * Subdomain Types
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
#include "Bridge/BridgeTypes.h"
#include "Features/API/APITypes.h"
#include "Ghost/GhostTypes.h"
#include "Memory/MemoryTypes.h"
#include "NPC/NPCBaseTypes.h"
#include "Protocol/ProtocolRequestTypes.h"
#include "Protocol/ProtocolTypes.h"
#include "Soul/SoulTypes.h"

/**
 * UE FP Core Type Aliases for SDK types
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
namespace SDKTypes {
using func::AsyncResult;
using func::ConfigBuilder;
using func::Curried;
using func::Either;
using func::Lazy;
using func::Maybe;
using func::Pipeline;
using func::TestResult;
using func::ValidationPipeline;

/**
 * Type aliases for SDK operations
 * User Story: As a maintainer, I need this section note so related declarations and logic stay easy to locate.
 */
using AgentCreationResult = Either<FString, FAgent>;
using AgentValidationResult = Either<FString, FAgentState>;
using AgentProcessResult = Either<FString, FAgentResponse>;
using AgentExportResult = Either<FString, FSoul>;
using MemoryStoreResult = Either<FString, FMemoryStore>;
using GhostTestResult = TestResult<FGhostTestResult>;
using BridgeValidationResult = Either<FString, FValidationResult>;
} // namespace SDKTypes
