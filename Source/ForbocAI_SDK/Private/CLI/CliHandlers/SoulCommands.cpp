// User Story: As a developer, I need this module to function.
#include "CLI/CliHandlers.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/NPC/NPCThunks.h"
#include "Features/CLI/Presentation/PresentationAdapters.h"
#include "Features/CLI/Soul/CLISoulAdapters.h"
#include "Features/CLI/Soul/CLISoulSelectors.h"
#include "Features/CLI/Soul/CLISoulThunks.h"
#include "Store.h"

namespace {

/** User Story: As a CLI Soul consumer, I need successful semantic output converted through a stable signature so the command runner can inspect it. @fn CLIOps::Handlers::Result SoulSuccess(const FString &Message) */
CLIOps::Handlers::Result SoulSuccess(const FString &Message) {
  return CLIOps::Handlers::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI Soul consumer, I need failed semantic output converted through a stable signature so invalid operations produce a nonzero command result. @fn CLIOps::Handlers::Result SoulFailure(const FString &Message) */
CLIOps::Handlers::Result SoulFailure(const FString &Message) {
  return CLIOps::Handlers::Result::Failure(TCHAR_TO_UTF8(*Message));
}

} // namespace

namespace CLIOps {
namespace Handlers {

/** User Story: As a CLI Soul consumer, I need every Soul command dispatched through RTK thunks and root-store selectors via a stable signature so presentation remains thin and semantic output remains observable. @fn HandlerResult HandleSoul(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
HandlerResult HandleSoul(rtk::EnhancedStore<FRuntimeState> &Store,
                         const FString &CommandKey,
                         const TArray<FString> &Args) {
  using func::just;
  using func::nothing;
  using ForbocAI::CLI::Presentation::formatCliMessage;
  using ForbocAI::CLI::Presentation::logCliMessage;
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Soul::FCLISoulState &State =
      ForbocAI::CLI::Soul::selectCliSoul(CLIState);
  const int32 First = State.Limits.FirstArgumentIndex;
  const int32 Second = State.Limits.SecondArgumentIndex;

  return CommandKey == Roles.SoulExport
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(SoulFailure(State.Messages.ExportUsage))
                    : [&]() -> HandlerResult {
                        const FSoulExportResult Exported =
                            Ops::exportSoul(Store, Args[First]);
                        logCliMessage(formatCliMessage(
                            State.Messages.Exported, Exported.TxId));
                        return just(SoulSuccess(Exported.TxId));
                      }())
         : CommandKey == Roles.SoulImport
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(SoulFailure(State.Messages.ImportUsage))
                    : [&]() -> HandlerResult {
                        const FSoul Imported =
                            Ops::importSoul(Store, Args[First]);
                        logCliMessage(formatCliMessage(
                            State.Messages.Imported, Imported.Id));
                        return just(SoulSuccess(Imported.Id));
                      }())
         : CommandKey == Roles.SoulList
             ? [&]() -> HandlerResult {
                 const TArray<FSoulListItem> Souls = Ops::listSouls(
                     Store,
                     ForbocAI::CLI::Soul::decodeSoulListLimit(Args, State));
                 const FString Output =
                     ForbocAI::CLI::Soul::selectSoulListOutput(Souls, State);
                 logCliMessage(Output);
                 return just(SoulSuccess(Output));
               }()
         : CommandKey == Roles.SoulChat
             ? (Args.Num() < State.Limits.DoubleArgumentCount
                    ? just(SoulFailure(State.Messages.ChatUsage))
                    : [&]() -> HandlerResult {
                        logCliMessage(formatCliMessage(
                            State.Messages.ChatUser, Args[Second]));
                        const FAgentResponse Response =
                            Ops::processNpc(Store, Args[First], Args[Second]);
                        logCliMessage(formatCliMessage(
                            State.Messages.ChatNpc, Response.Dialogue));
                        return just(SoulSuccess(Response.Dialogue));
                      }())
         : CommandKey == Roles.SoulVerify
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(SoulFailure(State.Messages.VerifyUsage))
                    : [&]() -> HandlerResult {
                        const FSoulVerifyResult Verified =
                            Ops::verifySoul(Store, Args[First]);
                        const FString Output =
                            Verified.bValid
                                ? State.Messages.VerificationValid
                                : formatCliMessage(
                                      State.Messages.VerificationInvalid,
                                      Verified.Reason);
                        logCliMessage(Output);
                        return just(Verified.bValid ? SoulSuccess(Output)
                                                    : SoulFailure(Output));
                      }())
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
