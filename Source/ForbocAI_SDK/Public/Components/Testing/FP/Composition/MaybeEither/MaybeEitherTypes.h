#pragma once

#include "CoreMinimal.h"

namespace Testing::FP::Composition {

struct FMaybeEitherLabels {
  FString TraversePresent;
  FString TraverseCount;
  FString TraverseFirst;
  FString SequenceMissing;
  FString LiftPresent;
  FString LiftValue;
  FString FoldRight;
  FString FoldValue;
  FString ForEach;
  FString FoldIndexed;
  FString FindPresent;
  FString FindValue;
  FString Any;
  FString All;
  FString GridCount;
  FString GridFirst;
  FString GridRowMajor;
};

struct FMaybeEitherFixture {
  std::vector<int> Values;
  int32 TraverseMultiplier;
  int32 TraverseExpectedCount;
  int32 FirstIndex;
  int32 TraverseFirstExpected;
  int32 OptionalPresent;
  int32 LiftFirst;
  int32 LiftSecond;
  int32 LiftThird;
  int32 LiftExpected;
  int32 FoldSeed;
  int32 FoldLimit;
  FString FoldError;
  int32 FoldExpected;
  int32 IndexedSeed;
  int32 IndexedExpected;
  int32 IndexedFindValue;
  int32 AnyThreshold;
  int32 AllThreshold;
  int32 GridRows;
  int32 GridColumns;
  int32 GridRowMultiplier;
  int32 GridExpectedCount;
  int32 GridFirstIndex;
  int32 GridFirstExpected;
  int32 GridRowMajorIndex;
  int32 GridRowMajorExpected;
  FMaybeEitherLabels Labels;
};

} // namespace Testing::FP::Composition
