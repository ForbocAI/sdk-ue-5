#pragma once

#include "Core/rtk.hpp"
#include "Features/CLI/CLIAdapters.h"

namespace CLISlice {

/** User Story: As a features cli consumer, I need to invoke initial clistate through a stable signature so the features cli workflow remains explicit and composable. @fn inline const ForbocAI::CLI::FCLIState &initialCLIState() */
inline const ForbocAI::CLI::FCLIState &initialCLIState() {
  static const func::Lazy<ForbocAI::CLI::FCLIState> State =
      func::lazy([]() { return ForbocAI::CLI::readCliState(); });
  return func::eval(State);
}

/** User Story: As a features cli consumer, I need to invoke create clislice through a stable signature so the features cli workflow remains explicit and composable. @fn inline rtk::Slice<ForbocAI::CLI::FCLIState> createCLISlice() */
inline rtk::Slice<ForbocAI::CLI::FCLIState> createCLISlice() {
  return rtk::createSlice<ForbocAI::CLI::FCLIState>(
      TEXT("cli"), initialCLIState(),
      [](rtk::ActionReducerMapBuilder<ForbocAI::CLI::FCLIState> &) {});
}

} // namespace CLISlice
