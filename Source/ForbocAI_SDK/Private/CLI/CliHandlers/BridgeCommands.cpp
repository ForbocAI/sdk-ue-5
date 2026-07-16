#include "CLI/CliHandlers.h"
#include "Features/CLI/Bridge/CLIBridgeAdapters.h"
#include "Features/CLI/Bridge/CLIBridgeSelectors.h"
#include "Features/CLI/Bridge/CLIBridgeThunks.h"
#include "Features/CLI/CLISelectors.h"
#include "Features/CLI/Presentation/PresentationAdapters.h"
#include "Features/API/Serialization/APISerializationAdapters.h"
#include "Store.h"

namespace {

/** User Story: As a cli cli handlers consumer, I need to invoke success through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result Success(const FString &Message) */
CLIOps::Handlers::Result Success(const FString &Message) {
  return CLIOps::Handlers::Result::Success(TCHAR_TO_UTF8(*Message));
}

/** User Story: As a cli cli handlers consumer, I need to invoke failure through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn CLIOps::Handlers::Result Failure(const FString &Message) */
CLIOps::Handlers::Result Failure(const FString &Message) {
  return CLIOps::Handlers::Result::Failure(TCHAR_TO_UTF8(*Message));
}

} // namespace

namespace CLIOps {
namespace Handlers {

/** User Story: As a cli cli handlers consumer, I need to invoke handle bridge through a stable signature so the cli cli handlers workflow remains explicit and composable. @fn HandlerResult HandleBridge(rtk::EnhancedStore<FRuntimeState> &Store, const FString &CommandKey, const TArray<FString> &Args) */
HandlerResult HandleBridge(rtk::EnhancedStore<FRuntimeState> &Store,
                           const FString &CommandKey,
                           const TArray<FString> &Args) {
  using func::just;
  using func::nothing;
  using ForbocAI::CLI::Presentation::formatCliMessage;
  const ForbocAI::CLI::FCLIState &CLIState = Store.getState().CLI;
  const ForbocAI::CLI::FCLICommandRoles &Roles =
      ForbocAI::CLI::selectCliCommandRoles(CLIState);
  const ForbocAI::CLI::Bridge::FCLIBridgeState &State =
      ForbocAI::CLI::Bridge::selectCliBridge(CLIState);
  const int32 First = CLIState.Parsing.FirstTokenIndex;

  return CommandKey == Roles.BridgeValidate
             ? (Args.Num() < State.Limits.RequiredArgumentCount
                    ? just(Failure(State.Messages.ValidateUsage))
                    : [&]() -> HandlerResult {
                        const func::Maybe<
                            ForbocAI::CLI::Bridge::FDecodedBridgePayload>
                            Payload =
                                ForbocAI::CLI::Bridge::decodeBridgePayload(
                                    Args[First], State);
                        return func::match(
                            Payload,
                            [&Store, &State](
                                const ForbocAI::CLI::Bridge::
                                    FDecodedBridgePayload &Decoded)
                                -> HandlerResult {
                              const FValidationResult Result =
                                  Ops::validateBridgePayload(
                                      Store, Decoded.Action, Decoded.Context,
                                      Decoded.NpcId);
                              ForbocAI::CLI::Presentation::logCliMessage(
                                  formatCliMessage(
                                      State.Messages.Validation,
                                      Result.bValid ? State.Messages.Pass
                                                    : State.Messages.Fail));
                              ForbocAI::CLI::Presentation::logCliMessageWhen(
                                  !Result.Reason.IsEmpty(),
                                  formatCliMessage(State.Messages.Reason,
                                                   Result.Reason));
                              ForbocAI::CLI::Presentation::logCliMessageWhen(
                                  !Result.CorrectedAction.Type.IsEmpty(),
                                  formatCliMessage(
                                      State.Messages.CorrectedAction,
                                      JsonInterop::StringifyObject(
                                          JsonInterop::ActionToObject(
                                              Result.CorrectedAction))));
                              return just(
                                  Success(State.Messages.ValidationDone));
                            },
                            [&State]() -> HandlerResult {
                              return just(
                                  Failure(State.Messages.ValidateUsage));
                            });
                      }())
         : CommandKey == Roles.BridgeRules
             ? [&]() -> HandlerResult {
                 ForbocAI::CLI::Presentation::logCliMessage(
                     State.Messages.FetchingRules);
                 const TArray<FBridgeRule> Rules =
                     Ops::getBridgeRules(Store);
                 Rules.Num() == State.Limits.EmptyItemCount
                     ? ForbocAI::CLI::Presentation::logCliMessage(
                           State.Messages.NoRules)
                     : func::for_each_array<FBridgeRule>(
                           Rules, [&State](const FBridgeRule &Rule) {
                             ForbocAI::CLI::Presentation::logCliMessage(
                                 formatCliMessage(
                                     State.Messages.RuleName,
                                     ForbocAI::CLI::Bridge::
                                         selectBridgeRuleName(Rule, State)));
                             ForbocAI::CLI::Presentation::logCliMessage(
                                 formatCliMessage(
                                     State.Messages.Description,
                                     ForbocAI::CLI::Bridge::
                                         selectBridgeRuleDescription(Rule,
                                                                     State)));
                             ForbocAI::CLI::Presentation::logCliMessage(
                                 formatCliMessage(
                                     State.Messages.Actions,
                                     ForbocAI::CLI::Bridge::
                                         selectBridgeRuleActions(Rule,
                                                                 State)));
                           });
                 return just(Success(State.Messages.RulesListed));
               }()
         : CommandKey == Roles.BridgePreset
             ? (Args.Num() < State.Limits.RequiredArgumentCount
                    ? just(Failure(State.Messages.PresetUsage))
                    : [&]() -> HandlerResult {
                        const FDirectiveRuleSet Preset =
                            Ops::loadBridgePreset(Store, Args[First]);
                        ForbocAI::CLI::Presentation::logCliMessage(
                            formatCliMessage(
                                State.Messages.PresetLoaded,
                                ForbocAI::CLI::Bridge::selectRulesetId(
                                    Preset, State),
                                Preset.RulesetRules.Num()));
                        return just(Success(State.Messages.PresetDone));
                      }())
         : CommandKey == Roles.RulesList
             ? [&]() -> HandlerResult {
                 const TArray<FDirectiveRuleSet> Rulesets =
                     Ops::listRulesets(Store);
                 Rulesets.Num() == State.Limits.EmptyItemCount
                     ? ForbocAI::CLI::Presentation::logCliMessage(
                           State.Messages.NoRulesets)
                     : func::for_each_array<FDirectiveRuleSet>(
                           Rulesets,
                           [&State](const FDirectiveRuleSet &Ruleset) {
                             ForbocAI::CLI::Presentation::logCliMessage(
                                 formatCliMessage(
                                     State.Messages.RulesetItem,
                                     ForbocAI::CLI::Bridge::selectRulesetId(
                                         Ruleset, State),
                                     Ruleset.RulesetRules.Num()));
                           });
                 return just(Success(State.Messages.RulesetsListed));
               }()
         : CommandKey == Roles.RulesPresets
             ? [&]() -> HandlerResult {
                 const TArray<FString> Presets =
                     Ops::listRulePresets(Store);
                 Presets.Num() == State.Limits.EmptyItemCount
                     ? ForbocAI::CLI::Presentation::logCliMessage(
                           State.Messages.NoPresets)
                     : func::for_each_array<FString>(
                           Presets, [&State](const FString &Preset) {
                             ForbocAI::CLI::Presentation::logCliMessage(
                                 formatCliMessage(State.Messages.PresetItem,
                                                  Preset));
                           });
                 return just(Success(State.Messages.PresetsListed));
               }()
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
