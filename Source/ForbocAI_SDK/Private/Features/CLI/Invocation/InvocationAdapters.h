#pragma once

#include "Features/CLI/CLITypes.h"
#include "Features/CLI/Invocation/InvocationTypes.h"

namespace CommandletInvocation {

/** User Story: As a features cli invocation consumer, I need to invoke resolve invocation through a stable signature so the features cli invocation workflow remains explicit and composable. @fn FInvocation ResolveInvocation(const FString &Params, const ForbocAI::CLI::FCLIState &CLIState) */
FInvocation ResolveInvocation(const FString &Params,
                              const ForbocAI::CLI::FCLIState &CLIState);

} // namespace CommandletInvocation
