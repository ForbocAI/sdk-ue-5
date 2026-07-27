#include "Systems/CLI/MemoryCommands/MemoryCommandsThunks.h"
#include "Entities/CLI/CLISelectors.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Systems/CLI/Memory/CLIMemoryAdapters.h"
#include "Entities/CLI/Memory/CLIMemorySelectors.h"
#include "Systems/CLI/Memory/CLIMemoryThunks.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"

namespace {

using MemoryResult = CLIOps::CommandRouting::Result;

/** User Story: As a CLI command-routing consumer, I need to invoke memory success through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result MemorySuccess(const FString &Message) */
CLIOps::CommandRouting::Result MemorySuccess(const FString &Message) {
  return CLIOps::CommandRouting::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI command-routing consumer, I need to invoke memory failure through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result MemoryFailure(const FString &Message) */
CLIOps::CommandRouting::Result MemoryFailure(const FString &Message) {
  return CLIOps::CommandRouting::Result::Failure(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI command-routing memory consumer, I need to invoke list memory through a stable signature so the CLI command-routing memory workflow remains explicit and composable. @fn MemoryResult ListMemory( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First) */
MemoryResult ListMemory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<MemoryResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
            State.Messages.Listing, Args[First], State.Limits.ListItemLimit));
        const TArray<FMemoryItem> Items =
            ForbocAI::CLI::Memory::selectMemoryItems(
                Ops::listMemory(Store, Args[First]),
                State.Limits.ListItemLimit);
        const FString Output =
            ForbocAI::CLI::Memory::selectMemoryListOutput(Items, State);
        ForbocAI::CLI::Presentation::logCliMessage(Output);
        return MemorySuccess(Output);
      },
      [&]() { return MemoryFailure(State.Messages.ListUsage); });
}

/** User Story: As a CLI command-routing memory consumer, I need to invoke recall memory through a stable signature so the CLI command-routing memory workflow remains explicit and composable. @fn MemoryResult RecallMemory( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First, int32 Second) */
MemoryResult RecallMemory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First,
    int32 Second) {
  return CLIOps::CommandRouting::matchCondition<MemoryResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        const TArray<FMemoryItem> Items =
            ForbocAI::CLI::Memory::selectMemoryItems(
                Ops::recallMemory(Store, Args[First], Args[Second],
                                  State.Limits.RecallItemLimit,
                                  State.Defaults.RecallSimilarity),
                State.Limits.RecallItemLimit);
        const FString Output =
            ForbocAI::CLI::Memory::selectMemoryRecallOutput(Items, State);
        ForbocAI::CLI::Presentation::logCliMessage(Output);
        return MemorySuccess(Output);
      },
      [&]() { return MemoryFailure(State.Messages.RecallUsage); });
}

/** User Story: As a CLI command-routing memory consumer, I need to invoke store memory through a stable signature so the CLI command-routing memory workflow remains explicit and composable. @fn MemoryResult StoreMemory( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First, int32 Second) */
MemoryResult StoreMemory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First,
    int32 Second) {
  return CLIOps::CommandRouting::matchCondition<MemoryResult>(
      Args.Num() >= State.Limits.DoubleArgumentCount,
      [&]() {
        Ops::storeMemory(Store, Args[First], Args[Second],
                         State.Defaults.Importance);
        ForbocAI::CLI::Presentation::logCliMessage(State.Messages.Stored);
        return MemorySuccess(State.Messages.StoreDone);
      },
      [&]() { return MemoryFailure(State.Messages.StoreUsage); });
}

/** User Story: As a CLI command-routing memory consumer, I need to invoke clear memory through a stable signature so the CLI command-routing memory workflow remains explicit and composable. @fn MemoryResult ClearMemory( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First) */
MemoryResult ClearMemory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First) {
  return CLIOps::CommandRouting::matchCondition<MemoryResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        ForbocAI::CLI::Presentation::logCliMessage(
            State.Messages.ClearWarning);
        Ops::clearMemory(Store, Args[First]);
        ForbocAI::CLI::Presentation::logCliMessage(State.Messages.Cleared);
        return MemorySuccess(State.Messages.ClearDone);
      },
      [&]() { return MemoryFailure(State.Messages.ClearUsage); });
}

/** User Story: As a CLI command-routing memory consumer, I need to invoke export memory through a stable signature so the CLI command-routing memory workflow remains explicit and composable. @fn MemoryResult ExportMemory( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First) */
MemoryResult ExportMemory(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<MemoryResult>(
      Args.Num() >= State.Limits.SingleArgumentCount,
      [&]() {
        ForbocAI::CLI::Presentation::logCliMessage(
            formatCliMessage(State.Messages.Exporting, Args[First]));
        const FString Output = ForbocAI::CLI::Memory::serializeMemories(
            Ops::listMemory(Store, Args[First]), State);
        ForbocAI::CLI::Presentation::logCliMessage(Output);
        return MemorySuccess(Output);
      },
      [&]() { return MemoryFailure(State.Messages.ExportUsage); });
}

/** User Story: As a CLI command-routing memory consumer, I need to invoke create memory dispatcher through a stable signature so the CLI command-routing memory workflow remains explicit and composable. @fn func::Dispatcher<FString, MemoryResult> CreateMemoryDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::Memory::FCLIMemoryState &State, int32 First, int32 Second) */
func::Dispatcher<FString, MemoryResult>
CreateMemoryDispatcher(rtk::EnhancedStore<FRuntimeState> &Store,
                       const ForbocAI::CLI::FCLICommandRoles &Roles,
                       const TArray<FString> &Args,
                       const ForbocAI::CLI::Memory::FCLIMemoryState &State,
                       int32 First, int32 Second) {
  return func::createDispatcher<FString, MemoryResult>({
      {Roles.MemoryList,
       [&]() { return ListMemory(Store, Args, State, First); }},
      {Roles.MemoryRecall,
       [&]() { return RecallMemory(Store, Args, State, First, Second); }},
      {Roles.MemoryStore,
       [&]() { return StoreMemory(Store, Args, State, First, Second); }},
      {Roles.MemoryClear,
       [&]() { return ClearMemory(Store, Args, State, First); }},
      {Roles.MemoryExport,
       [&]() { return ExportMemory(Store, Args, State, First); }},
  });
}

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI command-routing consumer, I need to invoke handle memory through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn RouteResult RouteMemoryCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteMemoryCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                           const FString &CommandKey,
                           const TArray<FString> &Args) {
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Memory::FCLIMemoryState &State =
      ForbocAI::CLI::Memory::selectCliMemory(CLIState);
  const int32 First = CLIState.Parsing.FirstTokenIndex;
  const int32 Second = CLIState.Parsing.SecondTokenIndex;
  return func::dispatch(
      CreateMemoryDispatcher(Store, Roles, Args, State, First, Second),
      CommandKey);
}

} // namespace CommandRouting
} // namespace CLIOps
