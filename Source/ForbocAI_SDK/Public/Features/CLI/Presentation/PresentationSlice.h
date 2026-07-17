#pragma once

#include "Core/rtk.hpp"
#include "Features/CLI/Presentation/PresentationAdapters.h"

namespace ForbocAI::CLI::Presentation {

/** User Story: As UE package assembly, I need authored CLI presentation mounted as one read-only RTK slice. @fn inline rtk::Slice<FCLIPresentationState> createCliPresentationSlice() */
inline rtk::Slice<FCLIPresentationState> createCliPresentationSlice() {
  const FCLIPresentationState InitialState = readCliPresentation();
  return rtk::createSlice<FCLIPresentationState>(
      InitialState.SliceName, InitialState,
      [](rtk::ActionReducerMapBuilder<FCLIPresentationState> &) {});
}

} // namespace ForbocAI::CLI::Presentation
