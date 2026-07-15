#pragma once

#include "CoreMinimal.h"
#include "Features/Memory/Local/Storage/Sqlite/SqliteTypes.h"

namespace Native::Sqlite {

FORBOCAI_SDK_API DB Open(const FString &Path);
FORBOCAI_SDK_API void Close(DB Database);
FORBOCAI_SDK_API void Clear(DB Database);
FORBOCAI_SDK_API void ClearPath(const FString &Path);

} // namespace Native::Sqlite
