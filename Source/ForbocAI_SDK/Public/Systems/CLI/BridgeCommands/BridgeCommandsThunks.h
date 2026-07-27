#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI bridge consumer, I need bridge commands delegated through one typed SDK thunk boundary. @fn RouteResult RouteBridgeCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteBridgeCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                           const FString &CommandKey,
                           const TArray<FString> &Args);

} // namespace CommandRouting
} // namespace CLIOps
