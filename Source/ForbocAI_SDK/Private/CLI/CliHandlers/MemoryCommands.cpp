#include "CLI/CliHandlers.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/Memory/CLIMemoryAdapters.h"
#include "Features/CLI/Memory/CLIMemorySelectors.h"
#include "Features/CLI/Memory/CLIMemoryThunks.h"
#include "Features/CLI/Presentation/PresentationAdapters.h"
#include "Store.h"

namespace {

/** User Story: As a cli cli handlers consumer, I need to invoke memory success through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result MemorySuccess(const FString &Message) */
CLIOps::Handlers::Result MemorySuccess(const FString &Message) {
  return CLIOps::Handlers::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a cli cli handlers consumer, I need to invoke memory failure through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result MemoryFailure(const FString &Message) */
CLIOps::Handlers::Result MemoryFailure(const FString &Message) {
  return CLIOps::Handlers::Result::Failure(TCHAR_TO_UTF8(*Message));
}

} // namespace

namespace CLIOps {
namespace Handlers {

/** User Story: As a cli cli handlers consumer, I need to invoke handle memory through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn HandlerResult HandleMemory(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
HandlerResult HandleMemory(rtk::EnhancedStore<FRuntimeState> &Store,
                           const FString &CommandKey,
                           const TArray<FString> &Args) {
  using func::just;
  using func::nothing;
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Memory::FCLIMemoryState &State =
      ForbocAI::CLI::Memory::selectCliMemory(CLIState);
  const int32 First = CLIState.Parsing.FirstTokenIndex;
  const int32 Second = CLIState.Parsing.SecondTokenIndex;

  return CommandKey == Roles.MemoryList
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(MemoryFailure(State.Messages.ListUsage))
                    : [&]() -> HandlerResult {
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.Listing,
                                             Args[First],
                                             State.Limits.ListItemLimit));
                        const TArray<FMemoryItem> Items =
                            ForbocAI::CLI::Memory::selectMemoryItems(
                                Ops::listMemory(Store, Args[First]),
                                State.Limits.ListItemLimit);
                        const FString Output =
                            ForbocAI::CLI::Memory::selectMemoryListOutput(
                                Items, State);
                        ForbocAI::CLI::Presentation::logCliMessage(Output);
                        return just(MemorySuccess(Output));
                      }())
         : CommandKey == Roles.MemoryRecall
             ? (Args.Num() < State.Limits.DoubleArgumentCount
                    ? just(MemoryFailure(State.Messages.RecallUsage))
                    : [&]() -> HandlerResult {
                        const TArray<FMemoryItem> Items =
                            ForbocAI::CLI::Memory::selectMemoryItems(
                                Ops::recallMemory(
                                    Store, Args[First], Args[Second],
                                    State.Limits.RecallItemLimit,
                                    State.Defaults.RecallSimilarity),
                                State.Limits.RecallItemLimit);
                        const FString Output =
                            ForbocAI::CLI::Memory::selectMemoryRecallOutput(
                                Items, State);
                        ForbocAI::CLI::Presentation::logCliMessage(Output);
                        return just(MemorySuccess(Output));
                      }())
         : CommandKey == Roles.MemoryStore
             ? (Args.Num() < State.Limits.DoubleArgumentCount
                    ? just(MemoryFailure(State.Messages.StoreUsage))
                    : (Ops::storeMemory(Store, Args[First], Args[Second],
                                        State.Defaults.Importance),
                       ForbocAI::CLI::Presentation::logCliMessage(
                           State.Messages.Stored),
                       just(MemorySuccess(State.Messages.StoreDone))))
         : CommandKey == Roles.MemoryClear
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(MemoryFailure(State.Messages.ClearUsage))
                    : (ForbocAI::CLI::Presentation::logCliMessage(
                           State.Messages.ClearWarning),
                       Ops::clearMemory(Store, Args[First]),
                       ForbocAI::CLI::Presentation::logCliMessage(
                           State.Messages.Cleared),
                       just(MemorySuccess(State.Messages.ClearDone))))
         : CommandKey == Roles.MemoryExport
             ? (Args.Num() < State.Limits.SingleArgumentCount
                    ? just(MemoryFailure(State.Messages.ExportUsage))
                    : [&]() -> HandlerResult {
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(State.Messages.Exporting,
                                             Args[First]));
                        const TArray<FMemoryItem> Items =
                            Ops::listMemory(Store, Args[First]);
                        const FString Output =
                            ForbocAI::CLI::Memory::serializeMemories(Items,
                                                                    State);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            Output);
                        return just(MemorySuccess(Output));
                      }())
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
