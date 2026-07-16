#pragma once

#include "Core/rtk.hpp"
#include "Features/Protocol/Logger/Configuration/ConfigurationAdapters.h"
#include "Features/Protocol/Logger/LoggerTypes.h"

namespace LoggerAdapters {

/** User Story: As a features protocol logger consumer, I need to invoke write protocol log adapter through a stable signature so the features protocol logger workflow remains explicit and composable. @fn inline void writeProtocolLogAdapter(const rtk::AnyAction &Action, const FString &Delta) */
inline void writeProtocolLogAdapter(const rtk::AnyAction &Action,
                                    const FString &Delta) {
  const auto &Data = LoggerConfiguration::loggerData();
  const FString Line =
      Data.Text.CategoryPrefix + Data.Text.FieldSeparator + Data.Text.Action +
      Data.Text.KeyValueSeparator + Action.Type + Data.Text.FieldSeparator +
      Data.Text.Payload + Data.Text.KeyValueSeparator +
      Action.describePayload() + Data.Text.FieldSeparator + Data.Text.Delta +
      Data.Text.KeyValueSeparator + Delta;
  LogForbocAIProtocol.IsSuppressed(ELogVerbosity::Display) || GLog == nullptr
      ? void()
      : (GLog->Serialize(*Line, ELogVerbosity::Display,
                         LogForbocAIProtocol.GetCategoryName()),
         LogForbocAIProtocol.PostTrigger(ELogVerbosity::Display), void());
}

} // namespace LoggerAdapters
