#include "CLI/RuntimeCommandlet.h"
#include "CLI/CLIModule.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/Config/ConfigThunks.h"
#include "Features/CLI/Invocation/InvocationAdapters.h"
#include "Features/CLI/Invocation/InvocationSelectors.h"
#include "Features/CLI/Presentation/PresentationSelectors.h"
#include "Store.h"

/** User Story: As a cli consumer, I need to invoke uforboc aicommandlet through a stable signature so the cli workflow remains explicit and composable. @fn UForbocAICommandlet::UForbocAICommandlet() */
UForbocAICommandlet::UForbocAICommandlet() {
  IsClient = false;
  IsEditor = false;
  IsServer = false;
  LogToConsole = true;
}

/** User Story: As a cli consumer, I need to invoke main through a stable signature so the cli workflow remains explicit and composable. @fn int32 UForbocAICommandlet::Main(const FString &Params) */
int32 UForbocAICommandlet::Main(const FString &Params) {
  rtk::EnhancedStore<FRuntimeState> &RuntimeStore = store();
  const CommandletInvocation::FInvocation Invocation =
      CommandletInvocation::ResolveInvocation(Params,
                                              RuntimeStore.getState().CLI);
  Ops::hydrateRuntimeConfig(RuntimeStore,
                            {Invocation.ApiUrl, Invocation.ApiKey});
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      &PresentationState = ForbocAI::CLI::Presentation::selectCliPresentation(
          RuntimeStore.getState());

  ForbocAI::CLI::Presentation::logCliMessage(
      ForbocAI::CLI::Presentation::selectCliCommandStartedLine(
          PresentationState, Invocation.Command));

  bool bCommandFailed = false;
  const FString Blank = PresentationState.Common.Blank;
  const FString Success = PresentationState.Runtime.CommandSucceeded;
  const FString FailureTemplate = PresentationState.Runtime.CommandFailed;
  createCommandPipeline(Invocation.Command, Invocation.Args)
      .then([Blank, Success]() {
        ForbocAI::CLI::Presentation::logCliMessage(Blank);
        ForbocAI::CLI::Presentation::logCliMessage(Success);
      })
      .catch_([&bCommandFailed, Blank,
               FailureTemplate](std::string Message) {
        bCommandFailed = true;
        ForbocAI::CLI::Presentation::logCliMessage(Blank);
        ForbocAI::CLI::Presentation::logCliError(
            ForbocAI::CLI::Presentation::formatCliMessage(
                FailureTemplate, UTF8_TO_TCHAR(Message.c_str())));
      })
      .execute();
  return bCommandFailed ? PresentationState.Defaults.FailureExitCode
                        : PresentationState.Defaults.SuccessExitCode;
}

/** User Story: As a cli consumer, I need to invoke execute command through a stable signature so the cli workflow remains explicit and composable. @fn UForbocAICommandlet::CommandResult UForbocAICommandlet::executeCommand(const FString &Command, const TArray<FString> &Args) */
UForbocAICommandlet::CommandResult
UForbocAICommandlet::executeCommand(const FString &Command,
                                    const TArray<FString> &Args) {
  return CLIOps::DispatchCommand(Command, Args);
}

/** User Story: As a cli consumer, I need to invoke create command pipeline through a stable signature so the cli workflow remains explicit and composable. @fn UForbocAICommandlet::CommandExecution UForbocAICommandlet::createCommandPipeline(const FString &Command, const TArray<FString> &Args) */
UForbocAICommandlet::CommandExecution
UForbocAICommandlet::createCommandPipeline(const FString &Command,
                                           const TArray<FString> &Args) {
  const ForbocAI::CLI::Presentation::FCLIPresentationState
      PresentationState =
          ForbocAI::CLI::Presentation::selectCliPresentation(
              store().getState());
  return UForbocAICommandlet::CommandExecution::create(
      [this, Command, Args,
       PresentationState](std::function<void()> Resolve,
                          std::function<void(std::string)> Reject) {
        const auto Validation =
            func::runValidation(commandValidationPipeline(), Command);
        func::ematch(
            Validation,
            [&Reject](const FString &Error) {
              Reject(TCHAR_TO_UTF8(*Error));
            },
            [this, &Command, &Args, &Resolve, &Reject,
             &PresentationState](const FString &) {
              const CommandResult Result = executeCommand(Command, Args);
              const FString Message =
                  Result.message.empty()
                      ? FString()
                      : FString(UTF8_TO_TCHAR(Result.message.c_str()));
              Result.isSuccessful()
                  ? (Resolve(), void())
                  : (Reject(TCHAR_TO_UTF8(
                         *(Message.IsEmpty()
                               ? ForbocAI::CLI::Presentation::
                                     selectCliDispatchFailedMessage(
                                         PresentationState, Command)
                               : Message))),
                     void());
            });
      });
}

/** User Story: As a cli consumer, I need to invoke command validation pipeline through a stable signature so the cli workflow remains explicit and composable. @fn CLITypes::ValidationPipeline<FString, FString> UForbocAICommandlet::commandValidationPipeline() */
CLITypes::ValidationPipeline<FString, FString>
UForbocAICommandlet::commandValidationPipeline() {
  const FRuntimeState &State = store().getState();
  return CommandletInvocation::selectCommandValidationPipeline(
      State.CLI,
      ForbocAI::CLI::Presentation::selectCliPresentation(State));
}
