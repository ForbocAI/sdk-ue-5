#pragma once

#include "CoreMinimal.h"

namespace Testing::Action {

/**
 * User Story: As a fixture decoder, I need each authored action name paired
 * with its typed reducer-test action so JSON never depends on enum ordinals.
 * @tparam TKind Reducer action-kind type represented by the fixture name.
 */
template <typename TKind> struct TTestingActionKind {
  FString Name;
  TKind Kind;
};

} // namespace Testing::Action
