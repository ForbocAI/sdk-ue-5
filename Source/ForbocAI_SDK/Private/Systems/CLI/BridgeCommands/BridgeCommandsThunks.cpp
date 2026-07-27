#include "Systems/CLI/BridgeCommands/BridgeCommandsThunks.h"
#include "Systems/CLI/Bridge/CLIBridgeAdapters.h"
#include "Systems/CLI/CommandRouting/CommandRoutingAdapters.h"
#include "Entities/CLI/Bridge/CLIBridgeSelectors.h"
#include "Systems/CLI/Bridge/CLIBridgeThunks.h"
#include "Entities/CLI/CLISelectors.h"
#include "Systems/CLI/Presentation/PresentationAdapters.h"
#include "Systems/API/Serialization/APISerializationAdapters.h"

namespace {

using BridgeResult = CLIOps::CommandRouting::Result;

/** User Story: As a CLI command-routing consumer, I need to invoke success through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result Success(const FString &Message) */
CLIOps::CommandRouting::Result Success(const FString &Message) {
  return CLIOps::CommandRouting::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI command-routing consumer, I need to invoke failure through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn CLIOps::CommandRouting::Result Failure(const FString &Message) */
CLIOps::CommandRouting::Result Failure(const FString &Message) {
  return CLIOps::CommandRouting::Result::Failure(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke present validation through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn BridgeResult PresentValidation( const FValidationResult &Result, const ForbocAI::CLI::Bridge::FCLIBridgeState &State) */
BridgeResult PresentValidation(
    const FValidationResult &Result,
    const ForbocAI::CLI::Bridge::FCLIBridgeState &State) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const FString Message = formatCliMessage(
      State.Messages.Validation,
      Result.bValid ? State.Messages.Pass : State.Messages.Fail);
  ForbocAI::CLI::Presentation::logCliMessage(Message);
  ForbocAI::CLI::Presentation::logCliMessageWhen(
      !Result.Reason.IsEmpty(),
      formatCliMessage(State.Messages.Reason, Result.Reason));
  ForbocAI::CLI::Presentation::logCliMessageWhen(
      !Result.CorrectedAction.Type.IsEmpty(),
      formatCliMessage(
          State.Messages.CorrectedAction,
          JsonInterop::StringifyObject(
              JsonInterop::ActionToObject(Result.CorrectedAction))));
  return Success(Message);
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke validate bridge through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn BridgeResult ValidateBridge( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Bridge::FCLIBridgeState &State, int32 First) */
BridgeResult ValidateBridge(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Bridge::FCLIBridgeState &State, int32 First) {
  return CLIOps::CommandRouting::matchCondition<BridgeResult>(
      Args.Num() >= State.Limits.RequiredArgumentCount,
      [&]() {
        const func::Maybe<ForbocAI::CLI::Bridge::FDecodedBridgePayload>
            Payload =
                ForbocAI::CLI::Bridge::decodeBridgePayload(Args[First], State);
        return func::match(
            Payload,
            [&Store, &State](
                const ForbocAI::CLI::Bridge::FDecodedBridgePayload &Decoded) {
              return PresentValidation(
                  Ops::validateBridgePayload(Store, Decoded.Action,
                                             Decoded.Context, Decoded.NpcId),
                  State);
            },
            [&State]() { return Failure(State.Messages.ValidateUsage); });
      },
      [&State]() { return Failure(State.Messages.ValidateUsage); });
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke log bridge rule through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn void LogBridgeRule(const FBridgeRule &Rule, const ForbocAI::CLI::Bridge::FCLIBridgeState &State) */
void LogBridgeRule(const FBridgeRule &Rule,
                   const ForbocAI::CLI::Bridge::FCLIBridgeState &State) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
      State.Messages.RuleName,
      ForbocAI::CLI::Bridge::selectBridgeRuleName(Rule, State)));
  ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
      State.Messages.Description,
      ForbocAI::CLI::Bridge::selectBridgeRuleDescription(Rule, State)));
  ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
      State.Messages.Actions,
      ForbocAI::CLI::Bridge::selectBridgeRuleActions(Rule, State)));
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke list bridge rules through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn BridgeResult ListBridgeRules( rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::Bridge::FCLIBridgeState &State) */
BridgeResult ListBridgeRules(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::Bridge::FCLIBridgeState &State) {
  ForbocAI::CLI::Presentation::logCliMessage(State.Messages.FetchingRules);
  const TArray<FBridgeRule> Rules = Ops::getBridgeRules(Store);
  Rules.Num() == State.Limits.EmptyItemCount
      ? ForbocAI::CLI::Presentation::logCliMessage(State.Messages.NoRules)
      : func::for_each_array<FBridgeRule>(
            Rules, [&State](const FBridgeRule &Rule) {
              LogBridgeRule(Rule, State);
            });
  return Success(State.Messages.RulesListed);
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke load bridge preset through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn BridgeResult LoadBridgePreset( rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args, const ForbocAI::CLI::Bridge::FCLIBridgeState &State, int32 First) */
BridgeResult LoadBridgePreset(
    rtk::EnhancedStore<FRuntimeState> &Store, const TArray<FString> &Args,
    const ForbocAI::CLI::Bridge::FCLIBridgeState &State, int32 First) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  return CLIOps::CommandRouting::matchCondition<BridgeResult>(
      Args.Num() >= State.Limits.RequiredArgumentCount,
      [&]() {
        const FDirectiveRuleSet Preset =
            Ops::loadBridgePreset(Store, Args[First]);
        ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
            State.Messages.PresetLoaded,
            ForbocAI::CLI::Bridge::selectRulesetId(Preset, State),
            Preset.RulesetRules.Num()));
        return Success(State.Messages.PresetDone);
      },
      [&]() { return Failure(State.Messages.PresetUsage); });
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke list rulesets through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn BridgeResult ListRulesets( rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::Bridge::FCLIBridgeState &State) */
BridgeResult ListRulesets(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::Bridge::FCLIBridgeState &State) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const TArray<FDirectiveRuleSet> Rulesets = Ops::listRulesets(Store);
  Rulesets.Num() == State.Limits.EmptyItemCount
      ? ForbocAI::CLI::Presentation::logCliMessage(State.Messages.NoRulesets)
      : func::for_each_array<FDirectiveRuleSet>(
            Rulesets, [&State](const FDirectiveRuleSet &Ruleset) {
              ForbocAI::CLI::Presentation::logCliMessage(formatCliMessage(
                  State.Messages.RulesetItem,
                  ForbocAI::CLI::Bridge::selectRulesetId(Ruleset, State),
                  Ruleset.RulesetRules.Num()));
            });
  return Success(State.Messages.RulesetsListed);
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke list rule presets through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn BridgeResult ListRulePresets( rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::Bridge::FCLIBridgeState &State) */
BridgeResult ListRulePresets(
    rtk::EnhancedStore<FRuntimeState> &Store,
    const ForbocAI::CLI::Bridge::FCLIBridgeState &State) {
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const TArray<FString> Presets = Ops::listRulePresets(Store);
  Presets.Num() == State.Limits.EmptyItemCount
      ? ForbocAI::CLI::Presentation::logCliMessage(State.Messages.NoPresets)
      : func::for_each_array<FString>(Presets, [&State](const FString &Preset) {
          ForbocAI::CLI::Presentation::logCliMessage(
              formatCliMessage(State.Messages.PresetItem, Preset));
        });
  return Success(State.Messages.PresetsListed);
}

/** User Story: As a CLI command-routing bridge consumer, I need to invoke create bridge dispatcher through a stable signature so the CLI command-routing bridge workflow remains explicit and composable. @fn func::Dispatcher<FString, BridgeResult> CreateBridgeDispatcher(rtk::EnhancedStore<FRuntimeState> &Store, const ForbocAI::CLI::FCLICommandRoles &Roles, const TArray<FString> &Args, const ForbocAI::CLI::Bridge::FCLIBridgeState &State, int32 First) */
func::Dispatcher<FString, BridgeResult>
CreateBridgeDispatcher(rtk::EnhancedStore<FRuntimeState> &Store,
                       const ForbocAI::CLI::FCLICommandRoles &Roles,
                       const TArray<FString> &Args,
                       const ForbocAI::CLI::Bridge::FCLIBridgeState &State,
                       int32 First) {
  return func::createDispatcher<FString, BridgeResult>({
      {Roles.BridgeValidate,
       [&]() { return ValidateBridge(Store, Args, State, First); }},
      {Roles.BridgeRules, [&]() { return ListBridgeRules(Store, State); }},
      {Roles.BridgePreset,
       [&]() { return LoadBridgePreset(Store, Args, State, First); }},
      {Roles.RulesList, [&]() { return ListRulesets(Store, State); }},
      {Roles.RulesPresets, [&]() { return ListRulePresets(Store, State); }},
  });
}

} // namespace

namespace CLIOps {
namespace CommandRouting {

/** User Story: As a CLI command-routing consumer, I need to invoke handle bridge through a stable signature so the CLI command-routing workflow remains explicit and composable. @fn RouteResult RouteBridgeCommand(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
RouteResult RouteBridgeCommand(rtk::EnhancedStore<FRuntimeState> &Store,
                           const FString &CommandKey,
                           const TArray<FString> &Args) {
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Bridge::FCLIBridgeState &State =
      ForbocAI::CLI::Bridge::selectCliBridge(CLIState);
  const int32 First = CLIState.Parsing.FirstTokenIndex;
  return func::dispatch(
      CreateBridgeDispatcher(Store, Roles, Args, State, First), CommandKey);
}

} // namespace CommandRouting
} // namespace CLIOps
