#pragma once

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/Scenario/ScenarioTypes.h"

namespace TestGame {
namespace Contract {

#define FORBOCAI_CONTRACT_SCHEMA_FIELDS(X)                               \
  X(kind)                                                                \
  X(value)                                                               \
  X(group)                                                               \
  X(command)                                                             \
  X(expectedRoutes)                                                      \
  X(outputAssertions)                                                    \
  X(id)                                                                  \
  X(title)                                                               \
  X(description)                                                         \
  X(eventType)                                                           \
  X(commands)                                                            \
  X(version)                                                             \
  X(slotContractVersion)                                                 \
  X(requiredCommandGroups)                                               \
  X(aliasRules)                                                          \
  X(scenarios)                                                           \
  X(npcCreateAlias)                                                      \
  X(ghostSessionAlias)                                                   \
  X(soulTransactionAlias)

struct FContractAuthorizationData {
  FString Header;
  FString Template;
};

struct FContractRequestData {
  FString Path;
  FString Method;
};

struct FContractSeparatorData {
  FString TrailingUrl;
};

struct FContractApiData {
  FString ReducerPath;
  FString TagType;
  FString EndpointName;
  FString TagId;
};

struct FContractSchemaData {
#define FORBOCAI_DECLARE_CONTRACT_SCHEMA_FIELD(Name) FString Name;
  FORBOCAI_CONTRACT_SCHEMA_FIELDS(FORBOCAI_DECLARE_CONTRACT_SCHEMA_FIELD)
#undef FORBOCAI_DECLARE_CONTRACT_SCHEMA_FIELD
};

struct FContractData {
  FString DefaultApiUrl;
  FContractApiData Api;
  FContractAuthorizationData Authorization;
  FContractRequestData Request;
  FContractSeparatorData Separators;
  FContractSchemaData Schema;
};

struct FTestGameContractRequest {
  rtk::FetchArgs Args;
  TMap<FString, FString> Headers;
};

struct FContractAliasRules {
  FString NpcCreateAlias;
  FString GhostSessionAlias;
  FString SoulTransactionAlias;
};

struct FContractResponse {
  FString Version;
  FString SlotContractVersion;
  TArray<FString> RequiredCommandGroups;
  FContractAliasRules AliasRules;
  TArray<FScenarioStep> Scenarios;
  bool bValid{};
};

struct FContractQueryResult {
  bool bSuccess{};
  FString Body;
  FString Error;
};

} // namespace Contract
} // namespace TestGame
