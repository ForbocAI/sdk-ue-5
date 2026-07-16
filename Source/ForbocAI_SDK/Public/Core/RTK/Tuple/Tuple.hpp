#pragma once

#include "Core/RTK/Prelude.hpp"

namespace rtk {

template <typename Value> struct Tuple {
  TArray<Value> Values;

  /** User Story: As a core rtk tuple consumer, I need to invoke tuple through a stable signature so the core rtk tuple workflow remains explicit and composable. @fn Tuple() */
  Tuple() {}
  /** User Story: As a core rtk tuple consumer, I need to invoke tuple through a stable signature so the core rtk tuple workflow remains explicit and composable. @fn explicit Tuple(const TArray<Value> &InitialValues) */
  explicit Tuple(const TArray<Value> &InitialValues) : Values(InitialValues) {}

  /** User Story: As a core rtk tuple consumer, I need to invoke concat through a stable signature so the core rtk tuple workflow remains explicit and composable. @fn Tuple<Value> concat(const TArray<Value> &AdditionalValues) const */
  Tuple<Value> concat(const TArray<Value> &AdditionalValues) const {
    Tuple<Value> Result(*this);
    Result.Values.Append(AdditionalValues);
    return Result;
  }

  /** User Story: As a core rtk tuple consumer, I need to invoke prepend through a stable signature so the core rtk tuple workflow remains explicit and composable. @fn Tuple<Value> prepend(const Value &Head) const */
  Tuple<Value> prepend(const Value &Head) const {
    Tuple<Value> Result;
    Result.Values.Add(Head);
    Result.Values.Append(Values);
    return Result;
  }
};

} // namespace rtk
