#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/rtk.hpp"
#include "Systems/Protocol/Logger/Configuration/LoggerConfigurationAdapters.h"
#include "Components/Protocol/Logger/LoggerTypes.h"

namespace LoggerAdapters {

/**
 * User Story: As an SDK host, I need protocol logs to preserve action routing
 * evidence without exposing API payload contents.
 * @fn inline const FString &describeProtocolPayloadAdapter()
 */
inline const FString &describeProtocolPayloadAdapter() {
  return LoggerConfiguration::loggerData().Text.OpaquePayload;
}

/** User Story: As a features protocol logger consumer, I need to invoke write protocol log adapter through a stable signature so the features protocol logger workflow remains explicit and composable. @fn inline void writeProtocolLogAdapter(const rtk::AnyAction &Action, const FString &Delta) */
inline void writeProtocolLogAdapter(const rtk::AnyAction &Action,
                                    const FString &Delta) {
  const auto &Data = LoggerConfiguration::loggerData();
  const FString Line =
      Data.Text.CategoryPrefix + Data.Text.FieldSeparator + Data.Text.Action +
      Data.Text.KeyValueSeparator + Action.Type + Data.Text.FieldSeparator +
      Data.Text.Payload + Data.Text.KeyValueSeparator +
      describeProtocolPayloadAdapter() + Data.Text.FieldSeparator + Data.Text.Delta +
      Data.Text.KeyValueSeparator + Delta;
  UE_LOG(LogForbocAIProtocol, Display, TEXT(FORBOCAI_SDK_AUTHORED_STRINGV03A110C67C3C), *Line);
}

} // namespace LoggerAdapters
