#pragma once

#include "CoreMinimal.h"

namespace Dependencies::Manifest {

struct FSqlite {
  FString ArchiveVersion;
  FString HeaderFile;
  FString ExtensionHeaderFile;
  FString SourceFile;
  FString ArchiveFile;
  FString ExtractDirectory;
  FString UrlFormat;
  FString DirectoryPrefix;
};

struct FSqliteVec {
  FString Release;
  FString VersionPrefix;
  FString SourceFile;
  FString HeaderFile;
  FString ArchiveFile;
  FString ExtractDirectory;
  FString UrlFormat;
  FString DirectoryPrefix;
  TArray<FString> ExtractSourceCandidates;
  TArray<FString> NestedSourceCandidates;
  TArray<FString> ExtractHeaderCandidates;
  TArray<FString> NestedHeaderCandidates;
};

struct FReadiness {
  FString Probe;
  int32 Dimension;
  FString VectorizerName;
  FString VectorizerVersion;
  FString VectorizerReady;
  FString VectorizerInvalid;
  FString VectorDatabaseName;
  FString VectorDatabaseReady;
  FString VectorDatabaseRebuild;
  FString VectorDatabaseMissing;
  FString VectorReady;
  FString DatabaseReadyFormat;
  FString DatabaseFailed;
  FString DownloadFailedFormat;
};

struct FManifest {
  FString PluginName;
  FString ThirdPartyDirectory;
  FString InfrastructureDirectory;
  FString IncludeDirectory;
  FString SourceDirectory;
  FString TemporaryDirectory;
  FString VectorsDirectory;
  FString DatabaseFile;
  float DownloadTimeoutSeconds;
  FSqlite Sqlite;
  FSqliteVec SqliteVec;
  FReadiness Readiness;
};

} // namespace Dependencies::Manifest
