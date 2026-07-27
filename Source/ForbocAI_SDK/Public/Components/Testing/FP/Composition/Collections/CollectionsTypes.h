#pragma once

#include "CoreMinimal.h"

namespace Testing::FP::Composition {

struct FCollectionsLabels {
  FString Fold;
  FString Filter;
  FString FindPresent;
  FString FindValue;
  FString Unique;
  FString Contains;
};

struct FCollectionsFixture {
  std::vector<int> Values;
  int32 FoldSeed;
  int32 FoldExpected;
  int32 FilterValue;
  int32 FilterExpectedCount;
  int32 FindValue;
  int32 UniqueExpectedCount;
  int32 ContainsValue;
  FCollectionsLabels Labels;
};

} // namespace Testing::FP::Composition
