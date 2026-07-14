// User Story: As a developer, I need this module to function.
#include "CLI/CliHandlers.h"
#include "CLI/CliOperations.h"
#include "Core/JsonInterop.h"
#include "RuntimeStore.h"

namespace {

struct FDecodedBridgePayload {
  FAgentAction Action;
  FBridgeValidationContext Context;
  FString NpcId;
};

FString JsonObjectField(const TSharedPtr<FJsonObject> &Object,
                        const FString &FieldName) {
  return Object.IsValid() && Object->HasTypedField<EJson::Object>(FieldName)
             ? JsonInterop::StringifyObject(Object->GetObjectField(FieldName))
             : FString(TEXT("{}"));
}

FString NumberPayloadJson(const FString &FieldName, double Value) {
  const TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
  Payload->SetNumberField(FieldName, Value);
  return JsonInterop::StringifyObject(Payload);
}

bool DecodeBridgePayload(const FString &Json,
                         FDecodedBridgePayload &Decoded) {
  TSharedPtr<FJsonObject> Root;
  return !JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid() ||
                 !Root->HasTypedField<EJson::Object>(TEXT("action"))
             ? false
             : [&]() {
                 const TSharedPtr<FJsonObject> ActionObject =
                     Root->GetObjectField(TEXT("action"));
                 Decoded.Action = JsonInterop::ActionFromObject(ActionObject);

                 double Distance = 0.0;
                 ActionObject->TryGetNumberField(TEXT("distance"), Distance)
                     ? (Decoded.Action.PayloadJson =
                            NumberPayloadJson(TEXT("distance"), Distance),
                        void())
                     : void();

                 Root->TryGetStringField(TEXT("npcId"), Decoded.NpcId);

                 Root->HasTypedField<EJson::Object>(TEXT("context"))
                     ? [&]() {
                         const TSharedPtr<FJsonObject> ContextObject =
                             Root->GetObjectField(TEXT("context"));
                         Decoded.Context.NpcStateJson = JsonObjectField(
                             ContextObject, TEXT("npcState"));
                         Decoded.Context.WorldStateJson = JsonObjectField(
                             ContextObject, TEXT("worldState"));
                         Decoded.Context.ConstraintsJson = JsonObjectField(
                             ContextObject, TEXT("constraints"));
                       }()
                     : void();

                 return !Decoded.Action.Type.IsEmpty();
               }();
}

} // namespace

namespace CLIOps {
namespace Handlers {

HandlerResult HandleBridge(rtk::EnhancedStore<FRuntimeState> &Store,
                          const FString &CommandKey,
                          const TArray<FString> &Args) {
  using func::just;
  using func::nothing;

  return CommandKey == TEXT("bridge_validate")
             ? (Args.Num() < 1
                    ? just(Result::Failure(
                          "Usage: bridge_validate <actionJson>"))
                    : [&]() -> HandlerResult {
                        FDecodedBridgePayload Payload;
                        return !DecodeBridgePayload(Args[0], Payload)
                                   ? just(Result::Failure(
                                         "Usage: bridge_validate <inline-json-payload|preset-macro>"))
                                   : [&]() -> HandlerResult {
                                       FValidationResult VResult =
                                           Ops::validateBridgePayload(
                                               Store, Payload.Action,
                                               Payload.Context,
                                               Payload.NpcId);
                                       UE_LOG(
                                           LogTemp, Display,
                                           TEXT("Validation: %s"),
                                           VResult.bValid ? TEXT("PASS")
                                                          : TEXT("FAIL"));
                                       return just(Result::Success(
                                           "Bridge validation done"));
                                     }();
                      }())
         : CommandKey == TEXT("bridge_rules")
             ? [&]() -> HandlerResult {
                 TArray<FBridgeRule> Rules = Ops::getBridgeRules(Store);
                 UE_LOG(LogTemp, Display,
                        TEXT("Found %d bridge rules"), Rules.Num());
                 return just(
                     Result::Success("Bridge rules listed"));
               }()
         : CommandKey == TEXT("bridge_preset")
             ? (Args.Num() < 1
                    ? just(Result::Failure(
                          "Usage: bridge_preset <presetName>"))
                    : [&]() -> HandlerResult {
                        FDirectiveRuleSet Preset =
                            Ops::loadBridgePreset(Store, Args[0]);
                        UE_LOG(LogTemp, Display,
                               TEXT("Loaded preset: %s"), *Preset.Id);
                        return just(Result::Success(
                            "Bridge preset loaded"));
                      }())
         : CommandKey == TEXT("rules_list")
             ? [&]() -> HandlerResult {
                 TArray<FDirectiveRuleSet> Rulesets =
                     Ops::listRulesets(Store);
                 UE_LOG(LogTemp, Display,
                        TEXT("Found %d rulesets"), Rulesets.Num());
                 return just(
                     Result::Success("Rulesets listed"));
               }()
         : CommandKey == TEXT("rules_presets")
             ? [&]() -> HandlerResult {
                 TArray<FString> Presets =
                     Ops::listRulePresets(Store);
                 UE_LOG(LogTemp, Display,
                        TEXT("Found %d presets"), Presets.Num());
                 return just(
                     Result::Success("Rule presets listed"));
               }()
         : CommandKey == TEXT("rules_register")
             ? (Args.Num() < 1
                    ? just(Result::Failure(
                          "Usage: rules_register <rulesetJson>"))
                    : [&]() -> HandlerResult {
                        FDirectiveRuleSet Ruleset;
                        Ruleset.Id = Args[0];
                        FDirectiveRuleSet Registered =
                            Ops::registerRuleset(Store, Ruleset);
                        UE_LOG(LogTemp, Display,
                               TEXT("Registered ruleset: %s"),
                               *Registered.Id);
                        return just(Result::Success(
                            "Ruleset registered"));
                      }())
         : CommandKey == TEXT("rules_delete")
             ? (Args.Num() < 1
                    ? just(Result::Failure(
                          "Usage: rules_delete <rulesetId>"))
                    : (Ops::deleteRuleset(Store, Args[0]),
                       just(Result::Success("Ruleset deleted"))))
             : nothing<Result>();
}

} // namespace Handlers
} // namespace CLIOps
