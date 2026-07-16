// User Story: As a developer, I need this module to function.
#include "CLI/CliHandlers.h"
#include "Features/CLI/System/SystemThunks.h"
#include "Features/Config/ConfigAdapters.h"
#include "Store.h"

namespace CLIOps {
namespace Handlers {

/** User Story: As a cli cli handlers consumer, I need to invoke handle system through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn HandlerResult HandleSystem(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
HandlerResult HandleSystem(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args) {
  (void)Args;
  using func::just;
  using func::nothing;

  return CommandKey == TEXT("version")
             ? [&]() -> HandlerResult {
                 UE_LOG(LogTemp, Display, TEXT("ForbocAI SDK v%s (UE5)"),
                        *SDKConfig::GetSdkVersion());
                 return just(Result::Success("Version printed"));
               }()
         : CommandKey == TEXT("status")
             ? [&]() -> HandlerResult {
                 FApiStatusResponse Status = Ops::checkApiStatus(Store);
                 UE_LOG(LogTemp, Display, TEXT("API: %s"), *Status.Status);
                 return just(Result::Success("Status checked"));
               }()
         : CommandKey == TEXT("doctor")
             ? [&]() -> HandlerResult {
                 UE_LOG(LogTemp, Display, TEXT("ForbocAI SDK v%s (UE5)"),
                        *SDKConfig::GetSdkVersion());
                 UE_LOG(LogTemp, Display, TEXT("API URL: %s"),
                        *SDKConfig::GetApiUrl());
                 UE_LOG(LogTemp, Display, TEXT("API Key: %s"),
                        SDKConfig::GetApiKey().IsEmpty()
                            ? TEXT("(not set)")
                            : TEXT("********"));
                 FApiStatusResponse Status = Ops::checkApiStatus(Store);
                 UE_LOG(LogTemp, Display, TEXT("API Status: %s (v%s)"),
                        *Status.Status, *Status.Version);
                 return just(Result::Success("Doctor check completed"));
               }()
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
