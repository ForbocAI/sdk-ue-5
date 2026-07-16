#pragma once

#include "Features/Config/State/StateAdapters.h"

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#if PLATFORM_MAC || PLATFORM_UNIX
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace SDKConfig {

/** User Story: As a features config file consumer, I need to invoke resolve home dir through a stable signature so the features config file workflow remains explicit and composable. @fn inline FString ResolveHomeDir() */
inline FString ResolveHomeDir() {
  const FString Home = FPlatformMisc::GetEnvironmentVariable(TEXT("HOME"));
  const FString Profile =
      FPlatformMisc::GetEnvironmentVariable(TEXT("USERPROFILE"));
  return !Home.IsEmpty()
             ? Home
             : (!Profile.IsEmpty() ? Profile : FPaths::ProjectDir());
}

/** User Story: As a features config file consumer, I need to invoke get config file path through a stable signature so the features config file workflow remains explicit and composable. @fn inline FString GetConfigFilePath() */
inline FString GetConfigFilePath() {
  return !ConfigFilePathOverrideStorage().IsEmpty()
             ? ConfigFilePathOverrideStorage()
             : FPaths::Combine(ResolveHomeDir(), TEXT(".forbocai.json"));
}

/** User Story: As a features config file consumer, I need to invoke load config json object through a stable signature so the features config file workflow remains explicit and composable. @fn inline TSharedPtr<FJsonObject> LoadConfigJsonObject() */
inline TSharedPtr<FJsonObject> LoadConfigJsonObject() {
  const FString ConfigPath = GetConfigFilePath();
  FString JsonString;
  return !FFileHelper::LoadFileToString(JsonString, *ConfigPath)
             ? MakeShared<FJsonObject>()
             : [&JsonString]() {
                 TSharedPtr<FJsonObject> JsonObject;
                 const TSharedRef<TJsonReader<>> Reader =
                     TJsonReaderFactory<>::Create(JsonString);
                 return (FJsonSerializer::Deserialize(Reader, JsonObject) &&
                         JsonObject.IsValid())
                            ? JsonObject
                            : MakeShared<FJsonObject>();
               }();
}

/**
 * User Story: As a features config file consumer, I need to invoke write config json object through a stable signature so the features config file workflow remains explicit and composable.
 * @fn inline bool WriteConfigJsonObject( const TSharedRef<FJsonObject> &JsonObject)
 */
inline bool WriteConfigJsonObject(
    const TSharedRef<FJsonObject> &JsonObject) {
  const FString ConfigPath = GetConfigFilePath();
  const FString Directory = FPaths::GetPath(ConfigPath);
  !Directory.IsEmpty()
      ? (IFileManager::Get().MakeDirectory(*Directory, true), void())
      : void();

  FString JsonString;
  const TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&JsonString);
  FJsonSerializer::Serialize(JsonObject, Writer);
  const bool bSaved = FFileHelper::SaveStringToFile(JsonString, *ConfigPath);
#if PLATFORM_MAC || PLATFORM_UNIX
  bSaved ? (void)chmod(TCHAR_TO_UTF8(*ConfigPath), 0600) : (void)0;
#endif
  return bSaved;
}

} // namespace SDKConfig
