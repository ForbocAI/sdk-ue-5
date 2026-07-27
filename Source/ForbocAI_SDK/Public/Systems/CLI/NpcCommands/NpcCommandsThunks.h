#pragma once

#include "Components/CLI/CommandRouting/CommandRoutingTypes.h"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI NPC consumer, I need NPC commands delegated through one typed SDK thunk boundary. @fn RouteResult RouteNpcCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteNpcCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                        const FString &CommandKey,
                        const TArray<FString> &Args);

} // namespace CommandRouting
} // namespace CLIOps
