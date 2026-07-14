// User Story: As a developer, I need this module to function.
#include "CLI/CliHandlers.h"
#include "Features/CLI/NPC/NPCThunks.h"
#include "Features/CLI/Soul/CLISoulThunks.h"
#include "Store.h"

namespace CLIOps {
namespace Handlers {

HandlerResult HandleSoul(rtk::EnhancedStore<FRuntimeState> &Store,
                        const FString &CommandKey,
                        const TArray<FString> &Args) {
  using func::just;
  using func::nothing;

  return CommandKey == TEXT("soul_export")
             ? (Args.Num() < 1
                    ? just(Result::Failure("Usage: soul_export <npcId>"))
                    : [&]() -> HandlerResult {
                        FSoulExportResult Exported =
                            Ops::exportSoul(Store, Args[0]);
                        UE_LOG(LogTemp, Display,
                               TEXT("Soul exported: %s"), *Exported.TxId);
                        return just(
                            Result::Success("Soul exported"));
                      }())
         : CommandKey == TEXT("soul_import")
             ? (Args.Num() < 1
                    ? just(Result::Failure("Usage: soul_import <txId>"))
                    : [&]() -> HandlerResult {
                        FSoul Imported =
                            Ops::importSoul(Store, Args[0]);
                        UE_LOG(LogTemp, Display,
                               TEXT("Soul imported: %s"), *Imported.Id);
                        return just(
                            Result::Success("Soul imported"));
                      }())
         : CommandKey == TEXT("soul_list")
             ? [&]() -> HandlerResult {
                 int32 Limit =
                     Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 50;
                 TArray<FSoulListItem> Souls =
                     Ops::listSouls(Store, Limit);
                 UE_LOG(LogTemp, Display, TEXT("Found %d souls"),
                        Souls.Num());
                 return just(Result::Success("Souls listed"));
               }()
         : CommandKey == TEXT("soul_chat")
             ? (Args.Num() < 2
                    ? just(Result::Failure(
                          "Usage: soul_chat <npcId> <message>"))
                    : [&]() -> HandlerResult {
                        UE_LOG(LogTemp, Display, TEXT("> You: %s"),
                               *Args[1]);
                        FAgentResponse Resp =
                            Ops::processNpc(Store, Args[0], Args[1]);
                        UE_LOG(LogTemp, Display, TEXT("> NPC: %s"),
                               *Resp.Dialogue);
                        return just(
                            Result::Success("Soul chat turn complete"));
                      }())
         : CommandKey == TEXT("soul_verify")
             ? (Args.Num() < 1
                    ? just(Result::Failure(
                          "Usage: soul_verify <txId>"))
                    : [&]() -> HandlerResult {
                        FSoulVerifyResult Verified =
                            Ops::verifySoul(Store, Args[0]);
                        UE_LOG(LogTemp, Display,
                               TEXT("Soul verification: %s"),
                               Verified.bValid ? TEXT("VALID")
                                               : TEXT("INVALID"));
                        return just(
                            Result::Success("Soul verified"));
                      }())
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
