#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Testing/FP/Composition/Map/MapTypes.h"

namespace Testing::FP::Composition {

/** User Story: As fp map-law verification, I need labels read from the shared JSON fixture so assertions contain no authored strings. @fn inline FMapLabels ReadMapLabels(const TSharedRef<FJsonObject> &Object) */
inline FMapLabels ReadMapLabels(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("leftIdentity")),
      DataAdapters::ReadStringField(Object, TEXT("rightIdentity")),
      DataAdapters::ReadStringField(Object, TEXT("associative")),
      DataAdapters::ReadStringField(Object, TEXT("rightBias")),
      DataAdapters::ReadStringField(Object, TEXT("preservesLeft")),
      DataAdapters::ReadStringField(Object, TEXT("preservesRight")),
      DataAdapters::ReadStringField(Object, TEXT("count")),
  };
}

/** User Story: As fp map-law verification, I need one typed fixture assembled from canonical JSON so map laws consume data rather than literals. @fn inline FMapFixture ReadMapFixture(const DataAdapters::FSettingsSource &Source) */
inline FMapFixture
ReadMapFixture(const DataAdapters::FSettingsSource &Source) {
  const TSharedRef<FJsonObject> Object =
      DataAdapters::ReadObjectField(Source, TEXT("maps"));
  const TSharedRef<FJsonObject> Labels =
      DataAdapters::ReadObjectField(Object, TEXT("labels"));
  return {
      DataAdapters::ReadStringField(Object, TEXT("leftOnlyKey")),
      DataAdapters::ReadStringField(Object, TEXT("sharedKey")),
      DataAdapters::ReadStringField(Object, TEXT("rightOnlyKey")),
      DataAdapters::ReadNumberField(Object, TEXT("leftOnlyValue")),
      DataAdapters::ReadNumberField(Object, TEXT("leftSharedValue")),
      DataAdapters::ReadNumberField(Object, TEXT("rightSharedValue")),
      DataAdapters::ReadNumberField(Object, TEXT("rightOnlyValue")),
      DataAdapters::ReadNumberField(Object, TEXT("expectedCount")),
      ReadMapLabels(Labels),
  };
}

} // namespace Testing::FP::Composition
