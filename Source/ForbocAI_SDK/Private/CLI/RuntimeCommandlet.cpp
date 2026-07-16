#include "CLI/RuntimeCommandlet.h"
#include "CLI/CLIModule.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/Invocation/InvocationAdapters.h"
#include "Features/Config/ConfigAdapters.h"
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
  SDKConfig::InitializeConfig();
  const CommandletInvocation::FInvocation Invocation =
      CommandletInvocation::ResolveInvocation(Params,
                                              store().getState().CLI);

  (!Invocation.ApiUrl.IsEmpty() || !Invocation.ApiKey.IsEmpty())
      ? (SDKConfig::SetApiConfig(
             Invocation.ApiUrl.IsEmpty() ? SDKConfig::GetApiUrl()
                                         : Invocation.ApiUrl,
             Invocation.ApiKey.IsEmpty() ? SDKConfig::GetApiKey()
                                         : Invocation.ApiKey),
         void())
      : void();

  UE_LOG(LogTemp, Display, TEXT("ForbocAI CLI (UE5) - Command: %s"),
         *Invocation.Command);

  bool bCommandFailed = false;
  createCommandPipeline(Invocation.Command, Invocation.Args)
      .then([]() {
        UE_LOG(LogTemp, Display, TEXT(""));
        UE_LOG(LogTemp, Display,
               TEXT("[RESULT] Command completed successfully"));
      })
      .catch_([&bCommandFailed](std::string Message) {
        bCommandFailed = true;
        UE_LOG(LogTemp, Error, TEXT(""));
        UE_LOG(LogTemp, Error, TEXT("[RESULT] Command failed: %s"),
               UTF8_TO_TCHAR(Message.c_str()));
      })
      .execute();
  return bCommandFailed ? 1 : 0;
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
  return UForbocAICommandlet::CommandExecution::create(
      [this, Command, Args](std::function<void()> Resolve,
                            std::function<void(std::string)> Reject) {
        const auto Validation =
            func::runValidation(commandValidationPipeline(), Command);
        func::ematch(
            Validation,
            [&Reject](const FString &Error) {
              Reject(TCHAR_TO_UTF8(*Error));
            },
            [this, &Command, &Args, &Resolve,
             &Reject](const FString &) {
              const CommandResult Result = executeCommand(Command, Args);
              const FString Message =
                  Result.message.empty()
                      ? FString()
                      : FString(UTF8_TO_TCHAR(Result.message.c_str()));
              Result.isSuccessful()
                  ? (Resolve(), void())
                  : (Reject(TCHAR_TO_UTF8(
                         *(Message.IsEmpty()
                               ? FString::Printf(TEXT("Command failed: %s"),
                                                 *Command)
                               : Message))),
                     void());
            });
      });
}

/** User Story: As a cli consumer, I need to invoke command validation pipeline through a stable signature so the cli workflow remains explicit and composable. @fn CLITypes::ValidationPipeline<FString, FString> UForbocAICommandlet::commandValidationPipeline() */
CLITypes::ValidationPipeline<FString, FString>
UForbocAICommandlet::commandValidationPipeline() {
  return func::validationPipeline<FString, FString>() |
         [](const FString &Command) -> CLITypes::Either<FString, FString> {
           return Command.IsEmpty()
                      ? CLITypes::make_left(
                            FString(TEXT("Command cannot be empty")), FString())
                      : CLITypes::make_right(FString(), Command);
         } |
         [](const FString &Command) -> CLITypes::Either<FString, FString> {
           return !ForbocAI::CLI::isValidCommandKey(
                      store().getState().CLI, Command)
                      ? CLITypes::make_left(
                            FString::Printf(TEXT("Invalid command: %s"),
                                             *Command),
                            FString())
                      : CLITypes::make_right(FString(), Command);
         };
}
