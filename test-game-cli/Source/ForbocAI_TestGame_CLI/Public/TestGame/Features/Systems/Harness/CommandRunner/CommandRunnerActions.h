#pragma once

#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"

namespace TestGame {
namespace CommandRunnerActions {

/** User Story: As a systems harness command runner consumer, I need to invoke aliases captured action creator through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline rtk::ActionCreator<CommandRunner::FCommandAliasUpdate> aliasesCapturedActionCreator() */
inline rtk::ActionCreator<CommandRunner::FCommandAliasUpdate>
aliasesCapturedActionCreator() {
  static const auto Creator =
      rtk::createAction<CommandRunner::FCommandAliasUpdate>(
          TEXT("testgame/commandRunner/aliasesCaptured"));
  return Creator;
}

/** User Story: As a systems harness command runner consumer, I need to invoke aliases reset action creator through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline rtk::ActionCreatorWithoutPayload aliasesResetActionCreator() */
inline rtk::ActionCreatorWithoutPayload aliasesResetActionCreator() {
  static const auto Creator =
      rtk::createAction(TEXT("testgame/commandRunner/aliasesReset"));
  return Creator;
}

/** User Story: As a systems harness command runner consumer, I need to invoke aliases captured through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline rtk::AnyAction aliasesCaptured(CommandRunner::FCommandAliasUpdate Update) */
inline rtk::AnyAction
aliasesCaptured(CommandRunner::FCommandAliasUpdate Update) {
  return aliasesCapturedActionCreator()(MoveTemp(Update));
}

/** User Story: As a systems harness command runner consumer, I need to invoke aliases reset through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline rtk::AnyAction aliasesReset() */
inline rtk::AnyAction aliasesReset() {
  return aliasesResetActionCreator()();
}

} // namespace CommandRunnerActions
} // namespace TestGame
