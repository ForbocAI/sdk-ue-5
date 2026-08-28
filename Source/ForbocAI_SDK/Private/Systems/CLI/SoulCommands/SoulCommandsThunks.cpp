// User Story: As a developer, I need this module to function.
#include "Systems/CLI/SoulCommands/SoulCommandsThunks.h"
#include "Entities/CLI/CLISelectors.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Systems/CLI/NPC/NPCThunks.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"
#include "Systems/Protocol/Process/ProcessAdapters.h"
#include "Systems/CLI/Soul/CLISoulAdapters.h"
#include "Entities/CLI/Soul/CLISoulSelectors.h"
#include "Systems/CLI/Soul/CLISoulThunks.h"

namespace {

using SoulResult = CLIOps::CommandRouting::Result;

/** User Story: As a CLI Soul consumer, I need successful semantic output converted through a stable signature so the command runner can inspect it. @fn CLIOps::CommandRouting::Result SoulSuccess(const FString &Message) */
CLIOps::CommandRouting::Result SoulSuccess(const FString &Message) {
  return CLIOps::CommandRouting::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI Soul consumer, I need failed semantic output converted through a stable signature so invalid operations produce a nonzero command result. @fn CLIOps::CommandRouting::Result SoulFailure(const FString &Message) */
CLIOps::CommandRouting::Result SoulFailure(const FString &Message) {
  return CLIOps::CommandRouting::Result::Failure(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI command-routing soul consumer, I need to invoke export soul through a stable signature so the CLI command-routing soul workflow remains explicit and composable. @fn SoulResult ExportSoul( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First) */
SoulResult ExportSoul(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<SoulResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        const FSoulExportResult Exported = Ops::exportSoul(Store, Args[First]);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Exported, Exported.TxId));
        return SoulSuccess(Exported.TxId);
      },
      [&]() { return SoulFailure(State.Messages.ExportUsage); });
}

/** User Story: As a CLI command-routing soul consumer, I need to invoke import soul through a stable signature so the CLI command-routing soul workflow remains explicit and composable. @fn SoulResult ImportSoul( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First) */
SoulResult ImportSoul(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<SoulResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        const FImportedNpc Imported = Ops::importNpcFromSoul(Store, Args[First]);
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Imported, Imported.NpcId));
        return SoulSuccess(Imported.NpcId);
      },
      [&]() { return SoulFailure(State.Messages.ImportUsage); });
}

/** User Story: As a CLI command-routing soul consumer, I need to invoke list souls through a stable signature so the CLI command-routing soul workflow remains explicit and composable. @fn SoulResult ListSouls(rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Soul::FCLISoulState &State) */
SoulResult ListSouls(rtk::EnhancedStore<FRuntimeState> &Store,
                     const TArray<FString> &Args,
                     const ForbocAI::CLI::Soul::FCLISoulState &State) {
  const TArray<FSoulListItem> Souls = Ops::listSouls(
      Store, ForbocAI::CLI::Soul::decodeSoulListLimit(Args, State));
  const FString Output =
      ForbocAI::CLI::Soul::selectSoulListOutput(Souls, State);
  ForbocAI::CLI::Presentation::logCliMessage(Output);
  return SoulSuccess(Output);
}

/** User Story: As a CLI command-routing soul consumer, I need to invoke chat with soul through a stable signature so the CLI command-routing soul workflow remains explicit and composable. @fn SoulResult ChatWithSoul( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First, int32 Second) */
SoulResult ChatWithSoul(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First,
    int32 Second) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<SoulResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.ChatUser, Args[Second]));
        const FAgentResponse Response = Ops::processNpc(
            Store,
            ProtocolProcess::ProcessInput(Args[First], Args[Second]));
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.ChatNpc, Response.Dialogue));
        return SoulSuccess(Response.Dialogue);
      },
      [&]() { return SoulFailure(State.Messages.ChatUsage); });
}

/** User Story: As a CLI command-routing soul consumer, I need to invoke verify soul through a stable signature so the CLI command-routing soul workflow remains explicit and composable. @fn SoulResult VerifySoul( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First) */
SoulResult VerifySoul(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<SoulResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        const FSoulVerifyResult Verified = Ops::verifySoul(Store, Args[First]);
        const FString Output =
            Verified.bValid
                ? State.Messages.VerificationValid
                : formatCliMessage(State.Messages.VerificationInvalid,
                                   Verified.Reason);
        ForbocAI::CLI::Presentation::logCliMessage(Output);
        return Verified.bValid ? SoulSuccess(Output) : SoulFailure(Output);
      },
      [&]() { return SoulFailure(State.Messages.VerifyUsage); });
}

/** User Story: As a CLI command-routing soul consumer, I need to invoke create soul dispatcher through a stable signature so the CLI command-routing soul workflow remains explicit and composable. @fn func::Dispatcher<FString, SoulResult> CreateSoulDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::Soul::FCLISoulState &State, int32 First, int32 Second) */
func::Dispatcher<FString, SoulResult>
CreateSoulDispatcher(rtk::EnhancedStore<FRuntimeState> &Store,
                     const ForbocAI::CLI::FCLICommandRoles &Roles,
                     const TArray<FString> &Args,
                     const ForbocAI::CLI::Soul::FCLISoulState &State,
                     int32 First, int32 Second) {
  return func::createDispatcher<FString, SoulResult>({
      {Roles.SoulExport,
       [&]() { return ExportSoul(Store, Args, State, First); }},
      {Roles.SoulImport,
       [&]() { return ImportSoul(Store, Args, State, First); }},
      {Roles.SoulList, [&]() { return ListSouls(Store, Args, State); }},
      {Roles.SoulChat,
       [&]() { return ChatWithSoul(Store, Args, State, First, Second); }},
      {Roles.SoulVerify,
       [&]() { return VerifySoul(Store, Args, State, First); }},
  });
}

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI Soul consumer, I need every Soul command dispatched through RTK thunks and root-store selectors via a stable signature so presentation remains thin and semantic output remains observable. @fn RouteResult RouteSoulCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteSoulCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                         const FString &CommandKey,
                         const TArray<FString> &Args) {
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Soul::FCLISoulState &State =
      ForbocAI::CLI::Soul::selectCliSoul(CLIState);
  const int32 First = State.Limits.FirstArgumentIndex;
  const int32 Second = State.Limits.SecondArgumentIndex;
  return func::dispatch(
      CreateSoulDispatcher(Store, Roles, Args, State, First, Second),
      CommandKey);
}

} // namespace CommandRouting
} // namespace CLIOps
