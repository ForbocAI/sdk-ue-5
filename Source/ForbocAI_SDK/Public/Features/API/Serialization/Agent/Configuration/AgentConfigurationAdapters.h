#pragma once

#include "Features/API/Serialization/Agent/Configuration/AgentConfigurationTypes.h"
#include "Features/Data/DataAdapters.h"

namespace JsonInterop::AgentConfiguration {

/** User Story: As an Agent codec consumer, I need one immutable authored contract so canonical wire fields and defaults stay synchronized. @fn inline const FAgentContractData &agentContractData() */
inline const FAgentContractData &agentContractData() {
  static const FAgentContractData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                     TEXT("Data/api/agent.json"));
    const TSharedRef<FJsonObject> Persona =
        DataAdapters::ReadObjectField(Source, TEXT("persona"));
    const TSharedRef<FJsonObject> Action =
        DataAdapters::ReadObjectField(Source, TEXT("action"));
    const TSharedRef<FJsonObject> Values =
        DataAdapters::ReadObjectField(Source, TEXT("values"));
    return FAgentContractData{
        {DataAdapters::ReadStringField(Persona, TEXT("traits")),
         DataAdapters::ReadStringField(Persona, TEXT("goals")),
         DataAdapters::ReadStringField(Persona, TEXT("relationships")),
         DataAdapters::ReadStringField(Persona, TEXT("world")),
         DataAdapters::ReadStringField(Persona, TEXT("speakingStyle")),
         DataAdapters::ReadStringField(Persona, TEXT("constraints"))},
        {DataAdapters::ReadStringField(Action, TEXT("type")),
         DataAdapters::ReadStringField(Action, TEXT("target")),
         DataAdapters::ReadStringField(Action, TEXT("reason")),
         DataAdapters::ReadStringField(Action, TEXT("confidence")),
         DataAdapters::ReadStringField(Action, TEXT("signature")),
         DataAdapters::ReadStringField(Action, TEXT("payload"))},
        DataAdapters::ReadFloatField(Values, TEXT("defaultConfidence")),
        DataAdapters::ReadStringField(Values, TEXT("emptyObject")),
        DataAdapters::ReadNumberField(Values, TEXT("minimumTypeLength"))};
  }();
  return Data;
}

} // namespace JsonInterop::AgentConfiguration
