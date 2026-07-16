#pragma once

#include "Core/rtk.hpp"
#include "Features/Bridge/BridgeSlice.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Ghost/GhostSlice.h"
#include "Features/Memory/MemorySlice.h"
#include "Features/NPC/NPCActions.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Soul/SoulSlice.h"

namespace NPCListeners {

/** User Story: As a features npc consumer, I need to invoke create npc removal listener through a stable signature so the features npc workflow remains explicit and composable. @fn template <typename State> inline rtk::Middleware<State> createNpcRemovalListener() */
template <typename State>
inline rtk::Middleware<State> createNpcRemovalListener() {
  return [](const rtk::MiddlewareApi<State> &Api)
             -> std::function<rtk::Dispatcher(rtk::Dispatcher)> {
    return [Api](rtk::Dispatcher Next) -> rtk::Dispatcher {
      return [Api, Next](const rtk::AnyAction &Action) -> rtk::AnyAction {
        const FString ActiveNpcIdBefore = Api.getState().NPCs.ActiveNpcId;
        const rtk::AnyAction Result = Next(Action);

        NPCActions::removeNPCActionCreator().match(Action)
            ? [&]() {
                const auto RemovedNpcId =
                    NPCActions::removeNPCActionCreator().extract(Action);
                RemovedNpcId.hasValue
                    ? (Api.dispatch(
                           DirectiveSlice::Actions::clearDirectivesForNpc(
                               RemovedNpcId.value)),
                       Api.dispatch(
                           BridgeSlice::Actions::validationCleared()),
                       Api.dispatch(GhostSlice::Actions::clearGhostSession()),
                       Api.dispatch(SoulSlice::Actions::clearSoulState()),
                       RemovedNpcId.value == ActiveNpcIdBefore
                           ? (Api.dispatch(MemorySlice::Actions::memoryClear()),
                              void())
                           : void(),
                       void())
                    : void();
              }()
            : void();

        return Result;
      };
    };
  };
}

} // namespace NPCListeners
