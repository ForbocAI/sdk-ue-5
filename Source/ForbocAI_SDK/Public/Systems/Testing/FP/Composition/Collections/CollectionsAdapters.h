#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Testing/FP/Composition/Collections/CollectionsTypes.h"

namespace Testing::FP::Composition {

/** User Story: As a fp composition collections consumer, I need to invoke read collections labels through a stable signature so the fp composition collections workflow remains explicit and composable. @fn inline FCollectionsLabels ReadCollectionsLabels(const TSharedRef<FJsonObject> &Object) */
inline FCollectionsLabels
ReadCollectionsLabels(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("fold")),
      DataAdapters::ReadStringField(Object, TEXT("filter")),
      DataAdapters::ReadStringField(Object, TEXT("findPresent")),
      DataAdapters::ReadStringField(Object, TEXT("findValue")),
      DataAdapters::ReadStringField(Object, TEXT("unique")),
      DataAdapters::ReadStringField(Object, TEXT("contains")),
  };
}

/** User Story: As a fp composition collections consumer, I need to invoke read collections fixture through a stable signature so the fp composition collections workflow remains explicit and composable. @fn inline FCollectionsFixture ReadCollectionsFixture(const DataAdapters::FSettingsSource &Source) */
inline FCollectionsFixture
ReadCollectionsFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("collections"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadNumberVectorField(Object, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV7E812B20E15D)),
      DataAdapters::ReadNumberField(Object, TEXT("foldSeed")),
      DataAdapters::ReadNumberField(Object, TEXT("foldExpected")),
      DataAdapters::ReadNumberField(Object, TEXT("filterValue")),
      DataAdapters::ReadNumberField(Object, TEXT("filterExpectedCount")),
      DataAdapters::ReadNumberField(Object, TEXT("findValue")),
      DataAdapters::ReadNumberField(Object, TEXT("uniqueExpectedCount")),
      DataAdapters::ReadNumberField(Object, TEXT("containsValue")),
      ReadCollectionsLabels(Labels),
  };
}

} // namespace Testing::FP::Composition
