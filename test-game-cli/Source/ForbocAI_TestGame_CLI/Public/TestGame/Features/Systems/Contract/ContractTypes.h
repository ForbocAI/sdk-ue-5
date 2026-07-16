#pragma once

#include "CoreMinimal.h"

namespace TestGame {
namespace Contract {

struct FContractAliasRules {
  FString NpcCreateAlias;
  FString GhostSessionAlias;
  FString SoulTransactionAlias;
};

struct FContractOutputAssertion {
  FString Kind;
  FString Value;
};

struct FContractCommandSpec {
  FString Group;
  FString Command;
  TArray<FString> ExpectedRoutes;
  TArray<FContractOutputAssertion> OutputAssertions;
  bool bHasOutputAssertions;

  /** User Story: As a systems contract consumer, I need command fields initialized so required output assertions are distinguishable from a missing field. @fn FContractCommandSpec() */
  FContractCommandSpec() : bHasOutputAssertions(false) {}
};

struct FContractScenario {
  FString Id;
  FString Title;
  FString Description;
  FString EventType;
  TArray<FContractCommandSpec> Commands;
};

struct FContractResponse {
  FString Version;
  FString SlotContractVersion;
  TArray<FString> RequiredCommandGroups;
  FContractAliasRules AliasRules;
  TArray<FContractScenario> Scenarios;
  bool bValid;

  /** User Story: As a features systems contract consumer, I need to invoke fcontract response through a stable signature so the features systems contract workflow remains explicit and composable. @fn FContractResponse() */
  FContractResponse() : bValid(false) {}
};

struct FRawContractResponse {
  bool bSuccess;
  FString Body;
  FString Error;

  /** User Story: As a features systems contract consumer, I need to invoke fraw contract response through a stable signature so the features systems contract workflow remains explicit and composable. @fn FRawContractResponse() */
  FRawContractResponse() : bSuccess(false) {}
};

} // namespace Contract
} // namespace TestGame
