#pragma once

#include "Features/Config/Environment/EnvironmentAdapters.h"
#include "Features/Config/Values/ValuesAdapters.h"

namespace SDKConfig {

/** User Story: As a features config lifecycle consumer, I need to invoke ensure initialized through a stable signature so the features config lifecycle workflow remains explicit and composable. @fn inline void EnsureInitialized() */
inline void EnsureInitialized() {
  !InitializedStorage() ? (InitializeConfig(), void()) : void();
}

/** User Story: As a features config lifecycle consumer, I need to invoke reload config through a stable signature so the features config lifecycle workflow remains explicit and composable. @fn inline void ReloadConfig() */
inline void ReloadConfig() {
  InitializedStorage() = false;
  InitializeConfig();
}

/** User Story: As a features config lifecycle consumer, I need to invoke initialize config through a stable signature so the features config lifecycle workflow remains explicit and composable. @fn inline void InitializeConfig() */
inline void InitializeConfig() {
  InitializedStorage()
      ? void()
      : (ResetToDefaults(), LoadFromConfigFile(), LoadFromEnvironment(),
         (void)(InitializedStorage() = true));
}

} // namespace SDKConfig
