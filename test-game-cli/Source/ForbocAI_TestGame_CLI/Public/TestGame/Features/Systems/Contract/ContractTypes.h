#pragma once

#include "CoreMinimal.h"
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

struct FContractSchemaData {
#define FORBOCAI_DECLARE_CONTRACT_SCHEMA_FIELD(Name) FString Name;
  FORBOCAI_CONTRACT_SCHEMA_FIELDS(FORBOCAI_DECLARE_CONTRACT_SCHEMA_FIELD)
#undef FORBOCAI_DECLARE_CONTRACT_SCHEMA_FIELD
};

struct FContractData {
  FContractSchemaData Schema;
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

} // namespace Contract
} // namespace TestGame
