#pragma once

#include "Core/fp.hpp"
#include "Core/rtk.hpp"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerActions.h"

namespace TestGame {

namespace CommandRunnerSliceDetail {

/** User Story: As a systems harness command runner consumer, I need to invoke merge alias maps through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline TMap<FString, FString> MergeAliasMaps(const TMap<FString, FString> &State, const TMap<FString, FString> &Update) */
inline TMap<FString, FString>
MergeAliasMaps(const TMap<FString, FString> &State,
               const TMap<FString, FString> &Update) {
  return func::fold_array<FString, TMap<FString, FString>>(
      func::map_keys<FString, FString>(Update), State,
      [&Update](const TMap<FString, FString> &Aliases,
                const FString &Alias) {
        return func::upsert_map_value<FString, FString>(
            Aliases, Alias, FString(),
            [&Update, &Alias](const FString &) {
              return func::map_value_or<FString, FString>(
                  Update, Alias, FString());
            });
      });
}

} // namespace CommandRunnerSliceDetail

/** User Story: As a systems harness command runner consumer, I need to invoke create command runner slice through a stable signature so the systems harness command runner workflow remains explicit and composable. @fn inline rtk::Slice<CommandRunner::FCommandAliasState> CreateCommandRunnerSlice() */
inline rtk::Slice<CommandRunner::FCommandAliasState>
CreateCommandRunnerSlice() {
  return rtk::createSlice<CommandRunner::FCommandAliasState>(
      TEXT("testgame/commandRunner"),
      CommandRunner::FCommandAliasState(),
      [](rtk::ActionReducerMapBuilder<CommandRunner::FCommandAliasState>
             &Builder) {
        Builder.addCase(
            CommandRunnerActions::aliasesCapturedActionCreator(),
            [](const CommandRunner::FCommandAliasState &State,
               const rtk::Action<CommandRunner::FCommandAliasUpdate> &Action)
                -> CommandRunner::FCommandAliasState {
              const CommandRunner::FCommandAliasUpdate &Update =
                  Action.PayloadValue;
              CommandRunner::FCommandAliasState Next = State;
              Next.NpcAliases = CommandRunnerSliceDetail::MergeAliasMaps(
                  State.NpcAliases, Update.NpcAliases);
              Next.GhostSessionAliases =
                  CommandRunnerSliceDetail::MergeAliasMaps(
                      State.GhostSessionAliases,
                      Update.GhostSessionAliases);
              Next.SoulTransactionAliases =
                  CommandRunnerSliceDetail::MergeAliasMaps(
                      State.SoulTransactionAliases,
                      Update.SoulTransactionAliases);
              return Next;
            });
        Builder.addCase(
            CommandRunnerActions::aliasesResetActionCreator(),
            [](const CommandRunner::FCommandAliasState &,
               const rtk::Action<rtk::FEmptyPayload> &)
                -> CommandRunner::FCommandAliasState {
              return CommandRunner::FCommandAliasState();
            });
      });
}

} // namespace TestGame
