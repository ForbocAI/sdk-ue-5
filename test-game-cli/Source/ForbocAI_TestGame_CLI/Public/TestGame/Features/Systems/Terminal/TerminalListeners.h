#pragma once
/**
 * Test-game cross-slice listener middleware — mirrors TS listeners.ts
 * Reactive side effects: NPC movement → UI message
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Entities/NPCs/NPCsSelectors.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Terminal/TerminalAdapters.h"
#include "TestGame/Features/Systems/Terminal/UI/UIActions.h"

namespace TestGame {

/**
 * Creates listener middleware that logs NPC movement to the UI message box.
 * Mirrors TS: npcsActions.moveNPC → uiActions.addMessage
 * User Story: As test-game reactive UI, I need listener middleware so NPC
 * movement and verdict application emit readable terminal messages.
 * @fn inline rtk::Middleware<FTestGameState> createGameListenerMiddleware()
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
                            ? ([&]() {
                                const FTerminalData &Data =
                                    TerminalAdapters::TerminalData();
                                TMap<FString, FString> Values;
                                Values.Add(Data.tokens.name,
                                           MaybeNpc.value.Name);
                                Values.Add(Data.tokens.x, FString::FromInt(
                                                              Payload.value
                                                                  .Position.X));
                                Values.Add(Data.tokens.y, FString::FromInt(
                                                              Payload.value
                                                                  .Position.Y));
                                Api.dispatch(UIActions::addMessage(
                                    GameAdapters::FormatGameTemplate(
                                        Data.messages.npcMoved, Values)));
                              }(),
                               void())
                            : void();
                      }()
                    : void();
              }()
            : void();

        return Result;
      };
    };
  };
}

} // namespace TestGame
