#pragma once

#include "Core/FP/Prelude.hpp"

namespace func {
namespace detail {

template <typename T> T failWithMessage(const std::string &Message) {
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS) || defined(_CPPUNWIND)
  throw std::runtime_error(Message);
#else
  std::abort();
#endif
}

} // namespace detail
} // namespace func
