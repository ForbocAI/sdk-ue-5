#pragma once

#include "Features/Testing/FP/Composition/Collections/CollectionsAdapters.h"
#include "Features/Testing/FP/Composition/CompositionTypes.h"
#include "Features/Testing/FP/Composition/MaybeEither/MaybeEitherAdapters.h"
#include "Features/Testing/FP/Composition/NameParity/NameParityAdapters.h"

namespace Testing::FP::Composition {

inline const DataAdapters::FSettingsSource &CompositionSettingsSource() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/fp/composition.json"));
  return Source;
}

inline const FCompositionFixtures &CompositionFixtures() {
  static const FCompositionFixtures Fixtures = {
      ReadCollectionsFixture(CompositionSettingsSource()),
      ReadMaybeEitherFixture(CompositionSettingsSource()),
      ReadNameParityFixture(CompositionSettingsSource()),
  };
  return Fixtures;
}

} // namespace Testing::FP::Composition
