#pragma once

#include "Features/Memory/Configuration/ConfigurationAdapters.h"
#include "Misc/Guid.h"

namespace MemoryIdentityAdapters {

/** User Story: As memory persistence, I need collision-resistant platform UUIDs wrapped in the canonical memory namespace so TS and UE emit the same identifier shape. @fn inline FString createMemoryId() */
inline FString createMemoryId() {
  return MemoryConfiguration::memoryData().Identity.MemoryPrefix +
         FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphensLower);
}

} // namespace MemoryIdentityAdapters
