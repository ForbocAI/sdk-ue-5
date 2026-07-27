#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI diagnostics consumer, I need health and contract commands delegated through one typed SDK thunk boundary. @fn RouteResult RouteDiagnosticsCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteDiagnosticsCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                                const FString &CommandKey,
                                const TArray<FString> &Args);

} // namespace CommandRouting
} // namespace CLIOps
