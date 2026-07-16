#pragma once

#include "CoreMinimal.h"

namespace MemoryConfiguration {

struct FIterationData {
  int32 InitialIndex;
  int32 Step;
  int32 SqliteVariableLength;
};

struct FDefaultsData {
  FString MemoryType;
  float Importance;
  float Similarity;
  int32 QueryLimit;
  int32 RecallLimit;
  float RecallThreshold;
  int32 ListLimit;
  int32 ListOffset;
  int32 MaxMemories;
};

struct FStatusData {
  FString Idle;
  FString Storing;
  FString Recalling;
  FString Error;
};

struct FTextData {
  FString Empty;
  FString Space;
};

struct FIdentityData {
  FString MemoryPrefix;
};

struct FVectorData {
  int32 Dimension;
  uint32 HashSeed;
  uint32 HashPrime;
  FString TokenPattern;
  FString TokenFlags;
  FString FeatureSeparator;
  uint32 SignMask;
  uint32 EvenRemainder;
  float PositiveWeight;
  float NegativeWeight;
};

struct FRegistryData {
  FString NodeMemory;
  FString BrowserMemory;
  FString BrowserMemoryMeta;
  FString Vector;
  FString VectorDatabase;
  FString Vectorizer;
  FString VectorizerImplementation;
};

struct FStoragePathsData {
  FString InfrastructureDirectory;
  FString VectorsDirectory;
  FString DatabaseNamePattern;
};

struct FStorageFieldsData {
  FString Id;
  FString Text;
  FString Timestamp;
  FString Type;
  FString Importance;
  FString Vector;
  FString Embedding;
  FString Distance;
};

struct FLanceData {
  FString DefaultDatabase;
  FString Table;
  FString DatabaseExtension;
  FString TableExtension;
};

struct FOramaSchemaData {
  FString Id;
  FString Text;
  FString Timestamp;
  FString Type;
  FString Importance;
  FString Embedding;
};

struct FOramaData {
  FString DefaultDatabase;
  FString SearchMode;
  FOramaSchemaData Schema;
};

struct FSqliteColumnsData {
  int32 Id;
  int32 Text;
  int32 Type;
  int32 Importance;
  int32 Timestamp;
  int32 Distance;
};

struct FSqliteBindingsData {
  int32 Id;
  int32 Text;
  int32 Type;
  int32 Importance;
  int32 Timestamp;
  int32 Embedding;
  int32 QueryEmbedding;
  int32 QueryLimit;
  int32 ListLimit;
  int32 ListOffset;
};

struct FSqliteData {
  FString DefaultDatabase;
  FString DatabaseExtension;
  TArray<FString> SidecarExtensions;
  FString InMemoryPath;
  FString UnsafePathSegment;
  FString JsonOpen;
  FString JsonClose;
  FString JsonSeparator;
  FString CreateVectorTable;
  FString Clear;
  FString Upsert;
  FString Search;
  FString List;
  FSqliteColumnsData Columns;
  FSqliteBindingsData Bindings;
  float DistanceOrigin;
};

struct FStorageData {
  FStoragePathsData Paths;
  FStorageFieldsData Fields;
  FLanceData Lance;
  FOramaData Orama;
  FSqliteData Sqlite;
};

struct FRuntimeTypesData {
  FString Undefined;
  FString String;
  FString Number;
};

struct FErrorsData {
  FString BrowserEnvironmentRequired;
  FString BrowserInitializationFailed;
  FString BrowserStoreFailed;
  FString BrowserRecallFailed;
  FString BrowserDatabaseMissing;
  FString BrowserSearchModeInvalid;
  FString NodeInitializationFailed;
  FString NodeStoreFailed;
  FString NodeRecallFailed;
  FString NodeClearFailed;
  FString InvalidDatabaseName;
  FString DatabasePathEscape;
  FString InvalidVectorRecord;
  FString EmptyQuery;
  FString LocalInitializationFailed;
  FString LocalNotInitialized;
  FString LocalStoreFailed;
  FString LocalRecallFailed;
  FString LocalClearFailed;
  FString SqliteUnavailable;
  FString SqlitePathRejected;
  FString SqliteDirectoryFailed;
  FString SqliteOpenFailed;
  FString SqliteExtensionFailed;
  FString SqliteSchemaFailed;
  FString SqlitePrepareFailed;
  FString SqliteBindFailed;
  FString SqliteStepFailed;
  FString SqliteInvalidLimit;
  FString SqliteInvalidOffset;
};

struct FMemoryData {
  FIterationData Iteration;
  FDefaultsData Defaults;
  FStatusData Status;
  FTextData Text;
  FIdentityData Identity;
  FVectorData Vector;
  FRegistryData Registry;
  FStorageData Storage;
  FRuntimeTypesData RuntimeTypes;
  FErrorsData Errors;
};

} // namespace MemoryConfiguration
