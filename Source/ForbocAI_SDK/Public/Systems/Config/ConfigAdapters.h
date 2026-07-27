#pragma once
#include "Components/AuthoredValues/AuthoredValuesTypes.h"

#include "Core/fp.hpp"
#include "Systems/Config/Contract/ContractAdapters.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#if PLATFORM_MAC || PLATFORM_UNIX
#include <sys/stat.h>
#endif

namespace ConfigSlice {

/**
 * @fn inline FString firstConfiguredValue(const TArray<FString> &Values, const FString &Fallback)
 * User Story: As configuration precedence, I need the first non-empty candidate selected without mutable branching.
 */
inline FString firstConfiguredValue(const TArray<FString> &Values,
                                    const FString &Fallback) {
  return func::or_else(
      func::find_array<FString>(Values,
                                [](const FString &Value) {
                                  return !Value.IsEmpty();
                                }),
      Fallback);
}

/**
 * @fn inline int32 readConfigNumber(const FString &Value, int32 Fallback)
 * User Story: As typed configuration, I need invalid numeric text rejected before it can replace an authored default.
 */
inline int32 readConfigNumber(const FString &Value, int32 Fallback) {
  return Value.IsNumeric() ? FCString::Atoi(*Value) : Fallback;
}

/**
 * @fn inline TMap<FString, FString> readConfigEnvironment()
 * User Story: As portable runtime configuration, I need environment access isolated at one adapter boundary.
 */
inline TMap<FString, FString> readConfigEnvironment() {
  const FConfigEnvironmentData &Keys = configRuntimeData().Environment;
  return {{Keys.HomeKey, FPlatformMisc::GetEnvironmentVariable(*Keys.HomeKey)},
          {Keys.ProfileKey,
           FPlatformMisc::GetEnvironmentVariable(*Keys.ProfileKey)},
          {Keys.ApiUrlKey,
           FPlatformMisc::GetEnvironmentVariable(*Keys.ApiUrlKey)},
          {Keys.ApiKeyKey,
           FPlatformMisc::GetEnvironmentVariable(*Keys.ApiKeyKey)},
          {Keys.DatabasePathKey,
           FPlatformMisc::GetEnvironmentVariable(*Keys.DatabasePathKey)},
          {Keys.VectorDimensionKey,
           FPlatformMisc::GetEnvironmentVariable(*Keys.VectorDimensionKey)},
          {Keys.MaxRecallResultsKey,
           FPlatformMisc::GetEnvironmentVariable(*Keys.MaxRecallResultsKey)}};
}

/**
 * @fn inline FString getConfigPath( const TMap<FString, FString> &Environment, const FString &FilePathOverride = FString())
 * User Story: As local SDK configuration, I need one portable user-scoped path with an explicit testable override.
 */
inline FString getConfigPath(
    const TMap<FString, FString> &Environment,
    const FString &FilePathOverride = FString()) {
  const FConfigRuntimeData &Data = configRuntimeData();
  const FString Home = firstConfiguredValue(
      {func::map_value_or(Environment, Data.Environment.HomeKey, FString()),
       func::map_value_or(Environment, Data.Environment.ProfileKey, FString()),
       Data.Path.HomeFallback},
      FPaths::ProjectDir());
  return !FilePathOverride.IsEmpty()
             ? FilePathOverride
             : FPaths::Combine(Home, Data.Path.FileName);
}

/**
 * @fn inline FConfigEntries readConfigEntries(const FString &FilePath)
 * User Story: As configuration hydration, I need persisted JSON normalized to string entries before it enters Redux state.
 */
inline FConfigEntries readConfigEntries(const FString &FilePath) {
  FString JsonText;
  return !FFileHelper::LoadFileToString(JsonText, *FilePath)
             ? FConfigEntries()
             : [&JsonText]() {
                 TSharedPtr<FJsonObject> Json;
                 const TSharedRef<TJsonReader<>> Reader =
                     TJsonReaderFactory<>::Create(JsonText);
                 checkf(FJsonSerializer::Deserialize(Reader, Json) &&
                            Json.IsValid(),
                        TEXT(FORBOCAI_SDK_AUTHORED_STRINGV03A110C67C3C),
                        *configRuntimeData().Errors.InvalidRoot);
                 return func::fold_array<FJsonObject::FStringType,
                                         FConfigEntries>(
                     func::map_keys<FJsonObject::FStringType,
                                    TSharedPtr<FJsonValue>>(Json->Values),
                     FConfigEntries(),
                     [&Json](FConfigEntries Entries,
                             const FJsonObject::FStringType &SharedKey) {
                       const FString Key(SharedKey.ToView());
                       const TSharedPtr<FJsonValue> Value =
                           Json->Values.FindChecked(SharedKey);
                       FString StringValue;
                       double NumberValue = FORBOCAI_SDK_AUTHORED_NUMBERV3FC4AA7B1C98;
                       const bool bString =
                           Value->TryGetString(StringValue);
                       const bool bNumber =
                           !bString && Value->TryGetNumber(NumberValue);
                       bNumber
                           ? (StringValue = FString::FromInt(
                                  static_cast<int32>(NumberValue)),
                              void())
                           : void();
                       bString || bNumber
                           ? (Entries.Add(Key, StringValue), void())
                           : void();
                       return Entries;
                     });
               }();
}

/**
 * @fn inline FConfigState readConfigState( const FConfigInvocationOverrides &Overrides, const TMap<FString, FString> &Environment, const FString &FilePathOverride = FString())
 * User Story: As the UE root store, I need invocation, environment, file, and authored defaults resolved into one typed snapshot.
 */
inline FConfigState readConfigState(
    const FConfigInvocationOverrides &Overrides,
    const TMap<FString, FString> &Environment,
    const FString &FilePathOverride = FString()) {
  const FConfigRuntimeData &Data = configRuntimeData();
  const FString FilePath = getConfigPath(Environment, FilePathOverride);
  const FConfigEntries Entries = readConfigEntries(FilePath);
  const auto Entry = [&Entries](const FString &Key) {
    return func::map_value_or(Entries, Key, FString());
  };
  const auto EnvironmentValue = [&Environment](const FString &Key) {
    return func::map_value_or(Environment, Key, FString());
  };
  return {Entries,
          Data.Fields,
          FilePath,
          Data.Defaults.SdkVersion,
          firstConfiguredValue(
              {Overrides.ApiUrl, EnvironmentValue(Data.Environment.ApiUrlKey),
               Entry(Data.Fields.ApiUrl)},
              Data.Defaults.ApiUrl),
          firstConfiguredValue(
              {Overrides.ApiKey, EnvironmentValue(Data.Environment.ApiKeyKey),
               Entry(Data.Fields.ApiKey)},
              Data.Defaults.ApiKey),
          firstConfiguredValue(
              {EnvironmentValue(Data.Environment.DatabasePathKey),
               Entry(Data.Fields.DatabasePath)},
              Data.Defaults.DatabasePath),
          readConfigNumber(firstConfiguredValue(
                               {EnvironmentValue(
                                    Data.Environment.VectorDimensionKey),
                                Entry(Data.Fields.VectorDimension)},
                               FString()),
                           Data.Defaults.VectorDimension),
          readConfigNumber(firstConfiguredValue(
                               {EnvironmentValue(
                                    Data.Environment.MaxRecallResultsKey),
                                Entry(Data.Fields.MaxRecallResults)},
                               FString()),
                           Data.Defaults.MaxRecallResults)};
}

/**
 * @fn inline FConfigState readConfigState( const FConfigInvocationOverrides &Overrides = FConfigInvocationOverrides(), const FString &FilePathOverride = FString())
 * User Story: As runtime bootstrap, I need the live environment captured once before the root store hydrates configuration.
 */
inline FConfigState readConfigState(
    const FConfigInvocationOverrides &Overrides = FConfigInvocationOverrides(),
    const FString &FilePathOverride = FString()) {
  return readConfigState(Overrides, readConfigEnvironment(), FilePathOverride);
}

/**
 * @fn inline FConfigState commitConfigEntryAdapter( const FConfigState &State, const FConfigEntryCommitted &Entry)
 * User Story: As a Config reducer, I need a pure state transition that keeps persisted entries and typed runtime fields synchronized.
 */
inline FConfigState commitConfigEntryAdapter(
    const FConfigState &State, const FConfigEntryCommitted &Entry) {
  const FConfigFieldData &Fields = State.Fields;
  FConfigState Next = State;
  Next.Entries = func::upsert_map_value<FString, FString>(
      State.Entries, Entry.Key, Entry.Value,
      [&Entry](const FString &) { return Entry.Value; });
  Next.ApiUrl = Entry.Key == Fields.ApiUrl ? Entry.Value : State.ApiUrl;
  Next.ApiKey = Entry.Key == Fields.ApiKey ? Entry.Value : State.ApiKey;
  Next.DatabasePath = Entry.Key == Fields.DatabasePath ? Entry.Value
                                                       : State.DatabasePath;
  Next.VectorDimension = Entry.Key == Fields.VectorDimension
                             ? readConfigNumber(Entry.Value,
                                                State.VectorDimension)
                             : State.VectorDimension;
  Next.MaxRecallResults = Entry.Key == Fields.MaxRecallResults
                              ? readConfigNumber(Entry.Value,
                                                 State.MaxRecallResults)
                              : State.MaxRecallResults;
  return Next;
}

/**
 * @fn inline FConfigState commitApiConfigurationAdapter( const FConfigState &State, const FConfigApiCommitted &Config)
 * User Story: As a host invocation, I need API overrides reduced into runtime state without mutating environment or persisted entries.
 */
inline FConfigState commitApiConfigurationAdapter(
    const FConfigState &State, const FConfigApiCommitted &Config) {
  FConfigState Next = State;
  Next.ApiUrl = Config.ApiUrl.IsEmpty() ? State.ApiUrl : Config.ApiUrl;
  Next.ApiKey = Config.ApiKey;
  return Next;
}

/**
 * @fn inline bool writeConfigState(const FConfigState &State)
 * User Story: As Config persistence, I need reducer-owned entries written at one filesystem adapter boundary with private permissions where supported.
 */
inline bool writeConfigState(const FConfigState &State) {
  const FString Directory = FPaths::GetPath(State.FilePath);
  !Directory.IsEmpty()
      ? (IFileManager::Get().MakeDirectory(*Directory, true), void())
      : void();
  const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
  func::for_each_array<FString>(
      func::map_keys(State.Entries),
      [&Json, &State](const FString &Key) {
        Json->SetStringField(Key, State.Entries.FindChecked(Key));
      });
  FString JsonText;
  const TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&JsonText);
  const bool bSerialized = FJsonSerializer::Serialize(Json, Writer);
  const bool bSaved =
      bSerialized && FFileHelper::SaveStringToFile(JsonText, *State.FilePath);
#if PLATFORM_MAC || PLATFORM_UNIX
  bSaved ? (void)chmod(TCHAR_TO_UTF8(*State.FilePath),
                       configRuntimeData().Serialization.FileMode)
         : (void)FORBOCAI_SDK_AUTHORED_NUMBERV60732C8368BA;
#endif
  return bSaved;
}

} // namespace ConfigSlice
