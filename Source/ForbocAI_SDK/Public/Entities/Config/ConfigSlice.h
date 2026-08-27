#pragma once

#include "Core/rtk.hpp"
#include "Entities/Config/ConfigActions.h"

namespace ConfigSlice {

/**
 * @fn inline rtk::Slice<FConfigState> createConfigSlice()
 * User Story: As the UE package root store, I need Config transitions owned by one RTK slice rather than process-global mutable values.
 */
inline rtk::Slice<FConfigState> createConfigSlice() {
  return rtk::createSlice<FConfigState>(
      configRuntimeData().Slice.Name, readConfigState(),
      [](rtk::ActionReducerMapBuilder<FConfigState> &Builder) {
        Builder.addCase(
            Actions::configurationHydratedActionCreator(),
            [](const FConfigState &,
               const rtk::Action<FConfigState> &Action) {
              const FConfigState &Hydrated = Action.PayloadValue;
              return FConfigState{
                  Hydrated.Entries, Hydrated.Fields, Hydrated.FilePath,
                  Hydrated.SdkVersion, Hydrated.ApiUrl,
                  Hydrated.ApiUrlSource, Hydrated.ApiKey,
                  Hydrated.DatabasePath, Hydrated.VectorDimension,
                  Hydrated.MaxRecallResults};
            });
        Builder.addCase(
            Actions::configurationEntryCommittedActionCreator(),
            [](const FConfigState &State,
               const rtk::Action<FConfigEntryCommitted> &Action) {
              return commitConfigEntryAdapter(State, Action.PayloadValue);
            });
        Builder.addCase(
            Actions::apiConfigurationCommittedActionCreator(),
            [](const FConfigState &State,
               const rtk::Action<FConfigApiCommitted> &Action) {
              return commitApiConfigurationAdapter(State,
                                                   Action.PayloadValue);
            });
      });
}

} // namespace ConfigSlice
