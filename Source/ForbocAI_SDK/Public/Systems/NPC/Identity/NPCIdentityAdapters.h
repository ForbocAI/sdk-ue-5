#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

namespace NPCId {

namespace detail {
/** User Story: As cross-SDK NPC identity generation, I need the shared authored prefix loaded from JSON so runtime code and tests use one source of truth. @fn inline const FString &GeneratedIdPrefix() */
inline const FString &GeneratedIdPrefix() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/npc/runtime.json"));
  static const FString Prefix =
      DataAdapters::ReadStringField(Source.Root, TEXT("generatedIdPrefix"));
  return Prefix;
}
} // namespace detail

/**
 * Generates an NPC id with the same authored prefix and UUID mechanics as TS.
 * User Story: As cross-SDK id generation, I need UE NPC ids to match the TS
 * format and collision guarantees so synchronized agents share one contract.
 * @fn inline FString GenerateNPCId()
 */
inline FString GenerateNPCId() {
  return detail::GeneratedIdPrefix() +
         FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
}

} // namespace NPCId
