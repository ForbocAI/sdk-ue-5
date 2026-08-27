#pragma once

#include "Systems/Config/Contract/ContractAdapters.h"

namespace ConfigSlice {

/** User Story: As endpoint configuration, I need trailing separators normalized by one pure adapter so request paths compose consistently. @fn inline FString normalizeApiUrl(const FString &ApiUrl) */
inline FString normalizeApiUrl(const FString &ApiUrl) {
  const FString &Separator = configRuntimeData().Connection.UrlSeparator;
  return !Separator.IsEmpty() && ApiUrl.EndsWith(Separator)
             ? normalizeApiUrl(ApiUrl.LeftChop(Separator.Len()))
             : ApiUrl;
}

/**
 * User Story: As SDK connection policy, I need explicit, local, and production origins selected by one deterministic function.
 * @fn inline FConfigApiSelection selectApiUrlAdapter( const FString &ConfiguredApiUrl, bool bLocalAvailable)
 */
inline FConfigApiSelection selectApiUrlAdapter(
    const FString &ConfiguredApiUrl, bool bLocalAvailable) {
  const FConfigRuntimeData &Data = configRuntimeData();
  return !ConfiguredApiUrl.IsEmpty()
             ? FConfigApiSelection{normalizeApiUrl(ConfiguredApiUrl),
                                   Data.Connection.Sources.Explicit}
             : bLocalAvailable
                   ? FConfigApiSelection{
                         normalizeApiUrl(Data.Connection.LocalApiUrl),
                         Data.Connection.Sources.Local}
                   : FConfigApiSelection{normalizeApiUrl(Data.Defaults.ApiUrl),
                                         Data.Connection.Sources.Production};
}

/**
 * User Story: As the Config reducer boundary, I need a resolved endpoint projected into an immutable state snapshot.
 * @fn inline FConfigState applyApiSelectionAdapter( const FConfigState &State, const FConfigApiSelection &Selection)
 */
inline FConfigState applyApiSelectionAdapter(
    const FConfigState &State, const FConfigApiSelection &Selection) {
  FConfigState Next = State;
  Next.ApiUrl = Selection.ApiUrl;
  Next.ApiUrlSource = Selection.ApiUrlSource;
  return Next;
}

/** User Story: As runtime hydration, I need configured endpoints recognized before any automatic availability probe executes. @fn inline bool isExplicitApiSelection(const FConfigState &State) */
inline bool isExplicitApiSelection(const FConfigState &State) {
  return State.ApiUrlSource ==
         configRuntimeData().Connection.Sources.Explicit;
}

} // namespace ConfigSlice
