#pragma once

#include "Features/Config/Environment/EnvironmentAdapters.h"
#include "Features/Config/Values/ValuesAdapters.h"

namespace SDKConfig {

inline void EnsureInitialized() {
  !InitializedStorage() ? (InitializeConfig(), void()) : void();
}

inline void ReloadConfig() {
  InitializedStorage() = false;
  InitializeConfig();
}

inline void InitializeConfig() {
  InitializedStorage()
      ? void()
      : (ResetToDefaults(), LoadFromConfigFile(), LoadFromEnvironment(),
         (void)(InitializedStorage() = true));
}

} // namespace SDKConfig
