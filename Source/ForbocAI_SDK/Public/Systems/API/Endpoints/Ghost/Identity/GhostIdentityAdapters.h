#pragma once

#include "Components/Ghost/Identity/IdentityTypes.h"
#include "Systems/API/Endpoints/Ghost/Configuration/EndpointsGhostConfigurationAdapters.h"
#include "Systems/API/Endpoints/Ghost/Validation/ValidationAdapters.h"

namespace APISlice::Detail {

/**
 * User Story: As Ghost evidence attribution, I need the exact API and SLM runtime identity decoded as one value.
 * @fn inline func::Maybe<FGhostRuntimeIdentity> DecodeGhostRuntimeIdentityObject( const TSharedPtr<FJsonObject> &Object)
 */
inline func::Maybe<FGhostRuntimeIdentity> DecodeGhostRuntimeIdentityObject(
    const TSharedPtr<FJsonObject> &Object) {
  const auto &Data = Endpoints::GhostConfiguration::ghostApiConfiguration();
  const auto &Fields = Data.Fields.RuntimeIdentity;
  FGhostRuntimeIdentity Identity;
  const bool bValid =
      DecodeGhostIdentityField(Object, Fields.ApiVersion,
                               Data.Limits.MinimumIdentityLength,
                               Identity.ApiVersion) &&
      DecodeGhostIdentityField(Object, Fields.SlmStatus,
                               Data.Limits.MinimumIdentityLength,
                               Identity.SlmStatus) &&
      DecodeGhostIdentityField(Object, Fields.SlmVersion,
                               Data.Limits.MinimumIdentityLength,
                               Identity.SlmVersion) &&
      DecodeGhostIdentityField(Object, Fields.SlotContractVersion,
                               Data.Limits.MinimumIdentityLength,
                               Identity.SlotContractVersion);
  return bValid ? func::just(Identity)
                : func::nothing<FGhostRuntimeIdentity>();
}

/**
 * User Story: As Ghost lifecycle decoding, I need nested runtime identity loaded only from an object field.
 * @fn inline func::Maybe<FGhostRuntimeIdentity> DecodeGhostRuntimeIdentityField( const TSharedPtr<FJsonObject> &Object, const FString &Field)
 */
inline func::Maybe<FGhostRuntimeIdentity> DecodeGhostRuntimeIdentityField(
    const TSharedPtr<FJsonObject> &Object, const FString &Field) {
  return !Object.IsValid() || !Object->HasTypedField<EJson::Object>(Field)
             ? func::nothing<FGhostRuntimeIdentity>()
             : DecodeGhostRuntimeIdentityObject(Object->GetObjectField(Field));
}

} // namespace APISlice::Detail
