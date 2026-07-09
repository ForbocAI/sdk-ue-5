#include "Features/API/APISlice.h"
#include "RuntimeStore.h"

namespace {

bool providesTags(const rtk::Api<FRuntimeState> &Api) {
  return Api.TagTypes.Num() > 0;
}

} // namespace

/**
 * Define the global API instance
 * User Story: As a maintainer, I need this note so the surrounding code intent stays clear during maintenance and debugging.
 */
rtk::Api<FRuntimeState> APISlice::ForbocAiApi = []() {
  rtk::Api<FRuntimeState> Api = rtk::createApi<FRuntimeState>(
      TEXT("forbocApi"),
	      TArray<FString>{TEXT("NPC"),  TEXT("Memory"), TEXT("Ghost"),
	                      TEXT("Soul"), TEXT("Bridge"), TEXT("Rule")});
  check(providesTags(Api));
  return Api;
}();
