#pragma once

#include "Core/RTK/Batch/AutoBatchTypes.hpp"
#include "Core/RTK/Utility/Utility.hpp"

namespace rtk {

/** User Story: As a core rtk batch consumer, I need to invoke auto batch enhancer through a stable signature so the core rtk batch workflow remains explicit and composable. @fn inline AutoBatchEnhancer autoBatchEnhancer( const AutoBatchOptions &Options = AutoBatchOptions()) */
inline AutoBatchEnhancer autoBatchEnhancer(
    const AutoBatchOptions &Options = AutoBatchOptions()) {
  return AutoBatchEnhancer{Options};
}

/** User Story: As a core rtk batch consumer, I need to invoke prepare auto batched through a stable signature so the core rtk batch workflow remains explicit and composable. @fn template <typename Payload> AnyAction prepareAutoBatched(const FString &Type, const Payload &Value) */
template <typename Payload>
AnyAction prepareAutoBatched(const FString &Type, const Payload &Value) {
  AnyAction Result = ActionCreator<Payload>{Type}(Value);
  Result.bAutoBatch = true;
  Result.Meta.Add(SHOULD_AUTOBATCH, LexToString(true));
  return Result;
}

} // namespace rtk
