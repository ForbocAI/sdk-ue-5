#include "Features/Dependencies/DependenciesAdapters.h"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Memory/Local/Storage/StorageAdapters.h"
#include "Features/Dependencies/Process/ProcessAdapters.h"
#include "Features/Vector/VectorAdapters.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"

namespace {

const TCHAR *SQLITE_ARCHIVE_VERSION = TEXT("3460100");
const TCHAR *SQLITE_VEC_RELEASE = TEXT("v0.1.6");

struct FDependenciesPaths {
  FString ThirdParty;
  FString Include;
  FString Source;
  FString Temporary;
  FString Vectors;
};

FDependenciesPaths dependenciesPaths() {
  const FString ThirdParty =
      FPaths::ProjectPluginsDir() / TEXT("ForbocAI_SDK/ThirdParty");
  const FString Infrastructure =
      FPaths::ProjectDir() / TEXT("local_infrastructure");
  return {ThirdParty,
          ThirdParty / TEXT("sqlite-vss/include"),
          ThirdParty / TEXT("sqlite-vss/src"),
          Infrastructure / TEXT("tmp"),
          Infrastructure / TEXT("vectors")};
}

bool copyFile(const FString &Source, const FString &Destination) {
  return IFileManager::Get().Copy(*Destination, *Source) == COPY_OK;
}

bool downloadArchive(const FString &Url, const FString &Destination) {
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  return PlatformFile.FileExists(*Destination)
             ? true
             : [&]() {
                 try {
                   AsyncAdapters::waitForResult(
                       Native::File::DownloadBinary(Url, Destination), 120.0);
                   return true;
                 } catch (const std::exception &Error) {
                   UE_LOG(LogTemp, Warning, TEXT("Dependency download failed: %s"),
                          UTF8_TO_TCHAR(Error.what()));
                   return false;
                 }
               }();
}

bool installSqlite(const FDependenciesPaths &Paths) {
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  const FString Header = Paths.Include / TEXT("sqlite3.h");
  const FString ExtensionHeader = Paths.Include / TEXT("sqlite3ext.h");
  const FString Source = Paths.Source / TEXT("sqlite3.c");
  return PlatformFile.FileExists(*Header) && PlatformFile.FileExists(*Source)
             ? true
             : [&]() {
                 const FString Archive =
                     Paths.Temporary / TEXT("sqlite-amalgamation.zip");
                 const FString Extracted =
                     Paths.Temporary / TEXT("sqlite-extract");
                 const FString Url = FString::Printf(
                     TEXT("https://www.sqlite.org/2024/"
                          "sqlite-amalgamation-%s.zip"),
                     SQLITE_ARCHIVE_VERSION);
                 return !downloadArchive(Url, Archive) ||
                                !DependenciesProcessAdapters::extractZip(Archive,
                                                                 Extracted)
                            ? false
                            : [&]() {
                                const FString Inner =
                                    DependenciesProcessAdapters::findSubdirWithPrefix(
                                        Extracted,
                                        TEXT("sqlite-amalgamation-"));
                                return !Inner.IsEmpty() &&
                                       copyFile(Inner / TEXT("sqlite3.h"),
                                                Header) &&
                                       copyFile(Inner / TEXT("sqlite3ext.h"),
                                                ExtensionHeader) &&
                                       copyFile(Inner / TEXT("sqlite3.c"),
                                                Source);
                              }();
               }();
}

FString firstExistingPath(const TArray<FString> &Candidates, int32 Index) {
  return Index >= Candidates.Num()
             ? FString()
             : FPlatformFileManager::Get().GetPlatformFile().FileExists(
                   *Candidates[Index])
                   ? Candidates[Index]
                   : firstExistingPath(Candidates, Index + 1);
}

bool installSqliteVec(const FDependenciesPaths &Paths) {
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  const FString Source = Paths.Source / TEXT("vec0.c");
  const FString Header = Paths.Include / TEXT("sqlite-vec.h");
  return PlatformFile.FileExists(*Source) && PlatformFile.FileExists(*Header)
             ? true
             : [&]() {
                 const FString Version =
                     FString(SQLITE_VEC_RELEASE).Replace(TEXT("v"), TEXT(""));
                 const FString Url = FString::Printf(
                     TEXT("https://github.com/asg017/sqlite-vec/releases/"
                          "download/%s/sqlite-vec-%s-amalgamation.zip"),
                     SQLITE_VEC_RELEASE, *Version);
                 const FString Archive =
                     Paths.Temporary / TEXT("sqlite-vec-amalgamation.zip");
                 const FString Extracted =
                     Paths.Temporary / TEXT("sqlite-vec-extract");
                 return !downloadArchive(Url, Archive) ||
                                !DependenciesProcessAdapters::extractZip(Archive,
                                                                 Extracted)
                            ? false
                            : [&]() {
                                const FString Nested =
                                    DependenciesProcessAdapters::findSubdirWithPrefix(
                                        Extracted, TEXT("sqlite-vec-"));
                                const FString SourceCandidate =
                                    firstExistingPath(
                                        {Extracted / TEXT("sqlite-vec.c"),
                                         Extracted / TEXT("vec0.c"),
                                         Nested / TEXT("sqlite-vec.c"),
                                         Nested / TEXT("src/sqlite-vec.c")},
                                        0);
                                const FString HeaderCandidate =
                                    firstExistingPath(
                                        {Extracted / TEXT("sqlite-vec.h"),
                                         Nested / TEXT("sqlite-vec.h")},
                                        0);
                                return !SourceCandidate.IsEmpty() &&
                                       !HeaderCandidate.IsEmpty() &&
                                       copyFile(SourceCandidate, Source) &&
                                       copyFile(HeaderCandidate, Header);
                              }();
               }();
}

bool installNativeAssets(const FDependenciesPaths &Paths) {
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  PlatformFile.CreateDirectoryTree(*Paths.Include);
  PlatformFile.CreateDirectoryTree(*Paths.Source);
  PlatformFile.CreateDirectoryTree(*Paths.Temporary);
  const bool bInstalled = installSqlite(Paths) && installSqliteVec(Paths);
  IFileManager::Get().DeleteDirectory(*Paths.Temporary, true, true);
  return bInstalled;
}

} // namespace

namespace DependenciesAdapters {

FNativeDependenciesReport checkNativeDependenciesAdapter() {
  const FDependenciesPaths Paths = dependenciesPaths();
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  const bool bVectorizer =
      VectorAdapters::embedVectorAdapter(TEXT("vector-readiness")).Num() ==
      384;
  const bool bAssets =
      PlatformFile.FileExists(*(Paths.Include / TEXT("sqlite3.h"))) &&
      PlatformFile.FileExists(*(Paths.Include / TEXT("sqlite-vec.h"))) &&
      PlatformFile.FileExists(*(Paths.Source / TEXT("sqlite3.c"))) &&
      PlatformFile.FileExists(*(Paths.Source / TEXT("vec0.c")));
  const bool bVectorDb = bAssets && WITH_FORBOC_SQLITE_VEC;

  FNativeDependenciesReport Report;
  Report.Vectorizer = {TEXT("SDK native vectorizer"), bVectorizer,
                       TEXT("deterministic-384"),
                       bVectorizer ? TEXT("Vectorizer available")
                                   : TEXT("Vectorizer dimension invalid")};
  Report.VectorDb = {
      TEXT("sqlite-vec"), bVectorDb, FString(SQLITE_VEC_RELEASE),
      bVectorDb ? TEXT("Vector database available")
                : bAssets ? TEXT("Native assets installed; rebuild required")
                          : TEXT("Native sqlite-vec assets missing")};
  return Report;
}

FString clearVectorArtifactsAdapter() {
  const FString Vectors = dependenciesPaths().Vectors;
  return !FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(
             *Vectors)
             ? FString()
             : IFileManager::Get().DeleteDirectory(*Vectors, true, true)
                   ? Vectors
                   : FString();
}

FDependenciesResult setupNativeDependenciesAdapter(const FDependenciesOptions &Options) {
  const FDependenciesPaths Paths = dependenciesPaths();
  Options.bForce ? (static_cast<void>(clearVectorArtifactsAdapter()), void())
                 : void();
  const bool bInstalled = installNativeAssets(Paths);
  const FNativeDependenciesReport Report = checkNativeDependenciesAdapter();

  FDependenciesResult Result;
  Result.Vector.bOk = bInstalled && Report.Vectorizer.bAvailable &&
                      Report.VectorDb.bAvailable;
  Result.Vector.Detail =
      Result.Vector.bOk
          ? TEXT("SDK vectorizer + sqlite-vec ready")
          : Report.VectorDb.Detail;

  FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(
      *Paths.Vectors);
  const FString DatabasePath = Paths.Vectors / TEXT("forbocai_vectors.db");
  Native::Sqlite::DB Database =
      Result.Vector.bOk ? Native::Sqlite::Open(DatabasePath) : nullptr;
  Result.Memory.bOk = Database != nullptr;
  Result.Memory.Detail =
      Result.Memory.bOk ? FString::Printf(TEXT("DB ready at %s"), *DatabasePath)
                        : TEXT("Memory database initialization failed");
  Database ? (Native::Sqlite::Close(Database), void()) : void();
  return Result;
}

} // namespace DependenciesAdapters
