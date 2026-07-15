#pragma once

#include "Core/fp.hpp"
#include "Features/Config/File/ConfigFileAdapters.h"

namespace SDKConfig {

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
