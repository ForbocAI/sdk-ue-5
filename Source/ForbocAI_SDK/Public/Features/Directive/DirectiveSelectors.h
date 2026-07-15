#pragma once

#include "Core/fp.hpp"
#include "Features/Directive/DirectiveAdapters.h"
#include "Features/Directive/DirectiveTypes.h"

namespace DirectiveSlice {

inline func::Maybe<FDirectiveRun> selectDirectiveById(
    const FDirectiveSliceState &State, const FString &Id) {
  return GetDirectiveAdapter().getSelectors().selectById(State.Entities, Id);
}

inline TArray<FDirectiveRun>
selectAllDirectives(const FDirectiveSliceState &State) {
  return GetDirectiveAdapter().getSelectors().selectAll(State.Entities);
}

inline FString selectActiveDirectiveId(const FDirectiveSliceState &State) {
  return State.ActiveDirectiveId;
}

inline func::Maybe<FDirectiveRun>
selectActiveDirective(const FDirectiveSliceState &State) {
  return State.ActiveDirectiveId.IsEmpty()
             ? func::nothing<FDirectiveRun>()
             : selectDirectiveById(State, State.ActiveDirectiveId);
}

} // namespace DirectiveSlice
