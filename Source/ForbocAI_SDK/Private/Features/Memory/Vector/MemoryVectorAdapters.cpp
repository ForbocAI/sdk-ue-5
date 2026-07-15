#include "Features/Memory/Vector/MemoryVectorAdapters.h"

namespace {

const int32 VECTOR_DIMENSION = 384;
const uint32 HASH_SEED = 2166136261u;
const uint32 HASH_PRIME = 16777619u;

uint32 mixHash(uint32 Hash, uint32 Value) {
  return (Hash ^ Value) * HASH_PRIME;
}

uint32 hashText(const FString &Text, int32 Index, uint32 Hash) {
  return Index >= Text.Len()
             ? Hash
             : hashText(Text, Index + 1,
                        mixHash(Hash, static_cast<uint32>(Text[Index])));
}

TArray<float> addFeature(const TArray<float> &Vector, uint32 Hash,
                         float Weight) {
  TArray<float> Next = Vector;
  const int32 Bucket = static_cast<int32>(Hash % VECTOR_DIMENSION);
  const float Sign = (Hash & 1u) == 0u ? 1.0f : -1.0f;
  Next[Bucket] += Sign * Weight;
  return Next;
}

TArray<float> embedText(const FString &Text, int32 Index,
                        const TArray<float> &Vector) {
  return Index >= Text.Len()
             ? Vector
             : embedText(
                   Text, Index + 1,
                   FChar::IsWhitespace(Text[Index])
                       ? Vector
                       : addFeature(
                             Vector,
                             hashText(FString::Printf(
                                          TEXT("%d:%d"), Index,
                                          static_cast<int32>(Text[Index])),
                                      0, HASH_SEED),
                             1.0f));
}

float sumSquares(const TArray<float> &Vector, int32 Index, float Acc) {
  return Index >= Vector.Num()
             ? Acc
             : sumSquares(Vector, Index + 1,
                          Acc + Vector[Index] * Vector[Index]);
}

TArray<float> normalize(const TArray<float> &Vector, int32 Index, float Norm,
                        const TArray<float> &Acc) {
  return Index >= Vector.Num()
             ? Acc
             : [&]() {
                 TArray<float> Next = Acc;
                 Next.Add(Norm > 0.0f ? Vector[Index] / Norm : 0.0f);
                 return normalize(Vector, Index + 1, Norm, Next);
               }();
}

TArray<float> emptyVector() {
  TArray<float> Vector;
  Vector.Init(0.0f, VECTOR_DIMENSION);
  return Vector;
}

} // namespace

namespace MemoryVectorAdapters {

TArray<float> embed(const FString &Text) {
  const TArray<float> Features = embedText(Text.ToLower(), 0, emptyVector());
  const float Norm = FMath::Sqrt(sumSquares(Features, 0, 0.0f));
  return normalize(Features, 0, Norm, TArray<float>());
}

} // namespace MemoryVectorAdapters
