#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI memory consumer, I need memory commands delegated through one typed SDK thunk boundary. @fn RouteResult RouteMemoryCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteMemoryCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                           const FString &CommandKey,
                           const TArray<FString> &Args);

} // namespace CommandRouting
} // namespace CLIOps
