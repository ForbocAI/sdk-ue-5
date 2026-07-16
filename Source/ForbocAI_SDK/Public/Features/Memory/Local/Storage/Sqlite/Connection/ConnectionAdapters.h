#pragma once

#include "Core/fp.hpp"
#include "Features/Memory/Local/Storage/Sqlite/SqliteTypes.h"

namespace Native::Sqlite {

/** User Story: As local memory setup, I need database creation to return its exact failure so initialization cannot report a false success. @fn FORBOCAI_SDK_API func::Either<FString, DB> open(const FString &Path) */
FORBOCAI_SDK_API func::Either<FString, DB> open(const FString &Path);

/** User Story: As local memory lifecycle, I need native handles closed at the storage boundary so thunks do not own SQLite details. @fn FORBOCAI_SDK_API void close(DB Database) */
FORBOCAI_SDK_API void close(DB Database);

/** User Story: As local memory lifecycle, I need clear execution to return native failure state so Redux only clears after persistence succeeds. @fn FORBOCAI_SDK_API func::Either<FString, bool> clear(DB Database) */
FORBOCAI_SDK_API func::Either<FString, bool> clear(DB Database);

/** User Story: As local memory lifecycle, I need database artifacts removed as one checked operation so stale sidecars cannot survive a clear. @fn FORBOCAI_SDK_API func::Either<FString, bool> clearPath(const FString &Path) */
FORBOCAI_SDK_API func::Either<FString, bool> clearPath(const FString &Path);

} // namespace Native::Sqlite
