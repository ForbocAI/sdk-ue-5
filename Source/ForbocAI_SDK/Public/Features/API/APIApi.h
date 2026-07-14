#pragma once

#include "Core/rtk.hpp"
#include "CoreMinimal.h"

struct FRuntimeState;

namespace APISlice {

inline bool providesTags(const rtk::Api<FRuntimeState> &apiDefinition) {
  return apiDefinition.TagTypes.Num() > 0;
}

inline rtk::Api<FRuntimeState> api = []() {
  rtk::Api<FRuntimeState> apiDefinition = rtk::createApi<FRuntimeState>(
      TEXT("forbocApi"),
      TArray<FString>{TEXT("NPC"), TEXT("Memory"), TEXT("Ghost"),
                      TEXT("Soul"), TEXT("Bridge"), TEXT("Rule")});
  check(providesTags(apiDefinition));
  return apiDefinition;
}();

} // namespace APISlice

#include "Features/API/APIAdapters.h"
#include "Endpoints/Arweave/ArweaveApi.h"
#include "Endpoints/Bridge/BridgeApi.h"
#include "Endpoints/Ghost/GhostApi.h"
#include "Endpoints/Memory/MemoryApi.h"
#include "Endpoints/NPC/NPCApi.h"
#include "Endpoints/Rules/RulesApi.h"
#include "Endpoints/Soul/SoulApi.h"
#include "Endpoints/System/SystemApi.h"
