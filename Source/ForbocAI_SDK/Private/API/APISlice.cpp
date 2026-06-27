#include "API/APISlice.h"
#include "RuntimeStore.h"

/**
 * Define the global API instance
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
rtk::Api<FStoreState> APISlice::ForbocAiApi = []() {
  return rtk::createApi<FStoreState>(
      TEXT("forbocApi"),
      TArray<FString>{TEXT("NPC"),  TEXT("Memory"), TEXT("Cortex"),
                      TEXT("Ghost"), TEXT("Soul"), TEXT("Bridge"),
                      TEXT("Rule")});
}();
