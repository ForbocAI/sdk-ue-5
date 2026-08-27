#pragma once

#include "Systems/Config/Resolution/Availability/AvailabilityAdapters.h"

namespace ConfigSlice {

using FApiAvailabilityProbe =
    TFunction<bool(const FConfigConnectionData &)>;

/**
 * User Story: As SDK startup, I need explicit origins accepted without probing and automatic origins resolved by one bounded availability decision.
 * @fn inline FConfigState resolveApiConfiguration( const FConfigState &State, const FApiAvailabilityProbe &Probe)
 */
inline FConfigState resolveApiConfiguration(
    const FConfigState &State, const FApiAvailabilityProbe &Probe) {
  return isExplicitApiSelection(State)
             ? State
             : applyApiSelectionAdapter(
                   State, selectApiUrlAdapter(
                              FString(),
                              Probe(configRuntimeData().Connection)));
}

/** User Story: As SDK startup, I need the canonical endpoint probe applied through a stable one-argument signature. @fn inline FConfigState resolveApiConfiguration(const FConfigState &State) */
inline FConfigState resolveApiConfiguration(const FConfigState &State) {
  return resolveApiConfiguration(
      State, [](const FConfigConnectionData &Connection) {
        return probeApiAvailability(Connection);
      });
}

} // namespace ConfigSlice
