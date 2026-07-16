#pragma once

#include "Core/rtk.hpp"
#include "CoreMinimal.h"
#include "Features/API/Transport/Configuration/TransportConfigurationAdapters.h"

struct FRuntimeState;

namespace APISlice {

/** User Story: As a features api consumer, I need to invoke provides tags through a stable signature so the features api workflow remains explicit and composable. @fn inline bool providesTags(const rtk::Api<FRuntimeState> &apiDefinition) */
inline bool providesTags(const rtk::Api<FRuntimeState> &apiDefinition) {
  return !apiDefinition.TagTypes.IsEmpty();
}

inline rtk::Api<FRuntimeState> api = []() {
  const Transport::FTransportQueryData &Data =
      Transport::transportQueryData();
  rtk::Api<FRuntimeState> apiDefinition = rtk::createApi<FRuntimeState>(
      Data.Api.ReducerPath,
      TArray<FString>{Data.Tags.Npc, Data.Tags.Ghost, Data.Tags.Soul,
                      Data.Tags.Bridge, Data.Tags.Rule});
  check(providesTags(apiDefinition));
  return apiDefinition;
}();

} // namespace APISlice

#include "Features/API/APIAdapters.h"
#include "Endpoints/Bridge/BridgeApi.h"
#include "Endpoints/Ghost/GhostApi.h"
#include "Endpoints/NPC/NPCApi.h"
#include "Endpoints/Rules/RulesApi.h"
#include "Endpoints/Soul/SoulApi.h"
#include "Endpoints/Soul/Provider/ProviderApi.h"
#include "Endpoints/Soul/Storage/StorageApi.h"
#include "Endpoints/System/SystemApi.h"
