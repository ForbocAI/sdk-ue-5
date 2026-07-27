#pragma once

#include "CoreMinimal.h"

namespace Testing::Dependencies::Command {

struct FSetupTestCommand {
  FString Key;
  TArray<FString> Arguments;
  FString Label;
};

struct FSetupTestFixtures {
  TArray<FSetupTestCommand> Commands;
};

} // namespace Testing::Dependencies::Command
