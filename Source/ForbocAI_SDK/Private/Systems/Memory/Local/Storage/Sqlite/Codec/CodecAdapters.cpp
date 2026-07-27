#include "Systems/Memory/Local/Storage/Sqlite/Codec/CodecAdapters.h"

#include "Systems/Memory/Configuration/MemoryConfigurationAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
#include "sqlite3.h"
#endif

namespace Native::Sqlite::CodecAdapters {
namespace {

/** User Story: As SQLite vector storage, I need recursive encoding so the FP core owns collection traversal. @fn FString buildJsonVectorRecursive(const TArray<float> &Vector, int32 Index, FString JsonVector) */
FString buildJsonVectorRecursive(const TArray<float> &Vector, int32 Index,
                                 FString JsonVector) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return Index == Vector.Num()
             ? JsonVector + Data.Storage.Sqlite.JsonClose
             : buildJsonVectorRecursive(
                   Vector, Index + Data.Iteration.Step,
                   JsonVector + FString::SanitizeFloat(Vector[Index]) +
                       (Index + Data.Iteration.Step < Vector.Num()
                            ? Data.Storage.Sqlite.JsonSeparator
                            : Data.Text.Empty));
}

#if WITH_FORBOC_SQLITE_VEC
/** User Story: As SQLite row decoding, I need nullable native text converted through one boundary so record validation remains explicit. @fn FString readColumnText(sqlite3_stmt *Statement, int32 Column) */
FString readColumnText(sqlite3_stmt *Statement, int32 Column) {
  const unsigned char *Value = sqlite3_column_text(Statement, Column);
  return Value ? UTF8_TO_TCHAR(reinterpret_cast<const char *>(Value))
               : MemoryConfiguration::memoryData().Text.Empty;
}

/** User Story: As SQLite recall, I need each native row validated before it enters SDK state so malformed records fail the whole query. @fn func::Either<FString, FMemoryItem> readMemoryItem(sqlite3_stmt *Statement) */
func::Either<FString, FMemoryItem> readMemoryItem(sqlite3_stmt *Statement) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  FMemoryItem Item;
  Item.Id = readColumnText(Statement, Data.Storage.Sqlite.Columns.Id);
  Item.Text = readColumnText(Statement, Data.Storage.Sqlite.Columns.Text);
  Item.Type = readColumnText(Statement, Data.Storage.Sqlite.Columns.Type);
  Item.Importance = static_cast<float>(sqlite3_column_double(
      Statement, Data.Storage.Sqlite.Columns.Importance));
  Item.Timestamp = static_cast<int64>(sqlite3_column_int64(
      Statement, Data.Storage.Sqlite.Columns.Timestamp));
  Item.Similarity = static_cast<float>(
      Data.Storage.Sqlite.DistanceOrigin - sqlite3_column_double(
                                               Statement,
                                               Data.Storage.Sqlite.Columns.Distance));
  return Item.Id.IsEmpty() || Item.Type.IsEmpty()
             ? func::make_left<FString, FMemoryItem>(
                   Data.Errors.InvalidVectorRecord)
             : func::make_right<FString, FMemoryItem>(Item);
}

/**
 * User Story: As SQLite recall, I need row collection to preserve native failure state while traversing functionally.
 * @fn func::Either<FString, TArray<FMemoryItem>> collectSearchRowsRecursive( sqlite3_stmt *Statement, const TArray<FMemoryItem> &Results)
 */
func::Either<FString, TArray<FMemoryItem>> collectSearchRowsRecursive(
    sqlite3_stmt *Statement, const TArray<FMemoryItem> &Results) {
  const int StepResult = sqlite3_step(Statement);
  return StepResult == SQLITE_DONE
             ? func::make_right<FString, TArray<FMemoryItem>>(Results)
             : StepResult != SQLITE_ROW
                   ? func::make_left<FString, TArray<FMemoryItem>>(
                         MemoryConfiguration::memoryData()
                             .Errors.SqliteStepFailed)
                   : func::ematch(
                         readMemoryItem(Statement),
                         [](const FString &Error) {
                           return func::make_left<
                               FString, TArray<FMemoryItem>>(Error);
                         },
                         [Statement, &Results](const FMemoryItem &Item) {
                           return collectSearchRowsRecursive(
                               Statement,
                               func::append_value<FMemoryItem>(Results, Item));
                         });
}
#endif

} // namespace

/** User Story: As SQLite vector storage, I need vectors encoded through the authored wire format so schema changes stay outside source. @fn FString buildJsonVector(const TArray<float> &Vector) */
FString buildJsonVector(const TArray<float> &Vector) {
  const MemoryConfiguration::FMemoryData &Data =
      MemoryConfiguration::memoryData();
  return buildJsonVectorRecursive(Vector, Data.Iteration.InitialIndex,
                                  Data.Storage.Sqlite.JsonOpen);
}

/** User Story: As SQLite vector storage, I need persisted records normalized without inventing identity so imports remain lossless. @fn FMemoryItem prepareStoredItem(const FMemoryItem &Item) */
FMemoryItem prepareStoredItem(const FMemoryItem &Item) {
  FMemoryItem StoredItem = Item;
  StoredItem.Similarity =
      MemoryConfiguration::memoryData().Defaults.Similarity;
  return StoredItem;
}

#if WITH_FORBOC_SQLITE_VEC
/** User Story: As SQLite recall, I need row decoding to distinguish completion from native errors so failed queries cannot look empty. @fn func::Either<FString, TArray<FMemoryItem>> collectSearchRows(sqlite3_stmt *Statement) */
func::Either<FString, TArray<FMemoryItem>>
collectSearchRows(sqlite3_stmt *Statement) {
  return collectSearchRowsRecursive(Statement, TArray<FMemoryItem>());
}
#endif

} // namespace Native::Sqlite::CodecAdapters
