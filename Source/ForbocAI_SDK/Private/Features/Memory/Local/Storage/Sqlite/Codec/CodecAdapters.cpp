#include "Features/Memory/Local/Storage/Sqlite/Codec/CodecAdapters.h"

#if WITH_FORBOC_SQLITE_VEC
extern "C" {
#include "sqlite3.h"
}
#endif

namespace Native::Sqlite::CodecAdapters {
namespace {

uint64 StableHashRecursive(const FString &Value, int32 Index, uint64 Hash) {
  return Index == Value.Len()
             ? Hash
             : StableHashRecursive(
                   Value, Index + 1,
                   (Hash ^ static_cast<uint64>(FChar::ToLower(Value[Index]))) *
                       1099511628211ull);
}

FString BuildJsonVectorRecursive(const TArray<float> &Vector, int32 Index,
                                 FString JsonVector) {
  return Index == Vector.Num()
             ? JsonVector + TEXT("]")
             : BuildJsonVectorRecursive(
                   Vector, Index + 1,
                   JsonVector + FString::SanitizeFloat(Vector[Index]) +
                       (Index + 1 < Vector.Num() ? TEXT(",") : TEXT("")));
}

#if WITH_FORBOC_SQLITE_VEC
FMemoryItem ReadMemoryItem(sqlite3_stmt *Statement) {
  FMemoryItem Item;
  const unsigned char *IdText = sqlite3_column_text(Statement, 0);
  const unsigned char *Text = sqlite3_column_text(Statement, 1);
  const unsigned char *TypeText = sqlite3_column_text(Statement, 2);
  Item.Id = IdText ? UTF8_TO_TCHAR(reinterpret_cast<const char *>(IdText))
                   : TEXT("");
  Item.Text = Text ? UTF8_TO_TCHAR(reinterpret_cast<const char *>(Text))
                   : TEXT("");
  Item.Type = TypeText
                  ? UTF8_TO_TCHAR(reinterpret_cast<const char *>(TypeText))
                  : TEXT("observation");
  Item.Importance = static_cast<float>(sqlite3_column_double(Statement, 3));
  Item.Timestamp = static_cast<int64>(sqlite3_column_int64(Statement, 4));
  Item.Similarity =
      static_cast<float>(1.0 - sqlite3_column_double(Statement, 5));
  return Item;
}

void CollectSearchRowsRecursive(sqlite3_stmt *Statement,
                                TArray<FMemoryItem> &Results) {
  const int StepResult = sqlite3_step(Statement);
  StepResult == SQLITE_ROW
      ? (Results.Add(ReadMemoryItem(Statement)),
         CollectSearchRowsRecursive(Statement, Results))
      : void();
}
#endif

} // namespace

FString BuildJsonVector(const TArray<float> &Vector) {
  return BuildJsonVectorRecursive(Vector, 0, TEXT("["));
}

FMemoryItem PrepareStoredItem(const FMemoryItem &Item) {
  FMemoryItem StoredItem = Item;
  const FString Seed = FString::Printf(TEXT("%s|%s|%lld"), *Item.Type,
                                       *Item.Text,
                                       static_cast<long long>(Item.Timestamp));
  StoredItem.Id = StoredItem.Id.IsEmpty()
                      ? FString::Printf(
                            TEXT("mem_%016llx"),
                            static_cast<unsigned long long>(
                                StableHashRecursive(
                                    Seed, 0, 1469598103934665603ull)))
                      : StoredItem.Id;
  StoredItem.Similarity = 0.0f;
  return StoredItem;
}

#if WITH_FORBOC_SQLITE_VEC
void CollectSearchRows(sqlite3_stmt *Statement,
                       TArray<FMemoryItem> &Results) {
  CollectSearchRowsRecursive(Statement, Results);
}
#endif

} // namespace Native::Sqlite::CodecAdapters
