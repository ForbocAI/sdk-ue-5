#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Protocol/Configuration/ProtocolConfigurationTypes.h"

namespace ProtocolConfiguration {

/** User Story: As protocol execution, I need one immutable authored contract shared with TS so handler policy, limits, messages, and formatting cannot drift between SDKs. @fn inline const FProtocolData &protocolData() */
inline const FProtocolData &protocolData() {
  static const FProtocolData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                     TEXT("Data/protocol/handlers.json"));
    const TSharedRef<FJsonObject> Iteration =
        DataAdapters::ReadObjectField(Source, TEXT("iteration"));
    const TSharedRef<FJsonObject> RuntimeTypes =
        DataAdapters::ReadObjectField(Source, TEXT("runtimeTypes"));
    const TSharedRef<FJsonObject> Actions =
        DataAdapters::ReadObjectField(Source, TEXT("actions"));
    const TSharedRef<FJsonObject> Errors =
        DataAdapters::ReadObjectField(Source, TEXT("errors"));
    const TSharedRef<FJsonObject> Limits =
        DataAdapters::ReadObjectField(Source, TEXT("limits"));
    const TSharedRef<FJsonObject> Classifications =
        DataAdapters::ReadObjectField(Source, TEXT("classifications"));
    const TSharedRef<FJsonObject> Roles =
        DataAdapters::ReadObjectField(Source, TEXT("roles"));
    const TSharedRef<FJsonObject> Tokens =
        DataAdapters::ReadObjectField(Source, TEXT("tokens"));
    const TSharedRef<FJsonObject> Formats =
        DataAdapters::ReadObjectField(Source, TEXT("formats"));
    const TSharedRef<FJsonObject> Debug =
        DataAdapters::ReadObjectField(Source, TEXT("debug"));
    const TSharedRef<FJsonObject> NpcDebug =
        DataAdapters::ReadObjectField(Debug, TEXT("npc"));
    const TSharedRef<FJsonObject> GhostDebug =
        DataAdapters::ReadObjectField(Debug, TEXT("ghost"));
    return FProtocolData{
        {DataAdapters::ReadNumberField(Iteration, TEXT("initialIndex")),
         DataAdapters::ReadNumberField(Iteration, TEXT("step"))},
        {DataAdapters::ReadStringField(RuntimeTypes, TEXT("function")),
         DataAdapters::ReadStringField(RuntimeTypes, TEXT("string"))},
        {DataAdapters::ReadStringField(Actions, TEXT("npcProcess")),
         DataAdapters::ReadStringField(Actions, TEXT("ghostProcess"))},
        {DataAdapters::ReadStringField(Errors, TEXT("missingMemoryStore")),
         DataAdapters::ReadStringField(Errors, TEXT("missingVectorMemory")),
         DataAdapters::ReadStringField(Errors, TEXT("unknownInstruction")),
         DataAdapters::ReadStringField(Errors, TEXT("validationFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("maxTurnsExceeded")),
         DataAdapters::ReadStringField(Errors,
                                       TEXT("unsupportedInstruction")),
         DataAdapters::ReadStringField(Errors, TEXT("missingNpcId")),
         DataAdapters::ReadStringField(Errors,
                                       TEXT("missingStructuredPersona")),
         DataAdapters::ReadStringField(Errors, TEXT("processingFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("npcNotFoundTemplate")),
         DataAdapters::ReadStringField(Errors,
                                       TEXT("apiStatusRequestFailed")),
         DataAdapters::ReadStringField(Errors,
                                       TEXT("apiContractRequestFailed"))},
        {DataAdapters::ReadNumberField(Limits, TEXT("maxTurns"))},
        {DataAdapters::ReadStringField(Classifications, TEXT("QueryVector")),
         DataAdapters::ReadStringField(Classifications, TEXT("Finalize"))},
        {DataAdapters::ReadStringField(Roles, TEXT("user")),
         DataAdapters::ReadStringField(Roles, TEXT("assistant"))},
        {DataAdapters::ReadStringField(Tokens, TEXT("npcId"))},
        {DataAdapters::ReadStringField(Formats, TEXT("runIdSeparator"))},
        {DataAdapters::ReadStringField(Debug, TEXT("environment")),
         DataAdapters::ReadStringField(Debug, TEXT("enabledValue")),
         {DataAdapters::ReadStringField(NpcDebug, TEXT("preDispatch")),
          DataAdapters::ReadStringField(NpcDebug, TEXT("rawAction")),
          DataAdapters::ReadStringField(NpcDebug, TEXT("unwrapThrow")),
          DataAdapters::ReadStringField(NpcDebug, TEXT("unwrapOk"))},
         {DataAdapters::ReadStringField(GhostDebug, TEXT("preDispatch")),
          DataAdapters::ReadStringField(GhostDebug, TEXT("rawAction")),
          DataAdapters::ReadStringField(GhostDebug, TEXT("unwrapThrow")),
          DataAdapters::ReadStringField(GhostDebug, TEXT("unwrapOk"))}}};
  }();
  return Data;
}

} // namespace ProtocolConfiguration
