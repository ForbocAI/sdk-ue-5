#pragma once

#include "Features/API/Transport/Configuration/TransportConfigurationTypes.h"
#include "Features/Data/DataAdapters.h"

namespace APISlice::Transport {

/**
 * @fn inline FTransportQueryData readTransportQueryData()
 * User Story: As an API transport maintainer, I need HTTP metadata loaded from the shared authored contract so TS and UE execute the same base-query policy.
 */
inline FTransportQueryData readTransportQueryData() {
  const DataAdapters::FSettingsSource Source = DataAdapters::SettingsSource(
      TEXT("ForbocAI_SDK"), TEXT("Data/api/transport.json"));
  const TSharedRef<FJsonObject> Methods =
      DataAdapters::ReadObjectField(Source, TEXT("methods"));
  const TSharedRef<FJsonObject> Headers =
      DataAdapters::ReadObjectField(Source, TEXT("headers"));
  const TSharedRef<FJsonObject> Errors =
      DataAdapters::ReadObjectField(Source, TEXT("errors"));
  const TSharedRef<FJsonObject> Timeouts =
      DataAdapters::ReadObjectField(Source, TEXT("timeouts"));
  const TSharedRef<FJsonObject> Api =
      DataAdapters::ReadObjectField(Source, TEXT("api"));
  const TSharedRef<FJsonObject> Tags =
      DataAdapters::ReadObjectField(Source, TEXT("tags"));
  return {{DataAdapters::ReadStringField(Methods, TEXT("get")),
           DataAdapters::ReadStringField(Methods, TEXT("post")),
           DataAdapters::ReadStringField(Methods, TEXT("delete"))},
          {DataAdapters::ReadStringField(Headers, TEXT("authorization")),
           DataAdapters::ReadStringField(Headers, TEXT("bearerPrefix"))},
          {DataAdapters::ReadStringField(
              Errors, TEXT("jsonDeserializationFailed"))},
          {DataAdapters::ReadNumberField(Timeouts, TEXT("disabled"))},
          {DataAdapters::ReadStringField(Api, TEXT("baseUrl")),
           DataAdapters::ReadStringField(Api, TEXT("reducerPath"))},
          {DataAdapters::ReadStringField(Tags, TEXT("npc")),
           DataAdapters::ReadStringField(Tags, TEXT("ghost")),
           DataAdapters::ReadStringField(Tags, TEXT("soul")),
           DataAdapters::ReadStringField(Tags, TEXT("bridge")),
           DataAdapters::ReadStringField(Tags, TEXT("rule"))}};
}

/**
 * @fn inline const FTransportQueryData &transportQueryData()
 * User Story: As an API transport consumer, I need immutable authored transport policy cached once per process so every endpoint shares one configuration value.
 */
inline const FTransportQueryData &transportQueryData() {
  static const FTransportQueryData Data = readTransportQueryData();
  return Data;
}

} // namespace APISlice::Transport
