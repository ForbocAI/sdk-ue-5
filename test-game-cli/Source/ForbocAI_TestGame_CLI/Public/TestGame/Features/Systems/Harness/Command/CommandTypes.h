#pragma once

#include "CoreMinimal.h"

namespace TestGame {

struct FOutputAssertion {
  FString Kind;
  FString Value;
};

struct FCommandSpec {
  FString Group;
  FString Command;
  TArray<FString> ExpectedRoutes;
  TArray<FOutputAssertion> OutputAssertions;
};

} // namespace TestGame
