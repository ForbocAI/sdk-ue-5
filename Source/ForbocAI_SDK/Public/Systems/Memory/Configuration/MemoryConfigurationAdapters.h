#pragma once

#include "ForbocAI_SDK/Public/Systems/Data/DataAdapters.h"
#include "Components/Memory/Configuration/MemoryConfigurationTypes.h"

namespace MemoryConfiguration {

/** User Story: As memory execution, I need one immutable authored contract shared with TS so vector, storage, status, and error policy cannot drift between SDKs. @fn inline const FMemoryData &memoryData() */
inline const FMemoryData &memoryData() {
  static const FMemoryData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("ForbocAI_SDK"),
                                     TEXT("Data/memory/runtime.json"));
    const TSharedRef<FJsonObject> Iteration =
        DataAdapters::ReadObjectField(Source, TEXT("iteration"));
    const TSharedRef<FJsonObject> Defaults =
        DataAdapters::ReadObjectField(Source, TEXT("defaults"));
    const TSharedRef<FJsonObject> Status =
        DataAdapters::ReadObjectField(Source, TEXT("status"));
    const TSharedRef<FJsonObject> Text =
        DataAdapters::ReadObjectField(Source, TEXT("text"));
    const TSharedRef<FJsonObject> Identity =
        DataAdapters::ReadObjectField(Source, TEXT("identity"));
    const TSharedRef<FJsonObject> Contract =
        DataAdapters::ReadObjectField(Source, TEXT("contract"));
    const TSharedRef<FJsonObject> ContractFields =
        DataAdapters::ReadObjectField(Contract, TEXT("fields"));
    const TSharedRef<FJsonObject> Vector =
        DataAdapters::ReadObjectField(Source, TEXT("vector"));
    const TSharedRef<FJsonObject> Registry =
        DataAdapters::ReadObjectField(Source, TEXT("registry"));
    const TSharedRef<FJsonObject> Storage =
        DataAdapters::ReadObjectField(Source, TEXT("storage"));
    const TSharedRef<FJsonObject> Paths =
        DataAdapters::ReadObjectField(Storage, TEXT("paths"));
    const TSharedRef<FJsonObject> Fields =
        DataAdapters::ReadObjectField(Storage, TEXT("fields"));
    const TSharedRef<FJsonObject> Lance =
        DataAdapters::ReadObjectField(Storage, TEXT("lance"));
    const TSharedRef<FJsonObject> Orama =
        DataAdapters::ReadObjectField(Storage, TEXT("orama"));
    const TSharedRef<FJsonObject> OramaSchema =
        DataAdapters::ReadObjectField(Orama, TEXT("schema"));
    const TSharedRef<FJsonObject> Sqlite =
        DataAdapters::ReadObjectField(Storage, TEXT("sqlite"));
    const TSharedRef<FJsonObject> Columns =
        DataAdapters::ReadObjectField(Sqlite, TEXT("columns"));
    const TSharedRef<FJsonObject> Bindings =
        DataAdapters::ReadObjectField(Sqlite, TEXT("bindings"));
    const TSharedRef<FJsonObject> RuntimeTypes =
        DataAdapters::ReadObjectField(Source, TEXT("runtimeTypes"));
    const TSharedRef<FJsonObject> Errors =
        DataAdapters::ReadObjectField(Source, TEXT("errors"));

    return FMemoryData{
        {DataAdapters::ReadNumberField(Iteration, TEXT("initialIndex")),
         DataAdapters::ReadNumberField(Iteration, TEXT("step")),
         DataAdapters::ReadNumberField(Iteration,
                                       TEXT("sqliteVariableLength"))},
        {DataAdapters::ReadStringField(Defaults, TEXT("memoryType")),
         DataAdapters::ReadFloatField(Defaults, TEXT("importance")),
         DataAdapters::ReadFloatField(Defaults, TEXT("similarity")),
         DataAdapters::ReadNumberField(Defaults, TEXT("queryLimit")),
         DataAdapters::ReadNumberField(Defaults, TEXT("recallLimit")),
         DataAdapters::ReadFloatField(Defaults, TEXT("recallThreshold")),
         DataAdapters::ReadNumberField(Defaults, TEXT("listLimit")),
         DataAdapters::ReadNumberField(Defaults, TEXT("listOffset")),
         DataAdapters::ReadNumberField(Defaults, TEXT("maxMemories"))},
        {DataAdapters::ReadStringField(Status, TEXT("idle")),
         DataAdapters::ReadStringField(Status, TEXT("storing")),
         DataAdapters::ReadStringField(Status, TEXT("recalling")),
         DataAdapters::ReadStringField(Status, TEXT("error"))},
        {DataAdapters::ReadStringField(Text, TEXT("empty")),
         DataAdapters::ReadStringField(Text, TEXT("space"))},
        {DataAdapters::ReadStringField(Identity, TEXT("memoryPrefix"))},
        {DataAdapters::ReadNumberField(Contract, TEXT("schemaVersion")),
         DataAdapters::ReadNumberField(Contract, TEXT("vectorVersion")),
         DataAdapters::ReadNumberField(Contract,
                                       TEXT("legacyVectorVersion")),
         DataAdapters::ReadStringField(Contract, TEXT("metadataFile")),
         DataAdapters::ReadStringField(Contract, TEXT("temporarySuffix")),
         DataAdapters::ReadStringField(Contract, TEXT("textEncoding")),
         DataAdapters::ReadStringField(Contract, TEXT("lineTerminator")),
         DataAdapters::ReadNumberField(Contract, TEXT("jsonIndent")),
         DataAdapters::ReadNumberField(Contract, TEXT("migrationBatchSize")),
         {DataAdapters::ReadStringField(ContractFields,
                                        TEXT("schemaVersion")),
          DataAdapters::ReadStringField(ContractFields,
                                        TEXT("vectorVersion"))}},
        {DataAdapters::ReadNumberField(Vector, TEXT("dimension")),
         DataAdapters::ReadUInt32Field(Vector, TEXT("hashSeed")),
         DataAdapters::ReadUInt32Field(Vector, TEXT("hashPrime")),
         DataAdapters::ReadStringField(Vector, TEXT("tokenPattern")),
         DataAdapters::ReadStringField(Vector, TEXT("tokenFlags")),
         DataAdapters::ReadStringField(Vector, TEXT("featureSeparator")),
         DataAdapters::ReadStringField(Vector, TEXT("unigramPrefix")),
         DataAdapters::ReadStringField(Vector, TEXT("bigramPrefix")),
         DataAdapters::ReadStringField(Vector, TEXT("characterPrefix")),
         DataAdapters::ReadNumberField(Vector, TEXT("characterNgramSize")),
         DataAdapters::ReadNumberField(
             Vector, TEXT("minimumCharacterTokenLength")),
         DataAdapters::ReadFloatField(Vector, TEXT("unigramWeight")),
         DataAdapters::ReadFloatField(Vector, TEXT("bigramWeight")),
         DataAdapters::ReadFloatField(Vector, TEXT("characterNgramWeight")),
         DataAdapters::ReadStringArrayField(Vector, TEXT("stopWords")),
         DataAdapters::ReadUInt32Field(Vector, TEXT("signMask")),
         DataAdapters::ReadUInt32Field(Vector, TEXT("evenRemainder")),
         DataAdapters::ReadFloatField(Vector, TEXT("positiveWeight")),
         DataAdapters::ReadFloatField(Vector, TEXT("negativeWeight"))},
        {DataAdapters::ReadStringField(Registry, TEXT("nodeMemory")),
         DataAdapters::ReadStringField(Registry, TEXT("browserMemory")),
         DataAdapters::ReadStringField(Registry, TEXT("browserMemoryMeta")),
         DataAdapters::ReadStringField(Registry, TEXT("vector")),
         DataAdapters::ReadStringField(Registry, TEXT("vectorDatabase")),
         DataAdapters::ReadStringField(Registry, TEXT("vectorizer")),
         DataAdapters::ReadStringField(Registry,
                                       TEXT("vectorizerImplementation"))},
         {{DataAdapters::ReadStringField(Paths,
                                        TEXT("infrastructureDirectory")),
         DataAdapters::ReadStringField(Paths, TEXT("vectorsDirectory")),
          DataAdapters::ReadStringField(Paths,
                                        TEXT("databaseNamePattern"))},
         {DataAdapters::ReadStringField(Fields, TEXT("id")),
          DataAdapters::ReadStringField(Fields, TEXT("text")),
          DataAdapters::ReadStringField(Fields, TEXT("timestamp")),
          DataAdapters::ReadStringField(Fields, TEXT("type")),
          DataAdapters::ReadStringField(Fields, TEXT("importance")),
          DataAdapters::ReadStringField(Fields, TEXT("vector")),
          DataAdapters::ReadStringField(Fields, TEXT("embedding")),
          DataAdapters::ReadStringField(Fields, TEXT("distance"))},
         {DataAdapters::ReadStringField(Lance, TEXT("defaultDatabase")),
          DataAdapters::ReadStringField(Lance, TEXT("table")),
          DataAdapters::ReadStringField(Lance, TEXT("databaseExtension")),
          DataAdapters::ReadStringField(Lance, TEXT("tableExtension")),
          DataAdapters::ReadStringField(Lance, TEXT("distanceType")),
          DataAdapters::ReadStringField(Lance, TEXT("overwriteMode"))},
         {DataAdapters::ReadStringField(Orama, TEXT("defaultDatabase")),
          DataAdapters::ReadStringField(Orama, TEXT("searchMode")),
          {DataAdapters::ReadStringField(OramaSchema, TEXT("id")),
           DataAdapters::ReadStringField(OramaSchema, TEXT("text")),
           DataAdapters::ReadStringField(OramaSchema, TEXT("timestamp")),
           DataAdapters::ReadStringField(OramaSchema, TEXT("type")),
           DataAdapters::ReadStringField(OramaSchema, TEXT("importance")),
           DataAdapters::ReadStringField(OramaSchema, TEXT("embedding"))}},
         {DataAdapters::ReadStringField(Sqlite, TEXT("defaultDatabase")),
          DataAdapters::ReadStringField(Sqlite, TEXT("databaseExtension")),
          DataAdapters::ReadStringArrayField(Sqlite,
                                             TEXT("sidecarExtensions")),
          DataAdapters::ReadStringField(Sqlite, TEXT("inMemoryPath")),
          DataAdapters::ReadStringField(Sqlite, TEXT("unsafePathSegment")),
          DataAdapters::ReadStringField(Sqlite, TEXT("jsonOpen")),
          DataAdapters::ReadStringField(Sqlite, TEXT("jsonClose")),
          DataAdapters::ReadStringField(Sqlite, TEXT("jsonSeparator")),
          DataAdapters::ReadStringField(Sqlite, TEXT("createVectorTable")),
          DataAdapters::ReadStringField(Sqlite, TEXT("tableExists")),
          DataAdapters::ReadStringField(Sqlite, TEXT("readUserVersion")),
          DataAdapters::ReadStringField(Sqlite, TEXT("writeUserVersion")),
          DataAdapters::ReadStringField(Sqlite, TEXT("listAll")),
          DataAdapters::ReadStringField(Sqlite, TEXT("dropVectorTable")),
          DataAdapters::ReadStringField(Sqlite, TEXT("beginTransaction")),
          DataAdapters::ReadStringField(Sqlite, TEXT("commitTransaction")),
          DataAdapters::ReadStringField(Sqlite, TEXT("rollbackTransaction")),
          DataAdapters::ReadStringField(Sqlite, TEXT("clear")),
          DataAdapters::ReadStringField(Sqlite, TEXT("upsert")),
          DataAdapters::ReadStringField(Sqlite, TEXT("search")),
          DataAdapters::ReadStringField(Sqlite, TEXT("list")),
          {DataAdapters::ReadNumberField(Columns, TEXT("id")),
           DataAdapters::ReadNumberField(Columns, TEXT("text")),
           DataAdapters::ReadNumberField(Columns, TEXT("type")),
           DataAdapters::ReadNumberField(Columns, TEXT("importance")),
           DataAdapters::ReadNumberField(Columns, TEXT("timestamp")),
           DataAdapters::ReadNumberField(Columns, TEXT("distance"))},
          {DataAdapters::ReadNumberField(Bindings, TEXT("id")),
           DataAdapters::ReadNumberField(Bindings, TEXT("text")),
           DataAdapters::ReadNumberField(Bindings, TEXT("type")),
           DataAdapters::ReadNumberField(Bindings, TEXT("importance")),
           DataAdapters::ReadNumberField(Bindings, TEXT("timestamp")),
           DataAdapters::ReadNumberField(Bindings, TEXT("embedding")),
           DataAdapters::ReadNumberField(Bindings, TEXT("queryEmbedding")),
           DataAdapters::ReadNumberField(Bindings, TEXT("queryLimit")),
           DataAdapters::ReadNumberField(Bindings, TEXT("listLimit")),
           DataAdapters::ReadNumberField(Bindings, TEXT("listOffset"))},
          DataAdapters::ReadFloatField(Sqlite, TEXT("distanceOrigin"))}},
        {DataAdapters::ReadStringField(RuntimeTypes, TEXT("undefined")),
         DataAdapters::ReadStringField(RuntimeTypes, TEXT("string")),
         DataAdapters::ReadStringField(RuntimeTypes, TEXT("number"))},
        {DataAdapters::ReadStringField(Errors,
                                       TEXT("browserEnvironmentRequired")),
         DataAdapters::ReadStringField(Errors,
                                       TEXT("browserInitializationFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("browserStoreFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("browserRecallFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("browserDatabaseMissing")),
         DataAdapters::ReadStringField(Errors,
                                       TEXT("browserSearchModeInvalid")),
         DataAdapters::ReadStringField(Errors, TEXT("nodeInitializationFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("nodeStoreFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("nodeRecallFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("nodeClearFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("invalidDatabaseName")),
         DataAdapters::ReadStringField(Errors, TEXT("databasePathEscape")),
         DataAdapters::ReadStringField(Errors, TEXT("invalidVectorRecord")),
         DataAdapters::ReadStringField(Errors, TEXT("emptyQuery")),
         DataAdapters::ReadStringField(Errors,
                                       TEXT("localInitializationFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("localNotInitialized")),
         DataAdapters::ReadStringField(Errors, TEXT("localStoreFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("localRecallFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("localClearFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteUnavailable")),
         DataAdapters::ReadStringField(Errors, TEXT("sqlitePathRejected")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteDirectoryFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteOpenFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteExtensionFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteSchemaFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqlitePrepareFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteBindFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteStepFailed")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteInvalidLimit")),
         DataAdapters::ReadStringField(Errors, TEXT("sqliteInvalidOffset")),
         DataAdapters::ReadStringField(Errors, TEXT("memoryContractInvalid")),
         DataAdapters::ReadStringField(Errors, TEXT("memoryContractNewer")),
         DataAdapters::ReadStringField(
             Errors, TEXT("memoryContractMigrationFailed"))}};
  }();
  return Data;
}

} // namespace MemoryConfiguration
