#pragma once

#include "Core/fp.hpp"

namespace Dependencies::DownloadAdapters {

/** User Story: As dependency setup, I need binary assets downloaded through one checked HTTP boundary so installers own no transport policy. @fn FORBOCAI_SDK_API func::AsyncResult<FString> downloadBinary(const FString &Url, const FString &Destination) */
FORBOCAI_SDK_API func::AsyncResult<FString>
downloadBinary(const FString &Url, const FString &Destination);

} // namespace Dependencies::DownloadAdapters
