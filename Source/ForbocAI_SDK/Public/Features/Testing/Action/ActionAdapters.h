#pragma once

#include "Core/fp.hpp"
#include "Features/Testing/Action/ActionTypes.h"

namespace Testing::Action {

/**
 * User Story: As a reducer-test fixture consumer, I need authored action names
 * resolved strictly so enum reordering cannot change a scenario's behavior.
 * @fn template <typename TKind> TKind ReadTestingActionKind( const FString &Name, const TArray<TTestingActionKind<TKind>> &Kinds)
 */
template <typename TKind>
TKind ReadTestingActionKind(
    const FString &Name, const TArray<TTestingActionKind<TKind>> &Kinds) {
  const func::Maybe<TTestingActionKind<TKind>> Match =
      func::find_array<TTestingActionKind<TKind>>(
          Kinds, [&Name](const TTestingActionKind<TKind> &Candidate) {
            return Candidate.Name == Name;
          });
  check(Match.hasValue);
  return Match.value.Kind;
}

} // namespace Testing::Action
