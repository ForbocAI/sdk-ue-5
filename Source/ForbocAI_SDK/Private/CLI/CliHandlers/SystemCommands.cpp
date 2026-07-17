// User Story: As a developer, I need this module to function.
#include "CLI/CliHandlers.h"
#include "Features/CLI/Presentation/PresentationSelectors.h"
#include "Features/CLI/System/SystemThunks.h"
#include "Features/Config/ConfigSelectors.h"
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
  const FRuntimeState &State = Store.getState();
  const ForbocAI::CLI::FCLICommandRoles &Roles = State.CLI.CommandRoles;
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      &PresentationState =
          ForbocAI::CLI::Presentation::selectCliPresentation(State);

  return CommandKey == Roles.Version
             ? [&]() -> HandlerResult {
                 ForbocAI::CLI::Presentation::logCliMessage(
                     ForbocAI::CLI::Presentation::selectCliVersionLine(
                         PresentationState,
                         ConfigSelectors::selectSdkVersion(State)));
                 return just(Result::Success(TCHAR_TO_UTF8(
                     *PresentationState.Results.VersionPrinted)));
               }()
         : CommandKey == Roles.Status
             ? [&]() -> HandlerResult {
                 FApiStatusResponse Status = Ops::checkApiStatus(Store);
                 ForbocAI::CLI::Presentation::logCliMessage(
                     ForbocAI::CLI::Presentation::selectCliStatusLine(
                         PresentationState, Status.Status));
                 return just(Result::Success(TCHAR_TO_UTF8(
                     *APISlice::Detail::ToJson(Status))));
               }()
         : CommandKey == Roles.Doctor
             ? [&]() -> HandlerResult {
                 ForbocAI::CLI::Presentation::logCliMessage(
                     ForbocAI::CLI::Presentation::selectCliVersionLine(
                         PresentationState,
                         ConfigSelectors::selectSdkVersion(State)));
                 ForbocAI::CLI::Presentation::logCliMessage(
                     ForbocAI::CLI::Presentation::selectCliApiUrlLine(
                         PresentationState,
                         ConfigSelectors::selectApiUrl(State)));
                 ForbocAI::CLI::Presentation::logCliMessage(
                     ForbocAI::CLI::Presentation::selectCliApiKeyLine(
                         PresentationState,
                         !ConfigSelectors::selectApiKey(State).IsEmpty()));
                 FApiStatusResponse Status = Ops::checkApiStatus(Store);
                 ForbocAI::CLI::Presentation::logCliMessage(
                     ForbocAI::CLI::Presentation::selectCliApiStatusLine(
                         PresentationState, Status.Status, Status.Version));
                 return just(Result::Success(TCHAR_TO_UTF8(
                     *PresentationState.Results.DoctorCompleted)));
               }()
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
