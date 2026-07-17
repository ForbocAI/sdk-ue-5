#pragma once

#include "Core/fp.hpp"
#include "Features/Config/ConfigTypes.h"

namespace ConfigSelectors {

/** User Story: As a runtime consumer, I need Config selected from the package root store. @fn template <typename RootState> inline const ConfigSlice::FConfigState &selectConfig(const RootState &State) */
template <typename RootState>
inline const ConfigSlice::FConfigState &selectConfig(const RootState &State) {
  return State.Config;
}

/** User Story: As API transport, I need the active API origin selected from root state. @fn template <typename RootState> inline const FString &selectApiUrl(const RootState &State) */
template <typename RootState>
inline const FString &selectApiUrl(const RootState &State) {
  return selectConfig(State).ApiUrl;
}

/** User Story: As authenticated transport, I need the active API key selected from root state. @fn template <typename RootState> inline const FString &selectApiKey(const RootState &State) */
template <typename RootState>
inline const FString &selectApiKey(const RootState &State) {
  return selectConfig(State).ApiKey;
}

/** User Story: As memory setup, I need the active database path selected from root state. @fn template <typename RootState> inline const FString &selectDatabasePath(const RootState &State) */
template <typename RootState>
inline const FString &selectDatabasePath(const RootState &State) {
  return selectConfig(State).DatabasePath;
}

/** User Story: As vector setup, I need the active vector dimension selected from root state. @fn template <typename RootState> inline int32 selectVectorDimension(const RootState &State) */
template <typename RootState>
inline int32 selectVectorDimension(const RootState &State) {
  return selectConfig(State).VectorDimension;
}

/** User Story: As memory recall, I need the active result limit selected from root state. @fn template <typename RootState> inline int32 selectMaxRecallResults(const RootState &State) */
template <typename RootState>
inline int32 selectMaxRecallResults(const RootState &State) {
  return selectConfig(State).MaxRecallResults;
}

/** User Story: As CLI diagnostics, I need the package version selected from Config state. @fn template <typename RootState> inline const FString &selectSdkVersion(const RootState &State) */
template <typename RootState>
inline const FString &selectSdkVersion(const RootState &State) {
  return selectConfig(State).SdkVersion;
}

/** User Story: As CLI config reads, I need a persisted entry selected without touching the filesystem. @fn template <typename RootState> inline FString selectConfigValue(const RootState &State, const FString &Key) */
template <typename RootState>
inline FString selectConfigValue(const RootState &State, const FString &Key) {
  const ConfigSlice::FConfigState &Config = selectConfig(State);
  return Key == Config.Fields.SdkVersion
             ? Config.SdkVersion
             : func::map_value_or(Config.Entries, Key, FString());
}

} // namespace ConfigSelectors
