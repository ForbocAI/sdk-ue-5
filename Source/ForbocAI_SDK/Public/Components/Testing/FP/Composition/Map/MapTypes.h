#pragma once

#include "CoreMinimal.h"

namespace Testing::FP::Composition {

struct FMapLabels {
  FString LeftIdentity;
  FString RightIdentity;
  FString Associative;
  FString RightBias;
  FString PreservesLeft;
  FString PreservesRight;
  FString Count;
};

struct FMapFixture {
  FString LeftOnlyKey;
  FString SharedKey;
  FString RightOnlyKey;
  int32 LeftOnlyValue;
  int32 LeftSharedValue;
  int32 RightSharedValue;
  int32 RightOnlyValue;
  int32 ExpectedCount;
  FMapLabels Labels;
};

} // namespace Testing::FP::Composition
