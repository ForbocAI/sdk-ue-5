#pragma once

#include "Systems/Testing/FP/Composition/Collections/CollectionsAdapters.h"
#include "Components/Testing/FP/Composition/CompositionTypes.h"
#include "Systems/Testing/FP/Composition/Map/MapAdapters.h"
#include "Systems/Testing/FP/Composition/MaybeEither/MaybeEitherAdapters.h"
#include "Systems/Testing/FP/Composition/NameParity/NameParityAdapters.h"

namespace Testing::FP::Composition {

/** User Story: As a testing fp composition consumer, I need to invoke composition settings source through a stable signature so the testing fp composition workflow remains explicit and composable. @fn inline const DataAdapters::FSettingsSource &CompositionSettingsSource() */
inline const DataAdapters::FSettingsSource &CompositionSettingsSource() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                   TEXT("Data/tests/fp/composition.json"));
  return Source;
}

/** User Story: As a testing fp composition consumer, I need to invoke composition fixtures through a stable signature so the testing fp composition workflow remains explicit and composable. @fn inline const FCompositionFixtures &CompositionFixtures() */
inline const FCompositionFixtures &CompositionFixtures() {
  static const FCompositionFixtures Fixtures = {
      ReadCollectionsFixture(CompositionSettingsSource()),
      ReadMapFixture(CompositionSettingsSource()),
      ReadMaybeEitherFixture(CompositionSettingsSource()),
      ReadNameParityFixture(CompositionSettingsSource()),
  };
  return Fixtures;
}

} // namespace Testing::FP::Composition
