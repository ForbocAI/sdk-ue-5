#pragma once

#include "Core/fp.hpp"
#include "Features/Bridge/BridgeTypes.h"
#include "Features/CLI/Bridge/CLIBridgeTypes.h"
#include "Features/Data/DataAdapters.h"
#include "Features/API/Serialization/APISerializationAdapters.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"

namespace ForbocAI {
namespace CLI {
namespace Bridge {

struct FDecodedBridgePayload {
  FAgentAction Action;
  FBridgeValidationContext Context;
  FString NpcId;
};

/** User Story: As a features cli bridge consumer, I need to invoke read cli bridge state through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FCLIBridgeState readCliBridgeState() */
inline FCLIBridgeState readCliBridgeState() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/cli/bridge.json"));
  const TSharedRef<FJsonObject> Limits =
      DataAdapters::ReadObjectField(Source, TEXT("limits"));
  const TSharedRef<FJsonObject> Syntax =
      DataAdapters::ReadObjectField(Source, TEXT("syntax"));
  const TSharedRef<FJsonObject> Fields =
      DataAdapters::ReadObjectField(Source, TEXT("fields"));
  const TSharedRef<FJsonObject> Messages =
      DataAdapters::ReadObjectField(Source, TEXT("messages"));
  return {
      {DataAdapters::ReadNumberField(Limits, TEXT("emptyItemCount")),
       DataAdapters::ReadNumberField(Limits, TEXT("requiredArgumentCount"))},
      {DataAdapters::ReadStringField(Syntax, TEXT("inlineObjectPrefix"))},
      {DataAdapters::ReadStringField(Fields, TEXT("action")),
       DataAdapters::ReadStringField(Fields, TEXT("distance")),
       DataAdapters::ReadStringField(Fields, TEXT("npcId")),
       DataAdapters::ReadStringField(Fields, TEXT("context")),
       DataAdapters::ReadStringField(Fields, TEXT("npcState")),
       DataAdapters::ReadStringField(Fields, TEXT("worldState")),
       DataAdapters::ReadStringField(Fields, TEXT("constraints"))},
      {DataAdapters::ReadStringField(Messages, TEXT("emptyObject")),
       DataAdapters::ReadStringField(Messages, TEXT("fetchingRules")),
       DataAdapters::ReadStringField(Messages, TEXT("noRules")),
       DataAdapters::ReadStringField(Messages, TEXT("unknownRule")),
       DataAdapters::ReadStringField(Messages, TEXT("notAvailable")),
       DataAdapters::ReadStringField(Messages, TEXT("ruleName")),
       DataAdapters::ReadStringField(Messages, TEXT("description")),
       DataAdapters::ReadStringField(Messages, TEXT("actions")),
       DataAdapters::ReadStringField(Messages, TEXT("actionSeparator")),
       DataAdapters::ReadStringField(Messages, TEXT("validateUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("validation")),
       DataAdapters::ReadStringField(Messages, TEXT("valid")),
       DataAdapters::ReadStringField(Messages, TEXT("pass")),
       DataAdapters::ReadStringField(Messages, TEXT("fail")),
       DataAdapters::ReadStringField(Messages, TEXT("reason")),
       DataAdapters::ReadStringField(Messages, TEXT("correctedAction")),
       DataAdapters::ReadStringField(Messages, TEXT("validationDone")),
       DataAdapters::ReadStringField(Messages, TEXT("rulesCount")),
       DataAdapters::ReadStringField(Messages, TEXT("rulesListed")),
       DataAdapters::ReadStringField(Messages, TEXT("presetUsage")),
       DataAdapters::ReadStringField(Messages, TEXT("presetLoaded")),
       DataAdapters::ReadStringField(Messages, TEXT("presetDone")),
       DataAdapters::ReadStringField(Messages, TEXT("rulesetsCount")),
       DataAdapters::ReadStringField(Messages, TEXT("rulesetsListed")),
       DataAdapters::ReadStringField(Messages, TEXT("noRulesets")),
       DataAdapters::ReadStringField(Messages, TEXT("rulesetItem")),
       DataAdapters::ReadStringField(Messages, TEXT("unknownRuleset")),
       DataAdapters::ReadStringField(Messages, TEXT("presetsCount")),
       DataAdapters::ReadStringField(Messages, TEXT("presetsListed")),
       DataAdapters::ReadStringField(Messages, TEXT("noPresets")),
       DataAdapters::ReadStringField(Messages, TEXT("presetItem"))}};
}

/** User Story: As a features cli bridge consumer, I need to invoke read payload text through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline func::Maybe<FString> readPayloadText( const FString &Input, const FCLIBridgeState &State) */
inline func::Maybe<FString> readPayloadText(
    const FString &Input, const FCLIBridgeState &State) {
  return Input.StartsWith(State.Syntax.InlineObjectPrefix)
             ? func::just(Input)
             : [&]() {
                 FString Json;
                 return FFileHelper::LoadFileToString(Json, *Input)
                            ? func::just(Json)
                            : func::nothing<FString>();
               }();
}

/** User Story: As a features cli bridge consumer, I need to invoke json object field through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FString jsonObjectField(const TSharedPtr<FJsonObject> &Object, const FString &FieldName, const FString &EmptyObject) */
inline FString jsonObjectField(const TSharedPtr<FJsonObject> &Object,
                               const FString &FieldName,
                               const FString &EmptyObject) {
  return Object.IsValid() && Object->HasTypedField<EJson::Object>(FieldName)
             ? JsonInterop::StringifyObject(Object->GetObjectField(FieldName))
             : EmptyObject;
}

/** User Story: As a features cli bridge consumer, I need to invoke number payload json through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FString numberPayloadJson(const FString &FieldName, double Value) */
inline FString numberPayloadJson(const FString &FieldName, double Value) {
  const TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
  Payload->SetNumberField(FieldName, Value);
  return JsonInterop::StringifyObject(Payload);
}

/** User Story: As a features cli bridge consumer, I need to invoke decode bridge context through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline FBridgeValidationContext decodeBridgeContext( const TSharedPtr<FJsonObject> &Root, const FCLIBridgeState &State) */
inline FBridgeValidationContext decodeBridgeContext(
    const TSharedPtr<FJsonObject> &Root, const FCLIBridgeState &State) {
  return Root->HasTypedField<EJson::Object>(State.Fields.Context)
             ? [&]() {
                 const TSharedPtr<FJsonObject> ContextObject =
                     Root->GetObjectField(State.Fields.Context);
                 FBridgeValidationContext Context;
                 Context.NpcStateJson = jsonObjectField(
                     ContextObject, State.Fields.NpcState,
                     State.Messages.EmptyObject);
                 Context.WorldStateJson = jsonObjectField(
                     ContextObject, State.Fields.WorldState,
                     State.Messages.EmptyObject);
                 Context.ConstraintsJson = jsonObjectField(
                     ContextObject, State.Fields.Constraints,
                     State.Messages.EmptyObject);
                 return Context;
               }()
             : FBridgeValidationContext();
}

/** User Story: As a features cli bridge consumer, I need to invoke decode bridge payload through a stable signature so the features cli bridge workflow remains explicit and composable. @fn inline func::Maybe<FDecodedBridgePayload> decodeBridgePayload( const FString &Input, const FCLIBridgeState &State) */
inline func::Maybe<FDecodedBridgePayload> decodeBridgePayload(
    const FString &Input, const FCLIBridgeState &State) {
  return func::match(
      readPayloadText(Input, State),
      [&State](const FString &Json) -> func::Maybe<FDecodedBridgePayload> {
        TSharedPtr<FJsonObject> Root;
        return !JsonInterop::ParseJsonObject(Json, Root) || !Root.IsValid() ||
                       !Root->HasTypedField<EJson::Object>(State.Fields.Action)
                   ? func::nothing<FDecodedBridgePayload>()
                   : [&]() -> func::Maybe<FDecodedBridgePayload> {
                       const TSharedPtr<FJsonObject> ActionObject =
                           Root->GetObjectField(State.Fields.Action);
                       return func::match(
                           JsonInterop::DecodeActionObject(ActionObject),
                           [&](const FAgentAction &Action) {
                             FDecodedBridgePayload Decoded;
                             Decoded.Action = Action;
                             double Distance = 0.0;
                             Decoded.Action.PayloadJson =
                                 ActionObject->TryGetNumberField(
                                     State.Fields.Distance, Distance)
                                     ? numberPayloadJson(State.Fields.Distance,
                                                         Distance)
                                     : Decoded.Action.PayloadJson;
                             Root->TryGetStringField(State.Fields.NpcId,
                                                     Decoded.NpcId);
                             Decoded.Context = decodeBridgeContext(Root, State);
                             return func::just(Decoded);
                           },
                           []() {
                             return func::nothing<FDecodedBridgePayload>();
                           });
                     }();
      },
      []() { return func::nothing<FDecodedBridgePayload>(); });
}

} // namespace Bridge
} // namespace CLI
} // namespace ForbocAI
