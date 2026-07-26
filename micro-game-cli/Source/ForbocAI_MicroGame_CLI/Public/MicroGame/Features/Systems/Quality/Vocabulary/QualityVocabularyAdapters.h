#pragma once

#include "Containers/SharedString.h"
#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Systems/Quality/QualityTypes.h"

namespace MicroGame::QualityVocabularyAdapters {

/** User Story: As a quality contract reader, I need JSON object keys preserved as authored metric and host identities. @fn inline TArray<FString> ObjectKeys(const TSharedRef<FJsonObject> &Object) */
inline TArray<FString> ObjectKeys(const TSharedRef<FJsonObject> &Object) {
  TArray<UE::FSharedString> Keys;
  Object->Values.GetKeys(Keys);
  return func::map_array<UE::FSharedString, FString>(
      Keys, [](const UE::FSharedString &Key) { return FString(*Key); });
}

/** User Story: As a quality harness, I need authored command records translated once into the canonical command data contract. @fn inline FQualityCommandData ReadCommand(const TSharedRef<FJsonObject> &Object) */
inline FQualityCommandData
ReadCommand(const TSharedRef<FJsonObject> &Object) {
  return {DataAdapters::ReadStringField(Object, TEXT("group")),
          DataAdapters::ReadOptionalStringField(Object, TEXT("command")),
          DataAdapters::ReadStringArrayField(Object,
                                             TEXT("expectedRoutes"))};
}

/** User Story: As a model evaluator, I need nested token alternatives decoded without flattening authored expectation groups. @fn inline TArray<TArray<FString>> ReadTokenGroups(const TSharedRef<FJsonObject> &Object) */
inline TArray<TArray<FString>>
ReadTokenGroups(const TSharedRef<FJsonObject> &Object) {
  return func::map_array<TSharedPtr<FJsonValue>, TArray<FString>>(
      DataAdapters::ReadArrayField(Object, TEXT("requiredTokenGroups")),
      [](const TSharedPtr<FJsonValue> &Value) {
        return func::map_array<TSharedPtr<FJsonValue>, FString>(
            Value->AsArray(), [](const TSharedPtr<FJsonValue> &Token) {
              return Token->AsString();
            });
      });
}

/** User Story: As a quality evaluator, I need every neutral probe decoded into one typed contract before execution. @fn inline FQualityProbe ReadProbe(const TSharedRef<FJsonObject> &Object) */
inline FQualityProbe ReadProbe(const TSharedRef<FJsonObject> &Object) {
  return {
      DataAdapters::ReadStringField(Object, TEXT("id")),
      DataAdapters::ReadStringField(Object, TEXT("category")),
      DataAdapters::ReadStringField(Object, TEXT("pairKey")),
      DataAdapters::ReadStringField(Object, TEXT("command")),
      DataAdapters::ReadStringField(Object, TEXT("reference")),
      DataAdapters::ReadStringField(Object, TEXT("exactResponse")),
      ReadTokenGroups(Object),
      DataAdapters::ReadStringArrayField(Object, TEXT("requiredPatterns")),
      DataAdapters::ReadStringArrayField(Object, TEXT("forbiddenPatterns")),
      DataAdapters::ReadFloatField(Object, TEXT("minimumReferenceF1")),
      DataAdapters::ReadNumberField(Object, TEXT("maximumWords")),
      DataAdapters::ReadBooleanField(Object,
                                     TEXT("coherenceUsesExpectations")),
      DataAdapters::ReadBooleanField(Object,
                                     TEXT("requiresUnknownBoundary"))};
}

/** User Story: As a cross-host latency evaluator, I need each host overhead budget keyed by its authored identity. @fn inline TMap<FString, FQualityHostData> ReadHosts(const TSharedRef<FJsonObject> &Object) */
inline TMap<FString, FQualityHostData>
ReadHosts(const TSharedRef<FJsonObject> &Object) {
  return func::fold_array<FString, TMap<FString, FQualityHostData>>(
      ObjectKeys(Object), {},
      [&Object](const TMap<FString, FQualityHostData> &Result,
                const FString &Key) {
        const TSharedRef<FJsonObject> Host =
            DataAdapters::ReadObjectField(Object, Key);
        return func::upsert_map_value<FString, FQualityHostData>(
            Result, Key, FQualityHostData(),
            [&Host](const FQualityHostData &) {
              return FQualityHostData{DataAdapters::ReadFloatField(
                  Host, TEXT("overheadBudgetMs"))};
            });
      });
}

/** User Story: As a quality harness, I need every authored command decoded in stable array order. @fn inline TArray<FQualityCommandData> ReadCommands(const TSharedRef<FJsonObject> &Object, const FString &Field) */
inline TArray<FQualityCommandData>
ReadCommands(const TSharedRef<FJsonObject> &Object,
             const FString &Field) {
  return func::map_array<TSharedPtr<FJsonValue>, FQualityCommandData>(
      DataAdapters::ReadObjectArrayField(Object, Field),
      [](const TSharedPtr<FJsonValue> &Value) {
        return ReadCommand(Value->AsObject().ToSharedRef());
      });
}

} // namespace MicroGame::QualityVocabularyAdapters
