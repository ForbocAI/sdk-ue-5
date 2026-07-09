#pragma once
/**
 * Directive adapters own directive entity indexing.
 */

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "CoreMinimal.h"
#include "Types.h"

namespace DirectiveSlice {

using namespace rtk;
using namespace func;

/**
 * Extracts the stable entity id from a directive run.
 * User Story: As directive entity adapters, I need a single id selector so
 * runs can be indexed and updated by id consistently.
 */
inline FString DirectiveIdSelector(const FDirectiveRun &Run) { return Run.Id; }

/**
 * Returns the entity adapter used to manage directive runs.
 * User Story: As directive reducers and selectors, I need a shared adapter so
 * entity operations stay consistent across the slice.
 */
inline rtk::EntityAdapter<FDirectiveRun> GetDirectiveAdapter() {
  return rtk::createEntityAdapter<FDirectiveRun>(&DirectiveIdSelector);
}

} // namespace DirectiveSlice
