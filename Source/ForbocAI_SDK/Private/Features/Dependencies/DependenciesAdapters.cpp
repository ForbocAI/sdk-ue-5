#include "Features/Dependencies/DependenciesAdapters.h"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Dependencies/Download/DownloadAdapters.h"
#include "Features/Dependencies/Process/DependenciesProcessAdapters.h"
#include "Features/Dependencies/Manifest/ManifestAdapters.h"
#include "Features/Memory/Local/Storage/Sqlite/Connection/ConnectionAdapters.h"
#include "Features/Vector/VectorAdapters.h"
#include "HAL/PlatformFileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

namespace {

struct FDependenciesPaths {
  FString ThirdParty;
  FString Include;
  FString Source;
  FString Temporary;
  FString Vectors;
};

/** User Story: As a features dependencies consumer, I need to invoke dependencies paths through a stable signature so the features dependencies workflow remains explicit and composable. @fn FDependenciesPaths dependenciesPaths() */
FDependenciesPaths dependenciesPaths() {
  const Dependencies::Manifest::FManifest &Settings =
      Dependencies::Manifest::manifest();
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(Settings.PluginName);
  check(Plugin.IsValid());
  const FString ThirdParty =
      Plugin->GetBaseDir() / Settings.ThirdPartyDirectory;
  const FString Infrastructure = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectDir() / Settings.InfrastructureDirectory);
  return {ThirdParty,
          ThirdParty / Settings.IncludeDirectory,
          ThirdParty / Settings.SourceDirectory,
          Infrastructure / Settings.TemporaryDirectory,
          Infrastructure / Settings.VectorsDirectory};
}

/** User Story: As a features dependencies consumer, I need to invoke copy file through a stable signature so the features dependencies workflow remains explicit and composable. @fn bool copyFile(const FString &Source, const FString &Destination) */
bool copyFile(const FString &Source, const FString &Destination) {
  return IFileManager::Get().Copy(*Destination, *Source) == COPY_OK;
}

/** User Story: As a features dependencies consumer, I need to invoke download archive through a stable signature so the features dependencies workflow remains explicit and composable. @fn bool downloadArchive(const FString &Url, const FString &Destination) */
bool downloadArchive(const FString &Url, const FString &Destination) {
  const Dependencies::Manifest::FManifest &Settings =
      Dependencies::Manifest::manifest();
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  return PlatformFile.FileExists(*Destination)
             ? true
             : [&]() {
                 try {
                   AsyncAdapters::waitForResult(
                       Dependencies::DownloadAdapters::downloadBinary(
                           Url, Destination),
                       Settings.DownloadTimeoutSeconds);
                   return true;
                 } catch (const std::exception &Error) {
                   const FString Message = FString::Format(
                       *Settings.Readiness.DownloadFailedFormat,
                       {FString(UTF8_TO_TCHAR(Error.what()))});
                   UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
                   return false;
                 }
               }();
}

/** User Story: As a features dependencies consumer, I need to invoke install sqlite through a stable signature so the features dependencies workflow remains explicit and composable. @fn bool installSqlite(const FDependenciesPaths &Paths) */
bool installSqlite(const FDependenciesPaths &Paths) {
  const Dependencies::Manifest::FSqlite &Settings =
      Dependencies::Manifest::manifest().Sqlite;
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  const FString Header = Paths.Include / Settings.HeaderFile;
  const FString ExtensionHeader = Paths.Include / Settings.ExtensionHeaderFile;
  const FString Source = Paths.Source / Settings.SourceFile;
  return PlatformFile.FileExists(*Header) && PlatformFile.FileExists(*Source)
             ? true
             : [&]() {
                 const FString Archive =
                     Paths.Temporary / Settings.ArchiveFile;
                 const FString Extracted =
                     Paths.Temporary / Settings.ExtractDirectory;
                 const FString Url = FString::Format(
                     *Settings.UrlFormat, {Settings.ArchiveVersion});
                 return !downloadArchive(Url, Archive) ||
                                !DependenciesProcessAdapters::extractZip(Archive,
                                                                 Extracted)
                            ? false
                            : [&]() {
                                const FString Inner =
                                    DependenciesProcessAdapters::findSubdirWithPrefix(
                                        Extracted,
                                        Settings.DirectoryPrefix);
                                return !Inner.IsEmpty() &&
                                       copyFile(Inner / Settings.HeaderFile,
                                                Header) &&
                                       copyFile(Inner / Settings.ExtensionHeaderFile,
                                                ExtensionHeader) &&
                                       copyFile(Inner / Settings.SourceFile,
                                                Source);
                              }();
               }();
}

/** User Story: As a features dependencies consumer, I need to invoke resolve candidate paths through a stable signature so the features dependencies workflow remains explicit and composable. @fn TArray<FString> resolveCandidatePaths(const FString &Root, const TArray<FString> &Candidates) */
TArray<FString> resolveCandidatePaths(const FString &Root,
                                      const TArray<FString> &Candidates) {
  return func::map_array<FString, FString>(
      Candidates, [&Root](const FString &Candidate) {
        return Root / Candidate;
      });
}

/** User Story: As a features dependencies consumer, I need to invoke first existing path through a stable signature so the features dependencies workflow remains explicit and composable. @fn FString firstExistingPath(const TArray<FString> &Candidates) */
FString firstExistingPath(const TArray<FString> &Candidates) {
  return func::match(
      func::find_array<FString>(Candidates, [](const FString &Candidate) {
        return FPlatformFileManager::Get().GetPlatformFile().FileExists(
            *Candidate);
      }),
      [](const FString &Candidate) { return Candidate; },
      []() { return FString(); });
}

/** User Story: As a features dependencies consumer, I need to invoke install sqlite vec through a stable signature so the features dependencies workflow remains explicit and composable. @fn bool installSqliteVec(const FDependenciesPaths &Paths) */
bool installSqliteVec(const FDependenciesPaths &Paths) {
  const Dependencies::Manifest::FSqliteVec &Settings =
      Dependencies::Manifest::manifest().SqliteVec;
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  const FString Source = Paths.Source / Settings.SourceFile;
  const FString Header = Paths.Include / Settings.HeaderFile;
  return PlatformFile.FileExists(*Source) && PlatformFile.FileExists(*Header)
             ? true
             : [&]() {
                 const FString Version =
                     FString(Settings.Release)
                         .Replace(*Settings.VersionPrefix, *FString());
                 const FString Url = FString::Format(
                     *Settings.UrlFormat, {Settings.Release, Version});
                 const FString Archive =
                     Paths.Temporary / Settings.ArchiveFile;
                 const FString Extracted =
                     Paths.Temporary / Settings.ExtractDirectory;
                 return !downloadArchive(Url, Archive) ||
                                !DependenciesProcessAdapters::extractZip(Archive,
                                                                 Extracted)
                            ? false
                            : [&]() {
                                const FString Nested =
                                    DependenciesProcessAdapters::findSubdirWithPrefix(
                                        Extracted, Settings.DirectoryPrefix);
                                const FString SourceCandidate = firstExistingPath(
                                    func::concat_arrays<FString>({
                                        resolveCandidatePaths(
                                            Extracted,
                                            Settings.ExtractSourceCandidates),
                                        resolveCandidatePaths(
                                            Nested,
                                            Settings.NestedSourceCandidates),
                                    }));
                                const FString HeaderCandidate = firstExistingPath(
                                    func::concat_arrays<FString>({
                                        resolveCandidatePaths(
                                            Extracted,
                                            Settings.ExtractHeaderCandidates),
                                        resolveCandidatePaths(
                                            Nested,
                                            Settings.NestedHeaderCandidates),
                                    }));
                                return !SourceCandidate.IsEmpty() &&
                                       !HeaderCandidate.IsEmpty() &&
                                       copyFile(SourceCandidate, Source) &&
                                       copyFile(HeaderCandidate, Header);
                              }();
               }();
}

/** User Story: As a features dependencies consumer, I need to invoke install native assets through a stable signature so the features dependencies workflow remains explicit and composable. @fn bool installNativeAssets(const FDependenciesPaths &Paths) */
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

/** User Story: As a features dependencies consumer, I need to invoke check native dependencies adapter through a stable signature so the features dependencies workflow remains explicit and composable. @fn FNativeDependenciesReport checkNativeDependenciesAdapter() */
FNativeDependenciesReport checkNativeDependenciesAdapter() {
  const Dependencies::Manifest::FManifest &Settings =
      Dependencies::Manifest::manifest();
  const FDependenciesPaths Paths = dependenciesPaths();
  IPlatformFile &PlatformFile =
      FPlatformFileManager::Get().GetPlatformFile();
  const bool bVectorizer =
      VectorAdapters::embedVectorAdapter(Settings.Readiness.Probe).Num() ==
      Settings.Readiness.Dimension;
  const bool bAssets =
      PlatformFile.FileExists(*(Paths.Include / Settings.Sqlite.HeaderFile)) &&
      PlatformFile.FileExists(*(Paths.Include / Settings.SqliteVec.HeaderFile)) &&
      PlatformFile.FileExists(*(Paths.Source / Settings.Sqlite.SourceFile)) &&
      PlatformFile.FileExists(*(Paths.Source / Settings.SqliteVec.SourceFile));
  const bool bVectorDb = bAssets && WITH_FORBOC_SQLITE_VEC;

  FNativeDependenciesReport Report;
  Report.Vectorizer = {Settings.Readiness.VectorizerName, bVectorizer,
                       Settings.Readiness.VectorizerVersion,
                       bVectorizer ? Settings.Readiness.VectorizerReady
                                   : Settings.Readiness.VectorizerInvalid};
  Report.VectorDb = {
      Settings.Readiness.VectorDatabaseName, bVectorDb,
      Settings.SqliteVec.Release,
      bVectorDb ? Settings.Readiness.VectorDatabaseReady
                : bAssets ? Settings.Readiness.VectorDatabaseRebuild
                          : Settings.Readiness.VectorDatabaseMissing};
  return Report;
}

/** User Story: As a features dependencies consumer, I need to invoke clear vector artifacts adapter through a stable signature so the features dependencies workflow remains explicit and composable. @fn FString clearVectorArtifactsAdapter() */
FString clearVectorArtifactsAdapter() {
  const FString Vectors = dependenciesPaths().Vectors;
  return !FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(
             *Vectors)
             ? FString()
             : IFileManager::Get().DeleteDirectory(*Vectors, true, true)
                   ? Vectors
                   : FString();
}

/** User Story: As a features dependencies consumer, I need to invoke setup native dependencies adapter through a stable signature so the features dependencies workflow remains explicit and composable. @fn FDependenciesResult setupNativeDependenciesAdapter(const FDependenciesOptions &Options) */
FDependenciesResult setupNativeDependenciesAdapter(const FDependenciesOptions &Options) {
  const Dependencies::Manifest::FManifest &Settings =
      Dependencies::Manifest::manifest();
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
          ? Settings.Readiness.VectorReady
          : Report.VectorDb.Detail;

  FPlatformFileManager::Get().GetPlatformFile().CreateDirectoryTree(
      *Paths.Vectors);
  const FString DatabasePath = Paths.Vectors / Settings.DatabaseFile;
  const func::Either<FString, Native::Sqlite::DB> Database =
      Result.Vector.bOk
          ? Native::Sqlite::open(DatabasePath)
          : func::make_left<FString, Native::Sqlite::DB>(
                Settings.Readiness.DatabaseFailed);
  Result.Memory.bOk = !Database.isLeft;
  Result.Memory.Detail =
      Result.Memory.bOk
          ? FString::Format(*Settings.Readiness.DatabaseReadyFormat,
                            {DatabasePath})
          : Database.left;
  Database.isLeft ? void() : Native::Sqlite::close(Database.right);
  return Result;
}

} // namespace DependenciesAdapters
