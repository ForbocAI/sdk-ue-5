#pragma once

#include "CoreMinimal.h"

namespace Testing::FP::Dispatcher {

struct FEntryFixture {
  FString Key;
  int32 Value;
};

struct FAssertedEntryFixture {
  FString Key;
  int32 Value;
  FString PresentLabel;
  FString ValueLabel;
};

struct FKeyLookupFixture {
  TArray<FAssertedEntryFixture> Entries;
};

struct FMissingKeyFixture {
  FEntryFixture Entry;
  FString MissingKey;
  FString Label;
};

struct FHasEntryFixture {
  FString Key;
  int32 Value;
  FString PresentLabel;
};

struct FHasAndKeysFixture {
  TArray<FHasEntryFixture> Entries;
  FString MissingKey;
  FString MissingLabel;
  int32 ExpectedCount;
  FString CountLabel;
};

struct FEitherLabels {
  FString HitSide;
  FString HitValue;
  FString MissSide;
  FString MissError;
};

struct FEitherMissFixture {
  FEntryFixture Entry;
  FString MissingKey;
  FString Error;
  FEitherLabels Labels;
};

struct FArgStrictLabels {
  FString HitPresent;
  FString HitValue;
  FString MissSide;
  FString MissError;
};

struct FArgStrictFixture {
  FString RegisteredKey;
  FString MissingKey;
  int32 Input;
  int32 MatchedInput;
  FString MatchedOutput;
  FString OtherOutput;
  FString MissingError;
  FArgStrictLabels Labels;
};

struct FDispatcherFixtures {
  FKeyLookupFixture KeyLookup;
  FMissingKeyFixture MissingKey;
  FHasAndKeysFixture HasAndKeys;
  FEitherMissFixture EitherMiss;
  FArgStrictFixture ArgStrict;
};

} // namespace Testing::FP::Dispatcher
