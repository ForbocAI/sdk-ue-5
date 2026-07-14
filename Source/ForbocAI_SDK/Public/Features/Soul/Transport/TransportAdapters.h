#pragma once

#include "Core/rtk.hpp"
#include "Core/ue_fp.hpp"
#include "Features/Config/ConfigAdapters.h"
#include "Features/Soul/SoulTypes.h"

namespace rtk {

inline FString ApiTransportAuthHeader() {
  const FString ApiKey = SDKConfig::GetApiKey();
  return ApiKey.IsEmpty() ? FString() : FString(TEXT("Bearer ")) + ApiKey;
}

inline bool IsApiOwnedTransportUrl(const FString &Url) {
  const FString ApiUrl = SDKConfig::GetApiUrl();
  return !Url.IsEmpty() && !ApiUrl.IsEmpty() && Url.StartsWith(ApiUrl);
}

inline FArweaveUploadInstruction
WithApiTransportAuth(FArweaveUploadInstruction Instruction) {
  const FString Url = !Instruction.UploadUrl.IsEmpty()
                          ? Instruction.UploadUrl
                          : Instruction.GatewayUrl;
  const FString Header = ApiTransportAuthHeader();
  return IsApiOwnedTransportUrl(Url) && !Header.IsEmpty()
             ? (Instruction.AuiAuthHeader = Header, Instruction)
             : Instruction;
}

inline FArweaveDownloadInstruction
WithApiTransportAuth(FArweaveDownloadInstruction Instruction) {
  const FString Url = !Instruction.DownloadUrl.IsEmpty()
                          ? Instruction.DownloadUrl
                          : Instruction.GatewayUrl;
  const FString Header = ApiTransportAuthHeader();
  return IsApiOwnedTransportUrl(Url) && !Header.IsEmpty()
             ? (Instruction.AdiAuthHeader = Header, Instruction)
             : Instruction;
}

} // namespace rtk
