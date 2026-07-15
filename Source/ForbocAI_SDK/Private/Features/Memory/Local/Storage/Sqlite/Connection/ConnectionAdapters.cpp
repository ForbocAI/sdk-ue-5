#include "Features/Memory/Local/Storage/Sqlite/Connection/ConnectionAdapters.h"

#include "Misc/Paths.h"

#if WITH_FORBOC_SQLITE_VEC
extern "C" {
#include "sqlite-vec.h"
#include "sqlite3.h"
}
#endif

namespace Native::Sqlite {

DB Open(const FString &Path) {
  const FString NormalizedPath = Path.IsEmpty()
                                     ? TEXT(":memory:")
                                     : FPaths::ConvertRelativePathToFull(Path);

  return (NormalizedPath != TEXT(":memory:") &&
          NormalizedPath.Contains(TEXT("..")))
             ? [&]() -> DB {
                 UE_LOG(LogTemp, Error,
                        TEXT("ForbocAI: Rejected database path containing "
                             "'..': %s"),
                        *NormalizedPath);
                 return static_cast<DB>(nullptr);
               }()
             :
#if WITH_FORBOC_SQLITE_VEC
             [&]() -> DB {
               sqlite3 *Database = nullptr;
               const int Result =
                   sqlite3_open(TCHAR_TO_UTF8(*NormalizedPath), &Database);
               return (Result != SQLITE_OK || !Database)
                          ? (Database
                                 ? (sqlite3_close(Database),
                                    static_cast<DB>(nullptr))
                                 : static_cast<DB>(nullptr))
                          : [&]() -> DB {
                              sqlite3_vec_init(Database, nullptr, nullptr);
                              const char *CreateSql =
                                  "CREATE VIRTUAL TABLE IF NOT EXISTS memories "
                                  "USING vec0(embedding float[384], "
                                  "+id text, +text text, +type text, "
                                  "+importance float, +timestamp integer);";
                              sqlite3_exec(Database, CreateSql, nullptr,
                                           nullptr, nullptr);
                              return reinterpret_cast<DB>(Database);
                            }();
             }()
#else
             [&]() -> DB {
               UE_LOG(LogTemp, Error,
                      TEXT("ForbocAI: Sqlite::Open requires "
                           "WITH_FORBOC_SQLITE_VEC=1. Native libs not "
                           "available."));
               return static_cast<DB>(nullptr);
             }()
#endif
      ;
}

void Close(DB Database) {
  !Database ? void()
            :
#if WITH_FORBOC_SQLITE_VEC
            (sqlite3_close(reinterpret_cast<sqlite3 *>(Database)), void())
#else
            ((void)Database, void())
#endif
      ;
}

void Clear(DB Database) {
#if WITH_FORBOC_SQLITE_VEC
  sqlite3 *Handle = reinterpret_cast<sqlite3 *>(Database);
  Handle ? (sqlite3_exec(Handle, "DELETE FROM memories;", nullptr, nullptr,
                         nullptr),
            void())
         : void();
#else
  (void)Database;
#endif
}

void ClearPath(const FString &Path) {
  const FString NormalizedPath = Path.IsEmpty()
                                     ? TEXT(":memory:")
                                     : FPaths::ConvertRelativePathToFull(Path);
  (NormalizedPath != TEXT(":memory:") && NormalizedPath.Contains(TEXT("..")))
      ? [&]() {
          UE_LOG(LogTemp, Error,
                 TEXT("ForbocAI: Rejected database path containing '..': %s"),
                 *NormalizedPath);
        }()
      : static_cast<void>(NormalizedPath);
}

} // namespace Native::Sqlite
