#pragma once

#include "TestGame/Features/Systems/CLI/CLIAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
#include "TestGame/Features/Systems/Harness/Game/GameThunks.h"
#include "TestGame/Features/Systems/Terminal/TerminalSelectors.h"
#include "Features/Config/ConfigActions.h"
#include "Features/Config/ConfigSelectors.h"

namespace TestGame::CLI {

using FCliPresenter = TFunction<void(const FTerminalProgressViewModel &)>;

namespace detail {

/** User Story: As a CLI operator, I need selected presentation emitted through an injected boundary so thunks never depend on Unreal logging. @fn inline void present(const FCliPresenter &Presenter, const FTerminalProgressViewModel &ViewModel) */
inline void present(const FCliPresenter &Presenter,
                    const FTerminalProgressViewModel &ViewModel) {
  Presenter ? (Presenter(ViewModel), void()) : void();
}

/** User Story: As a CLI operator, I need the contract command executed by the shared command runner so explicit contract output and game bootstrap test the same path. @fn inline int32 runContractCommand(FTestGameStore &Store, const FString &ApiUrl, const FCliPresenter &Presenter) */
inline int32 runContractCommand(FTestGameStore &Store,
                                const FString &ApiUrl,
                                const FCliPresenter &Presenter) {
  const CommandRunner::FCommandOutput Result = CommandRunner::Execute(
      GameAdapters::GameData().contractCommand.Command, Store, ApiUrl,
      CommandRunner::CreateCommandAliasState({}, {}, {}));
  present(Presenter, SelectContractViewModel(Result));
  return Result.Status == GameAdapters::GameRuntimeData().statuses.ok
             ? cliRuntimeData().exitCodes.success
             : cliRuntimeData().exitCodes.failure;
}

/** User Story: As a CLI operator, I need game execution composed around one package store so every scenario, alias, transcript, and RTK Query cache shares one root. @fn inline int32 runGameCommand(FTestGameStore &Store, const FString &Mode, const FString &ApiUrl, const FCliPresenter &Presenter) */
inline int32 runGameCommand(FTestGameStore &Store, const FString &Mode,
                            const FString &ApiUrl,
                            const FCliPresenter &Presenter) {
  const FGameProgressSink ProgressSink =
      [&Presenter](const FGameProgress &Progress) {
        present(Presenter, SelectTerminalProgressViewModel(Progress));
      };
  const FGameRunResult Result = RunGame(Store, Mode, ApiUrl, ProgressSink);
  return Result.bComplete ? cliRuntimeData().exitCodes.success
                          : cliRuntimeData().exitCodes.incomplete;
}

} // namespace detail

/** User Story: As a test-game CLI operator, I need one thunk to normalize host arguments, configure the SDK, route commands through the package root store, and return authored exit codes on every platform. @fn inline int32 runCli(const FString &Params, FTestGameStore &Store, const FCliPresenter &Presenter) */
inline int32 runCli(const FString &Params, FTestGameStore &Store,
                    const FCliPresenter &Presenter) {
  const FCliInvocation Invocation =
      normalizeInvocation(buildCliArgTokens(Params));
  Store.dispatch(ConfigSlice::Actions::configurationHydrated(
      ConfigSlice::readConfigState({Invocation.ApiUrl, Invocation.ApiKey})));
  const ConfigSlice::FConfigState &Config =
      ConfigSelectors::selectConfig(Store.getState());
  const FString ApiUrl = Config.ApiUrl;
  return requestsHelp(Invocation)
             ? (detail::present(Presenter, SelectUsageViewModel()),
                cliRuntimeData().exitCodes.success)
         : selectCommand(Invocation) == cliRuntimeData().commands.contract
             ? detail::runContractCommand(Store, ApiUrl, Presenter)
             : [&]() {
                 const FString Mode = resolveMode(Invocation);
                 return GameAdapters::GameRuntimeData().modes.all.Contains(
                            Mode)
                            ? detail::runGameCommand(Store, Mode, ApiUrl,
                                                     Presenter)
                            : (detail::present(
                                   Presenter,
                                   SelectInvalidModeViewModel(Mode)),
                               detail::present(Presenter,
                                               SelectUsageViewModel()),
                               cliRuntimeData().exitCodes.incomplete);
               }();
}

} // namespace TestGame::CLI
