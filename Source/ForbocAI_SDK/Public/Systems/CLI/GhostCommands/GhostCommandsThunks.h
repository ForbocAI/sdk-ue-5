#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI Ghost consumer, I need Ghost commands delegated through one typed SDK thunk boundary. @fn RouteResult RouteGhostCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteGhostCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args);

} // namespace CommandRouting
} // namespace CLIOps
