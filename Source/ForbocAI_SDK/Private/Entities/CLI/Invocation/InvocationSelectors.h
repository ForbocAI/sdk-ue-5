#pragma once

#include "Core/fp.hpp"
#include "Entities/CLI/CLISelectors.h"
#include "Entities/CLI/Presentation/PresentationSelectors.h"

namespace CommandletInvocation {

/** User Story: As CLI validation, I need a pure validation pipeline derived from CLI and presentation state before command execution. @fn inline func::ValidationPipeline<FString, FString> selectCommandValidationPipeline( const ForbocAI::CLI::FCLIState &CLIState, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState) */
inline func::ValidationPipeline<FString, FString>
selectCommandValidationPipeline(
    const ForbocAI::CLI::FCLIState &CLIState,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  const FString EmptyCommand = PresentationState.Runtime.EmptyCommand;
  const ForbocAI::CLI::Presentation::FCLIPresentationState Presentation =
      PresentationState;
  return func::validationPipeline<FString, FString>() |
         [EmptyCommand](const FString &Command)
             -> func::Either<FString, FString> {
           return Command.IsEmpty()
                      ? func::make_left(EmptyCommand, FString())
                      : func::make_right(FString(), Command);
         } |
         [CLIState, Presentation](const FString &Command)
             -> func::Either<FString, FString> {
           return !ForbocAI::CLI::isValidCommandKey(CLIState, Command)
                      ? func::make_left(
                            ForbocAI::CLI::Presentation::
                                selectCliInvalidCommandMessage(Presentation,
                                                               Command),
                            FString())
                      : func::make_right(FString(), Command);
         };
}

} // namespace CommandletInvocation
