#include "Systems/CLI/CLIThunks.h"
#include "Systems/CLI/CommandRouting/CommandRoutingThunks.h"
#include "Entities/CLI/Presentation/PresentationSelectors.h"
#include "Systems/Config/ConfigThunks.h"
#include "Systems/CLI/Invocation/InvocationAdapters.h"
#include "Entities/CLI/Invocation/InvocationSelectors.h"
#include "Systems/Store/StoreAdapters.h"
#include <exception>

namespace CLIOps {

namespace {

using CLIResult = func::TestResult<void>;
using CLICommandRoute = std::function<CLIOps::CommandRouting::RouteResult(
    rtk::EnhancedStore<FRuntimeState> &, const FString &,
    const TArray<FString> &)>;

/**
 * User Story: As a CLI boundary, I need validation failures and dispatch failures represented by one result type.
 * @fn func::TestResult<void> ValidateAndDispatch( rtk::EnhancedStore<FRuntimeState> &Store, const CommandletInvocation::FInvocation &Invocation, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState)
 */
func::TestResult<void> ValidateAndDispatch(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const CommandletInvocation::FInvocation &Invocation,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  const auto Validation = func::runValidation(
      CommandletInvocation::selectCommandValidationPipeline(
          Store.getState().CLI, PresentationState),
      Invocation.Command);
  return func::ematch(
      Validation,
      [](const FString &Error) {
        return func::TestResult<void>::Failure(TCHAR_TO_UTF8(*Error));
      },
      [&Invocation, &PresentationState](const FString &) {
        const func::TestResult<void> Result =
            CLIOps::DispatchCommand(Invocation.Command, Invocation.Args);
        const FString Message =
            Result.message.empty()
                ? FString()
                : FString(UTF8_TO_TCHAR(Result.message.c_str()));
        return Result.isSuccessful() || !Message.IsEmpty()
                   ? Result
                   : func::TestResult<void>::Failure(TCHAR_TO_UTF8(
                         *ForbocAI::CLI::Presentation::
                             selectCliDispatchFailedMessage(
                                 PresentationState, Invocation.Command)));
      });
}

/** User Story: As a CLI consumer, I need recursive command routing through one ordered route collection so dispatch remains explicit and composable. @fn CLIResult RouteRecursive( const std::vector<CLICommandRoute> &Routes, size_t Index, rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args, const ForbocAI::CLI::FCLIParsingSettings &Parsing, const ForbocAI::CLI::Presentation::FCLIPresentationState &PresentationState) */
CLIResult RouteRecursive(
    const std::vector<CLICommandRoute> &Routes, size_t Index,
    rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey,
    const TArray<FString> &Args,
    const ForbocAI::CLI::FCLIParsingSettings &Parsing,
    const ForbocAI::CLI::Presentation::FCLIPresentationState
        &PresentationState) {
  return CLIOps::CommandRouting::matchCondition<CLIResult>(
      Index < Routes.size(),
      [&]() {
        const CLIOps::CommandRouting::RouteResult Result =
            Routes[Index](Store, CommandKey, Args);
        return Result.hasValue
                   ? Result.value
                   : RouteRecursive(
                         Routes, Index + Parsing.NextIndexOffset, Store,
                         CommandKey, Args, Parsing, PresentationState);
      },
      [&]() {
        return CLIResult::Failure(TCHAR_TO_UTF8(
            *ForbocAI::CLI::Presentation::selectCliUnknownCommandMessage(
                PresentationState, CommandKey)));
      });
}

/** User Story: As a CLI consumer, I need one ordered command-route registry so each domain owns its command behavior. @fn const std::vector<CLICommandRoute> &CommandRoutes() */
const std::vector<CLICommandRoute> &CommandRoutes() {
  using namespace CLIOps::CommandRouting;
  static const std::vector<CLICommandRoute> Routes = {
      RouteDiagnosticsCommand, RouteNpcCommand,  RouteMemoryCommand,
      RouteGhostCommand,       RouteBridgeCommand, RouteSoulCommand,
      RouteConfigurationCommand, RouteDependencyCommand,
  };
  return Routes;
}

} // namespace

/** User Story: As an Unreal command host, I need one SDK-owned invocation workflow so the commandlet remains a thin transport adapter. @fn int32 ExecuteInvocation(const FString &Params) */
int32 ExecuteInvocation(const FString &Params) {
  rtk::EnhancedStore<FRuntimeState> &Store = StoreAdapters::RootStore();
  const CommandletInvocation::FInvocation Invocation =
      CommandletInvocation::ResolveInvocation(Params, Store.getState().CLI);
  Ops::hydrateRuntimeConfig(Store, {Invocation.ApiUrl, Invocation.ApiKey});
  const ForbocAI::CLI::Presentation::FCLIPresentationState PresentationState =
      ForbocAI::CLI::Presentation::selectCliPresentation(Store.getState());
  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliCommandStartedLine(
          PresentationState, Invocation.Command));
  const func::TestResult<void> Result =
      ValidateAndDispatch(Store, Invocation, PresentationState);
  ForbocAI::CLI::Presentation::logCliMessage(PresentationState.Common.Blank);
  Result.isSuccessful()
      ? ForbocAI::CLI::Presentation::logCliMessage(
            PresentationState.Runtime.CommandSucceeded)
      : ForbocAI::CLI::Presentation::logCliError(
            ForbocAI::CLI::Presentation::formatCliMessage(
                PresentationState.Runtime.CommandFailed,
                UTF8_TO_TCHAR(Result.message.c_str())));
  return Result.isSuccessful() ? PresentationState.Defaults.SuccessExitCode
                               : PresentationState.Defaults.FailureExitCode;
}

/** User Story: As an embedded CLI host, I need invocation overrides delegated to the SDK config thunk and root store. @fn void ConfigureInvocation(const FString &ApiUrl, const FString &ApiKey) */
void ConfigureInvocation(const FString &ApiUrl, const FString &ApiKey) {
  Ops::hydrateRuntimeConfig(StoreAdapters::RootStore(), {ApiUrl, ApiKey});
}

/** User Story: As a cli consumer, I need to invoke dispatch command through a stable signature so the cli workflow remains explicit and composable. @fn func::TestResult<void> DispatchCommand(const FString &CommandKey, const TArray<FString> &Args) */
func::TestResult<void> DispatchCommand(const FString &CommandKey,
                                       const TArray<FString> &Args) {
  rtk::EnhancedStore<FRuntimeState> &Store = StoreAdapters::RootStore();
  const FRuntimeState &State = Store.getState();
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      &PresentationState =
          ForbocAI::CLI::Presentation::selectCliPresentation(State);

  try {
    return RouteRecursive(CommandRoutes(), State.CLI.Parsing.FirstTokenIndex,
                          Store, CommandKey, Args, State.CLI.Parsing,
                          PresentationState);
  } catch (const std::exception &Error) {
    return CLIResult::Failure(std::string(Error.what()));
  }
}

} // namespace CLIOps
