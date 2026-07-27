#pragma once

#include "CoreMinimal.h"

namespace Testing::FP::Composition {

struct FNameParityLabels {
  FString IsJust;
  FString IsJustMissing;
  FString IsNothing;
  FString OrElse;
  FString RequireJust;
  FString PointerPresent;
  FString PointerValue;
  FString FlaggedPresent;
  FString FlaggedValue;
  FString FlaggedMissing;
  FString IsLeft;
  FString IsRightRejectsLeft;
  FString IsRight;
  FString EfmapRight;
  FString EfmapValue;
  FString PredicatePresent;
  FString PredicateValue;
  FString LiteralPresent;
  FString WildcardPresent;
  FString WildcardValue;
  FString MatchPresent;
  FString MatchValue;
  FString FallbackPresent;
  FString FallbackValue;
};

struct FNameParityFixture {
  int32 PresentValue;
  int32 OrElseFallback;
  FString MissingMessage;
  int32 PointerValue;
  FString FlaggedValue;
  bool bFlaggedPresent;
  FString FlaggedMissingValue;
  bool bFlaggedMissingPresent;
  FString FailureValue;
  int32 SuccessValue;
  int32 MapDelta;
  int32 MapExpected;
  int32 EvenDivisor;
  int32 EvenRemainder;
  int32 PredicateInput;
  FString PredicateValue;
  int32 LiteralInput;
  int32 LiteralExpected;
  FString LiteralValue;
  int32 WildcardInput;
  FString WildcardPrefix;
  FString WildcardExpected;
  int32 ExactInput;
  FString ExactValue;
  FString FallbackValue;
  int32 MatchInput;
  int32 FallbackInput;
  FNameParityLabels Labels;
};

} // namespace Testing::FP::Composition
