#pragma once

#include "Core/fp.hpp"
#include "Systems/Directive/DirectiveAdapters.h"
#include "Components/Directive/DirectiveTypes.h"

namespace DirectiveSlice {

/** User Story: As a features directive consumer, I need to invoke select directive by id through a stable signature so the features directive workflow remains explicit and composable. @fn inline func::Maybe<FDirectiveRun> selectDirectiveById( const FDirectiveSliceState &State, const FString &Id) */
inline func::Maybe<FDirectiveRun> selectDirectiveById(
    const FDirectiveSliceState &State, const FString &Id) {
  return GetDirectiveAdapter().getSelectors().selectById(State.Entities, Id);
}

/** User Story: As a features directive consumer, I need to invoke select all directives through a stable signature so the features directive workflow remains explicit and composable. @fn inline TArray<FDirectiveRun> selectAllDirectives(const FDirectiveSliceState &State) */
inline TArray<FDirectiveRun>
selectAllDirectives(const FDirectiveSliceState &State) {
  return GetDirectiveAdapter().getSelectors().selectAll(State.Entities);
}

/** User Story: As a features directive consumer, I need to invoke select active directive id through a stable signature so the features directive workflow remains explicit and composable. @fn inline FString selectActiveDirectiveId(const FDirectiveSliceState &State) */
inline FString selectActiveDirectiveId(const FDirectiveSliceState &State) {
  return State.ActiveDirectiveId;
}

/** User Story: As a features directive consumer, I need to invoke select active directive through a stable signature so the features directive workflow remains explicit and composable. @fn inline func::Maybe<FDirectiveRun> selectActiveDirective(const FDirectiveSliceState &State) */
inline func::Maybe<FDirectiveRun>
selectActiveDirective(const FDirectiveSliceState &State) {
  return State.ActiveDirectiveId.IsEmpty()
             ? func::nothing<FDirectiveRun>()
             : selectDirectiveById(State, State.ActiveDirectiveId);
}

} // namespace DirectiveSlice
