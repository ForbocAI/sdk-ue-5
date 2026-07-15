#pragma once

#include "Features/Testing/FP/Composition/Collections/CollectionsTypes.h"
#include "Features/Testing/FP/Composition/MaybeEither/MaybeEitherTypes.h"
#include "Features/Testing/FP/Composition/NameParity/NameParityTypes.h"

namespace Testing::FP::Composition {

struct FCompositionFixtures {
  FCollectionsFixture Collections;
  FMaybeEitherFixture MaybeEither;
  FNameParityFixture NameParity;
};

} // namespace Testing::FP::Composition
