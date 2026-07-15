#pragma once

#include "Core/rtk.hpp"
#include "Features/Protocol/Logger/LoggerTypes.h"

namespace LoggerAdapters {

inline void writeProtocolLogAdapter(const rtk::AnyAction &Action,
                                    const FString &Delta) {
  UE_LOG(LogForbocAIProtocol, Display,
         TEXT("[ForbocAI][Protocol] action=%s payload=%s delta=%s"),
         *Action.Type, *Action.describePayload(), *Delta);
}

} // namespace LoggerAdapters
