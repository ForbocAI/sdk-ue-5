#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI Soul consumer, I need Soul commands delegated through one typed SDK thunk boundary. @fn RouteResult RouteSoulCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteSoulCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                         const FString &CommandKey,
                         const TArray<FString> &Args);

} // namespace CommandRouting
} // namespace CLIOps
