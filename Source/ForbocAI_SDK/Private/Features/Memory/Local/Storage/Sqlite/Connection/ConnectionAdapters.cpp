#include "Features/Memory/Local/Storage/Sqlite/Connection/ConnectionAdapters.h"

#include "Features/Memory/Configuration/ConfigurationAdapters.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"

#if WITH_FORBOC_SQLITE_VEC
#include "sqlite-vec.h"
#include "sqlite3.h"
#endif

namespace Native::Sqlite {
namespace {

/** User Story: As local memory storage, I need raw paths rejected before normalization so parent traversal cannot disappear during canonicalization. @fn func::Either<FString, FString> resolveDatabasePath(const FString &Path) */
func::Either<FString, FString> resolveDatabasePath(const FString &Path) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const bool bRejected =
      Path.IsEmpty() ||
      (Path != Data.Storage.Sqlite.InMemoryPath &&
       Path.Contains(Data.Storage.Sqlite.UnsafePathSegment));
  return bRejected
             ? func::make_left<FString, FString>(
                   Data.Errors.SqlitePathRejected + Path)
             : func::make_right<FString, FString>(
                   Path == Data.Storage.Sqlite.InMemoryPath
                       ? Path
                       : FPaths::ConvertRelativePathToFull(Path));
}

/** User Story: As persistent memory setup, I need the database parent created before SQLite opens so first-run initialization works out of the box. @fn func::Either<FString, bool> ensureDatabaseDirectory(const FString &Path) */
func::Either<FString, bool> ensureDatabaseDirectory(const FString &Path) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Path == Data.Storage.Sqlite.InMemoryPath
             ? func::make_right<FString, bool>(true)
             : [&]() {
                 const FString Directory = FPaths::GetPath(Path);
                 IFileManager &Files = IFileManager::Get();
                 return Files.DirectoryExists(*Directory) ||
                                Files.MakeDirectory(*Directory, true)
                            ? func::make_right<FString, bool>(true)
                            : func::make_left<FString, bool>(
                                  Data.Errors.SqliteDirectoryFailed +
                                  Directory);
               }();
}

/** User Story: As memory clearing, I need file deletion to be idempotent but checked so absent files succeed and failed removals reject. @fn func::Either<FString, bool> deleteDatabaseFile(const FString &Path) */
func::Either<FString, bool> deleteDatabaseFile(const FString &Path) {
  IFileManager &Files = IFileManager::Get();
  return !Files.FileExists(*Path) || Files.Delete(*Path, false, true, true)
             ? func::make_right<FString, bool>(true)
             : func::make_left<FString, bool>(
                   MemoryConfiguration::memoryData().Errors.LocalClearFailed);
}

/** User Story: As memory clearing, I need SQLite sidecars removed recursively so WAL state cannot survive a successful clear. @fn func::Either<FString, bool> deleteSidecars(const FString &Path, int32 Index) */
func::Either<FString, bool> deleteSidecars(const FString &Path, int32 Index) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index >= Data.Storage.Sqlite.SidecarExtensions.Num()
             ? func::make_right<FString, bool>(true)
             : func::ebind(
                   deleteDatabaseFile(
                       Path + Data.Storage.Sqlite.SidecarExtensions[Index]),
                   [&Path, Index, &Data](bool) {
                     return deleteSidecars(Path,
                                           Index + Data.Iteration.Step);
                   });
}

} // namespace

/** User Story: As local memory setup, I need database creation to return its exact failure so initialization cannot report a false success. @fn func::Either<FString, DB> open(const FString &Path) */
func::Either<FString, DB> open(const FString &Path) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const func::Either<FString, FString> Resolved = resolveDatabasePath(Path);
  return func::ematch(
      Resolved,
      [](const FString &Error) {
        return func::make_left<FString, DB>(Error);
      },
      [&Data](const FString &ResolvedPath) {
        return func::ematch(
            ensureDatabaseDirectory(ResolvedPath),
            [](const FString &Error) {
              return func::make_left<FString, DB>(Error);
            },
            [&Data, &ResolvedPath](bool) {
#if WITH_FORBOC_SQLITE_VEC
              sqlite3 *Database = nullptr;
              const FTCHARToUTF8 PathUtf8(*ResolvedPath);
              const bool bOpened =
                  sqlite3_open(PathUtf8.Get(), &Database) == SQLITE_OK &&
                  Database;
              return !bOpened
                         ? (Database
                                ? (sqlite3_close(Database), void())
                                : void(),
                            func::make_left<FString, DB>(
                                Data.Errors.SqliteOpenFailed + ResolvedPath))
                         : [&]() {
                             const bool bExtension =
                                 sqlite3_vec_init(Database, nullptr,
                                                  nullptr) == SQLITE_OK;
                             return !bExtension
                                        ? (sqlite3_close(Database),
                                           func::make_left<FString, DB>(
                                               Data.Errors
                                                   .SqliteExtensionFailed))
                                        : [&]() {
                                            const FTCHARToUTF8 SchemaUtf8(
                                                *Data.Storage.Sqlite
                                                     .CreateVectorTable);
                                            const bool bSchema =
                                                sqlite3_exec(
                                                    Database,
                                                    SchemaUtf8.Get(), nullptr,
                                                    nullptr, nullptr) ==
                                                SQLITE_OK;
                                            return !bSchema
                                                       ? (sqlite3_close(
                                                              Database),
                                                          func::make_left<
                                                              FString, DB>(
                                                              Data.Errors
                                                                  .SqliteSchemaFailed))
                                                       : func::make_right<
                                                             FString, DB>(
                                                             reinterpret_cast<
                                                                 DB>(
                                                                 Database));
                                          }();
                           }();
#else
              return func::make_left<FString, DB>(
                  Data.Errors.SqliteUnavailable);
#endif
            });
      });
}

/** User Story: As local memory lifecycle, I need native handles closed at the storage boundary so thunks do not own SQLite details. @fn void close(DB Database) */
void close(DB Database) {
  !Database ? void()
            :
#if WITH_FORBOC_SQLITE_VEC
            (sqlite3_close(reinterpret_cast<sqlite3 *>(Database)), void())
#else
            ((void)Database, void())
#endif
      ;
}

/** User Story: As local memory lifecycle, I need clear execution to return native failure state so Redux only clears after persistence succeeds. @fn func::Either<FString, bool> clear(DB Database) */
func::Either<FString, bool> clear(DB Database) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  return !Handle
             ? func::make_left<FString, bool>(
                   Data.Errors.LocalNotInitialized)
             : [&]() {
                 const FTCHARToUTF8 SqlUtf8(*Data.Storage.Sqlite.Clear);
                 return sqlite3_exec(Handle, SqlUtf8.Get(), nullptr, nullptr,
                                     nullptr) == SQLITE_OK
                            ? func::make_right<FString, bool>(true)
                            : func::make_left<FString, bool>(
                                  Data.Errors.LocalClearFailed);
               }();
#else
  (void)Database;
  return func::make_left<FString, bool>(Data.Errors.SqliteUnavailable);
#endif
}

/** User Story: As local memory lifecycle, I need database artifacts removed as one checked operation so stale sidecars cannot survive a clear. @fn func::Either<FString, bool> clearPath(const FString &Path) */
func::Either<FString, bool> clearPath(const FString &Path) {
  const func::Either<FString, FString> Resolved = resolveDatabasePath(Path);
  return func::ematch(
      Resolved,
      [](const FString &Error) {
        return func::make_left<FString, bool>(Error);
      },
      [](const FString &ResolvedPath) {
        const MemoryConfiguration::FMemoryData &Data =
            MemoryConfiguration::memoryData();
        return ResolvedPath == Data.Storage.Sqlite.InMemoryPath
                   ? func::make_right<FString, bool>(true)
                   : func::ebind(
                         deleteDatabaseFile(ResolvedPath),
                         [&ResolvedPath, &Data](bool) {
                           return deleteSidecars(
                               ResolvedPath, Data.Iteration.InitialIndex);
                         });
      });
}

} // namespace Native::Sqlite
