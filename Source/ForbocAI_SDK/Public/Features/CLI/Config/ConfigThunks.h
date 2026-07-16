#pragma once

#include "Features/Config/ConfigAdapters.h"

namespace Ops {

/** User Story: As a features cli config consumer, I need to invoke set config value through a stable signature so the features cli config workflow remains explicit and composable. @fn inline void setConfigValue(const FString &Key, const FString &Value) */
inline void setConfigValue(const FString &Key, const FString &Value) {
  SDKConfig::SetConfigValue(Key, Value);
}

/** User Story: As a features cli config consumer, I need to invoke get config value through a stable signature so the features cli config workflow remains explicit and composable. @fn inline FString getConfigValue(const FString &Key) */
inline FString getConfigValue(const FString &Key) {
  return SDKConfig::GetConfigValue(Key);
}

} // namespace Ops
