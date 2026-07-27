#pragma once

#include "Core/rtk.hpp"
#include "Systems/Config/ConfigAdapters.h"

namespace ConfigSlice::Actions {

/** User Story: As Config hydration, I need one event creator that replaces the root-store snapshot after external resolution. @fn inline const rtk::ActionCreator<FConfigState> & configurationHydratedActionCreator() */
inline const rtk::ActionCreator<FConfigState> &
configurationHydratedActionCreator() {
  static const rtk::ActionCreator<FConfigState> Creator =
      rtk::createAction<FConfigState>(configRuntimeData().Slice.Name +
                                      configRuntimeData().Slice.ActionSeparator +
                                      configRuntimeData().Slice.Actions.Hydrated);
  return Creator;
}

/** User Story: As Config persistence, I need one event creator for a committed key/value pair. @fn inline const rtk::ActionCreator<FConfigEntryCommitted> & configurationEntryCommittedActionCreator() */
inline const rtk::ActionCreator<FConfigEntryCommitted> &
configurationEntryCommittedActionCreator() {
  static const rtk::ActionCreator<FConfigEntryCommitted> Creator =
      rtk::createAction<FConfigEntryCommitted>(
          configRuntimeData().Slice.Name +
          configRuntimeData().Slice.ActionSeparator +
          configRuntimeData().Slice.Actions.EntryCommitted);
  return Creator;
}

/** User Story: As host configuration, I need one event creator for non-persistent API invocation overrides. @fn inline const rtk::ActionCreator<FConfigApiCommitted> & apiConfigurationCommittedActionCreator() */
inline const rtk::ActionCreator<FConfigApiCommitted> &
apiConfigurationCommittedActionCreator() {
  static const rtk::ActionCreator<FConfigApiCommitted> Creator =
      rtk::createAction<FConfigApiCommitted>(
          configRuntimeData().Slice.Name +
          configRuntimeData().Slice.ActionSeparator +
          configRuntimeData().Slice.Actions.ApiCommitted);
  return Creator;
}

/** User Story: As Config hydration, I need a typed event value accepted by the root dispatch boundary. @fn inline rtk::AnyAction configurationHydrated(const FConfigState &State) */
inline rtk::AnyAction configurationHydrated(const FConfigState &State) {
  return configurationHydratedActionCreator()(State);
}

/** User Story: As Config persistence, I need a typed key/value event accepted by the root dispatch boundary. @fn inline rtk::AnyAction configurationEntryCommitted(const FConfigEntryCommitted &Entry) */
inline rtk::AnyAction
configurationEntryCommitted(const FConfigEntryCommitted &Entry) {
  return configurationEntryCommittedActionCreator()(Entry);
}

/** User Story: As host configuration, I need typed API override events accepted by the root dispatch boundary. @fn inline rtk::AnyAction apiConfigurationCommitted(const FConfigApiCommitted &Config) */
inline rtk::AnyAction
apiConfigurationCommitted(const FConfigApiCommitted &Config) {
  return apiConfigurationCommittedActionCreator()(Config);
}

} // namespace ConfigSlice::Actions
