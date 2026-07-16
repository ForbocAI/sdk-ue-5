#pragma once

#include "Features/Soul/Storage/StorageTypes.h"

namespace APISlice::SoulProviderEndpoint {

struct FSoulProviderUpload {
  FSoulProviderUploadResponse Response;
  int32 Status{};
};

struct FSoulProviderPayload {
  TArray<uint8> Payload;
  FString Url;
};

} // namespace APISlice::SoulProviderEndpoint
