#include "CLI/CliHandlers.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/NPC/CLINPCAdapters.h"
#include "Features/CLI/NPC/CLINPCSelectors.h"
#include "Features/CLI/NPC/NPCThunks.h"
#include "Features/CLI/Presentation/PresentationAdapters.h"
#include "Store.h"

namespace {

/** User Story: As a cli cli handlers consumer, I need to invoke npc success through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result NpcSuccess(const FString &Message) */
CLIOps::Handlers::Result NpcSuccess(const FString &Message) {
  return CLIOps::Handlers::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a cli cli handlers consumer, I need to invoke npc failure through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result NpcFailure(const FString &Message) */
CLIOps::Handlers::Result NpcFailure(const FString &Message) {
  return CLIOps::Handlers::Result::Failure(TCHAR_TO_UTF8(*Message));
}

} // namespace

namespace CLIOps {
namespace Handlers {

/** User Story: As a cli cli handlers consumer, I need to invoke handle npc through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn HandlerResult HandleNpc(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
HandlerResult HandleNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                        const FString &CommandKey,
                        const TArray<FString> &Args) {
  using func::just;
  using func::nothing;
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::NPC::FCLINPCState &State =
      ForbocAI::CLI::NPC::selectCliNpc(CLIState);
  const int32 First = State.Limits.FirstArgumentIndex;
  const int32 Second = CLIState.Parsing.SecondTokenIndex;

  return CommandKey == Roles.NpcCreate
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(NpcFailure(State.Messages.CreateUsage))
                    : [&]() -> HandlerResult {
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.Creating,
                                             Args[First]));
                        const FNPCInternalState Npc =
                            Ops::createNpc(Store, Args[First]);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.Created, Npc.Id));
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.State,
                                             Npc.State.JsonData));
                        return just(NpcSuccess(Npc.Id));
                      }())
         : CommandKey == Roles.NpcProcess
             ? (Args.Num() < State.Limits.DoubleArgumentCount
                    ? just(NpcFailure(State.Messages.ProcessUsage))
                    : [&]() -> HandlerResult {
                        const FString Observation =
                            ForbocAI::CLI::NPC::joinNpcArguments(
                                Args, Second, State.Syntax.MessageSeparator);
                        const FAgentResponse Response =
                            Ops::processNpc(Store, Args[First], Observation);
                        const FString Dialogue =
                            ForbocAI::CLI::NPC::selectDialogue(Response,
                                                               State);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(
                                State.Messages.Dialogue, Dialogue));
                        ForbocAI::CLI::Presentation::logCliMessageWhen(
                            !Response.Action.Type.IsEmpty(),
                            formatCliMessage(State.Messages.Action,
                                             Response.Action.Type));
                        return just(NpcSuccess(Dialogue));
                      }())
         : CommandKey == Roles.NpcState
             ? [&]() -> HandlerResult {
                 const func::Maybe<FNPCInternalState> Target =
                     Args.Num() >= State.Limits.SingleArgumentCount
                         ? Ops::getNpc(Store, Args[First])
                         : Ops::getActiveNpc(Store);
                 return func::match(
                     Target,
                     [&State](const FNPCInternalState &Npc) -> HandlerResult {
                       ForbocAI::CLI::Presentation::logCliMessage(
                           formatCliMessage(State.Messages.Id, Npc.Id));
                       ForbocAI::CLI::Presentation::logCliMessage(
                           formatCliMessage(State.Messages.Persona,
                                            Npc.Persona));
                       ForbocAI::CLI::Presentation::logCliMessage(
                           formatCliMessage(State.Messages.State,
                                            Npc.State.JsonData));
                       return just(
                           NpcSuccess(State.Messages.StatePrinted));
                     },
                     [&Args, &State, First]() -> HandlerResult {
                       return just(NpcFailure(
                           Args.Num() >= State.Limits.SingleArgumentCount
                               ? formatCliMessage(State.Messages.NotFound,
                                                  Args[First])
                               : State.Messages.NoActive));
                     });
               }()
         : CommandKey == Roles.NpcUpdate
             ? [&]() -> HandlerResult {
                 const func::Maybe<FNPCInternalState> Active =
                     Ops::getActiveNpc(Store);
                 const func::Maybe<FString> ActiveId =
                     Active.hasValue ? func::just(Active.value.Id)
                                     : func::nothing<FString>();
                 const func::Maybe<ForbocAI::CLI::NPC::FCLINPCUpdate> Update =
                     ForbocAI::CLI::NPC::decodeNpcUpdate(Args, ActiveId,
                                                         State);
                 return func::match(
                     Update,
                     [&Store, &State](
                         const ForbocAI::CLI::NPC::FCLINPCUpdate &Decoded)
                         -> HandlerResult {
                       return !Ops::getNpc(Store, Decoded.NpcId).hasValue
                                  ? just(NpcFailure(formatCliMessage(
                                        State.Messages.NotFound,
                                        Decoded.NpcId)))
                                  : (Ops::updateNpc(Store, Decoded.NpcId,
                                                    Decoded.Delta),
                                     ForbocAI::CLI::Presentation::
                                         logCliMessage(formatCliMessage(
                                             State.Messages.Updated,
                                             Decoded.NpcId,
                                             Decoded.Delta.JsonData)),
                                     just(NpcSuccess(
                                         State.Messages.UpdateDone)));
                     },
                     [&State]() -> HandlerResult {
                       return just(
                           NpcFailure(State.Messages.UpdateUsage));
                     });
               }()
         : CommandKey == Roles.NpcImport
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(NpcFailure(State.Messages.ImportUsage))
                    : [&]() -> HandlerResult {
                        const FImportedNpc Npc =
                            Ops::importNpcFromSoul(Store, Args[First]);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.Imported,
                                             Npc.NpcId));
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.Persona,
                                             Npc.Persona));
                        return just(NpcSuccess(State.Messages.ImportDone));
                      }())
         : CommandKey == Roles.NpcChat
             ? (Args.Num() < State.Limits.DoubleArgumentCount
                    ? just(NpcFailure(
                          Args.Num() == State.Limits.EmptyArgumentCount
                              ? State.Messages.ChatMissingId
                              : State.Messages.ChatUsage))
                    : [&]() -> HandlerResult {
                        const FString Message =
                            ForbocAI::CLI::NPC::joinNpcArguments(
                                Args, Second, State.Syntax.MessageSeparator);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.ChatHeader,
                                             Args[First]));
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.ChatUser,
                                             Message));
                        const FAgentResponse Response =
                            Ops::processNpc(Store, Args[First], Message);
                        const FString Dialogue =
                            ForbocAI::CLI::NPC::selectDialogue(Response,
                                                               State);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(
                                State.Messages.ChatNpc, Dialogue));
                        ForbocAI::CLI::Presentation::logCliMessageWhen(
                            !Response.Action.Type.IsEmpty(),
                            formatCliMessage(State.Messages.ChatAction,
                                             Response.Action.Type));
                        return just(NpcSuccess(Dialogue));
                      }())
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
