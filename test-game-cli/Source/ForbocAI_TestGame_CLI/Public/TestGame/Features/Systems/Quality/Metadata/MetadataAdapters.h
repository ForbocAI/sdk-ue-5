#pragma once

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerTypes.h"
#include "TestGame/Features/Systems/Quality/QualityAdapters.h"

namespace TestGame {
namespace QualityMetadataAdaptersDetail {

/** User Story: As a live evaluator, I need structured status metadata recovered from CLI output without bypassing the CLI boundary. @fn inline func::Maybe<TSharedRef<FJsonObject>> parseOutputRecord(const FString &Output) */
inline func::Maybe<TSharedRef<FJsonObject>>
parseOutputRecord(const FString &Output) {
  const int32 First = Output.Find(qualityData().Output.ObjectStart);
  const int32 Last = Output.Find(
      qualityData().Output.ObjectEnd, ESearchCase::CaseSensitive,
      ESearchDir::FromEnd);
  return First < qualityData().Numbers.EmptyCount || Last < First
             ? func::nothing<TSharedRef<FJsonObject>>()
             : [&]() {
                 TSharedPtr<FJsonObject> Record;
                 const FString Json = Output.Mid(
                     First,
                     Last - First + qualityData().Numbers.NextIndex);
                 const TSharedRef<TJsonReader<>> Reader =
                     TJsonReaderFactory<>::Create(Json);
                 return FJsonSerializer::Deserialize(Reader, Record) &&
                                Record.IsValid()
                            ? func::just(Record.ToSharedRef())
                            : func::nothing<TSharedRef<FJsonObject>>();
               }();
}

/** User Story: As a quality evaluator, I need model identity fields decoded through the authored status contract. @fn inline FString readStringField(const TSharedRef<FJsonObject> &Record, const FString &Field) */
inline FString readStringField(const TSharedRef<FJsonObject> &Record,
                               const FString &Field) {
  FString Value;
  return Record->TryGetStringField(Field, Value) ? Value
                                                 : qualityData().Output.Empty;
}

/** User Story: As a latency evaluator, I need the API inference budget decoded without string coercion ambiguity. @fn inline int32 readNumberField(const TSharedRef<FJsonObject> &Record, const FString &Field) */
inline int32 readNumberField(const TSharedRef<FJsonObject> &Record,
                             const FString &Field) {
  double Value{};
  return Record->TryGetNumberField(Field, Value)
             ? static_cast<int32>(Value)
             : qualityData().Numbers.FallbackInferenceLatencyBudgetMs;
}

} // namespace QualityMetadataAdaptersDetail

/** User Story: As a release reviewer, I need every report bound to the exact deployed API and SLM artifact tested through the CLI. @fn inline func::Maybe<FQualityModelMetadata> readQualityModelMetadata(const CommandRunner::FCommandOutput &Result) */
inline func::Maybe<FQualityModelMetadata>
readQualityModelMetadata(const CommandRunner::FCommandOutput &Result) {
  return func::match(
      QualityMetadataAdaptersDetail::parseOutputRecord(Result.Output),
      [](const TSharedRef<FJsonObject> &Record) {
        const FQualityMetadataFields &Fields = qualityData().MetadataFields;
        const FQualityModelMetadata Metadata{
            QualityMetadataAdaptersDetail::readStringField(Record,
                                                            Fields.ApiStatus),
            QualityMetadataAdaptersDetail::readStringField(Record,
                                                            Fields.ApiVersion),
            QualityMetadataAdaptersDetail::readNumberField(
                Record, Fields.InferenceLatencyBudgetMs),
            QualityMetadataAdaptersDetail::readStringField(Record,
                                                            Fields.SlmStatus),
            QualityMetadataAdaptersDetail::readStringField(Record,
                                                            Fields.SlmVersion),
            QualityMetadataAdaptersDetail::readStringField(
                Record, Fields.SlotContractVersion),
            QualityMetadataAdaptersDetail::readStringField(
                Record, Fields.SlmArtifactSha256)};
        const TArray<FString> Identity{
            Metadata.ApiStatus, Metadata.ApiVersion, Metadata.SlmStatus,
            Metadata.SlmVersion, Metadata.SlotContractVersion,
            Metadata.SlmArtifactSha256};
        return Identity.ContainsByPredicate(
                   [](const FString &Value) { return Value.IsEmpty(); })
                   ? func::nothing<FQualityModelMetadata>()
                   : func::just(Metadata);
      },
      []() { return func::nothing<FQualityModelMetadata>(); });
}

} // namespace TestGame
