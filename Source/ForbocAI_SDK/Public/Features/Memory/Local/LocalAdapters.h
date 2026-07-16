#pragma once

#include "Core/fp.hpp"
#include "Features/Memory/Configuration/ConfigurationAdapters.h"
#include "Features/Memory/Identity/IdentityAdapters.h"
#include "Features/Memory/Local/LocalTypes.h"
#include "Features/Memory/MemoryTypes.h"
#include "Features/Memory/Vector/MemoryVectorAdapters.h"
#include "Internationalization/Regex.h"
#include "Misc/Paths.h"
namespace MemoryLocalAdapters {

/** User Story: As native memory persistence, I need database names resolved beneath one authored vector directory so callers cannot escape SDK-owned storage. @fn inline func::Either<FString, MemoryLocalTypes::FMemoryDatabasePaths> resolveMemoryDatabasePathsAdapter(const FString &DatabaseName) */
inline func::Either<FString, MemoryLocalTypes::FMemoryDatabasePaths>
resolveMemoryDatabasePathsAdapter(const FString &DatabaseName) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  const FRegexPattern Pattern(Data.Storage.Paths.DatabaseNamePattern);
  FRegexMatcher Matcher(Pattern, DatabaseName);
  return !Matcher.FindNext()
             ? func::make_left<
                   FString, MemoryLocalTypes::FMemoryDatabasePaths>(
                   Data.Errors.InvalidDatabaseName)
             : [&]() {
                 const FString BaseDirectory =
                     FPaths::ConvertRelativePathToFull(FPaths::Combine(
                         FPaths::ProjectDir(),
                         Data.Storage.Paths.InfrastructureDirectory,
                         Data.Storage.Paths.VectorsDirectory));
                 const FString DatabasePath =
                     FPaths::ConvertRelativePathToFull(FPaths::Combine(
                         BaseDirectory,
                         DatabaseName +
                             Data.Storage.Sqlite.DatabaseExtension));
                 return !FPaths::IsUnderDirectory(DatabasePath, BaseDirectory)
                            ? func::make_left<
                                  FString,
                                  MemoryLocalTypes::FMemoryDatabasePaths>(
                                  Data.Errors.DatabasePathEscape)
                            : func::make_right<
                                  FString,
                                  MemoryLocalTypes::FMemoryDatabasePaths>(
                                  {DatabaseName, DatabasePath, true});
               }();
}

/** User Story: As native memory persistence, I need new records created in one pure adapter so thunks only coordinate storage effects. @fn inline FMemoryItem createMemoryItemAdapter(const FString &Text, const FString &Type, float Importance) */
inline FMemoryItem createMemoryItemAdapter(const FString &Text,
                                           const FString &Type,
                                           float Importance) {
  FMemoryItem Item;
  Item.Id = MemoryIdentityAdapters::createMemoryId();
  Item.Text = Text;
  Item.Type = Type;
  Item.Importance = Importance;
  Item.Timestamp = FDateTime::UtcNow().ToUnixTimestamp();
  Item.Embedding = MemoryVectorAdapters::embed(Text);
  return Item;
}

/** User Story: As native memory import, I need source records combined with deterministic embeddings without changing identity or chronology. @fn inline FMemoryItem createMemoryVectorRecordAdapter(const FMemoryItem &Item) */
inline FMemoryItem createMemoryVectorRecordAdapter(const FMemoryItem &Item) {
  FMemoryItem Record = Item;
  Record.Embedding = MemoryVectorAdapters::embed(Item.Text);
  return Record;
}

} // namespace MemoryLocalAdapters
