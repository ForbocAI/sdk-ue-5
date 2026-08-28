#pragma once

#include "Components/Ghost/Identity/IdentityTypes.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace Testing::API::Codec::Ghost {

/** User Story: As Ghost codec fixtures, I need runtime attribution read independently from production decoding. @fn inline FGhostRuntimeIdentity ReadGhostRuntimeIdentity(const TSharedRef<FJsonObject> &Object) */
inline FGhostRuntimeIdentity
ReadGhostRuntimeIdentity(const TSharedRef<FJsonObject> &Object) {
  FGhostRuntimeIdentity Identity;
  Identity.ApiVersion =
      DataAdapters::ReadStringField(Object, TEXT("apiVersion"));
  Identity.SlmStatus =
      DataAdapters::ReadStringField(Object, TEXT("slmStatus"));
  Identity.SlmVersion =
      DataAdapters::ReadStringField(Object, TEXT("slmVersion"));
  Identity.SlotContractVersion =
      DataAdapters::ReadStringField(Object, TEXT("slotContractVersion"));
  return Identity;
}

} // namespace Testing::API::Codec::Ghost
