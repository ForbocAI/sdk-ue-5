#pragma once

#include "MicroGame/Features/Systems/CLI/CLIAdapters.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerAdapters.h"
#include "MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
#include "MicroGame/Features/Systems/Harness/CommandDiagnostics/CommandDiagnosticsAdapters.h"
#include "MicroGame/Features/Systems/Harness/Verification/VerificationThunks.h"
#include "MicroGame/Features/Systems/Terminal/TerminalSelectors.h"
#include "Systems/CLI/CLIThunks.h"

namespace MicroGame::CLI {

using FCliPresenter = TFunction<void(const FTerminalProgressViewModel &)>;

namespace detail {

/** User Story: As a CLI operator, I need selected presentation emitted through an injected boundary so thunks never depend on Unreal logging. @fn inline void present(const FCliPresenter &Presenter, const FTerminalProgressViewModel &ViewModel) */
inline void present(const FCliPresenter &Presenter,
                    const FTerminalProgressViewModel &ViewModel) {
  Presenter ? (Presenter(ViewModel), void()) : void();
}

/** User Story: As a CLI operator, I need game execution composed around one package store while operations use the SDK root store. @fn inline int32 runGameCommand(FMicroGameStore &Store, const FString &Mode, const FCliPresenter &Presenter) */
inline int32 runGameCommand(FMicroGameStore &Store, const FString &Mode,
                            const FCliPresenter &Presenter) {
  const FGameProgressSink ProgressSink =
      [&Presenter](const FGameProgress &Progress) {
        present(Presenter, SelectTerminalProgressViewModel(Progress));
      };
  const CommandDiagnostics::FCommandDebugEnvironment DebugEnvironment =
      CommandDiagnostics::BeginCommandDiagnostics(Mode);
  const FGameRunResult Result = RunGame(Store, Mode, ProgressSink);
  CommandDiagnostics::EndCommandDiagnostics(Mode, DebugEnvironment);
  return Result.bComplete ? cliRuntimeData().exitCodes.success
                          : cliRuntimeData().exitCodes.incomplete;
}

} // namespace detail

/** User Story: As a micro-game CLI operator, I need one thunk to configure the SDK root, run the game, and return authored exit codes on every platform. @fn inline int32 runCli(const FString &Params, FMicroGameStore &Store, const FCliPresenter &Presenter) */
inline int32 runCli(const FString &Params, FMicroGameStore &Store,
                    const FCliPresenter &Presenter) {
  const FCliInvocation Invocation =
      normalizeInvocation(buildCliArgTokens(Params));
  CLIOps::ConfigureInvocation(Invocation.ApiUrl, Invocation.ApiKey);
  return requestsHelp(Invocation)
             ? (detail::present(Presenter, SelectUsageViewModel()),
                cliRuntimeData().exitCodes.success)
             : [&]() {
                 const FString Mode = resolveMode(Invocation);
                 return VerificationVocabularyAdapters::GameRuntimeData().modes.all.Contains(
                            Mode)
                            ? detail::runGameCommand(Store, Mode, Presenter)
                            : (detail::present(
                                   Presenter,
                                   SelectInvalidModeViewModel(Mode)),
                               detail::present(Presenter,
                                               SelectUsageViewModel()),
                               cliRuntimeData().exitCodes.incomplete);
               }();
}

} // namespace MicroGame::CLI
