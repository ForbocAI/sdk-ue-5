#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Testing/FP/Composition/MaybeEither/MaybeEitherTypes.h"

namespace Testing::FP::Composition {

/** User Story: As a fp composition maybe either consumer, I need to invoke read maybe either labels through a stable signature so the fp composition maybe either workflow remains explicit and composable. @fn inline FMaybeEitherLabels ReadMaybeEitherLabels(const TSharedRef<FJsonObject> &Object) */
inline FMaybeEitherLabels
ReadMaybeEitherLabels(const TSharedRef<FJsonObject> &Object) {
  FMaybeEitherLabels Labels;
  Labels.TraversePresent =
      DataAdapters::ReadStringField(Object, TEXT("traversePresent"));
  Labels.TraverseCount =
      DataAdapters::ReadStringField(Object, TEXT("traverseCount"));
  Labels.TraverseFirst =
      DataAdapters::ReadStringField(Object, TEXT("traverseFirst"));
  Labels.SequenceMissing =
      DataAdapters::ReadStringField(Object, TEXT("sequenceMissing"));
  Labels.LiftPresent =
      DataAdapters::ReadStringField(Object, TEXT("liftPresent"));
  Labels.LiftValue =
      DataAdapters::ReadStringField(Object, TEXT("liftValue"));
  Labels.FoldRight =
      DataAdapters::ReadStringField(Object, TEXT("foldRight"));
  Labels.FoldValue =
      DataAdapters::ReadStringField(Object, TEXT("foldValue"));
  Labels.ForEach = DataAdapters::ReadStringField(Object, TEXT("forEach"));
  Labels.FoldIndexed =
      DataAdapters::ReadStringField(Object, TEXT("foldIndexed"));
  Labels.FindPresent =
      DataAdapters::ReadStringField(Object, TEXT("findPresent"));
  Labels.FindValue =
      DataAdapters::ReadStringField(Object, TEXT("findValue"));
  Labels.Any = DataAdapters::ReadStringField(Object, TEXT("any"));
  Labels.All = DataAdapters::ReadStringField(Object, TEXT("all"));
  Labels.GridCount =
      DataAdapters::ReadStringField(Object, TEXT("gridCount"));
  Labels.GridFirst =
      DataAdapters::ReadStringField(Object, TEXT("gridFirst"));
  Labels.GridRowMajor =
      DataAdapters::ReadStringField(Object, TEXT("gridRowMajor"));
  return Labels;
}

/** User Story: As a fp composition maybe either consumer, I need to invoke read maybe either fixture through a stable signature so the fp composition maybe either workflow remains explicit and composable. @fn inline FMaybeEitherFixture ReadMaybeEitherFixture(const DataAdapters::FSettingsSource &Source) */
inline FMaybeEitherFixture
ReadMaybeEitherFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("maybeEither"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  FMaybeEitherFixture Fixture{};
  Fixture.Values = DataAdapters::ReadNumberVectorField(Object, TEXT("values"));
  Fixture.TraverseMultiplier =
      DataAdapters::ReadNumberField(Object, TEXT("traverseMultiplier"));
  Fixture.TraverseExpectedCount =
      DataAdapters::ReadNumberField(Object, TEXT("traverseExpectedCount"));
  Fixture.FirstIndex =
      DataAdapters::ReadNumberField(Object, TEXT("firstIndex"));
  Fixture.TraverseFirstExpected =
      DataAdapters::ReadNumberField(Object, TEXT("traverseFirstExpected"));
  Fixture.OptionalPresent =
      DataAdapters::ReadNumberField(Object, TEXT("optionalPresent"));
  Fixture.LiftFirst =
      DataAdapters::ReadNumberField(Object, TEXT("liftFirst"));
  Fixture.LiftSecond =
      DataAdapters::ReadNumberField(Object, TEXT("liftSecond"));
  Fixture.LiftThird =
      DataAdapters::ReadNumberField(Object, TEXT("liftThird"));
  Fixture.LiftExpected =
      DataAdapters::ReadNumberField(Object, TEXT("liftExpected"));
  Fixture.FoldSeed =
      DataAdapters::ReadNumberField(Object, TEXT("foldSeed"));
  Fixture.FoldLimit =
      DataAdapters::ReadNumberField(Object, TEXT("foldLimit"));
  Fixture.FoldError =
      DataAdapters::ReadStringField(Object, TEXT("foldError"));
  Fixture.FoldExpected =
      DataAdapters::ReadNumberField(Object, TEXT("foldExpected"));
  Fixture.IndexedSeed =
      DataAdapters::ReadNumberField(Object, TEXT("indexedSeed"));
  Fixture.IndexedExpected =
      DataAdapters::ReadNumberField(Object, TEXT("indexedExpected"));
  Fixture.IndexedFindValue =
      DataAdapters::ReadNumberField(Object, TEXT("indexedFindValue"));
  Fixture.AnyThreshold =
      DataAdapters::ReadNumberField(Object, TEXT("anyThreshold"));
  Fixture.AllThreshold =
      DataAdapters::ReadNumberField(Object, TEXT("allThreshold"));
  Fixture.GridRows =
      DataAdapters::ReadNumberField(Object, TEXT("gridRows"));
  Fixture.GridColumns =
      DataAdapters::ReadNumberField(Object, TEXT("gridColumns"));
  Fixture.GridRowMultiplier =
      DataAdapters::ReadNumberField(Object, TEXT("gridRowMultiplier"));
  Fixture.GridExpectedCount =
      DataAdapters::ReadNumberField(Object, TEXT("gridExpectedCount"));
  Fixture.GridFirstIndex =
      DataAdapters::ReadNumberField(Object, TEXT("gridFirstIndex"));
  Fixture.GridFirstExpected =
      DataAdapters::ReadNumberField(Object, TEXT("gridFirstExpected"));
  Fixture.GridRowMajorIndex =
      DataAdapters::ReadNumberField(Object, TEXT("gridRowMajorIndex"));
  Fixture.GridRowMajorExpected =
      DataAdapters::ReadNumberField(Object, TEXT("gridRowMajorExpected"));
  Fixture.Labels = ReadMaybeEitherLabels(Labels);
  return Fixture;
}

} // namespace Testing::FP::Composition
