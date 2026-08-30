#pragma once
/**
 * Micro-game cross-slice listener middleware — mirrors TS listeners.ts
 * Reactive side effects: NPC movement → UI message
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "CoreMinimal.h"
#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "MicroGame/Features/Entities/NPCs/NPCsActions.h"
#include "MicroGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationTypes.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationAdapters.h"
#include "MicroGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "MicroGame/Features/Systems/Terminal/UI/UIActions.h"

namespace MicroGame {

/**
 * Creates listener middleware that logs NPC movement to the UI message box.
 * Mirrors TS: npcsActions.moveNPC → uiActions.addMessage
 * User Story: As micro-game reactive UI, I need listener middleware so NPC
 * movement and verdict application emit readable terminal messages.
 * @fn inline rtk::Middleware<FMicroGameState> createGameListenerMiddleware()
 */
inline rtk::Middleware<FMicroGameState> createGameListenerMiddleware() {
  return [](const rtk::MiddlewareApi<FMicroGameState> &Api)
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
        func::mbind(
            NPCsActions::MoveNPCActionCreator().extract(Action),
            [&Api](const NPCsActions::FMoveNPCPayload &Payload) {
              const FMicroGameState &State = Api.getState();
              return func::fmap(
                  NPCsSelectors::SelectNpcById(State.NPCs, Payload.Id),
                  [&Api, &Payload](const FGameNPC &Npc) {
                    const FTerminalData &Data =
                        TerminalAdapters::TerminalData();
                    const TMap<FString, FString> Values{
                        {Data.tokens.name, Npc.Name},
                        {Data.tokens.x, FString::FromInt(Payload.Position.X)},
                        {Data.tokens.y, FString::FromInt(Payload.Position.Y)}};
                    return Api.dispatch(UIActions::addMessage(
                        VerificationAdapters::FormatGameTemplate(
                            Data.messages.npcMoved, Values)));
                  });
            });

        return Result;
      };
    };
  };
}

} // namespace MicroGame
