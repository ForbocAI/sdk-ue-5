#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"

#include "Interfaces/IPluginManager.h"

namespace DataAdapters {

/** User Story: As a features data consumer, I need to invoke module content dir through a stable signature so the features data workflow remains explicit and composable. @fn FString ModuleContentDir(const FString &ModuleName) */
FString ModuleContentDir(const FString &ModuleName) {
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(ModuleName);
  return Plugin.IsValid() ? Plugin->GetContentDir()
                          : FPaths::ProjectContentDir();
}

} // namespace DataAdapters
