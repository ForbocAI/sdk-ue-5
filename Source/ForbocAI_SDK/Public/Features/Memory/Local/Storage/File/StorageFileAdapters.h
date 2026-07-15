#pragma once

#include "Core/fp.hpp"
#include "CoreMinimal.h"

namespace Native::File {

FORBOCAI_SDK_API func::AsyncResult<FString>
DownloadBinary(const FString &Url, const FString &DestPath);

} // namespace Native::File
