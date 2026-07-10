#pragma once

#include "API/APICodecs.h"

namespace APISlice {

namespace Detail {

struct FArweaveUploadRequest {
  FArweaveUploadInstruction Instruction;
  FString SignedPayload;
  int32 MaxRetries;

  FArweaveUploadRequest() : MaxRetries(3) {}
};

struct FArweaveDownloadRequest {
  FArweaveDownloadInstruction Instruction;
};

} // namespace Detail

namespace Endpoints {

template <typename T>
using Thunk = rtk::ThunkAction<T, FRuntimeState>;

} // namespace Endpoints

} // namespace APISlice
