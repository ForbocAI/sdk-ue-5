#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Testing/FP/Composition/NameParity/NameParityTypes.h"

namespace Testing::FP::Composition {

/** User Story: As a fp composition name parity consumer, I need to invoke read name parity labels through a stable signature so the fp composition name parity workflow remains explicit and composable. @fn inline FNameParityLabels ReadNameParityLabels(const TSharedRef<FJsonObject> &Object) */
inline FNameParityLabels
ReadNameParityLabels(const TSharedRef<FJsonObject> &Object) {
  FNameParityLabels Labels;
  Labels.IsJust = DataAdapters::ReadStringField(Object, TEXT("isJust"));
  Labels.IsJustMissing =
      DataAdapters::ReadStringField(Object, TEXT("isJustMissing"));
  Labels.IsNothing =
      DataAdapters::ReadStringField(Object, TEXT("isNothing"));
  Labels.OrElse = DataAdapters::ReadStringField(Object, TEXT("orElse"));
  Labels.RequireJust =
      DataAdapters::ReadStringField(Object, TEXT("requireJust"));
  Labels.PointerPresent =
      DataAdapters::ReadStringField(Object, TEXT("pointerPresent"));
  Labels.PointerValue =
      DataAdapters::ReadStringField(Object, TEXT("pointerValue"));
  Labels.FlaggedPresent =
      DataAdapters::ReadStringField(Object, TEXT("flaggedPresent"));
  Labels.FlaggedValue =
      DataAdapters::ReadStringField(Object, TEXT("flaggedValue"));
  Labels.FlaggedMissing =
      DataAdapters::ReadStringField(Object, TEXT("flaggedMissing"));
  Labels.IsLeft = DataAdapters::ReadStringField(Object, TEXT("isLeft"));
  Labels.IsRightRejectsLeft =
      DataAdapters::ReadStringField(Object, TEXT("isRightRejectsLeft"));
  Labels.IsRight = DataAdapters::ReadStringField(Object, TEXT("isRight"));
  Labels.EfmapRight =
      DataAdapters::ReadStringField(Object, TEXT("efmapRight"));
  Labels.EfmapValue =
      DataAdapters::ReadStringField(Object, TEXT("efmapValue"));
  Labels.PredicatePresent =
      DataAdapters::ReadStringField(Object, TEXT("predicatePresent"));
  Labels.PredicateValue =
      DataAdapters::ReadStringField(Object, TEXT("predicateValue"));
  Labels.LiteralPresent =
      DataAdapters::ReadStringField(Object, TEXT("literalPresent"));
  Labels.WildcardPresent =
      DataAdapters::ReadStringField(Object, TEXT("wildcardPresent"));
  Labels.WildcardValue =
      DataAdapters::ReadStringField(Object, TEXT("wildcardValue"));
  Labels.MatchPresent =
      DataAdapters::ReadStringField(Object, TEXT("matchPresent"));
  Labels.MatchValue =
      DataAdapters::ReadStringField(Object, TEXT("matchValue"));
  Labels.FallbackPresent =
      DataAdapters::ReadStringField(Object, TEXT("fallbackPresent"));
  Labels.FallbackValue =
      DataAdapters::ReadStringField(Object, TEXT("fallbackValue"));
  return Labels;
}

/** User Story: As a fp composition name parity consumer, I need to invoke read name parity fixture through a stable signature so the fp composition name parity workflow remains explicit and composable. @fn inline FNameParityFixture ReadNameParityFixture(const DataAdapters::FSettingsSource &Source) */
inline FNameParityFixture
ReadNameParityFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("nameParity"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  FNameParityFixture Fixture{};
  Fixture.PresentValue =
      DataAdapters::ReadNumberField(Object, TEXT("presentValue"));
  Fixture.OrElseFallback =
      DataAdapters::ReadNumberField(Object, TEXT("orElseFallback"));
  Fixture.MissingMessage =
      DataAdapters::ReadStringField(Object, TEXT("missingMessage"));
  Fixture.PointerValue =
      DataAdapters::ReadNumberField(Object, TEXT("pointerValue"));
  Fixture.FlaggedValue =
      DataAdapters::ReadStringField(Object, TEXT("flaggedValue"));
  Fixture.bFlaggedPresent =
      DataAdapters::ReadBooleanField(Object, TEXT("flaggedPresent"));
  Fixture.FlaggedMissingValue =
      DataAdapters::ReadStringField(Object, TEXT("flaggedMissingValue"));
  Fixture.bFlaggedMissingPresent =
      DataAdapters::ReadBooleanField(Object, TEXT("flaggedMissingPresent"));
  Fixture.FailureValue =
      DataAdapters::ReadStringField(Object, TEXT("failureValue"));
  Fixture.SuccessValue =
      DataAdapters::ReadNumberField(Object, TEXT("successValue"));
  Fixture.MapDelta = DataAdapters::ReadNumberField(Object, TEXT("mapDelta"));
  Fixture.MapExpected =
      DataAdapters::ReadNumberField(Object, TEXT("mapExpected"));
  Fixture.EvenDivisor =
      DataAdapters::ReadNumberField(Object, TEXT("evenDivisor"));
  Fixture.EvenRemainder =
      DataAdapters::ReadNumberField(Object, TEXT("evenRemainder"));
  Fixture.PredicateInput =
      DataAdapters::ReadNumberField(Object, TEXT("predicateInput"));
  Fixture.PredicateValue =
      DataAdapters::ReadStringField(Object, TEXT("predicateValue"));
  Fixture.LiteralInput =
      DataAdapters::ReadNumberField(Object, TEXT("literalInput"));
  Fixture.LiteralExpected =
      DataAdapters::ReadNumberField(Object, TEXT("literalExpected"));
  Fixture.LiteralValue =
      DataAdapters::ReadStringField(Object, TEXT("literalValue"));
  Fixture.WildcardInput =
      DataAdapters::ReadNumberField(Object, TEXT("wildcardInput"));
  Fixture.WildcardPrefix =
      DataAdapters::ReadStringField(Object, TEXT("wildcardPrefix"));
  Fixture.WildcardExpected =
      DataAdapters::ReadStringField(Object, TEXT("wildcardExpected"));
  Fixture.ExactInput =
      DataAdapters::ReadNumberField(Object, TEXT("exactInput"));
  Fixture.ExactValue =
      DataAdapters::ReadStringField(Object, TEXT("exactValue"));
  Fixture.FallbackValue =
      DataAdapters::ReadStringField(Object, TEXT("fallbackValue"));
  Fixture.MatchInput =
      DataAdapters::ReadNumberField(Object, TEXT("matchInput"));
  Fixture.FallbackInput =
      DataAdapters::ReadNumberField(Object, TEXT("fallbackInput"));
  Fixture.Labels = ReadNameParityLabels(Labels);
  return Fixture;
}

} // namespace Testing::FP::Composition
