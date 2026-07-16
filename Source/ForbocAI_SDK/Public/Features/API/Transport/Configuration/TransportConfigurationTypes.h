#pragma once

#include "CoreMinimal.h"

namespace APISlice::Transport {

struct FTransportMethodData {
  FString Get;
  FString Post;
  FString Delete;
};

struct FTransportHeaderData {
  FString Authorization;
  FString BearerPrefix;
};

struct FTransportErrorData {
  FString JsonDeserializationFailed;
};

struct FTransportTimeoutData {
  int32 Disabled;
};

struct FTransportApiData {
  FString BaseUrl;
  FString ReducerPath;
};

struct FTransportTagData {
  FString Npc;
  FString Ghost;
  FString Soul;
  FString Bridge;
  FString Rule;
};

struct FTransportQueryData {
  FTransportMethodData Methods;
  FTransportHeaderData Headers;
  FTransportErrorData Errors;
  FTransportTimeoutData Timeouts;
  FTransportApiData Api;
  FTransportTagData Tags;
};

} // namespace APISlice::Transport
