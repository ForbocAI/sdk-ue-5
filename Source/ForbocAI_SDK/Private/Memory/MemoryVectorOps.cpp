// User Story: As a developer, I need this module to function.
#include "Memory/MemoryModuleInternal.h"
#include "Core/SdkVectorizer.h"

namespace MemoryInternal {

namespace Native {
namespace Sqlite {

TArray<FMemoryItem> VssSearch(Connection Db, const TArray<float> &Vector,
                              int32 Limit) {
  return ::Native::Sqlite::SearchRows(Db, Vector, Limit);
}

} // namespace Sqlite
} // namespace Native

namespace SQLiteVSS {

MemoryTypes::MemoryStoreRecallResult
VectorSearch(void *Handle, const FString &Query, int32 Limit) {
  try {
    return Handle == nullptr
               ? MemoryTypes::MemoryStoreRecallResult{
                     true, TEXT("Memory database is not open"),
                     TArray<FMemoryItem>()}
               : [&]() -> MemoryTypes::MemoryStoreRecallResult {
                   const MemoryTypes::MemoryStoreEmbeddingResult
                       EmbeddingResult = generateEmbedding(Handle, Query);
                   return EmbeddingResult.isLeft
                              ? MemoryTypes::MemoryStoreRecallResult{
                                    true, EmbeddingResult.left,
                                    TArray<FMemoryItem>()}
                              : MemoryTypes::MemoryStoreRecallResult{
                                    false, FString(),
                                    Native::Sqlite::VssSearch(
                                        Handle, EmbeddingResult.right, Limit)};
                 }();
  } catch (const std::exception &e) {
    return MemoryTypes::MemoryStoreRecallResult{
        true, FString(e.what()), TArray<FMemoryItem>()};
  }
}

MemoryTypes::MemoryStoreEmbeddingResult generateEmbedding(void *Handle,
                                                          const FString &Text) {
  return MemoryTypes::MemoryStoreEmbeddingResult{false, FString(),
                                                 ForbocAI::SDK::Vectorizer::Embed(Text)};
}

} // namespace SQLiteVSS

} // namespace MemoryInternal
