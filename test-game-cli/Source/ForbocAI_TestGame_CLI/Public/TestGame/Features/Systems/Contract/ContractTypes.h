#pragma once

#include "CoreMinimal.h"

namespace TestGame {
namespace Contract {

struct FContractAliasRules {
  FString NpcCreateAlias;
  FString BridgeValidateCommand;
};

struct FContractCommandSpec {
  FString Group;
  FString Command;
  TArray<FString> ExpectedRoutes;
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

  FContractResponse() : bValid(false) {}
};

struct FRawContractResponse {
  bool bSuccess;
  FString Body;
  FString Error;

  FRawContractResponse() : bSuccess(false) {}
};

} // namespace Contract
} // namespace TestGame
