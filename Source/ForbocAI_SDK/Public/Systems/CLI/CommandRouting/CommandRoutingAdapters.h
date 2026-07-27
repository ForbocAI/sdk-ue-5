#pragma once

#include "Core/fp.hpp"

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI route author, I need boolean boundary checks lifted into functional matching so command workflows remain composable. @fn template <typename Result, typename OnSatisfied, typename OnRejected> Result matchCondition(bool bCondition, OnSatisfied Satisfied, OnRejected Rejected) */
template <typename Result, typename OnSatisfied, typename OnRejected>
Result matchCondition(bool bCondition, OnSatisfied Satisfied,
                      OnRejected Rejected) {
  return func::match(
      func::fromNullable(true, bCondition),
      [&Satisfied](const bool &) { return Satisfied(); },
      [&Rejected]() { return Rejected(); });
}

} // namespace CommandRouting
} // namespace CLIOps
