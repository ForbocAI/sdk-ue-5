#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI setup consumer, I need dependency setup commands delegated through one typed SDK thunk boundary. @fn RouteResult RouteDependencyCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteDependencyCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args);

} // namespace CommandRouting
} // namespace CLIOps
