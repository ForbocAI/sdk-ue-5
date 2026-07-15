#include "Features/Dependencies/DependenciesAdapters.h"
#include "Features/Async/AsyncAdapters.h"
#include "Features/Memory/Local/Storage/StorageAdapters.h"
#include "Features/Dependencies/Process/DependenciesProcessAdapters.h"
#include "Features/Dependencies/Manifest/ManifestAdapters.h"
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

FDependenciesPaths dependenciesPaths() {
  const Dependencies::Manifest::FManifest &Settings =
      Dependencies::Manifest::manifest();
  const TSharedPtr<IPlugin> Plugin =
      IPluginManager::Get().FindPlugin(Settings.PluginName);
  check(Plugin.IsValid());
  const FString ThirdParty =
      Plugin->GetBaseDir() / Settings.ThirdPartyDirectory;
  const FString Infrastructure =
      FPaths::ProjectDir() / Settings.InfrastructureDirectory;
  return {ThirdParty,
          ThirdParty / Settings.IncludeDirectory,
          ThirdParty / Settings.SourceDirectory,
          Infrastructure / Settings.TemporaryDirectory,
          Infrastructure / Settings.VectorsDirectory};
}

bool copyFile(const FString &Source, const FString &Destination) {
  return IFileManager::Get().Copy(*Destination, *Source) == COPY_OK;
}

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
                       Native::File::DownloadBinary(Url, Destination),
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

TArray<FString> resolveCandidatePaths(const FString &Root,
                                      const TArray<FString> &Candidates) {
  return func::map_array<FString, FString>(
      Candidates, [&Root](const FString &Candidate) {
        return Root / Candidate;
      });
}

FString firstExistingPath(const TArray<FString> &Candidates) {
  return func::match(
      func::find_array<FString>(Candidates, [](const FString &Candidate) {
        return FPlatformFileManager::Get().GetPlatformFile().FileExists(
            *Candidate);
      }),
      [](const FString &Candidate) { return Candidate; },
      []() { return FString(); });
}

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
  Native::Sqlite::DB Database =
      Result.Vector.bOk ? Native::Sqlite::Open(DatabasePath) : nullptr;
  Result.Memory.bOk = Database != nullptr;
  Result.Memory.Detail =
      Result.Memory.bOk
          ? FString::Format(*Settings.Readiness.DatabaseReadyFormat,
                            {DatabasePath})
          : Settings.Readiness.DatabaseFailed;
  Database ? (Native::Sqlite::Close(Database), void()) : void();
  return Result;
}

} // namespace DependenciesAdapters
