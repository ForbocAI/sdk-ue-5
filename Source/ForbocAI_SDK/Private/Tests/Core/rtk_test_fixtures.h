// User Story: As a developer, I need this module to function.
#pragma once

#include "CoreMinimal.h"

struct FNpcFixtureState {
  FString Id;
  int32 Health;

  bool operator==(const FNpcFixtureState &Other) const {
    return Id == Other.Id && Health == Other.Health;
  }
};

struct FAppFixtureState {
  FNpcFixtureState ActiveNpc;
};
