#pragma once

#include "Core/rtk.hpp"
#include "Entities/Config/ConfigActions.h"
#include "Entities/Config/ConfigSelectors.h"

namespace Ops {

/** User Story: As a CLI invocation, I need external configuration hydrated into the package root store before command execution. @fn template <typename RootState> inline void hydrateRuntimeConfig( rtk::EnhancedStore<RootState> &Store, const ConfigSlice::FConfigInvocationOverrides &Overrides = ConfigSlice::FConfigInvocationOverrides(), const FString &FilePathOverride = FString()) */
template <typename RootState>
inline void hydrateRuntimeConfig(
    rtk::EnhancedStore<RootState> &Store,
    const ConfigSlice::FConfigInvocationOverrides &Overrides =
        ConfigSlice::FConfigInvocationOverrides(),
    const FString &FilePathOverride = FString()) {
  Store.dispatch(ConfigSlice::Actions::configurationHydrated(
      ConfigSlice::readConfigState(Overrides, FilePathOverride)));
}

/** User Story: As a CLI config write, I need the root reducer updated before its entry snapshot is persisted. @fn template <typename RootState> inline bool setConfigValue(rtk::EnhancedStore<RootState> &Store, const FString &Key, const FString &Value) */
template <typename RootState>
inline bool setConfigValue(rtk::EnhancedStore<RootState> &Store,
                           const FString &Key, const FString &Value) {
  Store.dispatch(ConfigSlice::Actions::configurationEntryCommitted(
      {Key, Value}));
  return ConfigSlice::writeConfigState(
      ConfigSelectors::selectConfig(Store.getState()));
}

/** User Story: As a CLI config read, I need persisted values selected from root state without filesystem access. @fn template <typename RootState> inline FString getConfigValue(const RootState &State, const FString &Key) */
template <typename RootState>
inline FString getConfigValue(const RootState &State, const FString &Key) {
  return ConfigSelectors::selectConfigValue(State, Key);
}

/** User Story: As a host invocation, I need API overrides dispatched as a non-persistent Config event. @fn template <typename RootState> inline void commitApiConfiguration(rtk::EnhancedStore<RootState> &Store, const FString &ApiUrl, const FString &ApiKey) */
template <typename RootState>
inline void commitApiConfiguration(rtk::EnhancedStore<RootState> &Store,
                                   const FString &ApiUrl,
                                   const FString &ApiKey) {
  Store.dispatch(ConfigSlice::Actions::apiConfigurationCommitted(
      {ApiUrl, ApiKey}));
}

} // namespace Ops
