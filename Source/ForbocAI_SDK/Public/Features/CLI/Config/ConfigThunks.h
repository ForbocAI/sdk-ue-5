#pragma once

#include "Features/Config/ConfigAdapters.h"

namespace Ops {

inline void setConfigValue(const FString &Key, const FString &Value) {
  SDKConfig::SetConfigValue(Key, Value);
}

inline FString getConfigValue(const FString &Key) {
  return SDKConfig::GetConfigValue(Key);
}

} // namespace Ops
