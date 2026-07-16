// User Story: As a developer, I need this module to function.
#pragma once

#include "CoreMinimal.h"

struct FNpcFixtureState {
  FString Id;
  int32 Health;

  /** User Story: As a tests core consumer, I need to compare values for equality through a stable signature so the tests core workflow remains explicit and composable. @fn bool operator==(const FNpcFixtureState &Other) const */
  bool operator==(const FNpcFixtureState &Other) const {
    return Id == Other.Id && Health == Other.Health;
  }
};

struct FAppFixtureState {
  FNpcFixtureState ActiveNpc;
};
