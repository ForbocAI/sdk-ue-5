#pragma once

#include "CoreMinimal.h"

namespace Testing::FP::Match {

struct FResultLabels {
  FString Present;
  FString Value;
};

struct FPredicateCaseFixture {
  int32 Input;
  FString Expected;
  FResultLabels Labels;
};

struct FPredicateFixture {
  int32 Pivot;
  FPredicateCaseFixture Negative;
  FPredicateCaseFixture Zero;
  FPredicateCaseFixture Positive;
};

struct FWildcardLabels {
  FString ExactPresent;
  FString ExactValue;
  FString FallbackPresent;
  FString FallbackValue;
};

struct FWildcardFixture {
  int32 ExactInput;
  FString ExactExpected;
  int32 FallbackInput;
  FString FallbackExpected;
  FWildcardLabels Labels;
};

struct FNoMatchFixture {
  int32 RegisteredInput;
  FString RegisteredResult;
  int32 MissingInput;
  FString Label;
};

struct FEqualsCaseFixture {
  FString Input;
  int32 Result;
  FResultLabels Labels;
};

struct FEqualsRegisteredFixture {
  FString Input;
  int32 Result;
};

struct FEqualsFixture {
  FEqualsCaseFixture First;
  FEqualsCaseFixture Second;
  FEqualsRegisteredFixture Third;
  FString MissingInput;
  FString MissingLabel;
};

struct FNullablePointerLabels {
  FString Present;
  FString Value;
  FString Missing;
};

struct FNullablePointerFixture {
  int32 Value;
  FNullablePointerLabels Labels;
};

struct FNullableValueLabels {
  FString Present;
  FString Value;
  FString Missing;
};

struct FNullableValueFixture {
  FString ValidValue;
  FString EmptyValue;
  bool bValidFlag;
  bool bInvalidFlag;
  FNullableValueLabels Labels;
};

struct FRequireJustLabels {
  FString Value;
  FString Error;
  FString Throws;
};

struct FRequireJustFixture {
  int32 Value;
  FString SuccessMessage;
  FString FailureMessage;
  FRequireJustLabels Labels;
};

struct FMatchFixtures {
  FPredicateFixture Predicate;
  FWildcardFixture Wildcard;
  FNoMatchFixture NoMatch;
  FEqualsFixture Equals;
  FNullablePointerFixture NullablePointer;
  FNullableValueFixture NullableValue;
  FRequireJustFixture RequireJust;
};

} // namespace Testing::FP::Match
