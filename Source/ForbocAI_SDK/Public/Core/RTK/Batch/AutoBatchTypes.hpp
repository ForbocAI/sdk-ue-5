#pragma once

#include "Core/RTK/Prelude.hpp"

namespace rtk {

struct AutoBatchOptions {
  enum class Type { Tick, Timer, Raf, Callback };
  Type Schedule = Type::Tick;
  float TimeoutSeconds = float{};
  std::function<void(std::function<void()>)> QueueNotification;
};

struct AutoBatchEnhancer {
  AutoBatchOptions Options;
};

} // namespace rtk
