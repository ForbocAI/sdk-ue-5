#pragma once
/**
 * Test-game cross-slice listener middleware — mirrors TS listeners.ts
 * Reactive side effects: NPC movement → UI message
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "CoreMinimal.h"
#include "Core/frmt.hpp"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame {

/**
 * Creates listener middleware that logs NPC movement to the UI message box.
 * Mirrors TS: npcsActions.moveNPC → uiActions.addMessage
 * User Story: As test-game reactive UI, I need listener middleware so NPC
 * movement and verdict application emit readable terminal messages.
 */
inline rtk::Middleware<FTestGameState> createGameListenerMiddleware() {
  return [](const rtk::MiddlewareApi<FTestGameState> &Api)
             -> std::function<rtk::Dispatcher(rtk::Dispatcher)> {
    return [Api](rtk::Dispatcher Next) -> rtk::Dispatcher {
      return [Api, Next](const rtk::AnyAction &Action) -> rtk::AnyAction {
        /**
         * Let reducers run first
         * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
         */
        const rtk::AnyAction Result = Next(Action);

        /**
         * React to NPC movement
         * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
         */
        NPCsActions::MoveNPCActionCreator().match(Action)
            ? [&]() {
                auto Payload =
                    NPCsActions::MoveNPCActionCreator().extract(Action);
                Payload.hasValue
                    ? [&]() {
                        const auto &State = Api.getState();
                        auto MaybeNpc =
                            NPCsSelectors::SelectNpcById(State.NPCs,
                                                         Payload.value.Id);
                        MaybeNpc.hasValue
                            ? (Api.dispatch(UIActions::addMessage(
                                   frmt::RuntimeString(
                                       TEXT("%s moved to %d,%d"),
                                       frmt::Args({
                                           frmt::Arg(MaybeNpc.value.Name),
                                           frmt::Arg(Payload.value.Position.X),
                                           frmt::Arg(Payload.value.Position.Y),
                                       })))),
                               void())
                            : void();
                      }()
                    : (void)0;
              }()
            : (void)0;

        /**
         * React to NPC verdict application
         * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
         */
        NPCsActions::ApplyNpcVerdictActionCreator().match(Action)
            ? [&]() {
                auto Payload =
                    NPCsActions::ApplyNpcVerdictActionCreator().extract(
                        Action);
                Payload.hasValue
                    ? [&]() {
                        const auto &State = Api.getState();
                        auto MaybeNpc =
                            NPCsSelectors::SelectNpcById(State.NPCs,
                                                         Payload.value.Id);
                        MaybeNpc.hasValue
                            ? (Api.dispatch(UIActions::addMessage(
                                   frmt::RuntimeString(
                                       TEXT("Verdict applied to %s "
                                            "(action: %s)"),
                                       frmt::Args({
                                           frmt::Arg(MaybeNpc.value.Name),
                                           frmt::Arg(Payload.value.Action.Type),
                                       })))),
                               void())
                            : void();
                      }()
                    : (void)0;
              }()
            : (void)0;

        return Result;
      };
    };
  };
}

} // namespace TestGame
