#pragma once

#include "ForbocAI_SDK/Public/Features/Data/DataAdapters.h"
#include "Features/Protocol/Configuration/ProtocolConfigurationTypes.h"

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
    const TSharedRef<FJsonObject> Errors =
        DataAdapters::ReadObjectField(Source, TEXT("errors"));
    const TSharedRef<FJsonObject> Limits =
        DataAdapters::ReadObjectField(Source, TEXT("limits"));
    const TSharedRef<FJsonObject> Classifications =
        DataAdapters::ReadObjectField(Source, TEXT("classifications"));
    const TSharedRef<FJsonObject> Roles =
        DataAdapters::ReadObjectField(Source, TEXT("roles"));
    const TSharedRef<FJsonObject> Text =
        DataAdapters::ReadObjectField(Source, TEXT("text"));
    const TSharedRef<FJsonObject> Formats =
        DataAdapters::ReadObjectField(Source, TEXT("formats"));
    const TSharedRef<FJsonObject> Debug =
        DataAdapters::ReadObjectField(Source, TEXT("debug"));
    return FProtocolData{
        {DataAdapters::ReadNumberField(Iteration, TEXT("initialIndex")),
         DataAdapters::ReadNumberField(Iteration, TEXT("step"))},
        {DataAdapters::ReadStringField(RuntimeTypes, TEXT("function")),
         DataAdapters::ReadStringField(RuntimeTypes, TEXT("string"))},
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
         DataAdapters::ReadStringField(Errors,
                                       TEXT("missingApiDecisionIntent"))},
        {DataAdapters::ReadNumberField(Limits, TEXT("maxTurns"))},
        {DataAdapters::ReadStringField(Classifications,
                                       TEXT("IdentifyActor")),
         DataAdapters::ReadStringField(Classifications, TEXT("QueryVector")),
         DataAdapters::ReadStringField(Classifications, TEXT("Decision")),
         DataAdapters::ReadStringField(Classifications, TEXT("Reasoning")),
         DataAdapters::ReadStringField(Classifications, TEXT("Finalize"))},
        {DataAdapters::ReadStringField(Roles, TEXT("user")),
         DataAdapters::ReadStringField(Roles, TEXT("assistant"))},
        {DataAdapters::ReadStringField(Text, TEXT("empty")),
         DataAdapters::ReadStringField(Text, TEXT("emptyObject")),
         DataAdapters::ReadStringField(Text, TEXT("wordSeparator"))},
        {DataAdapters::ReadStringField(Formats, TEXT("runIdSeparator"))},
        {DataAdapters::ReadStringField(Debug, TEXT("environment")),
         DataAdapters::ReadStringField(Debug, TEXT("enabledValue")),
         DataAdapters::ReadStringField(Debug, TEXT("preDispatch")),
         DataAdapters::ReadStringField(Debug, TEXT("rawAction")),
         DataAdapters::ReadStringField(Debug, TEXT("unwrapThrow")),
         DataAdapters::ReadStringField(Debug, TEXT("unwrapOk"))}};
  }();
  return Data;
}

} // namespace ProtocolConfiguration
