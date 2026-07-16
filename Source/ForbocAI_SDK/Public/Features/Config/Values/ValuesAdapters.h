#pragma once

#include "Core/fp.hpp"
#include "Features/Config/File/FileAdapters.h"

namespace SDKConfig {

/** User Story: As a features config values consumer, I need to invoke load from config file through a stable signature so the features config values workflow remains explicit and composable. @fn inline void LoadFromConfigFile() */
inline void LoadFromConfigFile() {
  const TSharedPtr<FJsonObject> Json = LoadConfigJsonObject();
  !Json.IsValid()
      ? void()
      : [&Json]() {
          FString StringValue;
          (Json->TryGetStringField(TEXT("apiUrl"), StringValue) &&
           !StringValue.IsEmpty())
              ? (void)(ApiUrlStorage() = StringValue)
              : (void)0;
          (Json->TryGetStringField(TEXT("apiKey"), StringValue) &&
           !StringValue.IsEmpty())
              ? (void)(ApiKeyStorage() = StringValue)
              : (void)0;
          (Json->TryGetStringField(TEXT("databasePath"), StringValue) &&
           !StringValue.IsEmpty())
              ? (void)(DatabasePathStorage() = StringValue)
              : (void)0;
          int32 NumberValue = 0;
          Json->TryGetNumberField(TEXT("vectorDimension"), NumberValue)
              ? (void)(VectorDimensionStorage() = NumberValue)
              : (void)0;
          Json->TryGetNumberField(TEXT("maxRecallResults"), NumberValue)
              ? (void)(MaxRecallResultsStorage() = NumberValue)
              : (void)0;
        }();
}

/** User Story: As a features config values consumer, I need to invoke save to config file through a stable signature so the features config values workflow remains explicit and composable. @fn inline bool SaveToConfigFile() */
inline bool SaveToConfigFile() {
  EnsureInitialized();
  const TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
  Json->SetStringField(TEXT("apiUrl"), ApiUrlStorage());
  !ApiKeyStorage().IsEmpty()
      ? Json->SetStringField(TEXT("apiKey"), ApiKeyStorage())
      : (void)0;
  !DatabasePathStorage().IsEmpty()
      ? Json->SetStringField(TEXT("databasePath"), DatabasePathStorage())
      : (void)0;
  Json->SetNumberField(TEXT("vectorDimension"), VectorDimensionStorage());
  Json->SetNumberField(TEXT("maxRecallResults"), MaxRecallResultsStorage());
  return WriteConfigJsonObject(Json);
}

/** User Story: As a features config values consumer, I need to invoke set config value through a stable signature so the features config values workflow remains explicit and composable. @fn inline void SetConfigValue(const FString &Key, const FString &Value) */
inline void SetConfigValue(const FString &Key, const FString &Value) {
  TSharedPtr<FJsonObject> Json = LoadConfigJsonObject();
  Json = Json.IsValid() ? Json : MakeShared<FJsonObject>();
  const bool bHandled = func::or_else(
      func::multi_match<FString, bool>(
          Key,
          {
              func::when<FString, bool>(
                  func::equals<FString>(FString(TEXT("apiUrl"))),
                  [&Json, &Value](const FString &) {
                    Json->SetStringField(TEXT("apiUrl"), Value);
                    return true;
                  }),
              func::when<FString, bool>(
                  func::equals<FString>(FString(TEXT("apiKey"))),
                  [&Json, &Value](const FString &) {
                    Json->SetStringField(TEXT("apiKey"), Value);
                    return true;
                  }),
              func::when<FString, bool>(
                  func::equals<FString>(FString(TEXT("databasePath"))),
                  [&Json, &Value](const FString &) {
                    Json->SetStringField(TEXT("databasePath"), Value);
                    return true;
                  }),
              func::when<FString, bool>(
                  func::equals<FString>(FString(TEXT("vectorDimension"))),
                  [&Json, &Value](const FString &) {
                    Json->SetNumberField(TEXT("vectorDimension"),
                                         FCString::Atoi(*Value));
                    return true;
                  }),
              func::when<FString, bool>(
                  func::equals<FString>(FString(TEXT("maxRecallResults"))),
                  [&Json, &Value](const FString &) {
                    Json->SetNumberField(TEXT("maxRecallResults"),
                                         FCString::Atoi(*Value));
                    return true;
                  }),
          }),
      false);
  bHandled && WriteConfigJsonObject(Json.ToSharedRef())
      ? (ReloadConfig(), void())
      : void();
}

/** User Story: As a features config values consumer, I need to invoke get config value through a stable signature so the features config values workflow remains explicit and composable. @fn inline FString GetConfigValue(const FString &Key) */
inline FString GetConfigValue(const FString &Key) {
  return Key == TEXT("version")
             ? GetSdkVersion()
             : [&Key]() {
                 const TSharedPtr<FJsonObject> Json = LoadConfigJsonObject();
                 const auto ReadString = [&Json](const FString &Field) {
                   FString Value;
                   return Json->TryGetStringField(Field, Value)
                              ? Value
                              : FString(TEXT(""));
                 };
                 const auto ReadNumber = [&Json](const FString &Field) {
                   int32 Value = 0;
                   return Json->TryGetNumberField(Field, Value)
                              ? FString::FromInt(Value)
                              : FString(TEXT(""));
                 };
                 return !Json.IsValid()
                            ? FString(TEXT(""))
                            : func::or_else(
                                  func::multi_match<FString, FString>(
                                      Key,
                                      {
                                          func::when<FString, FString>(
                                              func::equals<FString>(FString(
                                                  TEXT("apiUrl"))),
                                              [&ReadString](const FString &) {
                                                return ReadString(TEXT("apiUrl"));
                                              }),
                                          func::when<FString, FString>(
                                              func::equals<FString>(FString(
                                                  TEXT("apiKey"))),
                                              [&ReadString](const FString &) {
                                                return ReadString(TEXT("apiKey"));
                                              }),
                                          func::when<FString, FString>(
                                              func::equals<FString>(FString(
                                                  TEXT("databasePath"))),
                                              [&ReadString](const FString &) {
                                                return ReadString(
                                                    TEXT("databasePath"));
                                              }),
                                          func::when<FString, FString>(
                                              func::equals<FString>(FString(
                                                  TEXT("vectorDimension"))),
                                              [&ReadNumber](const FString &) {
                                                return ReadNumber(
                                                    TEXT("vectorDimension"));
                                              }),
                                          func::when<FString, FString>(
                                              func::equals<FString>(FString(
                                                  TEXT("maxRecallResults"))),
                                              [&ReadNumber](const FString &) {
                                                return ReadNumber(
                                                    TEXT("maxRecallResults"));
                                              }),
                                      }),
                                  FString(TEXT("")));
               }();
}

} // namespace SDKConfig
