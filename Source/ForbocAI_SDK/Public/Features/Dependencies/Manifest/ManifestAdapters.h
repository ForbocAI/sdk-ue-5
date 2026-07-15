#pragma once

#include "Features/Data/DataAdapters.h"
#include "Features/Dependencies/Manifest/ManifestTypes.h"

namespace Dependencies::Manifest {

inline const FManifest &manifest() {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(
          TEXT("ForbocAI_SDK"), TEXT("Data/dependencies/settings.json"));
  static const TSharedRef<FJsonObject> Plugin =
      DataAdapters::ReadObjectField(Source, TEXT("plugin"));
  static const TSharedRef<FJsonObject> Paths =
      DataAdapters::ReadObjectField(Source, TEXT("paths"));
  static const TSharedRef<FJsonObject> Download =
      DataAdapters::ReadObjectField(Source, TEXT("download"));
  static const TSharedRef<FJsonObject> Sqlite =
      DataAdapters::ReadObjectField(Download, TEXT("sqlite"));
  static const TSharedRef<FJsonObject> SqliteVec =
      DataAdapters::ReadObjectField(Download, TEXT("sqliteVec"));
  static const TSharedRef<FJsonObject> Readiness =
      DataAdapters::ReadObjectField(Source, TEXT("readiness"));
  static const FManifest Manifest = {
      DataAdapters::ReadStringField(Plugin, TEXT("name")),
      DataAdapters::ReadStringField(Plugin, TEXT("thirdPartyDirectory")),
      DataAdapters::ReadStringField(Paths, TEXT("infrastructureDirectory")),
      DataAdapters::ReadStringField(Paths, TEXT("includeDirectory")),
      DataAdapters::ReadStringField(Paths, TEXT("sourceDirectory")),
      DataAdapters::ReadStringField(Paths, TEXT("temporaryDirectory")),
      DataAdapters::ReadStringField(Paths, TEXT("vectorsDirectory")),
      DataAdapters::ReadStringField(Paths, TEXT("databaseFile")),
      DataAdapters::ReadFloatField(Download, TEXT("timeoutSeconds")),
      {
          DataAdapters::ReadStringField(Sqlite, TEXT("archiveVersion")),
          DataAdapters::ReadStringField(Sqlite, TEXT("headerFile")),
          DataAdapters::ReadStringField(Sqlite, TEXT("extensionHeaderFile")),
          DataAdapters::ReadStringField(Sqlite, TEXT("sourceFile")),
          DataAdapters::ReadStringField(Sqlite, TEXT("archiveFile")),
          DataAdapters::ReadStringField(Sqlite, TEXT("extractDirectory")),
          DataAdapters::ReadStringField(Sqlite, TEXT("urlFormat")),
          DataAdapters::ReadStringField(Sqlite, TEXT("directoryPrefix")),
      },
      {
          DataAdapters::ReadStringField(SqliteVec, TEXT("release")),
          DataAdapters::ReadStringField(SqliteVec, TEXT("versionPrefix")),
          DataAdapters::ReadStringField(SqliteVec, TEXT("sourceFile")),
          DataAdapters::ReadStringField(SqliteVec, TEXT("headerFile")),
          DataAdapters::ReadStringField(SqliteVec, TEXT("archiveFile")),
          DataAdapters::ReadStringField(SqliteVec, TEXT("extractDirectory")),
          DataAdapters::ReadStringField(SqliteVec, TEXT("urlFormat")),
          DataAdapters::ReadStringField(SqliteVec, TEXT("directoryPrefix")),
          DataAdapters::ReadStringArrayField(
              SqliteVec, TEXT("extractSourceCandidates")),
          DataAdapters::ReadStringArrayField(
              SqliteVec, TEXT("nestedSourceCandidates")),
          DataAdapters::ReadStringArrayField(
              SqliteVec, TEXT("extractHeaderCandidates")),
          DataAdapters::ReadStringArrayField(
              SqliteVec, TEXT("nestedHeaderCandidates")),
      },
      {
          DataAdapters::ReadStringField(Readiness, TEXT("probe")),
          DataAdapters::ReadNumberField(Readiness, TEXT("dimension")),
          DataAdapters::ReadStringField(Readiness, TEXT("vectorizerName")),
          DataAdapters::ReadStringField(Readiness, TEXT("vectorizerVersion")),
          DataAdapters::ReadStringField(Readiness, TEXT("vectorizerReady")),
          DataAdapters::ReadStringField(Readiness, TEXT("vectorizerInvalid")),
          DataAdapters::ReadStringField(Readiness, TEXT("vectorDatabaseName")),
          DataAdapters::ReadStringField(Readiness, TEXT("vectorDatabaseReady")),
          DataAdapters::ReadStringField(Readiness,
                                        TEXT("vectorDatabaseRebuild")),
          DataAdapters::ReadStringField(Readiness,
                                        TEXT("vectorDatabaseMissing")),
          DataAdapters::ReadStringField(Readiness, TEXT("vectorReady")),
          DataAdapters::ReadStringField(Readiness, TEXT("databaseReadyFormat")),
          DataAdapters::ReadStringField(Readiness, TEXT("databaseFailed")),
          DataAdapters::ReadStringField(Readiness, TEXT("downloadFailedFormat")),
      },
  };
  return Manifest;
}

} // namespace Dependencies::Manifest
