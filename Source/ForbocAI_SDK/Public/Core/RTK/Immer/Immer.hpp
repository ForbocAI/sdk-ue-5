#pragma once

#include "Core/RTK/Types/Types.hpp"

namespace rtk {

/** User Story: As a core rtk immer consumer, I need to invoke create next state through a stable signature so the core rtk immer workflow remains explicit and composable. @fn template <typename State, typename Recipe> State createNextState(const State &CurrentState, Recipe MutateDraft) */
template <typename State, typename Recipe>
State createNextState(const State &CurrentState, Recipe MutateDraft) {
  State DraftValue = CurrentState;
  MutateDraft(DraftValue);
  return DraftValue;
}

/** User Story: As a core rtk immer consumer, I need to invoke freeze through a stable signature so the core rtk immer workflow remains explicit and composable. @fn template <typename State> const State &freeze(const State &Value) */
template <typename State> const State &freeze(const State &Value) {
  return Value;
}

/** User Story: As a core rtk immer consumer, I need to invoke is draft through a stable signature so the core rtk immer workflow remains explicit and composable. @fn template <typename State> bool isDraft(const State &) */
template <typename State> bool isDraft(const State &) { return false; }

/** User Story: As a core rtk immer consumer, I need to invoke original through a stable signature so the core rtk immer workflow remains explicit and composable. @fn template <typename State> const State &original(const State &Value) */
template <typename State> const State &original(const State &Value) {
  return Value;
}

} // namespace rtk
