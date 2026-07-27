#pragma once

#include "Components/Testing/FP/Composition/Collections/CollectionsTypes.h"
#include "Components/Testing/FP/Composition/MaybeEither/MaybeEitherTypes.h"
#include "Components/Testing/FP/Composition/NameParity/NameParityTypes.h"

namespace Testing::FP::Composition {

struct FCompositionFixtures {
  FCollectionsFixture Collections;
  FMaybeEitherFixture MaybeEither;
  FNameParityFixture NameParity;
};

} // namespace Testing::FP::Composition
