#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
namespace detail {

/** User Story: As a core fp error consumer, I need to invoke fail with message through a stable signature so the core fp error workflow remains explicit and composable. @fn template <typename T> T failWithMessage(const std::string &Message) */
template <typename T> T failWithMessage(const std::string &Message) {
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
  throw std::runtime_error(Message);
#else
  std::abort();
#endif
}

} // namespace detail
} // namespace func
