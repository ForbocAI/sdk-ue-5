// User Story: As a developer, I need this module to function.
#include "CLI/CliHandlers.h"
#include "Features/CLI/NPC/NPCThunks.h"
#include "Store.h"

namespace CLIOps {
namespace Handlers {

HandlerResult HandleNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                       const FString &CommandKey,
                       const TArray<FString> &Args) {
  using func::just;
  using func::nothing;

  return CommandKey == TEXT("npc_create")
             ? [&]() -> HandlerResult {
                 // Vessel contract: pass empty when no persona is supplied so
                 // the API renders all slots as <unset>.
                 FString Persona =
                     Args.Num() > 0 ? Args[0] : TEXT("");
                 FNPCInternalState Npc = Ops::createNpc(Store, Persona);
                 UE_LOG(LogTemp, Display, TEXT("Created NPC: %s"),
                        *Npc.Id);
                 return just(Result::Success(TCHAR_TO_UTF8(*Npc.Id)));
               }()
         : CommandKey == TEXT("npc_process")
             ? (Args.Num() < 2
                    ? just(Result::Failure(
                          "Usage: npc_process <npcId> <text>"))
                    : [&]() -> HandlerResult {
                        FAgentResponse Resp =
                            Ops::processNpc(Store, Args[0], Args[1]);
                        UE_LOG(LogTemp, Display, TEXT("Verdict: %s"),
                               *Resp.Dialogue);
                        return just(
                            Result::Success("Protocol complete"));
                      }())
         : CommandKey == TEXT("npc_state")
             ? [&]() -> HandlerResult {
                 const func::Maybe<FNPCInternalState> Target =
                     Args.Num() > 0 ? Ops::getNpc(Store, Args[0])
                                    : Ops::getActiveNpc(Store);
                 return !Target.hasValue
                     ? [&]() -> HandlerResult {
                         UE_LOG(LogTemp, Display,
                                TEXT("No active NPC"));
                         return just(
                             Result::Success("No active NPC"));
                       }()
                     : [&]() -> HandlerResult {
                         UE_LOG(LogTemp, Display,
                                TEXT("NPC ID:      %s"),
                                *Target.value.Id);
                         UE_LOG(LogTemp, Display,
                                TEXT("Persona:     %s"),
                                *Target.value.Persona);
                         return just(
                             Result::Success("NPC state printed"));
                       }();
               }()
         : CommandKey == TEXT("npc_update")
             ? (Args.Num() < 2
                    ? just(Result::Failure(
                          "Usage: npc_update <npcId> "
                          "[-Mood=<value>] [-Inventory=<item,item>]"))
                    : [&]() -> HandlerResult {
                        FAgentState Delta;
                        Delta.JsonData =
                            Args.Num() > 2
                                ? FString::Printf(
                                      TEXT("{\"Mood\":\"%s\","
                                           "\"Inventory\":\"%s\"}"),
                                      *Args[1], *Args[2])
                                : FString::Printf(
                                      TEXT("{\"Mood\":\"%s\"}"),
                                      *Args[1]);
                        Ops::updateNpc(Store, Args[0], Delta);
                        UE_LOG(LogTemp, Display,
                               TEXT("NPC %s updated"), *Args[0]);
                        return just(
                            Result::Success("NPC updated"));
                      }())
         : CommandKey == TEXT("npc_import")
             ? (Args.Num() < 1
                    ? just(Result::Failure(
                          "Usage: npc_import <txId>"))
                    : [&]() -> HandlerResult {
                        FImportedNpc Npc =
                            Ops::importNpcFromSoul(Store, Args[0]);
                        UE_LOG(LogTemp, Display,
                               TEXT("NPC imported from soul: %s"),
                               *Npc.NpcId);
                        return just(Result::Success(
                            "NPC imported from soul"));
                      }())
         : CommandKey == TEXT("npc_chat")
             ? (Args.Num() < 2
                    ? just(Result::Failure(
                          "Usage: npc_chat <npcId> <message>"))
                    : [&]() -> HandlerResult {
                        UE_LOG(LogTemp, Display, TEXT("> You: %s"),
                               *Args[1]);
                        FAgentResponse Resp =
                            Ops::processNpc(Store, Args[0], Args[1]);
                        UE_LOG(LogTemp, Display, TEXT("> NPC: %s"),
                               *Resp.Dialogue);
                        !Resp.Action.Type.IsEmpty()
                            ? [&]() {
                                UE_LOG(LogTemp, Display,
                                       TEXT("> Action: %s"),
                                       *Resp.Action.Type);
                              }()
                            : (void)0;
                        return just(
                            Result::Success("Chat turn complete"));
                      }())
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
