// User Story: As a developer, I need this module to function.
#include "Systems/CLI/DiagnosticsCommands/DiagnosticsCommandsThunks.h"
#include "Entities/CLI/Presentation/PresentationSelectors.h"
#include "Systems/CLI/Diagnostics/DiagnosticsThunks.h"
#include "Entities/Config/ConfigSelectors.h"

namespace {

using DiagnosticsResult = CLIOps::CommandRouting::Result;

/**
 * User Story: As a CLI diagnostics consumer, I need the SDK version presented through a stable command result.
 * @fn DiagnosticsResult PrintVersion( const FRuntimeState &State, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState)
 */
DiagnosticsResult PrintVersion(
    const FRuntimeState &State,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliVersionLine(
          PresentationState, ConfigSelectors::selectSdkVersion(State)));
  return DiagnosticsResult::Success(
      TCHAR_TO_UTF8(*PresentationState.Results.VersionPrinted));
}

/**
 * User Story: As a CLI diagnostics consumer, I need API health presented through a stable command result.
 * @fn DiagnosticsResult PrintStatus( rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState)
 */
DiagnosticsResult PrintStatus(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  const FApiStatusResponse Status = Ops::checkApiStatus(Store);
  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliStatusLine(PresentationState,
                                                       Status.Status));
  return DiagnosticsResult::Success(
      TCHAR_TO_UTF8(*APISlice::Detail::ToJson(Status)));
}

/**
 * User Story: As a CLI diagnostics consumer, I need the API-owned verification contract presented without CLI-owned interpretation.
 * @fn DiagnosticsResult PrintContract(rtk::EnhancedStore<FRuntimeState> &Store)
 */
DiagnosticsResult PrintContract(rtk::EnhancedStore<FRuntimeState> &Store) {
  const FString Contract = AsyncAdapters::waitForResult(
      Ops::getMicroGameContract(Store));
  ForbocAI::CLI::Presentation::logCliMessage(Contract);
  return DiagnosticsResult::Success(TCHAR_TO_UTF8(*Contract));
}

/**
 * User Story: As a CLI diagnostics consumer, I need configuration and API readiness checked through SDK selectors and thunks.
 * @fn DiagnosticsResult RunDoctor( rtk::EnhancedStore<FRuntimeState> &Store, const FRuntimeState &State, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState)
 */
DiagnosticsResult RunDoctor(
    rtk::EnhancedStore<FRuntimeState> &Store, const FRuntimeState &State,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliVersionLine(
          PresentationState, ConfigSelectors::selectSdkVersion(State)));
  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliApiUrlLine(
          PresentationState, ConfigSelectors::selectApiUrl(State)));
  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliApiKeyLine(
          PresentationState, !ConfigSelectors::selectApiKey(State).IsEmpty()));
  const FApiStatusResponse Status = Ops::checkApiStatus(Store);
  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliApiStatusLine(
          PresentationState, Status.Status, Status.Version));
  return DiagnosticsResult::Success(
      TCHAR_TO_UTF8(*PresentationState.Results.DoctorCompleted));
}

/**
 * User Story: As a CLI diagnostics consumer, I need diagnostics commands resolved through a typed functional dispatcher.
 * @fn func::Dispatcher<FString, DiagnosticsResult> CreateDiagnosticsDispatcher( rtk::EnhancedStore<FRuntimeState> &Store, const FRuntimeState &State, const ForbocAI::CLI::FCLICommandRoles &Roles, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState)
 */
func::Dispatcher<FString, DiagnosticsResult>
CreateDiagnosticsDispatcher(
    rtk::EnhancedStore<FRuntimeState> &Store, const FRuntimeState &State,
    const ForbocAI::CLI::FCLICommandRoles &Roles,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  return func::createDispatcher<FString, DiagnosticsResult>({
      {Roles.Version, [&]() { return PrintVersion(State, PresentationState); }},
      {Roles.Status, [&]() { return PrintStatus(Store, PresentationState); }},
      {Roles.Contract, [&]() { return PrintContract(Store); }},
      {Roles.Doctor,
       [&]() { return RunDoctor(Store, State, PresentationState); }},
  });
}

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI diagnostics consumer, I need health and contract commands routed through composed SDK thunks. @fn RouteResult RouteDiagnosticsCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteDiagnosticsCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                                const FString &CommandKey,
                                const TArray<FString> &Args) {
  (void)Args;
  const FRuntimeState &State = Store.getState();
  const ForbocAI::CLI::FCLICommandRoles &Roles = State.CLI.CommandRoles;
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      &PresentationState =
          ForbocAI::CLI::Presentation::selectCliPresentation(State);
  return func::dispatch(
      CreateDiagnosticsDispatcher(Store, State, Roles, PresentationState),
      CommandKey);
}

} // namespace CommandRouting
} // namespace CLIOps
