#pragma once

#include "CoreMinimal.h"
#include "Core/fp.hpp"

/**
 * CLI Module — Pure command router.
 * Parses command keys and delegates to Ops functions.
 * No business logic lives here — all operations go through the store.
 * Mirrors TS cli.ts which routes to sdkOps.ts.
 * User Story: As an SDK integrator, I need this type or module note so I can understand the role of the surrounding API surface quickly.
 */
namespace CLIOps {

/**
 * Hydrates invocation-scoped SDK configuration before command dispatch.
 * @fn FORBOCAI_SDK_API void ConfigureInvocation(const FString &ApiUrl, const FString &ApiKey)
 * User Story: As an embedded CLI host, I need runtime overrides applied through the SDK root store without accessing that store directly.
 */
FORBOCAI_SDK_API void ConfigureInvocation(const FString &ApiUrl,
                                          const FString &ApiKey);

/**
 * Dispatches a CLI command through the SDK store.
 * @fn FORBOCAI_SDK_API func::TestResult<void> DispatchCommand(const FString &CommandKey, const TArray<FString> &Args)
 * @param CommandKey  Command in "domain_action" format (e.g. "npc_create",
 * "memory_list", "ghost_run", "soul_export").
 * @param Args        Positional arguments for the command.
 * @return TestResult indicating success or failure with message.
 * User Story: As an SDK integrator, I need this type or module note so I can understand the role of the surrounding API surface quickly.
 */
FORBOCAI_SDK_API func::TestResult<void>
DispatchCommand(const FString &CommandKey, const TArray<FString> &Args);

} // namespace CLIOps
