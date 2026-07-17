#pragma once

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "TestGame/Features/Data/DataAdapters.h"
#include "TestGame/Features/Systems/Quality/Report/ReportTypes.h"
#include "TestGame/Features/Systems/Quality/Report/Serialization/SerializationAdapters.h"

namespace TestGame {
namespace QualityReportAdaptersDetail {

/** User Story: As a portable test-game host, I need report paths composed from authored segments and the active Unreal project root. @fn inline FString reportPath(const TArray<FString> &Segments) */
inline FString reportPath(const TArray<FString> &Segments) {
  return func::fold_array<FString, FString>(
      Segments, FPaths::ProjectDir(),
      [](const FString &Path, const FString &Segment) {
        return FPaths::Combine(Path, Segment);
      });
}

/** User Story: As a quality report reader, I need existing structured reports decoded through one nullable filesystem boundary before promoting them to required references. @fn inline func::Maybe<TSharedPtr<FJsonObject>> readReportObject(const FString &Path) */
inline func::Maybe<TSharedPtr<FJsonObject>>
readReportObject(const FString &Path) {
  return !FPaths::FileExists(Path)
             ? func::nothing<TSharedPtr<FJsonObject>>()
             : [&]() {
                 FString Json;
                 check(FFileHelper::LoadFileToString(Json, *Path));
                 TSharedPtr<FJsonObject> Object;
                 const TSharedRef<TJsonReader<>> Reader =
                     TJsonReaderFactory<>::Create(Json);
                 check(FJsonSerializer::Deserialize(Reader, Object) &&
                       Object.IsValid());
                 return func::just(Object);
               }();
}

} // namespace QualityReportAdaptersDetail

/** User Story: As a quality host, I need one typed immutable view of report paths and operator messages. @fn inline const FQualityReportData &qualityReportData() */
inline const FQualityReportData &qualityReportData() {
  static const FQualityReportData Data = []() {
    const DataAdapters::FSettingsSource Source =
        DataAdapters::SettingsSource(TEXT("quality/report.json"));
    const TSharedRef<FJsonObject> Paths =
        DataAdapters::ReadObjectField(Source, TEXT("paths"));
    const TSharedRef<FJsonObject> Serialization =
        DataAdapters::ReadObjectField(Source, TEXT("serialization"));
    const TSharedRef<FJsonObject> Messages =
        DataAdapters::ReadObjectField(Source, TEXT("messages"));
    const TSharedRef<FJsonObject> Tokens =
        DataAdapters::ReadObjectField(Source, TEXT("tokens"));
    const TSharedRef<FJsonObject> AutomationNames =
        DataAdapters::ReadObjectField(Source, TEXT("automationNames"));
    const TSharedRef<FJsonObject> Stories =
        DataAdapters::ReadObjectField(Source, TEXT("stories"));
    return FQualityReportData{
        DataAdapters::ReadStringField(Source.Root, TEXT("host")),
        {DataAdapters::ReadStringArrayField(Paths, TEXT("current")),
         DataAdapters::ReadStringArrayField(Paths, TEXT("baseline"))},
        {DataAdapters::ReadNumberField(Serialization, TEXT("indentation")),
         DataAdapters::ReadStringField(Serialization, TEXT("lineEnding"))},
        {DataAdapters::ReadStringField(Messages, TEXT("currentMissing")),
         DataAdapters::ReadStringField(Messages,
                                       TEXT("absoluteGateFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("promoted")),
         DataAdapters::ReadStringField(Messages, TEXT("readFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("parseFailed")),
         DataAdapters::ReadStringField(Messages, TEXT("writeFailed"))},
        {DataAdapters::ReadStringField(Tokens, TEXT("path"))},
        {DataAdapters::ReadStringField(AutomationNames,
                                       TEXT("missingReport"))},
        {DataAdapters::ReadStringField(Stories, TEXT("missingReport"))}};
  }();
  return Data;
}

/** User Story: As a live quality run, I need the committed compatible-host baseline loaded before model probes execute. @fn inline func::Maybe<FQualityReport> readQualityBaseline() */
inline func::Maybe<FQualityReport> readQualityBaseline() {
  const FQualityReportData &Data = qualityReportData();
  return func::maybe_map(
      QualityReportAdaptersDetail::readReportObject(
          QualityReportAdaptersDetail::reportPath(Data.Paths.Baseline)),
      [](const TSharedPtr<FJsonObject> &Object) {
        return readQualityBaselineReport(Object.ToSharedRef());
      });
}

/** User Story: As a live game, I need every canonical quality run persisted even when its release gate fails. @fn inline FString writeCurrentQualityReport(const FQualityReport &Report) */
inline FString writeCurrentQualityReport(const FQualityReport &Report) {
  const FQualityReportData &Data = qualityReportData();
  const FString Path =
      QualityReportAdaptersDetail::reportPath(Data.Paths.Current);
  IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
  const FString Json =
      serializeQualityReport(Report) + Data.Serialization.LineEnding;
  check(FFileHelper::SaveStringToFile(Json, *Path));
  return Path;
}

/** User Story: As a terminal presenter, I need report locations rendered relative to the active project when possible. @fn inline FString displayQualityReportPath(const FString &Path) */
inline FString displayQualityReportPath(const FString &Path) {
  FString Relative = Path;
  FPaths::MakePathRelativeTo(Relative, *FPaths::ProjectDir());
  return Relative;
}

} // namespace TestGame
